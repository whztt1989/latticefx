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

#include <latticefx/core/BoundUtils.h>
#include <osg/BoundingSphere>
#include <osg/BoundingBox>
#include <osg/Array>


namespace lfx
{
namespace core
{


osg::BoundingSphere getBound( const osg::Vec3Array& array, const double pad )
{
    osg::BoundingSphere bs;
    unsigned int idx;
    for( idx = 0; idx < array.size(); ++idx )
    {
        bs.expandBy( array[ idx ] );
    }
    bs.radius() += pad;
    return( bs );
}
osg::BoundingBox getBound( const osg::Vec3Array& array, const osg::Vec3& pad )
{
    osg::BoundingBox bb;
    unsigned int idx;
    for( idx = 0; idx < array.size(); ++idx )
    {
        bb.expandBy( array[ idx ] );
    }
    bb._min -= pad;
    bb._max += pad;
    return( bb );
}


// core
}
// lfx
}
