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

#ifndef __LFX_CORE_MISC_UTILS_H__
#define __LFX_CORE_MISC_UTILS_H__ 1


#include <latticefx/core/Export.h>


namespace lfx
{
namespace core
{


/** \class MiscUtils MiscUtils.h <latticefx/core/MiscUtils.h>
\brief Place for misc utility routines
\details Place for misc utility routines
*/
class LATTICEFX_EXPORT MiscUtils
{
public:
	
	static bool is_close(double d1, double d2, double eps=.00001);
	static bool is_close(float f1, float f2, float eps=.00001f);
	static bool isnot_close(double d1, double d2, double eps=.00001);
	static bool isnot_close(float f1, float f2, float eps=.00001f);

};


/**@}*/


// core
}
// lfx
}


// __LFX_CORE_MISC_UTILS_H__
#endif
