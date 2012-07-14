#version 120


/** begin light **/

varying vec3 ecVertex;

vec4 fragmentLighting( vec4 baseColor, vec3 normal )
{
    vec3 lightVec = normalize( gl_LightSource[0].position.xyz - ecVertex );
    vec3 eyeVec = normalize( -ecVertex );
    vec3 reflectVec = normalize( -reflect( lightVec, normal ) );

    vec4 amb = gl_LightSource[0].ambient * baseColor;

    vec4 diff = gl_LightSource[0].diffuse * baseColor * max( dot( normal, lightVec ), 0. );
    diff = clamp( diff, 0., 1. );
   
    // Hm. front material shininess is negative for some reason. Hack in "10.0" for now.
    float specExp = 10.; // gl_FrontMaterial.shininess
    vec4 spec = gl_FrontLightProduct[0].specular *
        pow( max( dot( reflectVec, eyeVec ), 0. ), specExp );
    spec = clamp( spec, 0., 1. );

    return( gl_FrontLightModelProduct.sceneColor + amb + diff + spec );
}

/** end light **/


/** begin hardware mask **/

uniform sampler3D hmInput;
uniform vec4 hmParams;
const float hmAlpha = 0.;
const float hmRed = 1.;
const float hmScalar = 2.;
const float hmEqual = 1.;
const float hmLessThan = 2.;
const float hmGreaterThan = 3.;

// Return true if passed, false if failed.
bool hardwareMask( in vec3 tC, in vec4 baseColor )
{
    // hmParams has all mask parameters in a single vec4 uniform:
    //   Element 0: Input source (0=alpha, 1=red, 2=scalar
    //   Element 1: Mask operator (0=OFF, 1=EQ, 2=LT, 3=GT).
    //   Element 2: Operator negate flag (1=negate).
    //   Element 3: Reference value.

    if( hmParams[ 1 ] == 0. ) // Off
        return( true );

    float value;
    if( hmParams[ 0 ] == hmAlpha )
        value = baseColor.a;
    else if( hmParams[ 0 ] == hmRed )
        value = baseColor.r;
    else if( hmParams[ 0 ] == hmScalar )
    {
        // hmInput texture format is GL_ALPHA32F_ARB.
        value = texture3D( hmInput, tC ).a;
    }

    bool result;
    if( hmParams[ 1 ] == hmEqual )
        result = ( value == hmParams[ 3 ] );
    else if( hmParams[ 1 ] == hmLessThan )
        result = ( value < hmParams[ 3 ] );
    else if( hmParams[ 1 ] == hmGreaterThan )
        result = ( value > hmParams[ 3 ] );

    if( hmParams[ 2 ] == 1. ) // Negate
        result = !result;
    return( result );
}

/** end hardware mask **/



uniform sampler3D VolumeTexture;
uniform sampler2D TransferFunction;

varying vec3 Texcoord;
varying vec3 TexcoordUp;
varying vec3 TexcoordRight;
varying vec3 TexcoordBack;
varying vec3 TexcoordDown;
varying vec3 TexcoordLeft;
varying vec3 TexcoordFront;


bool TestInBounds(vec3 sample)
{
   return (sample.x > 0.0 && sample.x < 1.0 && sample.y > 0.0 && sample.y < 1.0 && sample.z > 0.0 && sample.z < 1.0);
}

void main( void )
{
    // Vectex shader always sends (eye oriented) quads. Much of the quad
    // might be outside the volume. Immediately discard if this is the case.
    if( !( TestInBounds( Texcoord ) ) )
        discard;


    // Get volume sample. Format is GL_KUMINANCE, so the same volume value
    // will be stored in fvBaseColor.r, g, and b. fvBaseColor.a will be 1.0.
    vec4 fvBaseColor = texture3D( VolumeTexture, Texcoord );


    if( !hardwareMask( Texcoord, fvBaseColor ) )
        discard;


    // Clip plane test.
    //if( dot( vec4( ecVertex, 1. ), gl_ClipPlane[ 0 ] ) < 0. )
    //    discard;


    vec4 fvUpColor = texture3D( VolumeTexture, TexcoordUp );
    vec4 fvRightColor = texture3D( VolumeTexture, TexcoordRight );
    vec4 fvBackColor = texture3D( VolumeTexture, TexcoordBack );
    vec4 fvDownColor = texture3D( VolumeTexture, TexcoordDown );
    vec4 fvLeftColor = texture3D( VolumeTexture, TexcoordLeft );
    vec4 fvFrontColor = texture3D( VolumeTexture, TexcoordFront );
    vec4 xVec = fvLeftColor - fvRightColor;
    vec4 yVec = fvDownColor - fvUpColor;
    vec4 zVec = fvFrontColor - fvBackColor;

    vec3 ocNormal = vec3( xVec.r, yVec.r, zVec.r );
    vec3 ecNormal = normalize( gl_NormalMatrix * ocNormal );

    //vec4 xfer = texture2D( TransferFunction, vec2(fvBaseColor.r, 0.0) );
    vec4 xfer = vec4( 1., 1., 1., 1. );
    fvBaseColor = fragmentLighting( xfer, ecNormal );
    fvBaseColor.a = 1.;


    gl_FragData[ 0 ] = fvBaseColor;

    // Support for second/glow render target.
    gl_FragData[ 1 ] = vec4( 0., 0., 0., 0. );
}
