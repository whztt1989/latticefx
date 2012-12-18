/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * Copyright 2012-2012 by Ames Laboratory
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *************** <auto-copyright.rb END do not edit this line> ***************/

#include <latticefx/core/DataSet.h>
#include <latticefx/core/VolumeRenderer.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/HierarchyUtils.h>
#include <latticefx/core/PagingThread.h>
#include <latticefx/core/TransferFunctionUtils.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <latticefx/core/DBDisk.h>
#ifdef LFX_USE_CRUNCHSTORE
#  include <latticefx/core/DBCrunchStore.h>
#  include <crunchstore/DataManager.h>
#  include <crunchstore/NullCache.h>
#  include <crunchstore/NullBuffer.h>
#  include <crunchstore/SQLiteStore.h>
#endif

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osg/ComputeBoundsVisitor>
#include <osg/Group>
#include <osg/ArgumentParser>
#include <osgGA/TrackballManipulator>

#include <osgwTools/Shapes.h>

#include <Poco/Path.h>
#include <Poco/Glob.h>
#include <boost/foreach.hpp>

#include <string>


static std::string logstr( "lfx.demo" );

using namespace lfx::core;


class ImageHierarchyLoader : public Preprocess
{
public:
    ImageHierarchyLoader()
    {
        setActionType( Preprocess::ADD_DATA );
    }
    ~ImageHierarchyLoader() {}

    virtual ChannelDataPtr operator()()
    {
        DBBase::StringSet results( _db->getAllKeys() );
        if( results.empty() )
        {
            LFX_FATAL_STATIC( logstr, "No keys in DB." );
        }

        // Determine the hierarchy maxDepth from the longest hierarchy name.
        unsigned int maxDepth( 1 );
        BOOST_FOREACH( const std::string& fName, results )
        {
            if( !valid( fName ) )
                continue;

            Poco::Path pocoPath( fName );
            const std::string& actualName( pocoPath.getFileName() );
            size_t depth( actualName.find_last_of( "-" ) - actualName.find_first_of( "-" ) );
            if( depth > maxDepth )
                maxDepth = depth;
        }

        // Create the ChannelData hierarchy.
        ChannelDataPtr cdp( (ChannelData*)NULL );
        try {
            AssembleHierarchy ah( maxDepth, 60000. );
            BOOST_FOREACH( const std::string& fName, results )
            {
                if( !valid( fName ) )
                    continue;

                // Create this ChannelData.
                ChannelDataOSGImagePtr cdImage( new ChannelDataOSGImage() );
                cdImage->setDBKey( DBKey( fName ) );

                // Get the hierarchy name string.
                Poco::Path pocoPath( fName );
                const std::string& actualName( pocoPath.getFileName() );
                const size_t firstLoc( actualName.find_first_of( "-" ) + 1 );
                const size_t lastLoc( actualName.find_last_of( "-" ) );
                const std::string hierarchyName( actualName.substr( firstLoc, lastLoc-firstLoc ) );

                LFX_DEBUG_STATIC( logstr, "Adding " + fName + ": " + hierarchyName );
                cdImage->setName( "volumedata" );
                ah.addChannelData( cdImage, hierarchyName );
            }
            cdp = ah.getRoot();
        } catch( std::exception& /*exc*/ ) {
            LFX_ERROR_STATIC( logstr, "Unable to assemble hierarchy." );
            return( cdp );
        }

        // Return the hierarchy root.
        cdp->setName( "volumedata" );
        return( cdp );
    }

protected:
    bool valid( const std::string& fileName )
    {
        const std::string nameOnly( osgDB::getSimpleFileName( fileName ) );
        const size_t firstDash( nameOnly.find_first_of( "-" ) );
        const size_t lastDash( nameOnly.find_last_of( "-" ) );
        if( ( firstDash == lastDash ) || ( firstDash == std::string::npos ) ||
            ( lastDash == std::string::npos ) )
            return( false );
        else
            return( true );
    }
};


DataSetPtr prepareVolume( const osg::Vec3& dims,
        const std::string& csFile, const std::string& diskPath )
{
    DataSetPtr dsp( new DataSet() );

    DBBasePtr dbBase;
#ifdef LFX_USE_CRUNCHSTORE
    if( !( csFile.empty() ) )
    {
        DBCrunchStorePtr cs( DBCrunchStorePtr( new DBCrunchStore() ) );

        crunchstore::DataManagerPtr manager( crunchstore::DataManagerPtr( new crunchstore::DataManager() ) );
        crunchstore::DataAbstractionLayerPtr cache( new crunchstore::NullCache );
        crunchstore::DataAbstractionLayerPtr buffer( new crunchstore::NullBuffer );
        manager->SetCache( cache );
        manager->SetBuffer( buffer );
        crunchstore::SQLiteStorePtr sqstore( new crunchstore::SQLiteStore );
        sqstore->SetStorePath( csFile );
        manager->AttachStore( sqstore, crunchstore::Store::BACKINGSTORE_ROLE );
        try {
            cs->setDataManager( manager );
        }
        catch( std::exception exc ) {
            LFX_FATAL_STATIC( logstr, std::string(exc.what()) );
            LFX_FATAL_STATIC( logstr, "Unable to set DataManager." );
            exit( 1 );
        }

        dbBase = (DBBasePtr)cs;
    }
#endif
    if( csFile.empty() )
    {
        DBDiskPtr disk( DBDiskPtr( new DBDisk() ) );
        disk->setRootPath( diskPath );
        dbBase = (DBBasePtr)disk;
    }
    dsp->setDB( dbBase );


    ImageHierarchyLoader* ihl = new ImageHierarchyLoader();
    ihl->setDB( dbBase );
    dsp->addPreprocess( PreprocessPtr( (Preprocess*)ihl ) );

    VolumeRendererPtr renderOp( new VolumeRenderer() );
    renderOp->setVolumeDims( dims );
    renderOp->setRenderMode( VolumeRenderer::RAY_TRACED );
    renderOp->setMaxSamples( 100.f );

    renderOp->addInput( "volumedata" );
    dsp->setRenderer( renderOp );

    renderOp->setTransferFunction( lfx::core::loadImageFromDat( "01.dat", LFX_ALPHA_RAMP_0_TO_1 ) );
    renderOp->setTransferFunctionDestination( Renderer::TF_RGBA );

    // Render when alpha values are greater than 0.15.
    renderOp->setHardwareMaskInputSource( Renderer::HM_SOURCE_ALPHA );
    renderOp->setHardwareMaskOperator( Renderer::HM_OP_GT );
    renderOp->setHardwareMaskReference( .15f );

    return( dsp );
}


osg::Node* createScene( const std::string fileName=std::string("") )
{
    osg::Group* root( new osg::Group );
    if( fileName.empty() )
    {
        osg::Geometry* geom( osgwTools::makeClosedCylinder(
            osg::Matrix::translate( 0., 0., -30. ), 60., 8., 8., true, true, osg::Vec2s(1,16) ) );
        osg::Vec4Array* c( new osg::Vec4Array() );
        c->push_back( osg::Vec4( 1., .5, 0., 1. ) );
        geom->setColorArray( c );
        geom->setColorBinding( osg::Geometry::BIND_OVERALL );

        osg::Geode* geode( new osg::Geode() );
        geode->addDrawable( geom );
        root->addChild( geode );
    }
    else
        root->addChild( osgDB::readNodeFile( fileName ) );

    return( root );
}


struct RTTInfo {
    RTTInfo( const float w, const float h )
      : winSize( osg::Vec2f( w, h ) )
    {
        windowSize = new osg::Uniform( "windowSize", winSize );
    }

    osg::Vec2f winSize;
    osg::ref_ptr< osg::Uniform > windowSize;

    osg::ref_ptr< osg::Texture2D > depthTex;
};

RTTInfo setupStandardRTTRendering( osgViewer::Viewer& viewer, osg::Node* scene )
{
    RTTInfo rttInfo( 1200, 690 );


    //
    // Step 1: Configure root camera to render to texture.
    //

    osg::Camera* rootCam = viewer.getCamera();

    // Viewer's Camera will render into there color and depth texture buffers:
    osg::Texture2D* colorTex = new osg::Texture2D;
    colorTex->setTextureWidth( rttInfo.winSize.x() );
    colorTex->setTextureHeight( rttInfo.winSize.y() );
    colorTex->setInternalFormat( GL_RGBA );
    colorTex->setBorderWidth( 0 );
    colorTex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    colorTex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );

    rttInfo.depthTex = new osg::Texture2D;
    rttInfo.depthTex->setTextureWidth( rttInfo.winSize.x() );
    rttInfo.depthTex->setTextureHeight( rttInfo.winSize.y() );
    rttInfo.depthTex->setInternalFormat( GL_DEPTH_COMPONENT );
    rttInfo.depthTex->setBorderWidth( 0 );
    rttInfo.depthTex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    rttInfo.depthTex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );

    rootCam->attach( osg::Camera::COLOR_BUFFER0, colorTex, 0, 0 );
    rootCam->attach( osg::Camera::DEPTH_BUFFER, rttInfo.depthTex.get(), 0, 0 );
    rootCam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER );


    //
    // Step 2: Create splat cam to display color texture to window
    //
    osg::Camera* splatCam = new osg::Camera;
    
    splatCam->setClearMask( 0 );
    splatCam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER, osg::Camera::FRAME_BUFFER );

    splatCam->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    splatCam->setRenderOrder( osg::Camera::POST_RENDER );

    osg::Geode* geode( new osg::Geode );
    geode->addDrawable( osgwTools::makePlane(
        osg::Vec3( -1,-1,0 ), osg::Vec3( 2,0,0 ), osg::Vec3( 0,2,0 ) ) );
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(
        0, colorTex, osg::StateAttribute::ON );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    geode->getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );

    splatCam->addChild( geode );


    //
    // Step 3: Arrange and attach the scene graph.
    //
    osg::Group* rootGroup( new osg::Group );
    rootGroup->addChild( scene );
    rootGroup->addChild( splatCam );
    viewer.setSceneData( rootGroup );


    return( rttInfo );
}

osg::Node* createStubGeometry( osg::Node* subgraph )
{
    // This function creates and returns a geode containing a stub
    // Drawable that renders nothing. But the stub Drawable has the
    // same bounding box as the subgraph parameter. The returned
    // value can be used to ensure that scene rendering computes
    // near & far planes that account for the presence of the
    // subgraph parameter, without actually rendering that subgraph.

    osg::ComputeBoundsVisitor cbv;
    cbv.setNodeMaskOverride( ~0u );
    subgraph->accept( cbv );

    osg::Geometry* stubGeom( new osg::Geometry );
    stubGeom->setInitialBound( cbv.getBoundingBox() );
    // The stock DrawCallback is a no-op and draws nothing.
    stubGeom->setDrawCallback( new osg::Drawable::DrawCallback() );

    osg::Geode* root( new osg::Geode );
    root->addDrawable( stubGeom );

    return( root );
}
void setupLfxVolumeRTRendering( const RTTInfo& rttInfo,
    osgViewer::Viewer& viewer, osg::Node* scene, osg::Node* volume )
{
    // Get the root Group node attached to the osgViewer::Viewer.
    osg::Node* sceneRootNode( viewer.getSceneData() );
    osg::Group* sceneRoot( dynamic_cast< osg::Group* >( sceneRootNode ) );
    if( sceneRoot == NULL )
    {
        LFX_ERROR_STATIC( logstr, "Viewer scene data must be a Group." );
        exit( 1 );
    }

    // Get the volume subgraph as a Group
    osg::Group* volumeGroup( dynamic_cast< osg::Group* >( volume ) );
    if( volumeGroup == NULL )
    {
        LFX_ERROR_STATIC( logstr, "Volume subgraph must be a Group." );
        exit( 1 );
    }


    //
    // Step 1: Add sub geomtry. Stub the volume subgraph into the
    // main scene, and stub the main scene subgraph into the volume.
    //
    sceneRoot->addChild( createStubGeometry( volume ) );
    volumeGroup->addChild( createStubGeometry( scene ) );


    //
    // Step 2: Create a Camera for rendering the LatticeFX volume.
    //
    osg::Camera* lfxCam( new osg::Camera );
    lfxCam->setName( "latticeFX-VolumeCamera" );

    lfxCam->setClearMask( 0 );
    lfxCam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER, osg::Camera::FRAME_BUFFER );

    lfxCam->setReferenceFrame( osg::Camera::RELATIVE_RF );
    lfxCam->setRenderOrder( osg::Camera::POST_RENDER );

    lfxCam->addChild( volume );
    sceneRoot->addChild( lfxCam );


    //
    // Step 3:
    // Add uniforms required by Lfx ray traced volume rendering shaders,
    // which Lfx itself is unable to set. The application MUST specify
    // this uniforms.
    //
    osg::StateSet* stateSet( lfxCam->getOrCreateStateSet() );

    stateSet->addUniform( rttInfo.windowSize.get() );

    stateSet->setTextureAttributeAndModes( 1, rttInfo.depthTex.get() );
    osg::Uniform* uniform = new osg::Uniform( osg::Uniform::SAMPLER_2D, "sceneDepth" ); uniform->set( 1 );
    stateSet->addUniform( uniform );
}


int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );
    //Log::instance()->setPriority( Log::PrioTrace, "lfx.core.page" );

    osg::ArgumentParser arguments( &argc, argv );

    LFX_CRITICAL_STATIC( logstr, "With no command line args, write image data as files using DBDisk." );
    LFX_CRITICAL_STATIC( logstr, "-dp <path> Specifies directory to use for DBDisk. Default: cwd." );
    LFX_CRITICAL_STATIC( logstr, "-cs <dbFile> Write volume image data files using DBCrunchStore." );

    std::string csFile;
#ifdef LFX_USE_CRUNCHSTORE
    arguments.read( "-cs", csFile );
#endif
    std::string diskPath;
    arguments.read( "-dp", diskPath );
    if( !diskPath.empty() && !csFile.empty() )
    {
        LFX_WARNING_STATIC( logstr, "Can't use both CrunchStore and DBDisk. Using CrunchStore..." );
        diskPath.clear();
    }

    osg::Vec3 dims( 50., 50., 50. );
    arguments.read( "-d", dims[0],dims[1],dims[2] );



    osgViewer::Viewer viewer;
    viewer.getCamera()->setClearColor( osg::Vec4( 0., 0., 0., 1. ) );
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    osgGA::TrackballManipulator* tbm( new osgGA::TrackballManipulator() );
    viewer.setCameraManipulator( tbm );



    osg::Node* scene( createScene() );

    RTTInfo rttInfo( setupStandardRTTRendering( viewer, scene ) );
    viewer.setUpViewInWindow( 20, 30, rttInfo.winSize.x(), rttInfo.winSize.y() );

    DataSetPtr dsp( prepareVolume( dims, csFile, diskPath ) );
    setupLfxVolumeRTRendering( rttInfo, viewer, scene, dsp->getSceneData() );


    // Really we would need to change the projection matrix and viewport
    // in an event handler that catches window size changes. We're cheating.
    PagingThread* pageThread( PagingThread::instance() );
    const osg::Camera* cam( viewer.getCamera() );
    pageThread->setTransforms( cam->getProjectionMatrix(), cam->getViewport() );

    osg::Vec3d eye, center, up;
    while( !viewer.done() )
    {
        tbm->getInverseMatrix().getLookAt( eye, center, up );
        pageThread->setTransforms( osg::Vec3( eye ) );
        viewer.frame();
    }
    return( 0 );
}