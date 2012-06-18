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

#ifndef __LATTICEFX_CHANNEL_DATA_IMAGE_SET_H__
#define __LATTICEFX_CHANNEL_DATA_IMAGE_SET_H__ 1


#include <latticefx/Export.h>
#include <latticefx/ChannelDataComposite.h>

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <set>
#include <map>


namespace lfx {



/** \class ChannelDataImageSet ChannelDataImageSet.h <latticefx/ChannelDataImageSet.h>
\brief Composite pattern container for sets of image data, such as texture octants.
\details TBD */
class LATTICEFX_EXPORT ChannelDataImageSet : public lfx::ChannelDataComposite
{
public:
    ChannelDataImageSet( const std::string& name=std::string( "" ) );
    ChannelDataImageSet( const ChannelDataImageSet& rhs );
    virtual ~ChannelDataImageSet();


    virtual ChannelDataImageSet* getAsSet() { return( this ); }


    /** \brief Prepare the ChannelData for processing by the DataSet.
    \details Prior to processing ChannelData in the LatticeFX data pipeline,
    the DataSet calls ChannelData::reset() on all attached ChannelData
    instances. This is useful in situations where data processing modifies
    ChannelData (such as a convolution filter or Gaussion blur). Dereived
    ChannelData classes must store a copy of the original data to protect
    against such destruction. reset() allows the ChannelData to refresh the
    working copy of the data from the original. */
    virtual void reset() {}

    /** \brief TBD
    \details TBD */
    static bool allImageSetData( const ChannelDataList& data );

protected:
};

typedef boost::shared_ptr< ChannelDataComposite > ChannelDataCompositePtr;


// lfx
}


// __LATTICEFX_CHANNEL_DATA_IMAGE_SET_H__
#endif