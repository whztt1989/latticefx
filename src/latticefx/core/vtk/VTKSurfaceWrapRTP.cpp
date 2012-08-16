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
#include <latticefx/core/vtk/VTKSurfaceWrapRTP.h>
#include <latticefx/core/vtk/ChannelDatavtkPolyDataMapper.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>

#include <vtkDataObject.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkCellDataToPointData.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkPolyDataNormals.h>

namespace lfx {

namespace core {

namespace vtk {

////////////////////////////////////////////////////////////////////////////////
lfx::core::ChannelDataPtr VTKSurfaceWrapRTP::channel( const lfx::core::ChannelDataPtr maskIn )
{
    lfx::core::vtk::ChannelDatavtkDataObjectPtr cddoPtr = 
        boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkDataObject >( 
        getInput( "vtkDataObject" ) );
    vtkDataObject* tempVtkDO = cddoPtr->GetDataObject();

    vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
    if( tempVtkDO->IsA( "vtkCompositeDataSet" ) )
    {
        vtkCompositeDataGeometryFilter* m_multiGroupGeomFilter = 
            vtkCompositeDataGeometryFilter::New();
        m_multiGroupGeomFilter->SetInput( tempVtkDO );
        c2p->SetInputConnection( m_multiGroupGeomFilter->GetOutputPort(0) );
        m_multiGroupGeomFilter->Delete();
    }
    else
    {
        vtkDataSetSurfaceFilter* m_surfaceFilter = 
            vtkDataSetSurfaceFilter::New();
        m_surfaceFilter->SetInput( tempVtkDO );
        c2p->SetInputConnection( m_surfaceFilter->GetOutputPort() );
        m_surfaceFilter->Delete();
    }
    vtkPolyDataNormals* normals = vtkPolyDataNormals::New();
    normals->SetInputConnection( c2p->GetOutputPort() );

    lfx::core::vtk::ChannelDatavtkPolyDataMapperPtr cdpd( 
        new lfx::core::vtk::ChannelDatavtkPolyDataMapper( normals->GetOutputPort(), "vtkPolyDataMapper" ) );

    c2p->Delete();
    normals->Delete();

    return( cdpd );
}
////////////////////////////////////////////////////////////////////////////////
}
}
}