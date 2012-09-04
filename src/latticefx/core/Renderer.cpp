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

#include <latticefx/core/Renderer.h>
#include <latticefx/core/LogMacros.h>

#include <osg/Image>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgDB/FileUtils>

#include <boost/foreach.hpp>
#include <Poco/Platform.h>


namespace lfx {
namespace core {


Renderer::Renderer( const std::string logNameSuffix )
  : OperationBase( OperationBase::RendererType ),
    LogBase( "lfx.core." + logNameSuffix ),
    _baseUnit( 4 ),
    _unitAssignmentCounter( 8 ),
    _tfRange( 0., 1. ),
    _tfDest( TF_ALPHA ),
    _tfDestMask( 0., 0., 0., 1. ),
    _hmSource( HM_SOURCE_ALPHA ),
    _hmReference( 0.f ),
    _hmOperator( HM_OP_OFF )
{
    // Create and register uniform information, and initial/default values
    // (if we have them -- in some cases, we don't know the actual initial
    // values until scene graph creation).
    UniformInfo info;
    info = UniformInfo( "tf1d", osg::Uniform::SAMPLER_1D, "1D transfer function sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "tf2d", osg::Uniform::SAMPLER_2D, "2D transfer function sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "tf3d", osg::Uniform::SAMPLER_3D, "3D transfer function sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "tfRange", osg::Uniform::FLOAT_VEC2, "Transfer function input range (x=min, y=max)." );
    info._prototype->set( _tfRange );
    registerUniform( info );

    info = UniformInfo( "tfDimension", osg::Uniform::INT, "Transfer function dimension: 0 (off), 1, 2, or 3.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "tfDest", osg::Uniform::FLOAT_VEC4, "Transfer function destination as rgba mask." );
    info._prototype->set( _tfDestMask );
    registerUniform( info );

    info = UniformInfo( "hmParams", osg::Uniform::FLOAT_VEC4, "Hardware mask parameters.", UniformInfo::PRIVATE );
    registerUniform( info );
}
Renderer::Renderer( const Renderer& rhs )
  : OperationBase( rhs ),
    LogBase( rhs ),
    _uniformInfo( rhs._uniformInfo ),
    _baseUnit( rhs._baseUnit ),
    _unitAssignmentCounter( rhs._unitAssignmentCounter ),
    _unitAssignmentMap( rhs._unitAssignmentMap ),
    _tfImage( rhs._tfImage ),
    _tfInputName( rhs._tfInputName ),
    _tfDest( rhs._tfDest ),
    _tfDestMask( rhs._tfDestMask ),
    _hmSource( rhs._hmSource ),
    _hmInputName( rhs._hmInputName ),
    _hmReference( rhs._hmReference ),
    _hmOperator( rhs._hmOperator )
{
}
Renderer::~Renderer()
{
}



Renderer::UniformInfo::UniformInfo( const std::string& name, const osg::Uniform::Type& type, const std::string& description, const AccessType access )
  : _description( description ),
    _access( access )
{
    _prototype = new osg::Uniform( type, name );
}
Renderer::UniformInfo::UniformInfo( const UniformInfo& rhs )
  : _description( rhs._description ),
    _access( rhs._access ),
    _prototype( rhs._prototype )
{
}
Renderer::UniformInfo::~UniformInfo()
{
}

void Renderer::registerUniform( const UniformInfo& info )
{
    _uniformInfo.push_back( info );
}
const Renderer::UniformInfoVector& Renderer::getUniforms() const
{
    return( _uniformInfo );
}
Renderer::UniformInfo& Renderer::getUniform( const std::string& name )
{
    BOOST_FOREACH( UniformInfo& info, _uniformInfo )
    {
        if( info._prototype->getName() == name )
            return( info );
    }
    LFX_WARNING( "getUniform(name): Can't find uniform \"" + name + "\"." );
    return( _uniformInfo[ 0 ] );
}
const Renderer::UniformInfo& Renderer::getUniform( const std::string& name ) const
{
    Renderer* nonConstThis( const_cast< Renderer* >( this ) );
    return( nonConstThis->getUniform( name ) );
}
osg::Uniform* Renderer::createUniform( const UniformInfo& info )
{
    return( new osg::Uniform( *( info._prototype ) ) );
}

std::string Renderer::uniformTypeAsString( const osg::Uniform::Type type )
{
    switch( type )
    {
    case osg::Uniform::FLOAT_MAT4:
        return( "FLOAT_MAT4" );
        break;
    case osg::Uniform::FLOAT_VEC2:
        return( "FLOAT_VEC2" );
        break;
    case osg::Uniform::FLOAT_VEC3:
        return( "FLOAT_VEC3" );
        break;
    case osg::Uniform::FLOAT_VEC4:
        return( "FLOAT_VEC4" );
        break;
    case osg::Uniform::FLOAT:
        return( "FLOAT" );
        break;
    case osg::Uniform::SAMPLER_1D:
        return( "SAMPLER_1D" );
        break;
    case osg::Uniform::SAMPLER_2D:
        return( "SAMPLER_2D" );
        break;
    case osg::Uniform::SAMPLER_3D:
        return( "SAMPLER_3D" );
        break;
    case osg::Uniform::INT:
        return( "INT" );
        break;
    case osg::Uniform::BOOL:
        return( "BOOL" );
        break;
    default:
        return( "Unsupported uniform type." );
        break;
    }
}



void Renderer::setTextureBaseUnit( const int baseUnit )
{
    _baseUnit = baseUnit;
    // Setting the base unit clears all previous texture unit assignments.
    resetTextureUnitAssignments();
}
int Renderer::getTextureBaseUnit() const
{
    return( _baseUnit );
}
int Renderer::getOrAssignTextureUnit( const std::string& key )
{
    UnitAssignmentMap::const_iterator it( _unitAssignmentMap.find( key ) );
    if( it == _unitAssignmentMap.end() )
    {
        // Adding the next available texture unit and store that assignment.
        _unitAssignmentMap[ key ] = _unitAssignmentCounter;
        return( _unitAssignmentCounter++ );
    }
    else
    {
        // Return the previously assigned unit.
        return( it->second );
    }
}
void Renderer::resetTextureUnitAssignments()
{
    // Clear all previous assignments.
    _unitAssignmentMap.clear();
    // The next assignment will start at the base unit and increment upwards.
    _unitAssignmentCounter = _baseUnit;
}


void Renderer::setTransferFunction( osg::Image* image )
{
    _tfImage = image;
}
osg::Image* Renderer::getTransferFunction()
{
    return( _tfImage.get() );
}
const osg::Image* Renderer::getTransferFunction() const
{
    return( _tfImage.get() );
}

void Renderer::setTransferFunctionInput( const std::string& inputName )
{
    _tfInputName = inputName;
}
const std::string& Renderer::getTransferFunctionInput() const
{
    return( _tfInputName );
}

void Renderer::setTransferFunctionInputRange( const osg::Vec2& range )
{
    _tfRange = range;
}
const osg::Vec2& Renderer::getTransferFunctionInputRange() const
{
    return( _tfRange );
}

void Renderer::setTransferFunctionDestination( const Renderer::TransferFunctionDestination dest )
{
    _tfDest = dest;

    switch( _tfDest )
    {
    case TF_RGB:
        _tfDestMask.set( 1., 1., 1., 0. );
        break;
    case TF_RGBA:
        _tfDestMask.set( 1., 1., 1., 1. );
        break;
    case TF_ALPHA:
        _tfDestMask.set( 0., 0., 0., 1. );
        break;
    }
}
Renderer::TransferFunctionDestination Renderer::getTransferFunctionDestination() const
{
    return( _tfDest );
}


void Renderer::setHardwareMaskInputSource( const HardwareMaskInputSource source )
{
    _hmSource = source;
}
const Renderer::HardwareMaskInputSource Renderer::getHardwareMaskInputSource() const
{
    return( _hmSource );
}

void Renderer::setHardwareMaskInput( const std::string& inputName )
{
    _hmInputName = inputName;
}
const std::string& Renderer::getHardwareMaskInput() const
{
    return( _hmInputName );
}

void Renderer::setHardwareMaskReference( const float reference )
{
    _hmReference = reference;
}
float Renderer::getHardwareMaskReference() const
{
    return( _hmReference );
}

void Renderer::setHardwareMaskOperator( const unsigned int& maskOp )
{
    _hmOperator = maskOp;
}
unsigned int Renderer::getHardwareMaskOperator() const
{
    return( _hmOperator );
}



void Renderer::addHardwareFeatureUniforms( osg::StateSet* stateSet )
{
    int tfDimension;
    osg::Image* function( getTransferFunction() );
    if( function == NULL )
    {
        // No transfer function specified.
        tfDimension = 0;
    }
    else if( ( function->t() == 1 ) && ( function->r() == 1 ) )
    {
        // 1D transfer function.
        tfDimension = 1;

        osg::Texture1D* tf1dTex( new osg::Texture1D( function ) );
        tf1dTex->setResizeNonPowerOfTwoHint( false );
        tf1dTex->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
        tf1dTex->setName( "donotpage" );
        stateSet->setTextureAttributeAndModes( getOrAssignTextureUnit( "tf1d" ), tf1dTex,
            osg::StateAttribute::OFF );
    }
    else if( function->r() == 1 )
    {
        // 2D transfer function.
        tfDimension = 2;

        osg::Texture2D* tf2dTex( new osg::Texture2D( function ) );
        tf2dTex->setResizeNonPowerOfTwoHint( false );
        tf2dTex->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
        tf2dTex->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
        tf2dTex->setName( "donotpage" );
        stateSet->setTextureAttributeAndModes( getOrAssignTextureUnit( "tf2d" ), tf2dTex,
            osg::StateAttribute::OFF );
    }
    else
    {
        // 3D transfer function.
        tfDimension = 3;

        osg::Texture3D* tf3dTex( new osg::Texture3D( function ) );
        tf3dTex->setResizeNonPowerOfTwoHint( false );
        tf3dTex->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
        tf3dTex->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
        tf3dTex->setWrap( osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE );
        tf3dTex->setName( "donotpage" );
        stateSet->setTextureAttributeAndModes( getOrAssignTextureUnit( "tf3d" ), tf3dTex,
            osg::StateAttribute::OFF );
    }

    // Assign transfer function texture sampler uniform, based on texture dimension.
    // Typically, only need to set one of these, as only one will ever be accessed
    // by the shader.
    //
    // OSX NVIDIA device driver should be able to determine that only one of
    // these uniforms can possibly be in use, but is failing to make that observation,
    // so we must set all three uniforms to unique values. We do this by disabling
    // the conditionals so that each case gets executed regardless of texture dimension.
#if( POCO_OS != POCO_OS_MAC_OS_X )
    if( tfDimension == 1 )
#endif
    {
        UniformInfo& info( getUniform( "tf1d" ) );
        info._prototype->set( getOrAssignTextureUnit( "tf1d" ) );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }
#if( POCO_OS != POCO_OS_MAC_OS_X )
    else if( tfDimension == 2 )
#endif
    {
        UniformInfo& info( getUniform( "tf2d" ) );
        info._prototype->set( getOrAssignTextureUnit( "tf2d" ) );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }
#if( POCO_OS != POCO_OS_MAC_OS_X )
    else if( tfDimension == 3 )
#endif
    {
        UniformInfo& info( getUniform( "tf3d" ) );
        info._prototype->set( getOrAssignTextureUnit( "tf3d" ) );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }


    {
        UniformInfo& info( getUniform( "tfRange" ) );
        info._prototype->set( _tfRange );
        stateSet->addUniform( createUniform( info ) );
    }

    {
        UniformInfo& info( getUniform( "tfDimension" ) );
        info._prototype->set( tfDimension );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }

    {
        UniformInfo& info( getUniform( "tfDest" ) );
        info._prototype->set( _tfDestMask );
        stateSet->addUniform( createUniform( info ) );
    }


    // Cram all mask parameters into a single vec4 uniform:
    //   Element 0: Input source (0=alpha, 1=red, 2=scalar, 1000=no mask)
    //   Element 1: Mask operator (0=EQ, -1=LT, 1=GT).
    //   Element 2: Operator negate flag (1=no negate, -1=negate).
    //   Element 3: Reference value.
    osg::Vec4 maskParams( 1000., 0., 0., _hmReference );
    if( _hmOperator != HM_OP_OFF )
    {
        if( _hmSource == HM_SOURCE_ALPHA )
            maskParams[ 0 ] = 0.f;
        else if( _hmSource == HM_SOURCE_RED )
            maskParams[ 0 ] = 1.f;
        else if( _hmSource == HM_SOURCE_SCALAR )
            maskParams[ 0 ] = 2.f;

        if( ( _hmOperator & HM_OP_EQ ) != 0 )
            maskParams[ 1 ] = 0.f;
        else if( ( _hmOperator & HM_OP_LT ) != 0 )
            maskParams[ 1 ] = -1.f;
        else if( ( _hmOperator & HM_OP_GT ) != 0 )
            maskParams[ 1 ] = 1.f;

        maskParams[ 2 ] = ( ( _hmOperator & HM_OP_NOT ) != 0 ) ? -1.f : 1.;
    }

    {
        UniformInfo& info( getUniform( "hmParams" ) );
        info._prototype->set( maskParams );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }
}


osg::Shader* Renderer::loadShader( const osg::Shader::Type type, const std::string& fileName )
{
    const std::string fullName( osgDB::findDataFile( fileName ) );
    if( fullName.empty() )
    {
        LFX_WARNING( "loadShader(): Can't find file \"" + fileName + "\"." );
        return( NULL );
    }

    osg::ref_ptr< osg::Shader > shader( new osg::Shader( type ) );
    shader->setName( fileName );
    if( !( shader->loadShaderSourceFromFile( fullName ) ) )
    {
        LFX_WARNING( "loadShader(): \"" + fullName + "\":" );
        LFX_WARNING( "\tloadShaderSourceFromFile() returned false." );
        return( NULL );
    }
    return( shader.release() );
}


// core
}
// lfx
}
