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
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/RTPOperation.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osgViewer/Viewer>

#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <fstream>
#include <sstream>


using namespace lfx::core;


/** \brief Example of a run time operation to mask off part of the data.
\details The data is a plot of -( x*x + y*y ). This mask operation
removes all data points of the graph that satisfy y + z > 0.
*/
class MyMask : public RTPOperation
{
public:
    MyMask()
        : RTPOperation( RTPOperation::Mask )
    {
        setInputNameAlias( POSITIONS, "vertices" );
    }
    virtual ~MyMask()
    {}

    typedef enum
    {
        POSITIONS
    } InputType;

    virtual ChannelDataPtr mask( const ChannelDataPtr maskIn )
    {
        ChannelDataPtr input = getInput( getInputNameAlias( POSITIONS ) );
        if( ( input == NULL ) )
        {
            LFX_WARNING_STATIC( "lfx.demo", "MyMask::mask(): Invalid input." );
            return( ChannelDataPtr( ( ChannelData* )( NULL ) ) );
        }

        // Get the threshold test value, configurable from the calling code using
        // setValue( "threshold", OperationValue( floatVal ) );
        float threshold( 0.f );
        if( hasValue( "threshold" ) )
        {
            threshold = getValue( "threshold" )->extract< float >();
        }

        osg::Vec3Array* xyz = static_cast< osg::Vec3Array* >( input->asOSGArray() );
        unsigned int size( xyz->getNumElements() );

        osg::ref_ptr< osg::ByteArray > maskData( new osg::ByteArray );
        maskData->resize( size );

        osg::ByteArray* maskInData = static_cast< osg::ByteArray* >( maskIn->asOSGArray() );
        signed char* maskInPtr = &( ( *maskInData )[ 0 ] );
        unsigned int idx;
        for( idx = 0; idx < size; ++idx, ++maskInPtr )
        {
            if( *maskInPtr == 0 )
            {
                continue;
            }

            signed char& maskValue( ( *maskData )[ idx ] );
            const osg::Vec3& v( ( *xyz )[ idx ] );
            maskValue = ( v.z() + v.y() > threshold ) ? 0 : 1;
        }

        ChannelDataOSGArrayPtr cdp( new ChannelDataOSGArray( input->getName(), maskData.get() ) );
        return( cdp );
    }

private:
    friend class boost::serialization::access;

    template< class Archive >
    void serialize( Archive& ar, const unsigned int version )
    {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP( RTPOperation );
    }
};
BOOST_CLASS_VERSION( MyMask, 0 );
BOOST_CLASS_EXPORT( MyMask );

class MyRenderer : public Renderer
{
public:
    MyRenderer()
        : Renderer()
    {
        setInputNameAlias( POSITIONS, "vertices" );
    }
    virtual ~MyRenderer()
    {}

    typedef enum
    {
        POSITIONS
    } InputType;

    virtual osg::Node* getSceneGraph( const ChannelDataPtr maskIn )
    {
        osg::ref_ptr< osg::Geode > geode( new osg::Geode );
        geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        osg::Geometry* geom = new osg::Geometry;
        ChannelDataPtr input( getInput( getInputNameAlias( POSITIONS ) )->getMaskedChannel( maskIn ) );
        geom->setVertexArray( input->asOSGArray() );

        unsigned int idx, size = geom->getVertexArray()->getNumElements();
        osg::DrawElementsUInt* deui( new osg::DrawElementsUInt( GL_POINTS, size ) );
        for( idx = 0; idx < size; idx++ )
        {
            ( *deui )[ idx ] = idx;
        }
        geom->addPrimitiveSet( deui );
        geode->addDrawable( geom );

        return( geode.release() );
    }

private:
    friend class boost::serialization::access;

    template< class Archive >
    void serialize( Archive& ar, const unsigned int version )
    {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP( Renderer );
    }
};
BOOST_CLASS_VERSION( MyRenderer, 0 );
BOOST_CLASS_EXPORT( MyRenderer );


DataSetPtr createDataSet()
{
    // Create a data set
    DataSetPtr dsp( new DataSet() );

    // Create a mask operation and add it to the data set.
    RTPOperationPtr maskOp( new MyMask() );
    maskOp->setValue( "threshold", OperationValue( -0.1f ) );
    //maskOp->setEnable( false ); // Optionally disable the mask operation.
    dsp->addOperation( maskOp );

    RendererPtr renderOp( new MyRenderer() );
    dsp->setRenderer( renderOp );

    return( dsp );
}
void addData( DataSetPtr dsp )
{
    // Create a vertex array for the graph - (x*x + y*y), with the plot
    // space ranging from -1 to 1 in both x and y.
    osg::ref_ptr< osg::Vec3Array > xyzData( new osg::Vec3Array );
    const unsigned int w( 400 ), h( 400 );
    xyzData->resize( w * h );
    unsigned int wIdx, hIdx;
    for( wIdx = 0; wIdx < w; ++wIdx )
    {
        for( hIdx = 0; hIdx < h; ++hIdx )
        {
            const float x( ( ( float )wIdx ) / w * 2. - 1. );
            const float y( ( ( float )hIdx ) / h * 2. - 1. );
            ( *xyzData )[( wIdx * w ) + hIdx ].set(
                x, y, -( x * x + y * y ) );
        }
    }

    ChannelDataOSGArrayPtr cdp( new ChannelDataOSGArray( "vertices", xyzData.get() ) );
    const Poco::UUID uuid( Poco::UUIDGenerator::defaultGenerator().create() );
    cdp->setDBKey( DBKey( cdp->getName() + "-" + uuid.toString() ) );

    dsp->addChannel( cdp );
    dsp->getOperation( 0 )->addInput( cdp->getName() );
    dsp->getRenderer()->addInput( cdp->getName() );
}



int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );

    std::ostringstream ostr;
    {
        // Create an example data set.
        DataSetPtr dsp( createDataSet() );
        // Serialize it.
        boost::archive::text_oarchive oa( ostr );
        oa << boost::serialization::make_nvp( "DataSet", *dsp );
    }
    std::cout << ostr.str() << std::endl;

    DataSetPtr dsp( DataSetPtr( new DataSet() ) );
    {
        std::istringstream istr( ostr.str() );
        // Deserialize the data set.
        boost::archive::text_iarchive ia( istr );
        ia >> boost::serialization::make_nvp( "DataSet", *dsp );
    }
    addData( dsp );

    osgViewer::Viewer viewer;
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( dsp->getSceneData() );
    return( viewer.run() );
}
