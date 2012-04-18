#version 120

#extension GL_ARB_draw_instanced : require


uniform sampler3D texPos;
uniform sampler3D texDir;
uniform vec3 texDim;


vec3 generateTexCoord( const in int iid )
{
    int s = int( texDim.x );
    int t = int( texDim.y );
    int p = int( texDim.z );
    int p1 = iid / (s*t);     // p1 = p coord in range (0 to (p-1) )
    int tiid = iid - ( p1 * (s*t) );
    int t1 = tiid / s;           // likewise, t1 = t coord...
    int s1 = tiid - ( t1 * s );  // ...and s1 = s coord.
    // Normalize (s1,t1,p1) to range (0 to (1-epsilon)).
    return( vec3( s1 / texDim.x, t1 / texDim.y, p1 / texDim.z ) );
}

mat3 makeOrientMat( const in vec3 dir )
{
    // Compute a vector at a right angle to the direction.
    // First try projection direction into xy rotated -90 degrees.
    // If that gives us a very short vector,
    // then project into yz instead, rotated -90 degrees.
    vec3 c = vec3( dir.y, -dir.x, 0.0 );
    if( dot( c, c ) < 0.1 )
        c = vec3( 0.0, dir.z, -dir.y );
        
    // Appears to be a bug in normalize when z==0
    //normalize( c.xyz );
    float l = length( c );
    c /= l;

    vec3 up = normalize( cross( dir, c ) );

    // Orientation uses the cross product vector as x,
    // the up vector as y, and the direction vector as z.
    return( mat3( c, up, dir ) );
}


void main()
{
    // Generate stp texture coords from the instance ID.
    vec3 tC = generateTexCoord( gl_InstanceIDARB );

    // Sample (look up) xyz position
    vec4 pos = texture3D( texPos, tC );

    // Sample (look up) direction vector and obtain the scale factor
    vec4 dir = texture3D( texDir, tC );
    float scale = length( dir.xyz );

    // Create an orientation matrix. Orient/transform the arrow.
    mat3 orientMat = makeOrientMat( normalize( dir.xyz ) );
    vec3 oVec = orientMat * (scale * gl_Vertex.xyz);
    vec4 hoVec = vec4( oVec + pos.xyz, 1.0 );
    gl_Position = gl_ModelViewProjectionMatrix * hoVec;

    gl_FrontColor = vec4( tC, 1. );
}
