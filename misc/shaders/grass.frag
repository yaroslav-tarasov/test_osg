// grass.frag
#version 150 compatibility
#extension GL_ARB_gpu_shader5 : enable

uniform sampler2D tex;

in mat4 viewworld_matrix;

// FIXME view matrix as attribute or view matrix as uniform 
#include "scene_params.hlsl"
#include "fog.hlsl"
#include "utils.hlsl"


in block
{
    vec2 texcoord;
    vec3 normal;
    vec3 viewpos;
    vec4 shadow_view;
    vec4 lightmap_coord;
} f_in;


out vec4  aFragColor;

void main()
{
	//vec4 dif_tex_col = texture2D(tex,gl_TexCoord[0].st);
	//vec3 result = dif_tex_col.rgb;

    vec3 normal = normalize(f_in.normal);
    float n_dot_l = saturate(fma(dot(normal, light_vec_view.xyz), 0.75, 0.25));

    vec4 dif_tex_col = texture2D(tex, f_in.texcoord);
    vec3 result = (ambient.rgb + diffuse.rgb * n_dot_l) * dif_tex_col.rgb * 0.65;
                
    aFragColor = vec4(apply_clear_fog(f_in.viewpos, result), dif_tex_col.a);
}
