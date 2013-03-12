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

#include <latticefx/core/DBOperations.h>
#include <latticefx/core/LogMacros.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/ChannelDataOSGImage.h>

#include <boost/foreach.hpp>



namespace lfx
{
namespace core
{


DBLoad::DBLoad( const DBKeyList& keys )
    : Preprocess(),
      LogBase( "lfx.core.db.load" ),
      _keys( keys )
{
    setActionType( Preprocess::ADD_DATA );
}
DBLoad::DBLoad( const DBLoad& rhs )
    : Preprocess( rhs ),
      LogBase( rhs ),
      _keys( rhs._keys )
{
}
DBLoad::~DBLoad()
{
}

ChannelDataPtr DBLoad::operator()()
{
    if( _db == NULL )
    {
        LFX_WARNING( "DBLoad: NULL _db." );
        return( ChannelDataPtr( ( ChannelData* )NULL ) );
    }

    if( _keys.size() > 1 )
    {
        // TBD We should be able to support a Preprocess
        // that can create more than one ChannelData.
        LFX_WARNING( "DBLoad: Currently only 1 key will be loaded." );
    }

    BOOST_FOREACH( DBKey key, _keys )
    {
        osg::ref_ptr< osg::Array > array( _db->loadArray( key ) );
        osg::ref_ptr< osg::Image > image( _db->loadImage( key ) );
        if( array.valid() )
        {
            std::string nameString( key );
            if( !( array->getName().empty() ) )
            {
                nameString = array->getName();
            }
            ChannelDataOSGArrayPtr cdap( new ChannelDataOSGArray( nameString, array.get() ) );
            cdap->setDBKey( key );
            return( cdap );
        }
        else if( image.valid() )
        {
            std::string nameString( key );
            if( !( image->getName().empty() ) )
            {
                nameString = image->getName();
            }
            ChannelDataOSGImagePtr cdip( new ChannelDataOSGImage( nameString, image.get() ) );
            cdip->setDBKey( key );
            return( cdip );
        }
    }
    return( ChannelDataPtr( ( ChannelData* )NULL ) );
}




DBSave::DBSave( DBBasePtr db )
    : RTPOperation( RTPOperation::Filter ),
      LogBase( "lfx.core.db.save" ),
      _db( db )
{
}
DBSave::DBSave( const DBSave& rhs )
    : RTPOperation( rhs ),
      LogBase( rhs ),
      _db( rhs._db )
{
}
DBSave::~DBSave()
{
}

void DBSave::filter( const ChannelDataPtr maskIn )
{
    if( _db == NULL )
    {
        LFX_WARNING( "DBSave: NULL _db." );
        return;
    }

    BOOST_FOREACH( ChannelDataPtr cdp, _inputs )
    {
        osg::Array* sourceArray( cdp->asOSGArray() );
        osg::Image* sourceImage( cdp->asOSGImage() );
        if( sourceArray != NULL )
        {
            _db->storeArray( sourceArray, cdp->getDBKey() );
        }
        else if( sourceImage != NULL )
        {
            _db->storeImage( sourceImage, cdp->getDBKey() );
        }
        else
        {
            LFX_WARNING( "DBSave: Can't save non-OSG data." );
            continue;
        }
    }
}


// core
}
// lfx
}
