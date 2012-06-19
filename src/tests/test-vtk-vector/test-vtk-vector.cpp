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
#include <latticefx/DataSet.h>
#include <latticefx/ChannelData.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/ChannelDatavtkPolyData.h>
#include <latticefx/ChannelDatavtkDataObject.h>
#include <latticefx/RTPOperation.h>
#include <latticefx/Renderer.h>
#include <latticefx/TextureUtils.h>
#include <latticefx/BoundUtils.h>
#include <latticefx/VectorRenderer.h>
#include <latticefx/TransferFunctionUtils.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture3D>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osgViewer/Viewer>
#include <osgDB/FileUtils>

#include <vtk_utils/DataSet.h>

#include <vtkDataObject.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkCellDataToPointData.h>
#include <vtkMaskPoints.h>
#include <vtkCompositeDataPipeline.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkMath.h>
#include <vtkPointData.h>

////////////////////////////////////////////////////////////////////////////////
class VTKVectorFieldRTP : public lfx::RTPOperation
{
public:
    VTKVectorFieldRTP()
        : 
        lfx::RTPOperation( lfx::RTPOperation::Channel )
    {}
    virtual ~VTKVectorFieldRTP()
    {
        ;
    }
    
    ///We are going to be creating a ChannelDatavtkPolyData so we override the 
    ///channel method since we do not have a ChannelData already
    virtual lfx::ChannelDataPtr channel( const lfx::ChannelDataPtr maskIn )
    {
        vtkDataObject* tempVtkDO = 
            boost::static_pointer_cast< lfx::ChannelDatavtkDataObject >( getInput( "vtkDataObject" ) )->GetDataObject();

        vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
        c2p->SetInput( tempVtkDO );
        //c2p->Update();

        vtkMaskPoints* ptmask = vtkMaskPoints::New();
        
        if( tempVtkDO->IsA( "vtkCompositeDataSet" ) )
        {
            vtkCompositeDataGeometryFilter* m_multiGroupGeomFilter = 
                vtkCompositeDataGeometryFilter::New();
            m_multiGroupGeomFilter->SetInputConnection( c2p->GetOutputPort() );
            //return m_multiGroupGeomFilter->GetOutputPort(0);
            ptmask->SetInputConnection( m_multiGroupGeomFilter->GetOutputPort(0) );
            m_multiGroupGeomFilter->Delete();
        }
        else
        {
            //m_geometryFilter->SetInputConnection( input );
            //return m_geometryFilter->GetOutputPort();
            vtkDataSetSurfaceFilter* m_surfaceFilter = 
                vtkDataSetSurfaceFilter::New();
            m_surfaceFilter->SetInputConnection( c2p->GetOutputPort() );
            //return m_surfaceFilter->GetOutputPort();
            ptmask->SetInputConnection( m_surfaceFilter->GetOutputPort() );
            m_surfaceFilter->Delete();
        }

        // get every nth point from the dataSet data
        ptmask->SetOnRatio( 1.0 );
        ptmask->Update();

        lfx::ChannelDatavtkPolyDataPtr cdpd( new lfx::ChannelDatavtkPolyData( ptmask->GetOutput(), "vtkPolyData" ) );

        ptmask->Delete();
        c2p->Delete();

        return( cdpd );
    }
    
protected:
};

typedef boost::shared_ptr< VTKVectorFieldRTP > VTKVectorFieldRTPPtr;

////////////////////////////////////////////////////////////////////////////////
class VTKVectorRenderer : public lfx::VectorRenderer
{
public:
    ///Default constructor
    ///We are really a fancy lfx::VectorRenderer specific to VTK data
    VTKVectorRenderer() 
        : 
        lfx::VectorRenderer()
    {}
    
    ///Destructor
    virtual ~VTKVectorRenderer()
    {}
    
    ///Set the active vector name to tell the render what to put in the textures
    ///\param activeVector The active vector name to use
    void SetActiveVector( const std::string& activeVector )
    {
        m_activeVector = activeVector;
    }

    ///Set the active scalar name to tell the render what to put in the textures
    ///\param activeScalar The active scalar name to use
    void SetActiveScalar( const std::string& activeScalar )
    {
        m_activeScalar = activeScalar;
    }

    ///We are overriding the lfx::VectorRenderer method and then calling it 
    ///once we have given it all of the data it needs.
    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn )
    {
        vtkPolyData* tempVtkPD = 
            boost::static_pointer_cast< lfx::ChannelDatavtkPolyData >( getInput( "vtkPolyData" ) )->GetPolyData();

        vtkPoints* points = tempVtkPD->GetPoints();
        size_t dataSize = points->GetNumberOfPoints();

        vtkPointData* pointData = tempVtkPD->GetPointData();
        vtkDataArray* vectorArray = pointData->GetVectors(m_activeVector.c_str());
        vtkDataArray* scalarArray = pointData->GetScalars(m_activeScalar.c_str());
        
        double scalarRange[ 2 ] = {0,1.0};
        scalarArray->GetRange( scalarRange );
        
        double x[3];
        double val;
        //double rgb[3];
        
        osg::ref_ptr< osg::Vec3Array > vertArray( new osg::Vec3Array );
        vertArray->resize( dataSize );
        osg::ref_ptr< osg::Vec3Array > dirArray( new osg::Vec3Array );
        dirArray->resize( dataSize );
        osg::ref_ptr< osg::FloatArray > colorArray( new osg::FloatArray );
        colorArray->resize( dataSize );
        
        for( size_t i = 0; i < dataSize; ++i )
        {
            //Get Position data
            points->GetPoint( i, x );
            (*vertArray)[ i ].set( x[0], x[1], x[2] );
            
            if( scalarArray )
            {
                //Setup the color array
                scalarArray->GetTuple( i, &val );
                val = vtkMath::ClampAndNormalizeValue( val, scalarRange );
                (*colorArray)[ i ] = val;
                //lut->GetColor( val, rgb );
                //*scalarI++ = val;//rgb[0];
                //*scalarI++ = rgb[1];
                //*scalarI++ = rgb[2];
            }
            
            if( vectorArray )
            {
                //Get Vector data
                vectorArray->GetTuple( i, x );
                osg::Vec3 v( x[0], x[1], x[2] );
                v.normalize();
                (*dirArray)[ i ].set( v.x(), v.y(), v.z() );
            }
        }

        setPointStyle( lfx::VectorRenderer::DIRECTION_VECTORS );
        
        //by this stage of the game the render has already had setInputs called 
        //on it by lfx::DataSet therefore we can modify the _inputs array
        lfx::ChannelDataOSGArrayPtr vertData( new lfx::ChannelDataOSGArray( vertArray.get(), "positions" ) );
        addInput( vertData );

        lfx::ChannelDataOSGArrayPtr dirData( new lfx::ChannelDataOSGArray( dirArray.get(), "directions" ) );
        addInput( dirData );

        lfx::ChannelDataOSGArrayPtr colorData( new lfx::ChannelDataOSGArray( colorArray.get(), "scalar" ) );
        addInput( colorData );
        
        // Configure transfer function.
        setTransferFunctionInput( "scalar" );
        setTransferFunction( lfx::loadImageFromDat( "01.dat" ) );
        setTransferFunctionDestination( lfx::Renderer::TF_RGBA );

#if WRITE_IMAGE_DATA            
        //osgDB::writeNodeFile( *(tempGeode.get()), "gpu_vector_field.ive" );
#endif
        
        return( lfx::VectorRenderer::getSceneGraph( maskIn ) );
    }
    
protected:
    ///The active vector to set which vector to use for rendering
    std::string m_activeVector;
    ///The active scalar to set which scalar to use for rendering
    std::string m_activeScalar;
};

typedef boost::shared_ptr< VTKVectorRenderer > VTKVectorRendererPtr;

////////////////////////////////////////////////////////////////////////////////
lfx::vtk_utils::DataSet* LoadDataSet( std::string filename )
{
    lfx::vtk_utils::DataSet* tempDataSet = new lfx::vtk_utils::DataSet();
    tempDataSet->SetFileName( filename );
    tempDataSet->SetUUID( "VTK_DATA_FILE", "test" );
    //ves::open::xml::DataValuePairPtr stringDVP =
    //    tempInfoPacket->GetProperty( "VTK_ACTIVE_DATA_ARRAYS" );
    /*std::vector< std::string > vecStringArray;
    if( stringDVP )
    {
    ves::open::xml::OneDStringArrayPtr stringArray =
    boost::dynamic_pointer_cast <
    ves::open::xml::OneDStringArray > (
    stringDVP->GetDataXMLObject() );
    vecStringArray = stringArray->GetArray();
    tempDataSet->SetActiveDataArrays( vecStringArray );
    }*/
    const std::string tempDataSetFilename = tempDataSet->GetFileName();
    std::cout << "|\tLoading data for file " << tempDataSetFilename << std::endl;
    //tempDataSet->SetArrow(
    //                        ves::xplorer::ModelHandler::instance()->GetArrow() );
    //Check and see if the data is part of a transient series
    /*if( tempInfoPacket->GetProperty( "VTK_TRANSIENT_SERIES" ) )
    {
    std::string precomputedSurfaceDir =
    tempInfoPacket->GetProperty( "VTK_TRANSIENT_SERIES" )->
    GetDataString();
    lastDataAdded->LoadTransientData( precomputedSurfaceDir );
    }
    else*/
    {
        tempDataSet->LoadData();
        tempDataSet->SetActiveVector( 0 );
        tempDataSet->SetActiveScalar( 0 );
    }

    vtkDataObject* tempVtkDataSet = tempDataSet->GetDataSet();
    //If the data load failed
    if( !tempVtkDataSet )
    {
        std::cout << "|\tData failed to load." << std::endl;
        //_activeModel->DeleteDataSet( tempDataSetFilename );
    }
    else
    {
        std::cout << "|\tData is loaded for file "
            << tempDataSetFilename
            << std::endl;
        //if( lastDataAdded->GetParent() == lastDataAdded )
        //{
        //_activeModel->GetDCS()->
        //AddChild( lastDataAdded->GetDCS() );
        //_activeModel->SetActiveDataSet( 0 );
        //}
        //m_datafileLoaded( tempDataSetFilename );
    }
    return tempDataSet;
}
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv )
{
    //Pre work specific to VTK
    vtkCompositeDataPipeline* prototype = vtkCompositeDataPipeline::New();
    vtkAlgorithm::SetDefaultExecutivePrototype( prototype );
    prototype->Delete();

    //Load the VTK data
    lfx::vtk_utils::DataSet* tempDataSet = LoadDataSet( argv[ 1 ] );
    
    //Create the DataSet for this visualization with VTK
    lfx::DataSetPtr dsp( new lfx::DataSet() );
    
    //1st Step
    lfx::ChannelDatavtkDataObjectPtr dobjPtr( new lfx::ChannelDatavtkDataObject( tempDataSet->GetDataSet(), "vtkDataObject" ) );
    dsp->addChannel( dobjPtr );
    
    //2nd Step - the output of this is a ChannelData containing vtkPolyData
    VTKVectorFieldRTPPtr vectorRTP( new VTKVectorFieldRTP() );
    vectorRTP->addInput( "vtkDataObject" );
    dsp->addOperation( vectorRTP );
    
    //3rd Step - now lets use out generic Renderer for vtkPolyData-to-an-instance-vector-field
    VTKVectorRendererPtr renderOp( new VTKVectorRenderer() );
    renderOp->SetActiveVector( "Momentum" );
    renderOp->SetActiveScalar( "Density" );
    renderOp->addInput( "vtkPolyData" );
    dsp->setRenderer( renderOp );

    std::cout << "lfx...creating data..." << std::endl;
    osg::Node* sceneNode = dsp->getSceneData();
    std::cout << "...finished creating data. " << std::endl;

    //Clean up the raw vtk memory
    delete tempDataSet;
    
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( sceneNode );

    return( viewer.run() );
}
