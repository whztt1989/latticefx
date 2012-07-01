/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2012 by Iowa State University
 *
 * Original Development Team:
 *   - ISU's Thermal Systems Virtual Engineering Group,
 *     Headed by Kenneth Mark Bryden, Ph.D., www.vrac.iastate.edu/~kmbryden
 *   - Reaction Engineering International, www.reaction-eng.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
 *************** <auto-copyright.rb END do not edit this line> ***************/
#ifndef EXTRACT_GEOMETRY_CALLBACK
#define  EXTRACT_GEOMETRY_CALLBACK
/*!\file ExtractGeometryCallback.h
 * ExtractGeometryCallback API.
 * \class lfx::vtk_utils::ExtractGeometryCallback
 *
 */
class vtkDataSet;
class vtkMultiBlockDataSet;
class vtkUnstructuredGrid;
class vtkPolyData;

#include <latticefx/utils/vtk/Export.h>
#include <latticefx/utils/vtk/DataObjectHandler.h>


namespace lfx
{
namespace vtk_utils
{
class LATTICEFX_VTK_UTILS_EXPORT ExtractGeometryCallback:
            public DataObjectHandler::DatasetOperatorCallback
{
public:
    ///Constructor
    ExtractGeometryCallback();

    ///Destructor
    virtual ~ExtractGeometryCallback();

    ///The operation to do on each vtkDataSet in the vtkDataObject
    ///\param dataset The vtkDataSet to operate on
    virtual void OperateOnDataset( vtkDataSet* dataset );

    ///Get the Mean Cell BBox length
    vtkDataObject* GetDataset();
    ///The polydata representing of a surface
    void SetPolyDataSurface( vtkPolyData* surface );
    
protected:
    ///THe multibblock dataset holding the sub sample
    vtkMultiBlockDataSet* m_dataset;
    ///The number of blocks
    int m_dataCounter;
    ///The surface for the sample region
    vtkPolyData* m_surface;
    ///Bounds for the dataset
    double m_bbox[ 6 ];
};
}// end of util namesapce
}// end of xplorer namesapce
#endif