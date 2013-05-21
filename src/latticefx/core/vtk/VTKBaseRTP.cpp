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
#include <latticefx/core/vtk/VTKBaseRTP.h>
#include <vtkBox.h>

namespace lfx
{

namespace core
{

namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetMinMaxScalarRangeValue( double const minVal, double const maxVal )
{
    m_minScalarValue = minVal;
    m_maxScalarValue = maxVal;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetRequestedValue( double const value )
{
    m_requestedValue = value;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetActiveScalar( std::string const scalarName )
{
    m_activeScalar = scalarName;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetActiceVector( std::string const vectorName )
{
    m_activeVector = vectorName;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetMaskValue( double const value )
{
    m_mask = value;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetPlaneDirection( const CuttingPlane::SliceDirection& planeDirection )
{
    m_planeDirection = planeDirection;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetRoiBox(const std::vector<double> &roiBox)
{
	m_roiBox = roiBox;
}
////////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkExtractGeometry> VTKBaseRTP::GetRoi(vtkDataObject *pdo)
{
	if (m_roiBox.size() < 6) return vtkSmartPointer<vtkExtractGeometry>();

	vtkSmartPointer<vtkBox> boxExtract = vtkSmartPointer<vtkBox>::New();
	boxExtract->SetBounds(&m_roiBox[0]);
 
	vtkSmartPointer<vtkExtractGeometry> extract = vtkSmartPointer<vtkExtractGeometry>::New();
	extract->SetImplicitFunction(boxExtract);

	if (m_roiExtractBoundaryCells)
		extract->SetExtractBoundaryCells(1);
	else
		extract->SetExtractBoundaryCells(0);

	extract->SetInput(pdo);

	return extract;
}
////////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkExtractGeometry> VTKBaseRTP::GetRoi(vtkAlgorithmOutput* pOutPin)
{
	if (m_roiBox.size() < 6) return vtkSmartPointer<vtkExtractGeometry>();

	vtkSmartPointer<vtkBox> boxExtract = vtkSmartPointer<vtkBox>::New();
	boxExtract->SetBounds(&m_roiBox[0]);
 
	vtkSmartPointer<vtkExtractGeometry> extract = vtkSmartPointer<vtkExtractGeometry>::New();
	extract->SetImplicitFunction(boxExtract);

	if (m_roiExtractBoundaryCells)
		extract->SetExtractBoundaryCells(1);
	else
		extract->SetExtractBoundaryCells(0);

	extract->SetInputConnection(0, pOutPin);

	return extract;
}
////////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkExtractPolyDataGeometry> VTKBaseRTP::GetRoiPoly(vtkAlgorithmOutput* pOutPin)
{
	if (m_roiBox.size() < 6) return vtkSmartPointer<vtkExtractPolyDataGeometry>();

	vtkSmartPointer<vtkBox> boxExtract = vtkSmartPointer<vtkBox>::New();
	boxExtract->SetBounds(&m_roiBox[0]);
 
	vtkSmartPointer<vtkExtractPolyDataGeometry> extract = vtkSmartPointer<vtkExtractPolyDataGeometry>::New();
	extract->SetImplicitFunction(boxExtract);

	if (m_roiExtractBoundaryCells)
		extract->SetExtractBoundaryCells(1);
	else
		extract->SetExtractBoundaryCells(0);
	

	extract->SetInputConnection(0, pOutPin);

	return extract;
}

////////////////////////////////////////////////////////////////////////////////
}
}
}
