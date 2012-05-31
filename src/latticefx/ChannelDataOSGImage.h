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

#ifndef __LATTICEFX_CHANNEL_DATA_OSG_IMAGE_H__
#define __LATTICEFX_CHANNEL_DATA_OSG_IMAGE_H__ 1


#include <latticefx/Export.h>
#include <latticefx/ChannelData.h>

#include <osg/Image>

#include <boost/shared_ptr.hpp>

#include <vector>



namespace lfx {


/** \class ChannelDataOSGImage ChannelDataOSGImage.h <latticefx/ChannelDataOSGImage.h>
\brief TBD
\details TBD */
class LATTICEFX_EXPORT ChannelDataOSGImage : public ChannelData
{
public:
    ChannelDataOSGImage( const std::string& name=std::string( "" ), osg::Image* Image=NULL );
    ChannelDataOSGImage( const ChannelDataOSGImage& rhs );
    virtual ~ChannelDataOSGImage();


    /** \brief Return a pointer to (possibly a copy of) the data.
    \details This function should be overridden in derived classes.

    Derived classes that store actual data (such as ChannelDataOSGArray) may
    return a pointer to the actual data. Other classes (such as those that
    represent procedural data or stat stored in a DB) may return NULL or a copy.
    See the derived class documentation for any special notes.
    \returns NULL if the operation is unsupported, memory is exhausted, or the
    operation fails for any reason. */
    virtual char* asCharPtr();
    /** \overload char* ChannelData::asCharPtr(); */
    virtual const char* asCharPtr() const;

    /** \brief
    \details */
    virtual void getDimensions( unsigned int& x, unsigned int& y, unsigned int& z );

    /** \brief
    \details */
    void setImage( osg::Image* image );
    /** \brief
    \details */
    osg::Image* getImage();
    /** \overload osg::Array* ChannelData::asOSGArray(); */
    const osg::Image* getImage() const;

    /** \brief Return this ChannelData with the specified mask applied.
    \detailt If \maskIn indicates no masking (no zero values), getMaskedChannel()
    may return a pointer to the original ChannelData. Otherwise, a data copy
    would likely be created. The actual implementation is determined by the
    derived class. */
    virtual ChannelDataPtr getMaskedChannel( const ChannelDataPtr maskIn );

    /** \brief Prepare the ChannelData for processing by the DataSet.
    \details Prior to processing ChannelData in the LatticeFX data pipeline,
    the DataSet calls ChannelData::reset() on all attached ChannelData
    instances. This is useful in situations where data processing modifies
    ChannelData (such as a convolution filter or Gaussion blur). Dereived
    ChannelData classes must store a copy of the original data to protect
    against such destruction. reset() allows the ChannelData to refresh the
    working copy of the data from the original. */
    virtual void reset();

protected:
    osg::ref_ptr< osg::Image > _image;
    osg::ref_ptr< osg::Image > _workingImage;
};

typedef boost::shared_ptr< ChannelDataOSGImage > ChannelDataOSGImagePtr;


// lfx
}


// __LATTICEFX_CHANNEL_DATA_OSG_IMAGE_H__
#endif