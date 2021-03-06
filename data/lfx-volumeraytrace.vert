#version 120


uniform vec3 volumeResolution;
uniform vec3 volumeDims;
uniform vec3 volumeCenter;

uniform mat3 osg_NormalMatrixInverse;

// This variable comes from osgWorks, osgwTools library,
//   MultiCameraProjectionMatrix class, which allows the
//   OSG near & far computation to span two Cameras.
uniform mat4 osgw_ProjectionMatrix;

flat varying vec3 tcEye;
flat varying vec3 edgeEps, maxEps, invRes;
flat varying float ecVolumeSize;


vec4 eyeToTexCoords( in vec4 ec, in vec4 ocCenter, in vec4 ocDims )
{
    vec4 oc = gl_ModelViewMatrixInverse * ec;
    return( ( oc - ocCenter ) / ocDims + vec4( .5 ) );
}

vec4 scaleBiasTexCoords( in vec4 tc )
{
    // Incoming (0,1) texture coordinates need to be scaled and biased
    // into the range (eps,1-eps), where epsilon is 1/2 of the inverse
    // volume resolution. (This forces the texture lookup to never extend
    // beyond the texel *center*, necessary for eliminating seams between
    // texture bricks.)
    return( vec4( tc.stp * ( 1. - invRes ) + edgeEps, 1. ) );
}

void main( void )
{
    vec4 ocCenter = vec4( volumeCenter, 1. );
    vec4 ocDims = vec4( volumeDims, 1. );

    vec4 ecDims = gl_ModelViewMatrix * ocDims;
    ecVolumeSize = length( ecDims.xyz );

    // Required for scaleBiasTexCoords() to work properly.
    invRes = 1. / volumeResolution;
    edgeEps = .5 / volumeResolution;

    maxEps = vec3( 1. - edgeEps );

    // Compute eye position in texture coordinate space.
    const vec4 ecEye = vec4( 0., 0., 0., 1. );
    tcEye = scaleBiasTexCoords( eyeToTexCoords( ecEye, ocCenter, ocDims ) ).xyz;

    gl_TexCoord[0] = scaleBiasTexCoords( gl_MultiTexCoord0 );

    // Do NOT use ftransform() -- Must use the osgWorks
    // MultiCameraProjectionMatrix for correct z testing.
    gl_Position = osgw_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
}
