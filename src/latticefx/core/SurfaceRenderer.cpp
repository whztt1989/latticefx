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

#include <latticefx/core/SurfaceRenderer.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/BoundUtils.h>
#include <latticefx/core/LogMacros.h>
#include <latticefx/core/JsonSerializer.h>

#include <osg/Geode>
#include <osg/Geometry>



// When sending warp vector arrays, specify these vertex attrib locations.
#define WARP_VERTEX_ATTRIB 13
#define WARP_NORMAL_ATTRIB 14
#define TF_INPUT_ATTRIB 15
// Note: GeForce 9800M supports only 0 .. 15.


namespace lfx
{
namespace core
{


SurfaceRenderer::SurfaceRenderer( const std::string& logName )
    : Renderer( "surf", logName )
{
    // Specify default ChannelData name aliases for the required inputs.
    setInputNameAlias( VERTEX, "positions" );
    setInputNameAlias( NORMAL, "normals" );
    setInputNameAlias( WARP_VERTEX, "warp_vertex" );
    setInputNameAlias( WARP_NORMAL, "warp_normal" );

    // Create and register uniform information, and initial/default values
    // (if we have them -- in some cases, we don't know the actual initial
    // values until scene graph creation).
    UniformInfo info;
    info = UniformInfo( "warpScale", osg::Uniform::FLOAT, "Vertex warp scale value." );
    info._prototype->set( 0.f );
    registerUniform( info );

    info = UniformInfo( "warpEnabled", osg::Uniform::BOOL, "Vertex warp enable toggle." );
    info._prototype->set( false );
    registerUniform( info );
}
SurfaceRenderer::SurfaceRenderer( const SurfaceRenderer& rhs )
    : Renderer( rhs )
{
}
SurfaceRenderer::~SurfaceRenderer()
{
}


osg::Node* SurfaceRenderer::getSceneGraph( const ChannelDataPtr maskIn )
{
    osg::ref_ptr< osg::Geode > geode( new osg::Geode );

    ChannelDataPtr posAlias( getInput( getInputNameAlias( VERTEX ) ) );
    if( posAlias == NULL )
    {
        LFX_WARNING( "getSceneGraph(): Unable to find required POSITION ChannelData." );
        return( NULL );
    }
    ChannelDataPtr posChannel( posAlias->getMaskedChannel( maskIn ) );
    osg::Array* sourceArray( posChannel->asOSGArray() );
    osg::Vec3Array* verts( static_cast< osg::Vec3Array* >( sourceArray ) );

    ChannelDataPtr normAlias( getInput( getInputNameAlias( NORMAL ) ) );
    if( normAlias == NULL )
    {
        LFX_WARNING( "getSceneGraph(): Unable to find required NORMAL ChannelData." );
        return( NULL );
    }
    ChannelDataPtr normChannel( normAlias->getMaskedChannel( maskIn ) );
    sourceArray = normChannel->asOSGArray();
    osg::Vec3Array* norms( static_cast< osg::Vec3Array* >( sourceArray ) );

    osg::Vec3Array* warpV( NULL );
    osg::Vec3Array* warpN( NULL );
    ChannelDataPtr warpVAlias( getInput( getInputNameAlias( WARP_VERTEX ) ) );
    ChannelDataPtr warpNAlias( getInput( getInputNameAlias( WARP_NORMAL ) ) );
    if( ( warpVAlias != NULL ) && ( warpNAlias != NULL ) )
    {
        ChannelDataPtr warpChannel( warpVAlias->getMaskedChannel( maskIn ) );
        sourceArray = warpChannel->asOSGArray();
        warpV = static_cast< osg::Vec3Array* >( sourceArray );

        warpChannel = warpNAlias->getMaskedChannel( maskIn );
        sourceArray = warpChannel->asOSGArray();
        warpN = static_cast< osg::Vec3Array* >( sourceArray );
    }

    osg::ref_ptr< osg::Geometry > geom( new osg::Geometry );
    addColorArray( geom.get() );
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );

    geom->setVertexArray( verts );
    geom->setNormalArray( norms );
    geom->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );

    if( warpV != NULL )
    {
        geom->setVertexAttribArray( WARP_VERTEX_ATTRIB, warpV );
        geom->setVertexAttribBinding( WARP_VERTEX_ATTRIB, osg::Geometry::BIND_PER_VERTEX );

        const osg::BoundingBox bb( getBound( *warpV, osg::Vec3( 0., 0., 0. ) ) );
        osg::BoundingBox bb2( getBound( *verts, osg::Vec3( 0., 0., 0. ) ) );
        bb2.expandBy( bb );
        geom->setInitialBound( bb2 );
    }
    if( warpN != NULL )
    {
        geom->setVertexAttribArray( WARP_NORMAL_ATTRIB, warpN );
        geom->setVertexAttribBinding( WARP_NORMAL_ATTRIB, osg::Geometry::BIND_PER_VERTEX );
    }

    if( getTransferFunction() != NULL )
    {
        // If using a transfer function, we must have a valid input.
        const ChannelDataPtr tfInputByName( getInput( getTransferFunctionInput() ) );
        if( tfInputByName == NULL )
        {
            LFX_WARNING( "getSceneGraph(): Unable to find input \"" +
                         getTransferFunctionInput() + "\"." );
            return( NULL );
        }
        const ChannelDataPtr tfInputChannel( tfInputByName->getMaskedChannel( maskIn ) );
        osg::Array* tfInputArray( tfInputChannel->asOSGArray() );
        // surface shader supports only vec3 tf input. Convert the tf input data to a vec3 array.
        osg::Vec3Array* tfInputArray3( ChannelDataOSGArray::convertToVec3Array( tfInputArray ) );
        geom->setVertexAttribArray( TF_INPUT_ATTRIB, tfInputArray3 );
        geom->setVertexAttribBinding( TF_INPUT_ATTRIB, osg::Geometry::BIND_PER_VERTEX );
    }

    if( _primitiveSetGenerator == NULL )
    {
        _primitiveSetGenerator = PrimitiveSetGeneratorPtr( new SimpleTrianglePrimitiveSetGenerator() );
    }
    ( *_primitiveSetGenerator )( this, geom.get() );

    if( geom->getNumPrimitiveSets() > 0 )
    {
        geode->addDrawable( geom.get() );
    }
    return( geode.release() );
}

osg::StateSet* SurfaceRenderer::getRootState()
{
    osg::ref_ptr< osg::StateSet > stateSet( new osg::StateSet );

    addHardwareFeatureUniforms( stateSet.get() );

    ChannelDataPtr warpVAlias( getInput( getInputNameAlias( WARP_VERTEX ) ) );
    ChannelDataPtr warpNAlias( getInput( getInputNameAlias( WARP_NORMAL ) ) );

    const bool warpEnabled( ( warpVAlias != NULL ) && ( warpNAlias != NULL ) );
    UniformInfo& info( getUniform( "warpEnabled" ) );
    info._prototype->set( warpEnabled );
    stateSet->addUniform( createUniform( getUniform( "warpEnabled" ) ) );

    osg::Program* program( new osg::Program() );
    program->addBindAttribLocation( "warpVertex", WARP_VERTEX_ATTRIB );
    program->addBindAttribLocation( "warpNormal", WARP_NORMAL_ATTRIB );
    program->addBindAttribLocation( "tfInput", TF_INPUT_ATTRIB );

    program->addShader( loadShader( osg::Shader::VERTEX, "lfx-surface.vert" ) );
    program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-surface.frag" ) );
    stateSet->setAttribute( program );

    if( warpEnabled )
    {
        stateSet->addUniform( createUniform( getUniform( "warpScale" ) ) );
    }

    return( stateSet.release() );
}


void SurfaceRenderer::setPrimitiveSetGenerator( PrimitiveSetGeneratorPtr primitiveSetGenerator )
{
    _primitiveSetGenerator = primitiveSetGenerator;
}
PrimitiveSetGeneratorPtr SurfaceRenderer::getPrimitiveSetGenerator()
{
    return( _primitiveSetGenerator );
}


void SurfaceRenderer::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	Renderer::serializeData( json );

	json->insertObj( SurfaceRenderer::getClassName(), true );
	_primitiveSetGenerator->serialize( json );
	json->popParent();
}

bool SurfaceRenderer::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !Renderer::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( SurfaceRenderer::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get SurfaceRenderer data";
		return false;
	}

	ObjBasePtr p = loadObj( json, pfactory, perr );
	if( !p )
	{
		if (perr) *perr = "Json: Failed to load the primitiveSetGenerator";
		json->popParent();
		return false;
	}

	_primitiveSetGenerator = boost::dynamic_pointer_cast<PrimitiveSetGenerator>( p );
	if( !_primitiveSetGenerator )
	{
		if (perr) *perr = "Json: Failed to convert the primitiveSetGenerator";
		json->popParent();
		return false;
	}

	json->popParent();
	return true;
}


PrimitiveSetGenerator::PrimitiveSetGenerator()
{
}
PrimitiveSetGenerator::PrimitiveSetGenerator( const PrimitiveSetGenerator& rhs )
{
}
PrimitiveSetGenerator::~PrimitiveSetGenerator()
{
}

void PrimitiveSetGenerator::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	ObjBase::serializeData( json );

	json->insertObj( PrimitiveSetGenerator::getClassName(), true );
	json->popParent();
}

bool PrimitiveSetGenerator::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !ObjBase::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( PrimitiveSetGenerator::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get PrimitiveSetGenerator data";
		return false;
	}

	json->popParent();
	return true;
}


//virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );

SimpleTrianglePrimitiveSetGenerator::SimpleTrianglePrimitiveSetGenerator()
    : PrimitiveSetGenerator()
{
}
SimpleTrianglePrimitiveSetGenerator::SimpleTrianglePrimitiveSetGenerator( const SimpleTrianglePrimitiveSetGenerator& rhs )
    : PrimitiveSetGenerator( rhs )
{
}
SimpleTrianglePrimitiveSetGenerator::~SimpleTrianglePrimitiveSetGenerator()
{
}

void SimpleTrianglePrimitiveSetGenerator::operator()( const SurfaceRenderer* /* surfaceRenderer */, osg::Geometry* geom )
{
    osg::DrawArrays* da( new osg::DrawArrays( GL_TRIANGLES, 0,
                         geom->getVertexArray()->getNumElements() ) );
    geom->addPrimitiveSet( da );
}

void SimpleTrianglePrimitiveSetGenerator::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	PrimitiveSetGenerator::serializeData( json );

	json->insertObj( SimpleTrianglePrimitiveSetGenerator::getClassName(), true );
	json->popParent();
}

//virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );

////////////////////////////////////////////////////////////////////////////////
void SurfaceRenderer::dumpState( std::ostream &os )
{
	Renderer::dumpState( os );

	dumpStateStart( SurfaceRenderer::getClassName(), os );
	// TODO:
	//_primitiveSetGenerator
	dumpStateEnd( SurfaceRenderer::getClassName(), os );
}

// core
}
// lfx
}
