#include "stdafx.h"
#include "shaders.h"


namespace shaders
{

    namespace plane_mat
    {

    const char* vs = STRINGIFY ( 
        attribute vec3 tangent;
        attribute vec3 binormal;
        varying   vec3 lightDir;

        out block
        {
            vec2 texcoord;
            vec3 normal;
            vec3 tangent;
            vec3 binormal;
            vec3 viewpos;
            vec4 shadow_view;
            vec4 lightmap_coord;
        } v_out;

        void main()
        {
            vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
            mat3 rotation = mat3(tangent, binormal, normal);
            vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
            lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
            lightDir = normalize(rotation * normalize(lightDir));
            gl_Position = ftransform();
            gl_TexCoord[0] = gl_MultiTexCoord1;

            v_out.tangent   = tangent;
            v_out.binormal  = binormal;
            v_out.normal    = normal;
            v_out.viewpos   = vertexInEye.xyz;
        }
    );


    const char* fs = { "#extension GL_ARB_gpu_shader5 : enable \n "
        STRINGIFY ( 

        uniform sampler2D           ViewLightMap;
        uniform sampler2D           Detail;
        uniform samplerCube         Env;
        uniform sampler2DShadow     ShadowSplit0;
        uniform sampler2DShadow     ShadowSplit1;
        uniform sampler2DShadow     ShadowSplit2;
        uniform sampler2D           ViewDecalMap;    

        // saturation helper
        float saturate( const in float x )
        {
            return clamp(x, 0.0, 1.0);
        }   

    )

        STRINGIFY ( 

        uniform sampler2D colorTex;
        uniform sampler2D normalTex;
        varying vec3 lightDir;

        in block
        {
            vec2 texcoord;
            vec3 normal;
            vec3 tangent;
            vec3 binormal;
            vec3 viewpos;
            vec4 shadow_view;
            vec4 lightmap_coord;
        } f_in;

        mat4 viewworld_matrix;

        void main (void)
        {
            // GET_SHADOW(f_in.viewpos, f_in);
            //#define GET_SHADOW(viewpos, in_frag) 
            float shadow = 1.0; 
            //bvec4 split_test = lessThanEqual(vec4(-viewpos.z), shadow_split_borders); 
            //if (split_test.x) 
            //    shadow = textureProj(ShadowSplit0, shadow0_matrix * in_frag.shadow_view); 
            //else if (split_test.y) 
            //    shadow = textureProj(ShadowSplit1, shadow1_matrix * in_frag.shadow_view); 
            //else if (split_test.z) 
            //    shadow = textureProj(ShadowSplit2, shadow2_matrix * in_frag.shadow_view);

            vec4  specular       = gl_LightSource[0].specular;     // FIXME 
            vec4  diffuse        = gl_LightSource[0].diffuse;      // FIXME 
            vec4  ambient        = gl_LightSource[0].ambient;      // FIXME 
            vec4  light_vec_view = vec4(lightDir,1);

            viewworld_matrix = gl_ModelViewMatrixInverse;
            vec4 base = texture2D(colorTex, gl_TexCoord[0].xy);
            vec3 bump = fma(texture2D(normalTex, gl_TexCoord[0].xy).xyz, vec3(2.0), vec3(-1.0));
            //vec3 bump = texture2D(normalTex, gl_TexCoord[0].xy).xyz;
            //bump = normalize(bump * 2.0 - 1.0);
            vec3  normal       = normalize(bump.x * f_in.tangent + bump.y * f_in.binormal + bump.z * f_in.normal);
            vec4  dif_tex_col  = texture2D(colorTex,gl_TexCoord[0].xy, -1.0);
            float glass_factor = 1.0 - dif_tex_col.a;

            // get dist to point and normalized to-eye vector
            float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
            float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
            float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
            vec3  to_eye          = -dist_to_pnt_rcp * f_in.viewpos;

            vec3 view_up_vec = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
            float normal_world_space_z = dot(view_up_vec, normal);


            float incidence_dot  = dot(to_eye, normal);
            float pow_fr         = pow(saturate(1.0 - incidence_dot), 3.0);
            vec3  refl_vec_view  = -to_eye + (2.0 * incidence_dot) * normal;
            vec3  refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
            float refl_min       = fma(glass_factor, 0.275, 0.125);
            float half_refl_z    = 0.5 * (refl_vec_world.z + normal_world_space_z);
            float fresnel        = mix(refl_min, 0.97, pow_fr) * fma(half_refl_z, 0.15, fma(glass_factor, 0.6, 0.25)); 

            float n_dot_l = shadow * saturate(dot(normal, light_vec_view.xyz));
            float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), 44.0) * 0.9;
            vec3  pure_spec_color = specular.rgb * specular_val;
            float spec_compose_fraction = 0.35;


            // const vec3 cube_color = texture(Env, refl_vec_world).rgb + pure_spec_color;
            vec3 cube_color = textureCube(Env, refl_vec_world).rgb + pure_spec_color;
            //vec3 cube_color = vec3(0.5f,0.5f,0.5f);


            vec3 non_ambient_term = diffuse.rgb * n_dot_l + spec_compose_fraction * pure_spec_color;
            // GET_LIGHTMAP(f_in.viewpos, f_in);
            // #define GET_LIGHTMAP(viewpos, in_frag) 
            // float height_world_lm      = in_frag.lightmap_coord.z; 
            // vec4  lightmap_data        = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; 
            // float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.4, 0.75), 0.0, 1.0); 
            // vec3  lightmap_color       = lightmap_data.rgb * lightmap_height_fade;  

            vec3 lightmap_color = vec3(0.1f,0.1f,0.1f); // FIXME dummy staff

            float up_dot_clamped = saturate(fma(normal_world_space_z, 0.55, 0.45));
            non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);

            float ao_trick = fma(up_dot_clamped, 0.4, 0.6);
            vec3  composed_lighting = ao_trick * ambient.rgb + non_ambient_term;
            vec3  day_result = mix(composed_lighting * dif_tex_col.rgb, cube_color, fresnel) + (1.0 - spec_compose_fraction) * pure_spec_color;
            float night_factor = step(ambient.a, 0.35);
            vec3  result = mix(day_result, vec3(0.90, 0.90, 0.86), night_factor * glass_factor);
            //ALFA-TEST// gl_FragColor = vec4( glass_factor,0,0,1.0f);
            gl_FragColor = vec4( result,1.0);

        }
    )

    };

    const char* get_shader(shader_t t)
    {
        if(t==VS)
            return vs;
        else if(t=FS)
            return fs;
        else 
            return nullptr;
    }

    } // ns plane_mat

    namespace default_mat 
    {
       const char* vs = STRINGIFY ( 
           attribute vec3 tangent;
           attribute vec3 binormal;
           varying   vec3 lightDir;

           out block
           {
               vec2 texcoord;
               vec3 normal;
               vec3 tangent;
               vec3 binormal;
               vec3 viewpos;
               vec4 shadow_view;
               vec4 lightmap_coord;
           } v_out;

           void main()
           {
               vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
               mat3 rotation = mat3(tangent, binormal, normal);
               vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
               lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
               lightDir = normalize(rotation * normalize(lightDir));
               gl_Position = ftransform();
               gl_TexCoord[0] = gl_MultiTexCoord1;

               v_out.tangent   = tangent;
               v_out.binormal  = binormal;
               v_out.normal    = normal;
               v_out.viewpos   = vertexInEye.xyz;
           }       
       );


       const char* fs = { 
       "#extension GL_ARB_gpu_shader5 : enable \n "

        STRINGIFY ( 

           uniform sampler2D           ViewLightMap;
           uniform sampler2D           Detail;
           uniform samplerCube         Env;
           uniform sampler2DShadow     ShadowSplit0;
           uniform sampler2DShadow     ShadowSplit1;
           uniform sampler2DShadow     ShadowSplit2;
           uniform sampler2D           ViewDecalMap;    

           // saturation helper
           float saturate( const in float x )
           {
               return clamp(x, 0.0, 1.0);
           }   

       )

       STRINGIFY ( 

           uniform sampler2D colorTex;
           uniform sampler2D normalTex;
           varying vec3 lightDir;

           in block
           {
               vec2 texcoord;
               vec3 normal;
               vec3 tangent;
               vec3 binormal;
               vec3 viewpos;
               vec4 shadow_view;
               vec4 lightmap_coord;
           } f_in;

           mat4 viewworld_matrix;

           void main (void)
           {
               // GET_SHADOW(f_in.viewpos, f_in);
               //#define GET_SHADOW(viewpos, in_frag) 
               float shadow = 1.0; 
               //bvec4 split_test = lessThanEqual(vec4(-viewpos.z), shadow_split_borders); 
               //if (split_test.x) 
               //    shadow = textureProj(ShadowSplit0, shadow0_matrix * in_frag.shadow_view); 
               //else if (split_test.y) 
               //    shadow = textureProj(ShadowSplit1, shadow1_matrix * in_frag.shadow_view); 
               //else if (split_test.z) 
               //    shadow = textureProj(ShadowSplit2, shadow2_matrix * in_frag.shadow_view);

               vec4  specular       = gl_LightSource[0].specular;     // FIXME 
               vec4  diffuse        = gl_LightSource[0].diffuse;      // FIXME 
               vec4  ambient        = gl_LightSource[0].ambient;      // FIXME 
               vec4  light_vec_view = vec4(lightDir,1);

               viewworld_matrix = gl_ModelViewMatrixInverse;
               vec4 base = texture2D(colorTex, gl_TexCoord[0].xy);
               vec3 bump = fma(texture2D(normalTex, gl_TexCoord[0].xy).xyz, vec3(2.0), vec3(-1.0));
               //vec3 bump = texture2D(normalTex, gl_TexCoord[0].xy).xyz;
               //bump = normalize(bump * 2.0 - 1.0);
               vec3  normal       = normalize(bump.x * f_in.tangent + bump.y * f_in.binormal + bump.z * f_in.normal);
               vec4  dif_tex_col  = texture2D(colorTex,gl_TexCoord[0].xy, -1.0);
               float glass_factor = 1.0 - dif_tex_col.a;

               // get dist to point and normalized to-eye vector
               float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
               float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
               float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
               vec3  to_eye          = -dist_to_pnt_rcp * f_in.viewpos;

               vec3 view_up_vec = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
               float normal_world_space_z = dot(view_up_vec, normal);


               float incidence_dot  = dot(to_eye, normal);
               float pow_fr         = pow(saturate(1.0 - incidence_dot), 3.0);
               vec3  refl_vec_view  = -to_eye + (2.0 * incidence_dot) * normal;
               vec3  refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
               float refl_min       = fma(glass_factor, 0.275, 0.125);
               float half_refl_z    = 0.5 * (refl_vec_world.z + normal_world_space_z);
               float fresnel        = mix(refl_min, 0.97, pow_fr) * fma(half_refl_z, 0.15, fma(glass_factor, 0.6, 0.25)); 

               float n_dot_l = shadow * saturate(dot(normal, light_vec_view.xyz));
               float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), 44.0) * 0.9;
               vec3  pure_spec_color = specular.rgb * specular_val;
               float spec_compose_fraction = 0.35;


               // const vec3 cube_color = texture(Env, refl_vec_world).rgb + pure_spec_color;
               vec3 cube_color = textureCube(Env, refl_vec_world).rgb + pure_spec_color;
               //vec3 cube_color = vec3(0.5f,0.5f,0.5f);


               vec3 non_ambient_term = diffuse.rgb * n_dot_l + spec_compose_fraction * pure_spec_color;
               // GET_LIGHTMAP(f_in.viewpos, f_in);
               // #define GET_LIGHTMAP(viewpos, in_frag) 
               // float height_world_lm      = in_frag.lightmap_coord.z; 
               // vec4  lightmap_data        = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; 
               // float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.4, 0.75), 0.0, 1.0); 
               // vec3  lightmap_color       = lightmap_data.rgb * lightmap_height_fade;  

               vec3 lightmap_color = vec3(0.1f,0.1f,0.1f); // FIXME dummy staff

               float up_dot_clamped = saturate(fma(normal_world_space_z, 0.55, 0.45));
               non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);

               float ao_trick = fma(up_dot_clamped, 0.4, 0.6);
               vec3  composed_lighting = ao_trick * ambient.rgb + non_ambient_term;
               vec3  day_result = mix(composed_lighting * dif_tex_col.rgb, cube_color, fresnel) + (1.0 - spec_compose_fraction) * pure_spec_color;
               float night_factor = step(ambient.a, 0.35);
               vec3  result = mix(day_result, vec3(0.90, 0.90, 0.86), night_factor * glass_factor);
               //ALFA-TEST// gl_FragColor = vec4( glass_factor,0,0,1.0f);
               gl_FragColor = vec4( result,1.0);

           }
       )

       };   

       const char* get_shader(shader_t t)
       {
           if(t==VS)
               return vs;
           else if(t=FS)
               return fs;
           else 
               return nullptr;
       }
    
    }  // ns default_mat


}  // ns shaders