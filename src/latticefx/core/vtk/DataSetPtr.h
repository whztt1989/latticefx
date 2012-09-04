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
#ifndef VE_XPLORER_CFD_DATA_SET_PTR_H
#define VE_XPLORER_CFD_DATA_SET_PTR_H

//#include <ves/util/PointerTypes.h>
#include <boost/smart_ptr/shared_ptr.hpp>

/**
 * \file
 *
 * Include this file to get a forward declaration of the type
 * ves::xplorer:DataSet and its pointer types.
 * For the full declaration of ves::xplorer::DataSet
 * ves/xplorer/DataSet.h must be included, too.
 */

namespace lfx
{
namespace core
{
namespace vtk
{        
class DataSet;
/// Typedef for the SmartPtr types.
//typedef ves::util::ClassPtrDef<DataSet>::type  DataSetPtr;
//typedef ves::util::SharedPtrDef<DataSet>::type DataSetSharedPtr;
//typedef ves::util::WeakPtrDef<DataSet>::type   DataSetWeakPtr;
//typedef ves::util::ScopedPtrDef<DataSet>::type DataSetScopedPtr;
typedef boost::shared_ptr< DataSet > DataSetPtr;

}
}
}
#endif
