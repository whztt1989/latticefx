/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
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
* -----------------------------------------------------------------
* Date modified: $Date$
* Version:       $Rev$
* Author:        $Author$
* Id:            $Id$
* -----------------------------------------------------------------
*
*************** <auto-copyright.rb END do not edit this line> **************/

#include <latticefx/core/DataSet.h>
#include <latticefx/core/PagingThread.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/ChannelDataLOD.h>
#include <latticefx/core/Preprocess.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/PageData.h>
#include <latticefx/core/PagingCallback.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osgwTools/Shapes.h>
#include <osg/Geode>
#include <osg/Geometry>

#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>


class ColorProcess : public lfx::Preprocess
{
public:
    ColorProcess()
      : lfx::Preprocess()
    {
        setActionType( lfx::Preprocess::REPLACE_DATA );
    }

    virtual lfx::ChannelDataPtr operator()()
    {
        lfx::ChannelDataOSGArrayPtr input( boost::static_pointer_cast< lfx::ChannelDataOSGArray >( _inputs[ 0 ] ) );

        osg::Vec3Array* color( new osg::Vec3Array );
        color->push_back( osg::Vec3( 0., 1., 0. ) );
        lfx::ChannelDataOSGArrayPtr newData( new lfx::ChannelDataOSGArray( color, input->getName() ) );

        lfx::ChannelDataLODPtr cdLOD( new lfx::ChannelDataLOD( input->getName() ) );
        cdLOD->setRange( cdLOD->addChannel( input ),
            lfx::RangeValues( 0., 40000. ) );
        cdLOD->setRange( cdLOD->addChannel( newData ),
            lfx::RangeValues( 40000., FLT_MAX ) );
        return( cdLOD );
    }
};

class BoxRenderer : public lfx::Renderer
{
public:
    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn )
    {
        lfx::ChannelDataOSGArray* cda( static_cast< lfx::ChannelDataOSGArray* >( _inputs[ 0 ].get() ) );
        osg::Vec3Array* c( static_cast< osg::Vec3Array* >( cda->asOSGArray() ) );

        osg::Vec4Array* color( new osg::Vec4Array );
        color->push_back( osg::Vec4( (*c)[ 0 ], 1.f ) );

        osg::Geode* geode( new osg::Geode() );

        osg::Geometry* geom( osgwTools::makeBox( osg::Vec3( .5, .5, .5 ) ) );
        geom->setColorArray( color );
        geom->setColorBinding( osg::Geometry::BIND_OVERALL );
        geode->addDrawable( geom );

        return( geode );
    }
};

lfx::DataSetPtr createDataSet()
{
    osg::Vec3Array* c( new osg::Vec3Array );
    c->push_back( osg::Vec3( 1., 0., 0. ) );
    lfx::ChannelDataOSGArrayPtr colorData( new lfx::ChannelDataOSGArray( c, "color" ) );

    lfx::DataSetPtr dsp( new lfx::DataSet() );
    dsp->addChannel( colorData );

    ColorProcess* op( new ColorProcess );
    op->addInput( "color" );
    dsp->addPreprocess( lfx::PreprocessPtr( op ) );

    BoxRenderer* renderOp( new BoxRenderer );
    renderOp->addInput( "color" );
    dsp->setRenderer( lfx::RendererPtr( renderOp ) );

    return( dsp );
}


int main( int argc, char** argv )
{
    lfx::Log::instance()->setPriority( lfx::Log::PrioInfo, lfx::Log::Console );

    osg::ArgumentParser arguments( &argc, argv );

    lfx::DataSetPtr dsp( createDataSet() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 20, 30, 800, 460 );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );

    viewer.setSceneData( dsp->getSceneData() );

    // Really we would need to change the projection matrix and viewport
    // in an event handler that catches window size changes. We're cheating.
    lfx::PagingThread* pageThread( lfx::PagingThread::instance() );
    const osg::Camera* cam( viewer.getCamera() );
    pageThread->setTransforms( cam->getProjectionMatrix(), cam->getViewport() );

    osg::Vec3d eye, center, up;
    while( !viewer.done() )
    {
        cam->getViewMatrixAsLookAt( eye, center, up );
        pageThread->setTransforms( osg::Vec3( eye ) );
        viewer.frame();
    }
    return( 0 );
}
