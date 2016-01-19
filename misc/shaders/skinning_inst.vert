/*  -*-c++-*- 
 *  Copyright (C) 2008 Cedric Pinson <cedric.pinson@plopbyte.net>
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#extension GL_ARB_gpu_shader5 : enable 
#extension GL_ARB_texture_rectangle : enable

in vec4 boneWeight0;
in vec4 boneWeight1;
in vec4 boneWeight2;
in vec4 boneWeight3;

uniform float osg_SimulationTime;
uniform int   nbBonesPerVertex;

vec4    position;
vec3    normal;

uniform sampler2DRect          animationTex;
uniform sampler2DRect instanceMatrixTexture;

mat4  getBoneMatrix ( int idx )
{
		int  animID = idx * 150 + int(osg_SimulationTime * (gl_InstanceID % 600 + 200) ) % 150;
		vec2 animCoord = vec2((animID % 4096) * 4.0, animID / 4096);
		return mat4(texture2DRect(animationTex, animCoord),
					texture2DRect(animationTex, animCoord + vec2(1.0, 0.0)),
					texture2DRect(animationTex, animCoord + vec2(2.0, 0.0)),
					texture2DRect(animationTex, animCoord + vec2(3.0, 0.0)));
}

// accumulate position and normal in global scope
void computeAcummulatedNormalAndPosition(vec4 boneWeight)
{
    int matrixIndex;
    float matrixWeight;
    for (int i = 0; i < 2; i++)
    {
        matrixIndex =  int(boneWeight[0]);
        matrixWeight = boneWeight[1];
        mat4 matrix = getBoneMatrix(matrixIndex);
        // correct for normal if no scale in bone
        mat3 matrixNormal = mat3(matrix);
        position += matrixWeight * (matrix *     gl_Vertex ); 
        normal   += matrixWeight * (matrixNormal * gl_Normal );    

        boneWeight = boneWeight.zwxy;
    }
}

uniform mat4                lightmap_matrix;


#define SAVE_LIGHTMAP_VARYINGS_VP(out_vert, view_pos_4)           \
out_vert.lightmap_coord.xyw = (lightmap_matrix * view_pos_4).xyw; \
out_vert.lightmap_coord.z = (viewworld_matrix * view_pos_4).z;

void  shadow_vs_main (vec4 viewpos); 

attribute vec3 tangent;
attribute vec3 binormal;
out mat4 viewworld_matrix;

out block
{
    vec2 texcoord;
    vec3 normal;
    vec3 vnormal;
    vec3 tangent;
    vec3 binormal;
    vec3 viewpos;
    vec4 shadow_view;
    vec4 lightmap_coord;
} v_out;

void main( void )
{
    
	vec2 instanceCoord = vec2((gl_InstanceID % 4096) * 4.0, gl_InstanceID / 4096);
	mat4 instanceModelMatrix = mat4(texture2DRect(instanceMatrixTexture, instanceCoord),
									texture2DRect(instanceMatrixTexture, instanceCoord + vec2(1.0, 0.0)),
									texture2DRect(instanceMatrixTexture, instanceCoord + vec2(2.0, 0.0)),
									texture2DRect(instanceMatrixTexture, instanceCoord + vec2(3.0, 0.0)));
	
	mat3 normalMatrix = mat3(instanceModelMatrix[0][0], instanceModelMatrix[0][1], instanceModelMatrix[0][2],
							 instanceModelMatrix[1][0], instanceModelMatrix[1][1], instanceModelMatrix[1][2],
							 instanceModelMatrix[2][0], instanceModelMatrix[2][1], instanceModelMatrix[2][2]);

    position = vec4(0.0,0.0,0.0,0.0);
    normal   = vec3(0.0,0.0,0.0);

    // there is 2 bone data per attributes
    if (nbBonesPerVertex > 0)
        computeAcummulatedNormalAndPosition(boneWeight0);
    if (nbBonesPerVertex > 2)
        computeAcummulatedNormalAndPosition(boneWeight1);
    if (nbBonesPerVertex > 4)
        computeAcummulatedNormalAndPosition(boneWeight2);
    if (nbBonesPerVertex > 6)
        computeAcummulatedNormalAndPosition(boneWeight3);

	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * instanceModelMatrix * position;
    normal = gl_NormalMatrix * normalMatrix * normal;

    mat3 rotation = mat3(tangent, binormal, normal);
    vec4 viewpos = gl_ModelViewMatrix * position;
    viewworld_matrix = inverse(gl_ModelViewMatrix);

    v_out.tangent   = tangent;
    v_out.binormal  = binormal;
    v_out.normal    = normal;
    v_out.vnormal   = mat3(gl_ModelViewMatrix) * normal;
    v_out.viewpos   = viewpos.xyz;
    v_out.texcoord  = gl_MultiTexCoord1.xy;

    shadow_vs_main(viewpos);

    SAVE_LIGHTMAP_VARYINGS_VP(v_out, viewpos);
}
