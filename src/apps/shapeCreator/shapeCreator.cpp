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

#include <latticefx/core/HierarchyUtils.h>

#include <latticefx/core/DBDisk.h>
#ifdef LFX_USE_CRUNCHSTORE
#  include <latticefx/core/DBCrunchStore.h>
#  include <crunchstore/DataManager.h>
#  include <crunchstore/NullCache.h>
#  include <crunchstore/NullBuffer.h>
#  include <crunchstore/SQLiteStore.h>
#endif

#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osg/ArgumentParser>
#include <osgViewer/Viewer>
#include <osg/io_utils>



const std::string logstr( "lfx.demo" );

using namespace lfx::core;



/** TBD Does not yet support _prune. */
class CubeVolumeBrickData : public VolumeBrickData
{
public:
    CubeVolumeBrickData( const bool prune )
      : VolumeBrickData( prune ),
        _brickRes( 32, 32, 32 ),
        _cubeMin( .2, .2, .2 ),
        _cubeMax( .8, .8, .8 )
    {
        setNumBricks( osg::Vec3s( 8, 8, 8 ) );
    }
    virtual ~CubeVolumeBrickData()
    {}

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const
    {
        const int idx( brickIndex( brickNum ) );
        if( ( idx < 0 ) || ( idx >= _images.size() ) )
            return( NULL );


        const osg::Vec3f brick( brickNum[0], brickNum[1], brickNum[2] );
        const osg::Vec3f invNumBricks( 1. / _numBricks[0], 1. / _numBricks[1], 1. / _numBricks[2] );

        // Compute brick min, max, and extents in normalized 0 <-> 1 volume coordinates.
        const osg::Vec3f brickMin( brick[0] * invNumBricks[0], brick[1] * invNumBricks[1], 
            brick[2] * invNumBricks[2] );
        const osg::Vec3f brickMax( brickMin + invNumBricks );
        const osg::Vec3f extent( brickMax - brickMin );

        if( _prune && pruneTest( brickMin, brickMax ) )
            return( NULL );

        unsigned char* data( new unsigned char[ _brickRes[0] * _brickRes[1] * _brickRes[2] ] );
        unsigned char* ptr( data );
        for( int rIdx=0; rIdx<_brickRes[2]; ++rIdx )
        {
            const float rVal( (float)rIdx/(float)_brickRes[2] * extent[2] + brickMin[2] );
            for( int tIdx=0; tIdx<_brickRes[1]; ++tIdx )
            {
                const float tVal( (float)tIdx/(float)_brickRes[1] * extent[1] + brickMin[1] );
                for( int sIdx=0; sIdx<_brickRes[0]; ++sIdx )
                {
                    const float sVal( (float)sIdx / (float)_brickRes[0] * extent[0] + brickMin[0] );

                    if( ( sVal >= _cubeMin[0] ) && ( sVal <= _cubeMax[0] ) &&
                        ( tVal >= _cubeMin[1] ) && ( tVal <= _cubeMax[1] ) &&
                        ( rVal >= _cubeMin[2] ) && ( rVal <= _cubeMax[2] ) )
                            *ptr++ = 255;
                    else
                            *ptr++ = 0;
                }
            }
        }

        osg::ref_ptr< osg::Image > image( new osg::Image() );
        image->setImage( _brickRes[0], _brickRes[1], _brickRes[2],
            GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
            (unsigned char*) data, osg::Image::USE_NEW_DELETE );
        return( image.release() );
    }

protected:
    bool pruneTest( const osg::Vec3f& bMin, const osg::Vec3f& bMax ) const
    {
        return( ( bMin[0] > _cubeMax[0] ) ||
            ( bMax[0] < _cubeMin[0] ) ||
            ( bMin[1] > _cubeMax[1] ) ||
            ( bMax[1] < _cubeMin[1] ) ||
            ( bMin[2] > _cubeMax[2] ) ||
            ( bMax[2] < _cubeMin[2] )
            );
    }

    osg::Vec3s _brickRes;
    osg::Vec3f _cubeMin, _cubeMax;
};

/** TBD Does not yet support _prune. */
class SphereVolumeBrickData : public VolumeBrickData
{
public:
    SphereVolumeBrickData( const bool prune, const bool soft )
      : VolumeBrickData( prune ),
        _brickRes( 32, 32, 32 ),
        _center( .5, .5, .5 ),
        _minRad( soft ? .1 : .45 ),
        _maxRad( .45 )
    {
        setNumBricks( osg::Vec3s( 8, 8, 8 ) );
    }
    virtual ~SphereVolumeBrickData()
    {}

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const
    {
        const int idx( brickIndex( brickNum ) );
        if( ( idx < 0 ) || ( idx >= _images.size() ) )
            return( NULL );


        const osg::Vec3f brick( brickNum[0], brickNum[1], brickNum[2] );
        const osg::Vec3f invNumBricks( 1. / _numBricks[0], 1. / _numBricks[1], 1. / _numBricks[2] );

        // Compute brick min, max, and extents in normalized 0 <-> 1 volume coordinates.
        const osg::Vec3f brickMin( brick[0] * invNumBricks[0], brick[1] * invNumBricks[1], 
            brick[2] * invNumBricks[2] );
        const osg::Vec3f brickMax( brickMin + invNumBricks );
        const osg::Vec3f extent( brickMax - brickMin );

        if( _prune && pruneTest( brickMin, brickMax ) )
            return( NULL );

        const float transition( _maxRad - _minRad );

        unsigned char* data( new unsigned char[ _brickRes[0] * _brickRes[1] * _brickRes[2] ] );
        unsigned char* ptr( data );
        for( int rIdx=0; rIdx<_brickRes[2]; ++rIdx )
        {
            const float rVal( (float)rIdx/(float)_brickRes[2] * extent[2] + brickMin[2] );
            for( int tIdx=0; tIdx<_brickRes[1]; ++tIdx )
            {
                const float tVal( (float)tIdx/(float)_brickRes[1] * extent[1] + brickMin[1] );
                for( int sIdx=0; sIdx<_brickRes[0]; ++sIdx )
                {
                    const float sVal( (float)sIdx / (float)_brickRes[0] * extent[0] + brickMin[0] );

                    const osg::Vec3f pt( sVal - _center[0], tVal - _center[1], rVal - _center[2] );
                    const float length( pt.length() );

                    if( length <= _minRad )
                        *ptr++ = 255;
                    else if( length > _maxRad )
                        *ptr++ = 0;
                    else
                    {
                        float norm( ( length - _minRad ) / transition );
                        *ptr++ = (unsigned char)( ( 1.f - norm ) * 255.f );
                    }
                }
            }
        }

        osg::ref_ptr< osg::Image > image( new osg::Image() );
        image->setImage( _brickRes[0], _brickRes[1], _brickRes[2],
            GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
            (unsigned char*) data, osg::Image::USE_NEW_DELETE );
        return( image.release() );
    }

protected:
    bool pruneTest( const osg::Vec3f& bMin, const osg::Vec3f& bMax ) const
    {
        const osg::Vec3f p0( bMin[0]-.5f, bMin[1]-.5f, bMin[2]-.5f );
        const osg::Vec3f p1( bMax[0]-.5f, bMin[1]-.5f, bMin[2]-.5f );
        const osg::Vec3f p2( bMin[0]-.5f, bMax[1]-.5f, bMin[2]-.5f );
        const osg::Vec3f p3( bMax[0]-.5f, bMax[1]-.5f, bMin[2]-.5f );
        const osg::Vec3f p4( bMin[0]-.5f, bMin[1]-.5f, bMax[2]-.5f );
        const osg::Vec3f p5( bMax[0]-.5f, bMin[1]-.5f, bMax[2]-.5f );
        const osg::Vec3f p6( bMin[0]-.5f, bMax[1]-.5f, bMax[2]-.5f );
        const osg::Vec3f p7( bMax[0]-.5f, bMax[1]-.5f, bMax[2]-.5f );

        return( ( p0.length() > _maxRad ) &&
            ( p1.length() > _maxRad ) &&
            ( p2.length() > _maxRad ) &&
            ( p3.length() > _maxRad ) &&
            ( p4.length() > _maxRad ) &&
            ( p5.length() > _maxRad ) &&
            ( p6.length() > _maxRad ) &&
            ( p7.length() > _maxRad )
            );
    }

    osg::Vec3s _brickRes;
    osg::Vec3f _center;
    float _minRad, _maxRad;
};

class ConeVolumeBrickData : public VolumeBrickData
{
public:
    ConeVolumeBrickData( const bool prune )
      : VolumeBrickData( prune ),
        _brickRes( 32, 32, 32 ),
        _baseRad( .475 )
    {
        setNumBricks( osg::Vec3s( 4, 4, 4 ) );
    }
    virtual ~ConeVolumeBrickData()
    {}

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const
    {
        const int idx( brickIndex( brickNum ) );
        if( ( idx < 0 ) || ( idx >= _images.size() ) )
            return( NULL );


        const osg::Vec3f brick( brickNum[0], brickNum[1], brickNum[2] );
        const osg::Vec3f invNumBricks( 1. / _numBricks[0], 1. / _numBricks[1], 1. / _numBricks[2] );

        // Compute brick min, max, and extents in normalized 0 <-> 1 volume coordinates.
        const osg::Vec3f brickMin( brick[0] * invNumBricks[0], brick[1] * invNumBricks[1], 
            brick[2] * invNumBricks[2] );
        const osg::Vec3f brickMax( brickMin + invNumBricks );
        const osg::Vec3f extent( brickMax - brickMin );

        if( _prune && pruneTest( brickMin, brickMax ) )
        {
            //std::cout << "Pruning " << brickNum << std::endl;
            return( NULL );
        }

        unsigned char* data( new unsigned char[ _brickRes[0] * _brickRes[1] * _brickRes[2] ] );
        unsigned char* ptr( data );
        for( int rIdx=0; rIdx<_brickRes[2]; ++rIdx )
        {
            const float rVal( (float)rIdx/(float)_brickRes[2] * extent[2] + brickMin[2] );
            const float scaledRad( ( 1.f - rVal ) * _baseRad );
            for( int tIdx=0; tIdx<_brickRes[1]; ++tIdx )
            {
                const float tVal( (float)tIdx/(float)_brickRes[1] * extent[1] + brickMin[1] );
                for( int sIdx=0; sIdx<_brickRes[0]; ++sIdx )
                {
                    const float sVal( (float)sIdx / (float)_brickRes[0] * extent[0] + brickMin[0] );

                    const osg::Vec2f pt( sVal - .5f, tVal - .5f );
                    const float length( pt.length() );

                    if( length <= scaledRad )
                        *ptr++ = 255;
                    else
                        *ptr++ = 0;
                }
            }
        }

        osg::ref_ptr< osg::Image > image( new osg::Image() );
        image->setImage( _brickRes[0], _brickRes[1], _brickRes[2],
            GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
            (unsigned char*) data, osg::Image::USE_NEW_DELETE );
        return( image.release() );
    }

protected:
    bool pruneTest( const osg::Vec3f& bMin, const osg::Vec3f& bMax ) const
    {
        const osg::Vec2f p0( bMin[0]-.5f, bMin[1]-.5f );
        const osg::Vec2f p1( bMax[0]-.5f, bMin[1]-.5f );
        const osg::Vec2f p2( bMin[0]-.5f, bMax[1]-.5f );
        const osg::Vec2f p3( bMax[0]-.5f, bMax[1]-.5f );
        const float scaledRad( ( 1.f - bMin[2] ) * _baseRad );

        return( ( p0.length() > scaledRad ) &&
            ( p1.length() > scaledRad ) &&
            ( p2.length() > scaledRad ) &&
            ( p3.length() > scaledRad ) );
    }

    osg::Vec3s _brickRes;
    float _baseRad;
};


void createDataSet( const std::string& csFile, VolumeBrickData* shapeGen )
{
    SaveHierarchy* saver( new SaveHierarchy( shapeGen, "shapevolume" ) );

    // Configure database to use
#ifdef LFX_USE_CRUNCHSTORE
    if( !( csFile.empty() ) )
    {
        DBCrunchStorePtr cs( DBCrunchStorePtr( new DBCrunchStore() ) );

        crunchstore::DataManagerPtr manager( crunchstore::DataManagerPtr( new crunchstore::DataManager() ) );
        crunchstore::DataAbstractionLayerPtr cache( new crunchstore::NullCache );
        crunchstore::DataAbstractionLayerPtr buffer( new crunchstore::NullBuffer );
        manager->SetCache( cache );
        manager->SetBuffer( buffer );
        crunchstore::SQLiteStorePtr sqstore( new crunchstore::SQLiteStore );
        sqstore->SetStorePath( csFile );
        manager->AttachStore( sqstore, crunchstore::Store::BACKINGSTORE_ROLE );
        try {
            cs->setDataManager( manager );
        }
        catch( std::exception exc ) {
            LFX_FATAL_STATIC( logstr, std::string(exc.what()) );
            LFX_FATAL_STATIC( logstr, "Unable to set DataManager." );
            exit( 1 );
        }

        saver->save( (DBBasePtr)cs );
    }
#endif
    if( csFile.empty() )
    {
        DBDiskPtr disk( DBDiskPtr( new DBDisk() ) );
        saver->save( (DBBasePtr)disk );
    }
}


int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );
    Log::instance()->setPriority( Log::PrioInfo, "lfx.core.hier" );

    LFX_CRITICAL_STATIC( logstr, "With no command line args, write image data as files using DBDisk." );
    LFX_CRITICAL_STATIC( logstr, "-cs <dbFile> Write volume image data files using DBCrunchStore." );
    LFX_CRITICAL_STATIC( logstr, "-cube Generate a cube data set. This is the default if no other shape is specified." );
    LFX_CRITICAL_STATIC( logstr, "-cone Generate a cone data set." );
    LFX_CRITICAL_STATIC( logstr, "-sphere Generate a sphere data set." );
    LFX_CRITICAL_STATIC( logstr, "-ssphere Generate a soft sphere data set." );
    LFX_CRITICAL_STATIC( logstr, "-prune Do not generate empty subvolumes.." );

    osg::ArgumentParser arguments( &argc, argv );

    const bool prune( arguments.find( "-prune" ) > 0 );

    std::string csFile;
#ifdef LFX_USE_CRUNCHSTORE
    arguments.read( "-cs", csFile );
#endif

    bool softSphere( arguments.find( "-ssphere" ) > 0 );

    VolumeBrickData* shapeGen( NULL );
    if( ( arguments.find( "-sphere" ) > 0 ) || softSphere )
        shapeGen = new SphereVolumeBrickData( prune, softSphere );
    else if( arguments.find( "-cone" ) > 0 )
        shapeGen = new ConeVolumeBrickData( prune );
    else
        shapeGen = new CubeVolumeBrickData( prune );

    createDataSet( csFile, shapeGen );

    return( 0 );
}
