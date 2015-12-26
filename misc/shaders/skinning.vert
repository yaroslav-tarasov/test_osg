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

in vec4 boneWeight0;
in vec4 boneWeight1;
in vec4 boneWeight2;
in vec4 boneWeight3;

uniform int nbBonesPerVertex;
uniform mat4 matrixPalette[MAX_MATRIX];

vec4    position;
vec3    normal;


// accumulate position and normal in global scope
void computeAcummulatedNormalAndPosition(vec4 boneWeight)
{
    int matrixIndex;
    float matrixWeight;
    for (int i = 0; i < 2; i++)
    {
        matrixIndex =  int(boneWeight[0]);
        matrixWeight = boneWeight[1];
        mat4 matrix = matrixPalette[matrixIndex];
        // correct for normal if no scale in bone
        mat3 matrixNormal = mat3(matrix);
        position += matrixWeight * (matrix * gl_Vertex );
        normal += matrixWeight * (matrixNormal * gl_Normal );

        boneWeight = boneWeight.zwxy;
    }
}


uniform mat4             lightmap_matrix;


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

    normal = gl_NormalMatrix * normal;


	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * position;

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
