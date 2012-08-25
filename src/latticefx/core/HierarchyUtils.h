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

#ifndef __LFX_CORE_HEIRARCHY_UTILS_H__
#define __LFX_CORE_HEIRARCHY_UTILS_H__ 1

#include <latticefx/core/Export.h>
#include <latticefx/core/ChannelData.h>

#include <osg/Vec3>

#include <vector>



namespace lfx {
namespace core {


class HierarchyCallback
{
public:
    HierarchyCallback() {}
    HierarchyCallback( const HierarchyCallback& rhs ) {}
    virtual ~HierarchyCallback() {}

    virtual void operator()( ChannelDataPtr cdp, const std::string& hierarchyNameString ) {}

protected:
};

LATTICEFX_EXPORT void traverseHeirarchy( ChannelDataPtr root, HierarchyCallback& cb );


class LATTICEFX_EXPORT AssembleHierarchy
{
public:
    typedef std::vector< double > RangeVec;

    AssembleHierarchy( RangeVec ranges );
    AssembleHierarchy( unsigned int maxDepth, double baseRange=25000. );
    AssembleHierarchy( const AssembleHierarchy& rhs );
    virtual ~AssembleHierarchy();

    void addChannelData( ChannelDataPtr cdp, const std::string nameString,
            const osg::Vec3& offset=osg::Vec3( 0., 0., 0. ), const unsigned int depth=0 );

    ChannelDataPtr getRoot() const;

protected:
    void recurseInit( ChannelDataPtr cdp, unsigned int depth );

    ChannelDataPtr _root;
    RangeVec _ranges;

    ChannelDataPtr _iterator;
};


// core
}
// lfx
}


// __LFX_CORE_HEIRARCHY_UTILS_H__
#endif
