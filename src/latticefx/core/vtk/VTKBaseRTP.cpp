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
	if (m_roiBox.size() < 6)
    {
        std::cout << "VTKBaseRTP::GetRoi : No bounding box set." << std::endl;
        return vtkSmartPointer<vtkExtractGeometry>();
    }

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
	if (m_roiBox.size() < 6)
    {
        std::cout << "VTKBaseRTP::GetRoi : No bounding box set." << std::endl;
        return vtkSmartPointer<vtkExtractGeometry>();
    }

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
	if (m_roiBox.size() < 6)
    {
        std::cout << "VTKBaseRTP::GetRoiPoly : No bounding box set." << std::endl;
        return vtkSmartPointer<vtkExtractPolyDataGeometry>();
    }

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
void VTKBaseRTP::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	RTPOperation::serializeData( json );

	json->insertObj( VTKBaseRTP::getClassName(), true);
	json->insertObjValue( "requestedValue",  m_requestedValue );
	json->insertObjValue( "minScalarValue",  m_minScalarValue );
	json->insertObjValue( "maxScalarValue",  m_maxScalarValue );
	json->insertObjValue( "activeScalar",  m_activeScalar );
	json->insertObjValue( "activeVector",  m_activeVector );
	json->insertObjValue( "mask",  m_mask );
	json->insertObjValue( "planeDirection",  CuttingPlane::getEnumName( m_planeDirection ) );
	json->insertObjValue( "roiExtractBoundaryCells",  m_roiExtractBoundaryCells );
	json->insertArray( "roiBox", true );
	for( unsigned int i=0; i < m_roiBox.size(); i++ )
	{
		json->insertArrValue( m_roiBox[i] );
	}

	json->popParent();
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool VTKBaseRTP::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !RTPOperation::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( VTKBaseRTP::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get VTKBaseRTP data";
		return false;
	}

	json->getValue( "requestedValue",  &m_requestedValue );
	json->getValue( "minScalarValue",  &m_minScalarValue );
	json->getValue( "maxScalarValue",  &m_maxScalarValue );
	json->getValue( "activeScalar",  &m_activeScalar );
	json->getValue( "activeVector",  &m_activeVector );
	json->getValue( "mask",  &m_mask );
	json->getValue( "roiExtractBoundaryCells",  &m_roiExtractBoundaryCells );

	std::string name;
	json->getValue( "planeDirection", &name, CuttingPlane::getEnumName( m_planeDirection ) );
	m_planeDirection = CuttingPlane::getEnumFromName( name );

	m_roiBox.clear();
	json->getArray( "roiBox", true );
	for( unsigned int i=0; i<json->size(); i++)
	{
		double d=0;
		json->getValue( i, &d );
		m_roiBox.push_back( d );
	}

	json->popParent();
	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
}
}
}
