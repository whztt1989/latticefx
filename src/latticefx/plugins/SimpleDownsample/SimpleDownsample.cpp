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

#include <latticefx/core/PluginManager.h>
#include <latticefx/core/LogMacros.h>
#include <Poco/ClassLibrary.h>

#include <latticefx/core/Preprocess.h>
#include <latticefx/core/ChannelDataOSGImage.h>

#include <osgDB/WriteFile>
#include <osg/Image>

#include <cstring>


using namespace lfx::core;


osg::Image* downsample( osg::Image* srcImage )
{
    osg::ref_ptr< osg::Image > image( new osg::Image );

    // Compute half sizes and round up if source dimension is odd.
    const int s( srcImage->s() / 2 + ( ( ( srcImage->s() & 0x1 ) == 1 ) ? 1 : 0 ) );
    const int t( srcImage->t() / 2 + ( ( ( srcImage->t() & 0x1 ) == 1 ) ? 1 : 0 ) );
    const int r( srcImage->r() / 2 + ( ( ( srcImage->r() & 0x1 ) == 1 ) ? 1 : 0 ) );

    image->allocateImage( s, t, r, srcImage->getPixelFormat(),
                          srcImage->getDataType(), srcImage->getPacking() );

    unsigned int pixelSize( image->getPixelSizeInBits() / 8 );
    if( pixelSize == 0 )
    {
        pixelSize = 1;
    }

    unsigned int sIdx, tIdx, rIdx;
    for( rIdx = 0; rIdx < r; rIdx++ )
    {
        const unsigned int srcR( ( float )rIdx / ( float )r * srcImage->r() );
        for( tIdx = 0; tIdx < t; tIdx++ )
        {
            const unsigned int srcT( ( float )tIdx / ( float )t * srcImage->t() );
            for( sIdx = 0; sIdx < s; sIdx++ )
            {
                const unsigned int srcS( ( float )sIdx / ( float )s * srcImage->s() );
                std::memcpy( image->data( sIdx, tIdx, rIdx ), srcImage->data( srcS, srcT, srcR ), pixelSize );
            }
        }
    }

    return( image.release() );
}


class Downsample : public Preprocess
{
public:
    Downsample()
        : Preprocess()
    {}
    Downsample( const Downsample& rhs )
        : Preprocess( rhs )
    {}
    virtual ~Downsample()
    {}

    virtual OperationBase* create()
    {
        return( new Downsample );
    }

    virtual ChannelDataPtr operator()()
    {
        ChannelDataPtr cdp;
        if( !( _inputs.empty() ) )
        {
            cdp = _inputs[ 0 ];
        }
        if( cdp == NULL )
        {
            LFX_WARNING_STATIC( "lfx.plugin.SimpleDownsample", "ReduceLOD: NULL input." );
            return( ChannelDataPtr( ( ChannelData* )NULL ) );
        }
        ChannelDataOSGImage* dataImage( static_cast <
                                        ChannelDataOSGImage* >( cdp.get() ) );
        osg::Image* srcImage( dataImage->getImage() );
        if( srcImage == NULL )
        {
            LFX_WARNING_STATIC( "lfx.plugin.SimpleDownsample", "ReduceLOD: NULL source image." );
            return( ChannelDataPtr( ( ChannelData* )NULL ) );
        }

        osg::Image* newImage( downsample( srcImage ) );

        // Debug.
        osgDB::writeImageFile( *newImage, "out.ive" );

        ChannelDataOSGImagePtr newData(
            new ChannelDataOSGImage( dataImage->getName(), newImage ) );
        return( newData );
    }

protected:
};

// Register the MyPreprocess operation with the PluginManager
// This declares a static object initialized when the plugin is loaded.
REGISTER_OPERATION(
    new Downsample(),
    Downsample,
    "Preprocess",
    "Reduce stp dimensions by 1/2, creating a volume data set 1/8th original size."
)


// Poco ClassLibrary manifest registration. Add a POCO_EXPORT_CLASS
// for each class (operation) in the plugin.
POCO_BEGIN_MANIFEST( OperationBase )
POCO_EXPORT_CLASS( Downsample )
POCO_END_MANIFEST
