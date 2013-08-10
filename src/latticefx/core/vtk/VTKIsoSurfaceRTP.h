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
#ifndef __LATTICEFX_CORE_VTK_ISOSURFACE_RTP_OPERATION_H__
#define __LATTICEFX_CORE_VTK_ISOSURFACE_RTP_OPERATION_H__ 1

#include <latticefx/core/vtk/VTKBaseRTP.h>

#include <latticefx/core/vtk/Export.h>

namespace lfx
{

namespace core
{

namespace vtk
{

/** \class VTKIsosurfaceRTP VTKIsosurfaceRTP.h <latticefx/core/vtk/VTKIsosurfaceRTP.h>
 \brief Class the creates an isosurface polydata from a vtk dataset.
 \details This class takes a vtkDataObject in a ChannelDatavtkDataObject with the
 name vtkDataObject and creates a vtkPolyData with the vector field. */

class LATTICEFX_CORE_VTK_EXPORT VTKIsoSurfaceRTP : public VTKBaseRTP
{
public:

    ///Default constructor
    VTKIsoSurfaceRTP()
        :
        VTKBaseRTP( lfx::core::RTPOperation::Channel )
    {
        ;
    }

    ///Destructor
    virtual ~VTKIsoSurfaceRTP()
    {
        ;
    }

	virtual std::string getClassName() const { return "VTKIsoSurfaceRTP"; }

    ///We are going to be creating a ChannelDatavtkPolyData so we override the
    ///channel method since we do not have a ChannelData already
    virtual lfx::core::ChannelDataPtr channel( const lfx::core::ChannelDataPtr maskIn );

	virtual void dumpState( std::ostream &os );

protected:

	virtual void serializeData( JsonSerializer *json ) const;
	virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );
};

typedef boost::shared_ptr< VTKIsoSurfaceRTP > VTKIsoSurfaceRTPPtr;

}
}
}
// __LATTICEFX_CORE_VTK_ISOSURFACE_RTP_OPERATION_H__
#endif