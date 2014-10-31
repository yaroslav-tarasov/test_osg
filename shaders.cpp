#include "stdafx.h"
#include "shaders.h"


namespace shaders
{
    //template <typename S>
    //__forceinline S luminance_crt( color_t<S> const & col )
    //{
    //    return S(0.299f * col.r + 0.587f * col.g + 0.114f * col.b);
    //}

    ////
    //template <typename S>
    //__forceinline S luminance_lcd( color_t<S> const & col )
    //{
    //    return S(0.2127f * col.r + 0.7152f * col.g + 0.0722f * col.b);
    //}


    namespace plane_mat
    {

    const char* vs = {
        "#extension GL_ARB_gpu_shader5 : enable \n"

        STRINGIFY ( 
        attribute vec3 tangent;
        attribute vec3 binormal;
        varying   vec3 lightDir;
        varying   float illum; 
        
        float luminance_crt(  const in vec4 col )
        {
            return (0.299f * col.r + 0.587f * col.g + 0.114f * col.b);
        }        


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
            // lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
            // lightDir = normalize(rotation * normalize(lightDir));
            lightDir = vec3(gl_LightSource[0].position.xyz);

            gl_Position = ftransform();
            gl_TexCoord[0] = gl_MultiTexCoord1;


            v_out.tangent   = tangent;
            v_out.binormal  = binormal;
            v_out.normal    = normal;
            v_out.viewpos   = vertexInEye.xyz;
            v_out.texcoord  = gl_TexCoord[0].xy ;

            illum = luminance_crt(gl_LightSource[0].ambient + gl_LightSource[0].diffuse); // FIXME Этот расчет должен быть в основной программе, а не для каждого фрагмента
        }
    )
    };


    const char* fs = { "#extension GL_ARB_gpu_shader5 : enable \n "
        STRINGIFY ( 
    
        // layout(early_fragment_tests) in;

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
        varying   vec3 lightDir;
        varying   float illum; 

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
            
            ambient.a = illum;   

            viewworld_matrix = gl_ModelViewMatrixInverse;
            vec4 base = texture2D(colorTex, f_in.texcoord.xy);
            vec3 bump = fma(texture2D(normalTex, f_in.texcoord.xy).xyz, vec3(2.0), vec3(-1.0));
            //vec3 bump = texture2D(normalTex, gl_TexCoord[0].xy).xyz;
            //bump = normalize(bump * 2.0 - 1.0);
            vec3  normal       = normalize(bump.x * f_in.tangent + bump.y * f_in.binormal + bump.z * f_in.normal);
            vec4  dif_tex_col  = texture2D(colorTex,f_in.texcoord.xy, -1.0);
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

            vec3 lightmap_color = vec3(0.1f,0.1f,0.1f); // FIXME dummy code

            float up_dot_clamped = saturate(fma(normal_world_space_z, 0.55, 0.45));
            non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);

            float ao_trick = fma(up_dot_clamped, 0.4, 0.6);
            vec3  composed_lighting = ao_trick * ambient.rgb + non_ambient_term;
            vec3  day_result = mix(composed_lighting * dif_tex_col.rgb, cube_color, fresnel) + (1.0 - spec_compose_fraction) * pure_spec_color;
            float night_factor = step(ambient.a, 0.35);
            vec3  result = mix(day_result, vec3(0.90, 0.90, 0.86), night_factor * glass_factor);
            //ALFA-TEST// gl_FragColor = vec4( glass_factor,0,0,1.0f);
            //LIGHT_VIEW_TEST//gl_FragColor = vec4(lightDir,1.0);    
            gl_FragColor = vec4( result,1.0);
        }
    )

    };

    const char* get_shader(shader_t t)
    {
        if(t==VS)
            return vs;
        else if(t==FS)
            return fs;
        else 
            return nullptr;
    }

    } // ns plane_mat

    namespace default_mat 
    {
       const char* vs = {  
           "#extension GL_ARB_gpu_shader5 : enable \n"
           STRINGIFY ( 
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
               // lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
               // lightDir = normalize(rotation * normalize(lightDir));
               lightDir = vec3(gl_LightSource[0].position.xyz);
               gl_Position = ftransform();
               gl_TexCoord[0] = gl_MultiTexCoord1;

               v_out.tangent   = tangent;
               v_out.binormal  = binormal;
               v_out.normal    = normal;
               v_out.viewpos   = vertexInEye.xyz;
               v_out.texcoord  = gl_TexCoord[0].xy ;
           }       
       )
       };


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
               float glass_factor = /*1.0 - dif_tex_col.a*/0;

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
               float refl_min       = 0.10 + glass_factor * 0.30;
               float half_refl_z    = 0.5 * (refl_vec_world.z + normal_world_space_z);
               float fresnel        = mix(refl_min, 0.6, pow_fr) * fma(half_refl_z, 0.15, fma(glass_factor, 0.6, 0.)); 

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

               vec3 lightmap_color = vec3(0.1f,0.1f,0.1f); // FIXME dummy code

               float up_dot_clamped = saturate(fma(normal_world_space_z, 0.55, 0.45));
               non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);

               float ao_trick = fma(up_dot_clamped, 0.4, 0.6);
               vec3  composed_lighting = ao_trick * ambient.rgb + non_ambient_term;
               vec3  day_result = mix(composed_lighting * dif_tex_col.rgb, cube_color, fresnel) + (1.0 - spec_compose_fraction) * pure_spec_color;
               float night_factor = step(ambient.a, 0.35);
               vec3  result = mix(day_result, vec3(0.90, 0.90, 0.86), night_factor * glass_factor);

               gl_FragColor = vec4( result,1.0);

           }
       )

       };   

       const char* get_shader(shader_t t)
       {
           if(t==VS)
               return vs;
           else if(t==FS)
               return fs;
           else 
               return nullptr;
       }
    
    }  // ns default_mat

    namespace building_mat 
    {
        const char* vs = {  
            "#extension GL_ARB_gpu_shader5 : enable \n"
            STRINGIFY ( 
            
            float luminance_crt(  const in vec4 col )
            {
                return (0.299f * col.r + 0.587f * col.g + 0.114f * col.b);
            }

            varying   vec3  lightDir;
            varying   float illum; 

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                // mat3 rotation = mat3(tangent, binormal, normal);
                vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
                //lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
                //lightDir = normalize(rotation * normalize(lightDir));
                lightDir = vec3(gl_LightSource[0].position.xyz);;
                gl_Position = ftransform();
                gl_TexCoord[0] = gl_MultiTexCoord1;

                v_out.normal    = normal;
                v_out.viewpos   = vertexInEye.xyz;
                v_out.texcoord  = gl_TexCoord[0].xy ;
                
                illum = luminance_crt(gl_LightSource[0].ambient + gl_LightSource[0].diffuse); // FIXME Этот расчет должен быть в основной программе, а не для каждого фрагмента
            }       
            )
        };


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
            
            // hardlight function
            vec3 hardlight( const in vec3 color, const in vec3 hl )
            {
                vec3 hl_pos = step(vec3(0.0), hl);
                return (vec3(1.0) - hl_pos) * color * (hl + vec3(1.0)) +
                    hl_pos * mix(color, vec3(1.0), hl);
            }

            // texture detail factor
            float tex_detail_factor( const in vec2 tex_c_mod, const in float coef )
            {
                vec2 grad_vec = fwidth(tex_c_mod);
                float detail_fac = exp(coef * dot(grad_vec, grad_vec));
                return detail_fac * (2.0 - detail_fac);
            }

            // ramp_up
            float ramp_up( const in float x )
            {
                return x * fma(x, -0.5, 1.5);
            }

            // ramp_down
            float ramp_down( const in float x )
            {
                return x * fma(x, 0.5, 0.5);
            }


            )

            STRINGIFY ( 

            uniform sampler2D           colorTex;
            uniform sampler2D           NightTex;
            varying   vec3  lightDir;
            varying   float illum;

            in block
            {
                vec2 texcoord;
                vec3 normal;
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

                ambient.a = illum;

                vec4  light_vec_view = vec4(lightDir,1);

                viewworld_matrix = gl_ModelViewMatrixInverse;
         

                // get dist to point and normalized to-eye vector
                float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
                float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
                float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
                vec3  to_eye          = -dist_to_pnt_rcp * f_in.viewpos;

                vec3  normal = normalize(f_in.normal);
                float incidence_dot = dot(to_eye, normal);
                vec3  refl_vec_view = -to_eye + (2.0 * incidence_dot) * normal;

                vec3  view_up_vec = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
                float normal_world_space_z = dot(view_up_vec, normal);

                // diffuse color and glass factor (make windows color look darker)
                vec4 dif_tex_col = texture2D(colorTex, f_in.texcoord);
                dif_tex_col.rgb *= fma(dif_tex_col.a, 0.6, 0.4);
                float glass_factor = 1.0 - dif_tex_col.a;

                // get diffuse and specular value
                float n_dot_l = ramp_up(shadow * saturate(dot(normal, light_vec_view.xyz)));
                float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), 10.0) * 2.0;
                vec3 spec_color = specular.rgb * specular_val;

                // lightmaps
                vec3 non_ambient_term = n_dot_l * diffuse.rgb;

                // GET_LIGHTMAP(f_in.viewpos, f_in);
                // #define GET_LIGHTMAP(viewpos, in_frag) 
                // float height_world_lm      = in_frag.lightmap_coord.z; 
                // vec4  lightmap_data        = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; 
                // float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.4, 0.75), 0.0, 1.0); 
                // vec3  lightmap_color       = lightmap_data.rgb * lightmap_height_fade;  

                vec3 lightmap_color = vec3(0.2f,0.2f,0.2f); // FIXME dummy code


                //    LIGHTMAP_BUILDING_HEIGHT_TRICK;
                float up_dot_clamped = saturate(fma(normal_world_space_z, 0.4, 0.6));
                non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);

                // overall lighting
                vec3 light_color = ambient.rgb + non_ambient_term;

                // apply detail texture
                float detail_factor = dif_tex_col.a * tex_detail_factor(f_in.texcoord * textureSize2D(colorTex, 0), -0.075);
                if (detail_factor > 0.01)
                    dif_tex_col.rgb = hardlight(dif_tex_col.rgb, detail_factor * fma(texture2D(Detail, f_in.texcoord * 9.73f).rrr, vec3(0.5), vec3(-0.25)));

                vec3 day_result = light_color * dif_tex_col.rgb;
                vec3 night_tex = vec3(0.0f,0.0f,0.0f); // I'm not sure
                if (glass_factor > 0.25)
                {
                    vec3 refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
                    refl_vec_world.z = abs(refl_vec_world.z);
                    float fresnel = saturate(fma(pow(1.0 - incidence_dot, 2.0), 0.65, 0.35)) * fma(refl_vec_world.z, 0.15, 0.85);
                    vec3 cube_color = textureCube(Env, refl_vec_world).rgb;
                    cube_color = vec3(1.0f,1.0f,1.0f); // FIXME dummy code
                    day_result = mix(day_result, cube_color, glass_factor * fresnel) + spec_color * glass_factor;
                    night_tex = texture2D(NightTex, f_in.texcoord).rgb;
                }

                float night_factor = step(ambient.a, 0.35);
                vec3 result = mix(day_result, night_tex,  night_factor * glass_factor ); // 
               
                // gl_FragColor =  mix(texture2D(colorTex,f_in.texcoord), texture2D(NightTex, f_in.texcoord),night_factor);
                
                
                gl_FragColor = vec4( result,1.0);

            }
            )

        };   

        const char* get_shader(shader_t t)
        {
            if(t==VS)
                return vs;
            else if(t==FS)
                return fs;
            else 
                return nullptr;
        }

    }  // ns building_mat

    namespace tree_mat 
    {
        const char* vs = {  
            "#extension GL_ARB_gpu_shader5 : enable \n"
            STRINGIFY ( 

            float luminance_crt(  const in vec4 col )
            {
                return (0.299f * col.r + 0.587f * col.g + 0.114f * col.b);
            }

            varying   vec3  lightDir;
            varying   float illum; 

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                // mat3 rotation = mat3(tangent, binormal, normal);
                vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
                //lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
                //lightDir = normalize(rotation * normalize(lightDir));
                lightDir = vec3(gl_LightSource[0].position.xyz);;
                gl_Position = ftransform();
                gl_TexCoord[0] = gl_MultiTexCoord1;

                v_out.normal    = normal;
                v_out.viewpos   = vertexInEye.xyz;
                v_out.texcoord  = gl_TexCoord[0].xy ;

                illum = luminance_crt(gl_LightSource[0].ambient + gl_LightSource[0].diffuse); // FIXME Этот расчет должен быть в основной программе, а не для каждого фрагмента
            }       
            )
        };


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

            // hardlight function
            vec3 hardlight( const in vec3 color, const in vec3 hl )
            {
                vec3 hl_pos = step(vec3(0.0), hl);
                return (vec3(1.0) - hl_pos) * color * (hl + vec3(1.0)) +
                    hl_pos * mix(color, vec3(1.0), hl);
            }

            // texture detail factor
            float tex_detail_factor( const in vec2 tex_c_mod, const in float coef )
            {
                vec2 grad_vec = fwidth(tex_c_mod);
                float detail_fac = exp(coef * dot(grad_vec, grad_vec));
                return detail_fac * (2.0 - detail_fac);
            }

            // ramp_up
            float ramp_up( const in float x )
            {
                return x * fma(x, -0.5, 1.5);
            }

            // ramp_down
            float ramp_down( const in float x )
            {
                return x * fma(x, 0.5, 0.5);
            }


            )

            STRINGIFY ( 

            uniform sampler2D       colorTex;
            uniform sampler2D       NightTex;
            varying   vec3  lightDir;
            varying   float illum;

            in block
            {
                vec2 texcoord;
                vec3 normal;
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
                // ambient.a = illum;
                vec4  light_vec_view = vec4(lightDir,1);
                viewworld_matrix = gl_ModelViewMatrixInverse;


                vec3 normal = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
                float n_dot_l = shadow * saturate(fma(dot(normal, light_vec_view.xyz), 0.5, 0.5));

                vec4 dif_tex_col = texture2D(colorTex, f_in.texcoord);
                vec3 result = (ambient.rgb + diffuse.rgb * n_dot_l) * dif_tex_col.rgb;

                // FragColor = vec4(apply_scene_fog(f_in.viewpos, result), dif_tex_col.a);
                gl_FragColor = vec4( result, dif_tex_col.a);
            }

            )

        };   

        const char* get_shader(shader_t t)
        {
            if(t==VS)
                return vs;
            else if(t==FS)
                return fs;
            else 
                return nullptr;
        }

    }  // ns tree_mat

    namespace ground_mat 
    {
        const char* vs = {  
            "#extension GL_ARB_gpu_shader5 : enable \n"
            STRINGIFY ( 
            attribute vec3 tangent;
            attribute vec3 binormal;
            varying   vec3 lightDir;
            varying   float illum; 
            
            float luminance_crt(  const in vec4 col )
            {
                return (0.299f * col.r + 0.587f * col.g + 0.114f * col.b);
            }

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 tangent;
                vec3 binormal;
                vec3 viewpos;
                vec2 detail_uv;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                // mat3 rotation = mat3(tangent, binormal, normal);
                vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
                // lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
                // lightDir = normalize(rotation * normalize(lightDir));
                lightDir = vec3(gl_LightSource[0].position.xyz);
                gl_Position = ftransform();
                gl_TexCoord[0] = gl_MultiTexCoord1;

                
                v_out.normal    = normal;
                v_out.tangent   = tangent;
                v_out.binormal  = binormal;
                 
                v_out.viewpos   = vertexInEye.xyz;
                v_out.detail_uv = gl_TexCoord[0].xy * 0.03;
                v_out.texcoord  = gl_TexCoord[0].xy;

                illum = luminance_crt(gl_LightSource[0].ambient + gl_LightSource[0].diffuse); // FIXME Этот расчет должен быть в основной программе, а не для каждого фрагмента
            }       
            )
        };


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

            // hardlight function
            vec3 hardlight( const in vec3 color, const in vec3 hl )
            {
                vec3 hl_pos = step(vec3(0.0), hl);
                return (vec3(1.0) - hl_pos) * color * (hl + vec3(1.0)) +
                    hl_pos * mix(color, vec3(1.0), hl);
            }

            // texture detail factor
            float tex_detail_factor( const in vec2 tex_c_mod, const in float coef )
            {
                vec2 grad_vec = fwidth(tex_c_mod);
                float detail_fac = exp(coef * dot(grad_vec, grad_vec));
                return detail_fac * (2.0 - detail_fac);
            }

            // ramp_up
            float ramp_up( const in float x )
            {
                return x * fma(x, -0.5, 1.5);
            }

            // ramp_down
            float ramp_down( const in float x )
            {
                return x * fma(x, 0.5, 0.5);
            }


            )

            STRINGIFY ( 

            uniform sampler2D colorTex;
            varying vec3 lightDir;
            varying float illum;

            in block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 tangent;
                vec3 binormal;
                vec3 viewpos;
                vec2 detail_uv;
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
                // FIXME dummy code
                specular.a = 0; // it's not rainy day hallelujah

                float rainy_value = 0.666 * specular.a;

                vec3 dif_tex_col = texture2D(colorTex, f_in.texcoord).rgb;
                dif_tex_col *= fma(dif_tex_col, rainy_value.xxx, vec3(1.0 - rainy_value));
                float detail_factor = tex_detail_factor(f_in.texcoord * textureSize2D(colorTex, 0), -0.02);
                vec3 normal_noise = vec3(0.0);
                //if (detail_factor > 0.01)
                {
                    normal_noise = detail_factor * fma(texture2D(Detail, f_in.detail_uv).rgb, vec3(0.6), vec3(-0.3));
                    dif_tex_col = hardlight(dif_tex_col, normal_noise.ggg);
                }

                vec3 normal = normalize(0.8 * f_in.normal + (normal_noise.x * f_in.tangent + normal_noise.y * f_in.binormal));
                float n_dot_l = saturate(dot(normal, light_vec_view.xyz));


                // get dist to point and normalized to-eye vector
                float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
                float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
                float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
                vec3 to_pnt = dist_to_pnt_rcp * f_in.viewpos;

                // reflection vector
                float incidence_dot = dot(-to_pnt, normal);
                vec3 refl_vec_view = fma(normal, vec3(2.0 * incidence_dot), to_pnt);

                // specular
                float specular_val = pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), fma(rainy_value, 3.0, 3.0)) * fma(rainy_value, 0.8, 0.3);
                vec3 specular_color = specular_val * specular.rgb;

                // GET_LIGHTMAP(f_in.viewpos, f_in);
                // #define GET_LIGHTMAP(viewpos, in_frag) 
                // float height_world_lm      = in_frag.lightmap_coord.z; 
                // vec4  lightmap_data        = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; 
                // float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.4, 0.75), 0.0, 1.0); 
                // vec3  lightmap_color       = lightmap_data.rgb * lightmap_height_fade;  

                vec3 lightmap_color = vec3(0.0f,0.0f,0.0f); // FIXME dummy code

                // LIGHTMAP_SHADOW_TRICK(shadow);
                vec3 non_ambient_term = max(lightmap_color, shadow * (diffuse.rgb * n_dot_l + specular_color));

                // result
                vec3 result = (ambient.rgb + non_ambient_term) * dif_tex_col.rgb;
                // reflection when rain
                if (rainy_value >= 0.01)
                {
                    vec3 refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
                    float fresnel = saturate(fma(pow(1.0 - incidence_dot, 5.0), 0.25, 0.05));
                    vec3 cube_color = textureCube(Env, refl_vec_world).rgb;
                    result = mix(result, lightmap_color + cube_color + specular_color, fresnel * rainy_value);
                }

                gl_FragColor = vec4( result,1.0);

            }
            )

        };   

        const char* get_shader(shader_t t)
        {
            if(t==VS)
                return vs;
            else if(t==FS)
                return fs;
            else 
                return nullptr;
        }

    }  // ns ground_mat

    namespace concrete_mat 
    {
        const char* vs = {  
            "#extension GL_ARB_gpu_shader5 : enable \n"
            STRINGIFY ( 
            attribute vec3 tangent;
            attribute vec3 binormal;
            varying   vec3 lightDir;
            varying   float illum; 
            
            mat4 decal_matrix;

            float luminance_crt(  const in vec4 col )
            {
                return (0.299f * col.r + 0.587f * col.g + 0.114f * col.b);
            }

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 tangent;
                vec3 binormal;
                vec3 viewpos;
                vec2 detail_uv;
                vec4 shadow_view;
                vec4 lightmap_coord;
                vec4 decal_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                // mat3 rotation = mat3(tangent, binormal, normal);
                vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;

                lightDir = vec3(gl_LightSource[0].position.xyz);
                gl_Position = ftransform();
                gl_TexCoord[0] = gl_MultiTexCoord1;

                v_out.normal    = normal;
                v_out.tangent   = tangent;
                v_out.binormal  = binormal;

                v_out.viewpos   = vertexInEye.xyz;
                // v_out.detail_uv = position.xy * 0.045;
                v_out.detail_uv = gl_Vertex.xy * 0.045; // FIXME dont no how
                v_out.texcoord  = gl_TexCoord[0].xy;
                
                // SAVE_DECAL_VARYINGS_VP
                v_out.decal_coord = (decal_matrix * vec4(v_out.viewpos,1.0)).xyzw;

                illum = luminance_crt(gl_LightSource[0].ambient + gl_LightSource[0].diffuse); // FIXME Этот расчет должен быть в основной программе, а не для каждого фрагмента
            }       
            )
        };


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

            // hardlight function
            vec3 hardlight( const in vec3 color, const in vec3 hl )
            {
                vec3 hl_pos = step(vec3(0.0), hl);
                return (vec3(1.0) - hl_pos) * color * (hl + vec3(1.0)) +
                    hl_pos * mix(color, vec3(1.0), hl);
            }

            // texture detail factor
            float tex_detail_factor( const in vec2 tex_c_mod, const in float coef )
            {
                vec2 grad_vec = fwidth(tex_c_mod);
                float detail_fac = exp(coef * dot(grad_vec, grad_vec));
                return detail_fac * (2.0 - detail_fac);
            }

            // ramp_up
            float ramp_up( const in float x )
            {
                return x * fma(x, -0.5, 1.5);
            }

            // ramp_down
            float ramp_down( const in float x )
            {
                return x * fma(x, 0.5, 0.5);
            }


            )

            STRINGIFY ( 

            uniform sampler2D colorTex;
            varying vec3 lightDir;
            varying float illum;

            in block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 tangent;
                vec3 binormal;
                vec3 viewpos;
                vec2 detail_uv;
                vec4 shadow_view;
                vec4 lightmap_coord;
                vec4 decal_coord;
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
                // FIXME dummy code
                specular.a = 0; // it's not rainy day hallelujah

                float rainy_value = specular.a;

                vec3 dif_tex_col = texture2D(colorTex, f_in.texcoord).rgb;
                float tex_mix_val = rainy_value * 0.7;
                dif_tex_col *= fma(dif_tex_col, tex_mix_val.xxx, vec3(1.0 - tex_mix_val));
                float detail_factor = tex_detail_factor(f_in.texcoord * textureSize2D(colorTex, 0), -0.015);
                vec3 concrete_noise = vec3(0.0);
                if (detail_factor > 0.01)
                {
                    concrete_noise = detail_factor * fma(texture2D(Detail, f_in.detail_uv).rgb, vec3(0.48), vec3(-0.24));
                    dif_tex_col = hardlight(dif_tex_col, concrete_noise.bbb);
                }

                // FIXME
                // APPLY_DECAL(f_in, dif_tex_col);
                vec4 decal_data = textureProj(ViewDecalMap, f_in.decal_coord).rgba; 
                // dif_col.rgb = fma(dif_col.rgb, vec3(1.0 - decal_data.a), decal_data.rgb);        // FIXME
                decal_data.a = 0.0; //FIXME Dummy code 

                // get dist to point and normalized to-eye vector
                float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
                float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
                float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
                vec3 to_pnt = dist_to_pnt_rcp * f_in.viewpos;

                // reflection vector

                vec3 normal = normalize(f_in.normal + (concrete_noise.x * f_in.tangent + concrete_noise.y * f_in.binormal) * (1.0 - decal_data.a));
                float incidence_dot = dot(-to_pnt, normal);
                vec3 refl_vec_view = fma(normal, vec3(2.0 * incidence_dot), to_pnt);
                
                // diffuse term
                float n_dot_l = shadow * saturate(dot(normal, light_vec_view.xyz)) * fma(rainy_value, -0.7, 1.0);

                // specular
                float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), fma(rainy_value, 5.0, 1.5)) * fma(rainy_value, 0.9, 0.7);
                vec3 specular_color = specular_val * specular.rgb;

                // GET_LIGHTMAP(f_in.viewpos, f_in);
                // #define GET_LIGHTMAP(viewpos, in_frag) 
                // float height_world_lm      = in_frag.lightmap_coord.z; 
                // vec4  lightmap_data        = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; 
                // float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.4, 0.75), 0.0, 1.0); 
                // vec3  lightmap_color       = lightmap_data.rgb * lightmap_height_fade;  

                vec3 lightmap_color = vec3(0.0f,0.0f,0.0f); // FIXME dummy code

                // FIXME
                // LIGHTMAP_SHADOW_TRICK(shadow);

                vec3 non_ambient_term = max(lightmap_color, diffuse.rgb * n_dot_l + specular_color);

                // result
                vec3 result = (ambient.rgb + non_ambient_term) * dif_tex_col.rgb;
                // reflection when rain
                if (rainy_value >= 0.01)
                {
                    vec3 refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
                    float fresnel = saturate(fma(pow(1.0 - incidence_dot, 5.0), 0.45, 0.05));
                    vec3 cube_color = textureCube(Env, refl_vec_world).rgb;
                    result = mix(result, lightmap_color + cube_color, fresnel * rainy_value) + (fma(fresnel, 0.5, 0.5) * rainy_value) * specular_color;
                }

                gl_FragColor = vec4( result,1.0);
            }
            )

        };   

        const char* get_shader(shader_t t)
        {
            if(t==VS)
                return vs;
            else if(t==FS)
                return fs;
            else 
                return nullptr;
        }

    }  // ns concrete_mat

    namespace railing_mat 
    {
        const char* vs = {  
            "#extension GL_ARB_gpu_shader5 : enable \n"
            STRINGIFY ( 
            attribute vec3 tangent;
            attribute vec3 binormal;
            varying   vec3 lightDir;
            varying   float illum; 

            float luminance_crt(  const in vec4 col )
            {
                return (0.299f * col.r + 0.587f * col.g + 0.114f * col.b);
            }

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                // mat3 rotation = mat3(tangent, binormal, normal);
                vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;

                lightDir = vec3(gl_LightSource[0].position.xyz);
                gl_Position = ftransform();
                gl_TexCoord[0] = gl_MultiTexCoord1;

                v_out.normal    = normal;
                v_out.viewpos   = vertexInEye.xyz;
                v_out.texcoord  = gl_TexCoord[0].xy;


                illum = luminance_crt(gl_LightSource[0].ambient + gl_LightSource[0].diffuse); // FIXME Этот расчет должен быть в основной программе, а не для каждого фрагмента
            }       
            )
        };


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

            // hardlight function
            vec3 hardlight( const in vec3 color, const in vec3 hl )
            {
                vec3 hl_pos = step(vec3(0.0), hl);
                return (vec3(1.0) - hl_pos) * color * (hl + vec3(1.0)) +
                    hl_pos * mix(color, vec3(1.0), hl);
            }

            // texture detail factor
            float tex_detail_factor( const in vec2 tex_c_mod, const in float coef )
            {
                vec2 grad_vec = fwidth(tex_c_mod);
                float detail_fac = exp(coef * dot(grad_vec, grad_vec));
                return detail_fac * (2.0 - detail_fac);
            }

            // ramp_up
            float ramp_up( const in float x )
            {
                return x * fma(x, -0.5, 1.5);
            }

            // ramp_down
            float ramp_down( const in float x )
            {
                return x * fma(x, 0.5, 0.5);
            }


            )

            STRINGIFY ( 

            uniform sampler2D colorTex;
            varying vec3 lightDir;
            varying float illum;

            in block
            {
                vec2 texcoord;
                vec3 normal;
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
                // FIXME dummy code

                vec3 normal = normalize(f_in.normal);
                float n_dot_l = saturate(fma(dot(normal, light_vec_view.xyz), 0.6, 0.4));

                // get dist to point and normalized to-eye vector
                float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
                float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
                float dist_to_pnt = dist_to_pnt_rcp * dist_to_pnt_sqr;

                vec3 to_pnt = dist_to_pnt_rcp * f_in.viewpos;
                vec3 half_v = normalize(-to_pnt + light_vec_view.xyz);
                float specular_val = pow(saturate(dot(half_v, normal)), 8.0) * 0.5;

                vec3 non_ambient_term = shadow * (diffuse.rgb * n_dot_l + specular.xyz * specular_val);
                // GET_LIGHTMAP(f_in.viewpos, f_in);
                 vec3 lightmap_color = vec3(0.0f,0.0f,0.0f); // FIXME dummy code

                non_ambient_term = max(lightmap_color, non_ambient_term);

                vec4 dif_tex_col = texture2D(colorTex, f_in.texcoord);
                vec3 result = (ambient.rgb + non_ambient_term) * dif_tex_col.rgb;

                gl_FragColor = vec4( result,dif_tex_col.a);
                
            }
            )

        };   

        const char* get_shader(shader_t t)
        {
            if(t==VS)
                return vs;
            else if(t==FS)
                return fs;
            else 
                return nullptr;
        }

    }  // ns railing_mat

}  // ns shaders