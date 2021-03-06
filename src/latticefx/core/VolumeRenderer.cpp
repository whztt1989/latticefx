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

#include <latticefx/core/VolumeRenderer.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/TextureUtils.h>
#include <latticefx/core/LogMacros.h>
#include <latticefx/core/JsonSerializer.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/BlendFunc>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osg/Depth>
#include <osg/CullFace>
#include <osgDB/FileUtils>

#include <boost/foreach.hpp>

#include <sstream>


namespace lfx
{
namespace core
{

////////////////////////////////////////////////////////////////////////////////
SpatialVolume::SpatialVolume()
    : _volumeDims( osg::Vec3f( 1., 1., 1. ) ),
      _volumeOrigin( osg::Vec3f( 0., 0., 0. ) )
{
}

////////////////////////////////////////////////////////////////////////////////
SpatialVolume::SpatialVolume( const SpatialVolume& rhs )
    : _volumeDims( rhs._volumeDims ),
      _volumeOrigin( rhs._volumeOrigin )
{
}

////////////////////////////////////////////////////////////////////////////////
SpatialVolume::~SpatialVolume()
{
}

////////////////////////////////////////////////////////////////////////////////
void SpatialVolume::setVolumeDims( const osg::Vec3& volDims )
{
    _volumeDims = volDims;
}

////////////////////////////////////////////////////////////////////////////////
osg::Vec3 SpatialVolume::getVolumeDims() const
{
    return( _volumeDims );
}

////////////////////////////////////////////////////////////////////////////////
void SpatialVolume::setVolumeOrigin( const osg::Vec3& volOrigin )
{
    _volumeOrigin = volOrigin;
}

////////////////////////////////////////////////////////////////////////////////
osg::Vec3 SpatialVolume::getVolumeOrigin() const
{
    return( _volumeOrigin );
}

////////////////////////////////////////////////////////////////////////////////
void SpatialVolume::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	//Renderer::serializeData( json );

	json->insertObj( SpatialVolume::getClassName(), true );
	Renderer::serialize( json, "volumeDims", _volumeDims );
	Renderer::serialize( json, "volumeOrigin", _volumeOrigin );
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool SpatialVolume::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// get to this classes data
	if ( !json->getObj( SpatialVolume::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get SpatialVolume data";
		return false;
	}

	Renderer::load( json, "volumeDims", _volumeDims );
	Renderer::load( json, "volumeOrigin", _volumeOrigin );

	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void SpatialVolume::dumpState( std::ostream &os )
{
	ObjBase::dumpStateStart( SpatialVolume::getClassName(), os );

	os << "_volumeDims: (" << _volumeDims.x() << ", " << _volumeDims.y() << ", " << _volumeDims.z() << ")" << std::endl;
	os << "_volumeOrigin: (" << _volumeOrigin.x() << ", " << _volumeOrigin.y() << ", " << _volumeOrigin.z() << ")" << std::endl;

	ObjBase::dumpStateEnd( SpatialVolume::getClassName(), os );
}


////////////////////////////////////////////////////////////////////////////////
VolumeRenderer::VolumeRenderer( const std::string& logName )
    : Renderer( "vol", logName ),
      _renderMode( SLICES ),
      _numPlanes( 100.f ),
      _maxSamples( 100.f ),
      _transparencyScalar( 1.f ),
      _transparencyEnable( true )
{
    // Specify default ChannelData name aliases for the required inputs.
    setInputNameAlias( VOLUME_DATA, "volumedata" );

    // Create and register uniform information, and initial/default values
    // (if we have them -- in some cases, we don't know the actual initial
    // values until scene graph creation).
    UniformInfo info;
    info = UniformInfo( "VolumeTexture", osg::Uniform::SAMPLER_3D, "Volume texture data sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "volumeDims", osg::Uniform::FLOAT_VEC3, "World coordinate volume texture dimensions.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "volumeResolution", osg::Uniform::FLOAT_VEC3, "Volume texture resolution.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "volumeCenter", osg::Uniform::FLOAT_VEC3, "Volume center location.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "volumeNumPlanes", osg::Uniform::FLOAT, "Number of planes to render the volume." );
    registerUniform( info );

    info = UniformInfo( "volumeMaxSamples", osg::Uniform::FLOAT, "Max ray Samples for ray traced rendering." );
    registerUniform( info );

    info = UniformInfo( "volumeTransparency", osg::Uniform::FLOAT, "Alpha coefficient, default: 1.0." );
    registerUniform( info );

    info = UniformInfo( "volumeTransparencyEnable", osg::Uniform::BOOL, "Blending enable, default: true." );
    registerUniform( info );

    info = UniformInfo( "volumeClipPlaneEnable0", osg::Uniform::INT, "Clip plane 0: 1=enabled, 0=disabled." );
    registerUniform( info );
    info = UniformInfo( "volumeClipPlaneEnable1", osg::Uniform::INT, "Clip plane 1: 1=enabled, 0=disabled." );
    registerUniform( info );
    info = UniformInfo( "volumeClipPlaneEnable2", osg::Uniform::INT, "Clip plane 2: 1=enabled, 0=disabled." );
    registerUniform( info );
    info = UniformInfo( "volumeClipPlaneEnable3", osg::Uniform::INT, "Clip plane 3: 1=enabled, 0=disabled." );
    registerUniform( info );
    info = UniformInfo( "volumeClipPlaneEnable4", osg::Uniform::INT, "Clip plane 4: 1=enabled, 0=disabled." );
    registerUniform( info );
    info = UniformInfo( "volumeClipPlaneEnable5", osg::Uniform::INT, "Clip plane 5: 1=enabled, 0=disabled." );
    registerUniform( info );
}

////////////////////////////////////////////////////////////////////////////////
VolumeRenderer::VolumeRenderer( const VolumeRenderer& rhs )
    : Renderer( rhs ),
      _renderMode( rhs._renderMode ),
      _numPlanes( rhs._numPlanes ),
      _maxSamples( rhs._maxSamples ),
      _transparencyScalar( rhs._transparencyScalar ),
      _transparencyEnable( rhs._transparencyEnable )
{
}

////////////////////////////////////////////////////////////////////////////////
VolumeRenderer::~VolumeRenderer()
{
}

////////////////////////////////////////////////////////////////////////////////
// Used for SLICES mode.
////////////////////////////////////////////////////////////////////////////////
osg::Geometry* VolumeRenderer::createDAIGeometry( int nInstances )
{
    osg::ref_ptr< osg::Geometry > geom( new osg::Geometry );
    addColorArray( geom.get() );
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );

    const float halfDimX( .5 );
    const float halfDimZ( .5 );

    osg::Vec3Array* v = new osg::Vec3Array;
    v->resize( 4 );
    geom->setVertexArray( v );

    // Geometry for a single quad.
    ( *v )[ 0 ] = osg::Vec3( -halfDimX, -halfDimZ, 0. );
    ( *v )[ 1 ] = osg::Vec3( halfDimX, -halfDimZ, 0. );
    ( *v )[ 2 ] = osg::Vec3( -halfDimX, halfDimZ, 0. );
    ( *v )[ 3 ] = osg::Vec3( halfDimX, halfDimZ, 0. );

    // Use the DrawArraysInstanced PrimitiveSet and tell it to draw nInstances instances.
    geom->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLE_STRIP, 0, 4, nInstances ) );

    return( geom.release() );
}

////////////////////////////////////////////////////////////////////////////////
// Used for RAY_TRACED mode.
////////////////////////////////////////////////////////////////////////////////
osg::Geometry* VolumeRenderer::createCubeGeometry()
{
    osg::ref_ptr< osg::Geometry > geom( new osg::Geometry );
	geom->setName( "lfx::core::VolumeRenderer Cube" );
    addColorArray( geom.get() );
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );

    const osg::Vec3 hd( _volumeDims * .5 );
    const osg::Vec3& c( _volumeOrigin );

    geom->setColorBinding( osg::Geometry::BIND_OFF );

    osg::Vec3Array* v( new osg::Vec3Array );
    v->resize( 8 );
    geom->setVertexArray( v );

    ( *v )[0].set( -hd.x() + c.x(), -hd.y() + c.y(), -hd.z() + c.z() );
    ( *v )[1].set( hd.x() + c.x(), -hd.y() + c.y(), -hd.z() + c.z() );
    ( *v )[2].set( -hd.x() + c.x(), hd.y() + c.y(), -hd.z() + c.z() );
    ( *v )[3].set( hd.x() + c.x(), hd.y() + c.y(), -hd.z() + c.z() );
    ( *v )[4].set( -hd.x() + c.x(), -hd.y() + c.y(), hd.z() + c.z() );
    ( *v )[5].set( hd.x() + c.x(), -hd.y() + c.y(), hd.z() + c.z() );
    ( *v )[6].set( -hd.x() + c.x(), hd.y() + c.y(), hd.z() + c.z() );
    ( *v )[7].set( hd.x() + c.x(), hd.y() + c.y(), hd.z() + c.z() );

    osg::Vec3Array* tc( new osg::Vec3Array );
    tc->resize( 8 );
    geom->setTexCoordArray( 0, tc );

    ( *tc )[0].set( 0.f, 0.f, 0.f );
    ( *tc )[1].set( 1.f, 0.f, 0.f );
    ( *tc )[2].set( 0.f, 1.f, 0.f );
    ( *tc )[3].set( 1.f, 1.f, 0.f );
    ( *tc )[4].set( 0.f, 0.f, 1.f );
    ( *tc )[5].set( 1.f, 0.f, 1.f );
    ( *tc )[6].set( 0.f, 1.f, 1.f );
    ( *tc )[7].set( 1.f, 1.f, 1.f );

    GLushort indices[] =
    {
        2, 0, 6, 6, 0, 4, // -x face
        1, 3, 5, 5, 3, 7,   // +x face
        0, 1, 4, 4, 1, 5, // -y face
        3, 2, 7, 7, 2, 6,   // +y face
        1, 0, 3, 3, 0, 2, // -z face
        4, 5, 6, 6, 5, 7    // +z face
    };

    osg::DrawElementsUShort* deus( new osg::DrawElementsUShort( GL_TRIANGLES, sizeof( indices ) / sizeof( GLushort ), indices ) );
    geom->addPrimitiveSet( deus );

    return( geom.release() );
}

////////////////////////////////////////////////////////////////////////////////
osg::Node* VolumeRenderer::getSceneGraph( const ChannelDataPtr maskIn )
{
    osg::ref_ptr< osg::Geode > geode( new osg::Geode );

    osg::Geometry* geom;
    if( _renderMode == SLICES )
    {
        geom = createDAIGeometry( _numPlanes );
    }
    else
    {
        geom = createCubeGeometry();
    }
    geode->addDrawable( geom );

    // OSG has no clue where our vertex shader will place the geometric data,
    // so specify an initial bound to allow proper culling and near/far computation.
    osg::BoundingBox bb( ( _volumeDims * -.5 ) + _volumeOrigin, ( _volumeDims * .5 ) + _volumeOrigin );
    geom->setInitialBound( bb );


    osg::StateSet* stateSet( geode->getOrCreateStateSet() );

    ChannelDataPtr dataPtr( getInput( getInputNameAlias( VOLUME_DATA ) ) );
    if( dataPtr == NULL )
    {
        LFX_WARNING( "getSceneGraph(): Unable to find required VOLUME_DATA ChannelData." );
        return( NULL );
    }
    ChannelDataOSGImage* dataImagePtr( static_cast <
                                       ChannelDataOSGImage* >( dataPtr.get() ) );


    const std::string imageFileName( dataImagePtr->getDBKey() );
    osg::Texture3D* volumeTexture( NULL );
    if( _db != NULL )
    {
        // Create empty stub texture, to be paged in at run-time.
        volumeTexture = createTexture( imageFileName, NULL );
        volumeTexture->setDataVariance( osg::Object::DYNAMIC ); // for paging.
    }
    else
    {
        // Create texture with actual Image data, not paged.
        volumeTexture = createTexture( imageFileName, dataImagePtr->asOSGImage() );
    }

    stateSet->setTextureAttributeAndModes(
        getOrAssignTextureUnit( "volumeTex" ), volumeTexture );

    {
        UniformInfo& info( getUniform( "volumeResolution" ) );
        unsigned int x, y, z;
        dataImagePtr->getDimensions( x, y, z );
        osg::Vec3 res( x, y, z );
        if( res.length2() == 0. )
            // Must be a paged texture and we don't have the data yet.?
            // Add a bogus resolution, just so we have something.
        {
            res.set( 100., 100., 100. );
        }
        info._prototype->set( res );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }


    return( geode.release() );
}

////////////////////////////////////////////////////////////////////////////////
osg::StateSet* VolumeRenderer::getRootState()
{
    osg::ref_ptr< osg::StateSet > stateSet( new osg::StateSet() );

    // position, direction, transfer function input, and hardware mask input texture
    // units are the same for all time steps, so set their sampler uniform unit
    // values in the root state.
    {
        UniformInfo& info( getUniform( "VolumeTexture" ) );
        info._prototype->set( getOrAssignTextureUnit( "volumeTex" ) );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }

    if( ( getTransferFunction() != NULL ) &&
            !( getTransferFunctionInput().empty() ) )
    {
        LFX_WARNING( "getRootState(): Transfer function input is not supported and will be ignored." );
    }

    // Set default values for volume shader uniforms.
    {
        UniformInfo& info( getUniform( "volumeDims" ) );
        info._prototype->set( osg::Vec3f( _volumeDims ) );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }
    {
        UniformInfo& info( getUniform( "volumeCenter" ) );
        info._prototype->set( osg::Vec3f( _volumeOrigin ) );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }
    if( _renderMode == SLICES )
    {
        {
            UniformInfo& info( getUniform( "volumeNumPlanes" ) );
            info._prototype->set( _numPlanes );
            stateSet->addUniform( createUniform( info ) );
        }
    }
    else
    {
        {
            UniformInfo& info( getUniform( "volumeMaxSamples" ) );
            info._prototype->set( _maxSamples );
            stateSet->addUniform( createUniform( info ) );
        }
    }
    {
        UniformInfo& info( getUniform( "volumeTransparency" ) );
        info._prototype->set( _transparencyScalar );
        stateSet->addUniform( createUniform( info ) );
    }
    {
        UniformInfo& info( getUniform( "volumeTransparencyEnable" ) );
        info._prototype->set( _transparencyEnable );
        stateSet->addUniform( createUniform( info ) );
    }

    for( unsigned int idx = 0; idx < 6; idx++ )
    {
        std::ostringstream ostr;
        ostr << "volumeClipPlaneEnable" << idx;
        UniformInfo& info( getUniform( ostr.str() ) );
        info._prototype->set( 0 );
        stateSet->addUniform( createUniform( info ) );
    }

    osg::BlendFunc* fn = new osg::BlendFunc();
    fn->setFunction( osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
    stateSet->setAttributeAndModes( fn, osg::StateAttribute::ON );

    if( _renderMode == SLICES )
    {
        // Do not need to write the depth buffer.
        osg::Depth* depth( new osg::Depth( osg::Depth::LESS, 0., 1., false ) );
        stateSet->setAttributeAndModes( depth );
    }
    else // RAY_TRACED
    {
        // For ray traced, disable depth test (the shader will take care
        // of that). and render back faces only (back half of volume).
        stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );

        osg::CullFace* cf( new osg::CullFace( osg::CullFace::FRONT ) );
        stateSet->setAttributeAndModes( cf );
    }

    // Set base class transfer function and volume texture uniforms.
    addHardwareFeatureUniforms( stateSet.get() );

    osg::Program* program = new osg::Program();
    stateSet->setAttribute( program );
    if( _renderMode == SLICES )
    {
        program->addShader( loadShader( osg::Shader::VERTEX, "lfx-volumetricslice.vert" ) );
        program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-volumetricslice.frag" ) );
    }
    else
    {
        program->addShader( loadShader( osg::Shader::VERTEX, "lfx-volumeraytrace.vert" ) );
        program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-volumeraytrace.frag" ) );
    }

    return( stateSet.release() );
}

////////////////////////////////////////////////////////////////////////////////
std::string VolumeRenderer::getEnumName( RenderMode e ) const
{
	if( e == RAY_TRACED) return "RAY_TRACED";

	return "SLICES";
}

////////////////////////////////////////////////////////////////////////////////
VolumeRenderer::RenderMode VolumeRenderer::getEnumFromName( const std::string &name ) const
{
	if( !name.compare( "RAY_TRACED" )) return RAY_TRACED;
	return SLICES;
}

////////////////////////////////////////////////////////////////////////////////
void VolumeRenderer::setNumPlanes( const float& numPlanes )
{
    if( numPlanes <= 0. )
    {
        LFX_WARNING( "setNumPlanes must be > 0.0. Using 100.0." );
        _numPlanes = 100.;
    }
    else
    {
        _numPlanes = numPlanes;
    }
}

////////////////////////////////////////////////////////////////////////////////
float VolumeRenderer::getNumPlanes() const
{
    return( _numPlanes );
}

////////////////////////////////////////////////////////////////////////////////
osg::Texture3D* VolumeRenderer::createTexture( const DBKey& key, osg::Image* image )
{
    // Create dummy Texture / Image as placeholder until real image data is paged in.
    osg::ref_ptr< osg::Texture3D > tex( new osg::Texture3D );
    tex->setResizeNonPowerOfTwoHint( false );
    tex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE );
    tex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
    tex->setWrap( osg::Texture2D::WRAP_R, osg::Texture2D::CLAMP_TO_EDGE );
    tex->setBorderWidth( 0 );

    // Can't use mipmapping because of a shortcoming in NVIDIA's anisotropic filtering.
    // Best results are obtained by disabling mimapping and using LINEAR filtering.
    tex->setUseHardwareMipMapGeneration( false );
    tex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR );
    tex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );

    if( image == NULL )
    {
        osg::Image* dummyImage( new osg::Image );
        dummyImage->setFileName( key );
        tex->setImage( dummyImage );
    }
    else
    {
        tex->setImage( image );
        tex->setName( "donotpage" );
    }

    return( tex.release() );
}

////////////////////////////////////////////////////////////////////////////////
bool VolumeRenderer::validInputs() const
{
    return( getInput( getInputNameAlias( VOLUME_DATA ) ) != NULL );
}

////////////////////////////////////////////////////////////////////////////////
void VolumeRenderer::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	Renderer::serializeData( json );
	SpatialVolume::serializeData( json );

	json->insertObj( VolumeRenderer::getClassName(), true );
	json->insertObjValue( "renderMode", getEnumName( _renderMode ) );
	json->insertObjValue( "numPlanes", _numPlanes );
	json->insertObjValue( "maxSamples", _maxSamples );
	json->insertObjValue( "transparencyScalar", _transparencyScalar );
	json->insertObjValue( "transparencyEnable", _transparencyEnable );
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool VolumeRenderer::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !Renderer::loadData( json, pfactory, perr )) return false;
	if ( !SpatialVolume::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( VolumeRenderer::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get VolumeRenderer data";
		return false;
	}

	std::string name;
	json->getValue( "renderMode", &name, getEnumName( _renderMode ) );
	_renderMode = getEnumFromName( name );

	json->getValue( "numPlanes", &_numPlanes, _numPlanes );
	json->getValue( "maxSamples", &_maxSamples, _maxSamples );
	json->getValue( "transparencyScalar", &_transparencyScalar, _transparencyScalar );
	json->getValue( "transparencyEnable", &_transparencyEnable, _transparencyEnable );

	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void VolumeRenderer::dumpState( std::ostream &os )
{
	Renderer::dumpState( os );
	SpatialVolume::dumpState( os );

	dumpStateStart( VolumeRenderer::getClassName(), os );

	os << "_renderMode: " << getEnumName( _renderMode ) << std::endl;
	os << "_numPlanes: " << _numPlanes << std::endl;
	os << "_maxSamples: " << _maxSamples << std::endl;
	os << "_transparencyScalar: " << _transparencyScalar << std::endl;
	os << "_transparencyEnable: " << _transparencyEnable << std::endl;

	dumpStateEnd( VolumeRenderer::getClassName(), os );
}

// core
}
// lfx
}
