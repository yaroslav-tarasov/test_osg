// grass.vert
#version 150 compatibility
#extension GL_ARB_gpu_shader5 : enable


#include "scene_params.hlsl" 
#include "utils.hlsl" 
#include "spot_lights.hlsl"

#pragma alpha_to_coverage on

out mat4 viewworld_matrix;

out block
{
    vec2 texcoord;
    vec3 normal;
    vec3 viewpos;
    vec4 shadow_view;
    vec4 lightmap_coord;
} v_out;

uniform int			cc;
uniform float		spacing;
uniform float		osg_SimulationTime;
uniform float		osg_FrameTime;
uniform sampler2D	texHeightMap;
uniform mat4		orthoInverseViewMatrix;
uniform bool		bLock;
uniform float		heightAdjust;
uniform float		windFactor;
uniform float		grassStretch;


uniform sampler2D   texMap;

float IntNoise1( int x )
{
	int iSeed = 1000;
	
	x = iSeed+x*57;

	int n = x;
	n =	(n<<13) ^ n;
    return ( 1.0 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);    
}

void simple2()
{
	// x : left right
	// y : back forward
	// z : up down

	float count = cc;							// this should be a uniform 2*2 instances
	/* const */ float r = gl_InstanceID / count;
	float len = count*spacing; 					// length of side of grid

	// calc placement in ranges 0-1 for both x and y
	vec2 pos2;
	pos2.x = fract( r );
	pos2.y = floor( r ) / count;
	
	// add random displacement
	pos2.x = mod( pos2.x+IntNoise1( gl_InstanceID ), 1.0 );
	pos2.y = mod( pos2.y+IntNoise1( gl_InstanceID ), 1.0 );



	
	// do displacement here (this is what lets the grass scroll when the camera moves)
	vec4 thisCamPos = gl_ModelViewMatrixInverse*vec4(0,0,0,1);
	float xNorm = -mod( thisCamPos.x, len )/len;

	float yNorm = -mod( thisCamPos.y, len )/len;
	pos2.x = mod( pos2.x+xNorm, 1.0 );
	pos2.y = mod( pos2.y+yNorm, 1.0 );

	//if( length(pos2) > 0.1 )
	if (texture2D(texMap,vec2( pos2.y, pos2.x)).r==0)
	{
		gl_Position = vec4(1.1,1.1,1.1,1.0);
		return;
	}
	
	pos2 -= vec2(0.5);

	// offsets for grass so it's always under the camera
	vec4 cam_world_pos;
	if ( bLock )
	{
		// draw grass at this view cam position
		cam_world_pos = gl_ModelViewMatrixInverse*vec4(0,0,0,1);
	}
	else
	{
		// draw grass at othro cam position
		cam_world_pos = orthoInverseViewMatrix*vec4(0,0,0,1);
	}

	float xxx = cam_world_pos.x;
	float yyy = cam_world_pos.y;

	vec4 colorHeight = texture2D(texHeightMap,vec2( (pos2.y+0.5), 1.0-(pos2.x+0.5)));
	float zzz = colorHeight.r;

	// move top part of vertices (wind simulations and deforming of quad)
	if ( gl_Vertex.z!=0.0 )
	{
		// wind simulation
		xxx += cos( osg_FrameTime*windFactor+gl_InstanceID )*grassStretch;
		yyy += sin( osg_FrameTime*windFactor+gl_InstanceID )*grassStretch;
		
		// random height adjustment
		zzz = zzz - IntNoise1( gl_InstanceID )*0.3;
		
		// height multiplier
		zzz = zzz+heightAdjust;
	}
	
	vec4 pos = vec4( pos2.x*len+xxx, pos2.y*len+yyy, zzz, 1.0 );
	vec4 newX = vec4( 1.0, 0.0, 0.0, 0.0 );
	vec4 newY = vec4( 0.0, 1.0, 0.0, 0.0 );
	vec4 newZ = vec4( 0.0, 0.0, 1.0, 0.0 );
	mat4 mV = mat4( newX, newY, newZ, pos );

	// orientation (billboard)
	vec3 X = vec3( gl_ModelViewMatrix[0][0], gl_ModelViewMatrix[1][0], gl_ModelViewMatrix[2][0] );
    vec3 Y = vec3( gl_ModelViewMatrix[0][1], gl_ModelViewMatrix[1][1], gl_ModelViewMatrix[2][1] );
    vec3 Z = vec3( gl_ModelViewMatrix[0][2], gl_ModelViewMatrix[1][2], gl_ModelViewMatrix[2][2] );
		
	vec3 xx = gl_Vertex.x*X;
	vec3 yy = gl_Vertex.y*Y;
	xx.z = 0.0;
	yy.z = 0.0;
	vec3 vertex = xx + yy + vec3( 0.0, 0.0, gl_Vertex.z );

/////////////////
// FIXME not tested
	viewworld_matrix = inverse(gl_ModelViewMatrix);

	//vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
    vec4 vertexInEye = gl_ModelViewMatrix * mV * vec4( vertex, 1.0 );

    v_out.normal = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
    //v_out.normal    = normal;
    v_out.viewpos   = vertexInEye.xyz;
    v_out.texcoord  = gl_TexCoord[0].st;

	
///////////////

    gl_Position = gl_ModelViewProjectionMatrix * mV * vec4( vertex, 1.0 );
}

void simple()
{
	 gl_Position = gl_ModelViewProjectionMatrix * vec4( gl_Vertex.xyz, 1.0 );
}

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	simple();
}

	