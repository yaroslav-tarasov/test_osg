#include "stdafx.h"
#include "shaders.h"

#include "av/precompiled.h"

namespace shaders
{

    namespace include_mat
    {

#define  SHADERS_GETTER(getter_name,vs_name, fs_name) \
        const char* getter_name(shader_t t)           \
        {                                             \
            if(t==VS)                                 \
                return vs_name;                       \
            else if(t==FS)                            \
                return fs_name;                       \
            else                                      \
                return nullptr;                       \
        }                                             \


#define  LIGHT_MAPS                                                                                                  \
"$define GET_LIGHTMAP(viewpos, in_frag) \\"                                                                          \
"\n /*const*/ float height_world_lm = in_frag.lightmap_coord.z; \\"                                                  \
"\n /*const*/ vec4 lightmap_data = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; \\"                       \
"\n /*const*/ float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.4, 0.75), 0.0, 1.0); \\"   \
"\n vec3 lightmap_color = lightmap_data.rgb * lightmap_height_fade;                                              "   \
"\n                                                                                                              "   \
"\n$define LIGHTMAP_VARYINGS \\"                                                                                     \
"\n    vec4 lightmap_coord                                                                                       "   \
"\n                                                                                                              "   \
"\n$define SAVE_LIGHTMAP_VARYINGS_VP(out_vert, view_pos_4) \\"                                                       \
"\n    out_vert.lightmap_coord.xyw = (lightmap_matrix * view_pos_4).xyw; \\"                                         \
"\n    out_vert.lightmap_coord.z = (viewworld_matrix * view_pos_4).z;                                            "   \
"\n                                                                                                              "   \
"\n$define GET_LIGHTMAP_ZTRICK(in_frag) \\"                                                                          \
"\n    /*const*/ float height_world_lm = in_frag.lightmap_coord.z; \\"                                               \
"\n    /*const*/ vec4 lightmap_data = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; \\"                    \
"\n    /*const*/ float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.075, 1.5), 0.0, 1.0); \\"   \
"\n    vec3 lightmap_color = lightmap_data.rgb * lightmap_height_fade;                                           "   \
"\n                                                                                                              "   \
"\n$define LIGHTMAP_SHADOW_TRICK(shadow_force) \\"                                                                   \
"\n    /*const*/ float lm_shadow_omit_coef = clamp(fma(lightmap_data.w, -0.2, 1.6), 0.0, 1.0); \\"                   \
"\n    lightmap_color *= fma(mix(shadow_force, 1.0, lm_shadow_omit_coef), 0.7, 0.3);                             "   \
"\n                                                                                                              "   \
"\n$define LIGHTMAP_BUILDING_HEIGHT_TRICK \\"                                                                        \
"\n    lightmap_color *= clamp(fma(height_world_lm, -2.5, 1.25), 0.0, 1.0);\n                                    "


#define  SHADOW_INCLUDE                                                                                                      \
    "\n float shadow_fs_main (float illum );   "                                                                             \
    "\n void  shadow_vs_main (vec4 viewpos);   "                                                                             \
    "\n$define SAVE_SHADOWS_VARYINGS_VP(ind, out_vert, view_pos_4) \\"                                                       \
    "\n    mat4 EyePlane##ind =  transpose(shadowMatrix##ind); \\"                                                           \
    "\n    out_vert##ind = vec4(dot( view_pos_4, EyePlane##ind[0]),dot( view_pos_4, EyePlane##ind[1] ),dot( view_pos_4, EyePlane##ind[2]),dot( view_pos_4, EyePlane##ind[3] ) );                                            "   \
    "\n           "  \
"$define GENERATE_SHADOW(ind,shadow_view) \\" \
    "\n  float shadowOrg##ind = shadow2D( shadowTexture##ind,shadow_view##ind .xyz+vec3(0.0,0.0,fZOffSet) ).r;                 \\" \
    "\n  float shadow0##ind = shadow2D( shadowTexture##ind ,shadow_view##ind .xyz+vec3(-fTexelSize,-fTexelSize,fZOffSet) ).r;   \\" \
    "\n  float shadow1##ind = shadow2D( shadowTexture##ind ,shadow_view##ind .xyz+vec3( fTexelSize,-fTexelSize,fZOffSet) ).r;   \\" \
    "\n  float shadow2##ind = shadow2D( shadowTexture##ind ,shadow_view##ind .xyz+vec3( fTexelSize, fTexelSize,fZOffSet) ).r;   \\" \
    "\n  float shadow3##ind = shadow2D( shadowTexture##ind ,shadow_view##ind .xyz+vec3(-fTexelSize, fTexelSize,fZOffSet) ).r;   \\" \
    "\n  float shadow##ind = ( 2.0*shadowOrg##ind + shadow0##ind + shadow1##ind + shadow2##ind + shadow3##ind)/6.0;                    \\" \
    "\n  float term##ind = map##ind*(1.0-shadow##ind);                                                                          \n\n     " 


#define INCLUDE_FUNCS                                                                                    \
        STRINGIFY (                                                                                      \
\n        float saturate( const in float x )                                                               \
\n        {                                                                                                \
\n            return clamp(x, 0.0, 1.0);                                                                   \
\n        }                                                                                                \
\n                                                                                                         \
\n        float lerp(float a, float b, float w)                                                            \
\n        {                                                                                                \
\n            return a + w*(b-a);                                                                          \
\n        }                                                                                                \
\n                                                                                                         \
\n        vec3 hardlight( const in vec3 color, const in vec3 hl )                                          \
\n        {                                                                                                \
\n            vec3 hl_pos = step(vec3(0.0), hl);                                                           \
\n            return (vec3(1.0) - hl_pos) * color * (hl + vec3(1.0)) +                                     \
\n                hl_pos * mix(color, vec3(1.0), hl);                                                      \
\n        }                                                                                                \
\n                                                                                                         \
\n        float tex_detail_factor( const in vec2 tex_c_mod, const in float coef )                          \
\n        {                                                                                                \
\n            vec2 grad_vec = fwidth(tex_c_mod);                                                           \
\n            float detail_fac = exp(coef * dot(grad_vec, grad_vec));                                      \
\n            return detail_fac * (2.0 - detail_fac);                                                      \
\n        }                                                                                                \
\n                                                                                                         \
\n        float ramp_up( const in float x )                                                                \
\n        {                                                                                                \
\n            return x * fma(x, -0.5, 1.5);                                                                \
\n        }                                                                                                \
\n                                                                                                         \
\n        float ramp_down( const in float x )                                                              \
\n        {                                                                                                \
\n            return x * fma(x, 0.5, 0.5);                                                                 \
\n        }                                                                                                \
\n                                                                                                         \
\n                                                                                                         \
        )

#define INCLUDE_FOG_FUNCS                                                                                        \
    STRINGIFY (                                                                                                  \
\n                float fog_decay_factor( const in vec3 view_pos )                                                 \
\n                {                                                                                                \
\n                    return exp(-/*fog_params*/SceneFogParams.a * dot(view_pos, view_pos));                       \
\n                }                                                                                                \
\n                vec3 apply_scene_fog( const in vec3 view_pos, const in vec3 color )                              \
\n                {                                                                                                \
\n                    vec3 view_vec_fog = (mat3(viewworld_matrix) * view_pos) * vec3(1.0, 1.0, 0.8);               \
\n                    /*return mix(texture(envTex, view_vec_fog).rgb, color, fog_decay_factor(view_vec_fog));*/    \
\n                    return mix(textureLod(envTex, view_vec_fog, 3.0).rgb, color, fog_decay_factor(view_vec_fog));   \
\n                }                                                                                                \
\n                                                                                                                 \
\n                vec3 apply_clear_fog( const in vec3 view_pos, const in vec3 color )                              \
\n                {                                                                                                \
\n                    return mix(/*fog_params*/SceneFogParams.rgb, color, fog_decay_factor(view_pos));             \
\n                }                                                                                                \
\n                                                                                                                 \
                )


#define INCLUDE_VS                                                                                     \
    STRINGIFY (                                                                                        \
\n    float luminance_crt( const in vec4 col )                                                         \
\n    {                                                                                                \
\n        const vec4 crt = vec4(0.299, 0.587, 0.114, 0.0);                                             \
\n        return dot(col,crt);                                                                         \
\n    }                                                                                                \
\n                                                                                                     \
\n                                                                                                     \
\n     uniform sampler2D       baseTexture;                                                            \
\n     uniform int             baseTextureUnit;                                                        \
\n     /*uniform sampler2DShadow shadowTexture0;*/                                                     \
\n     /*uniform int             shadowTextureUnit0;*/                                                 \
\n     uniform sampler2DShadow shadowTextureRGB;                                                       \
\n     /*uniform mat4            shadowMatrix0;*/                                                      \
\n     uniform mat4            lightmap_matrix;                                                        \
     )



#ifdef EXPERIMENTAL_RGB_CAM 
#define INCLUDE_PCF_EXT                                                                                    \
	STRINGIFY (                                                                                            \
    \n        float PCF4E(sampler2DShadow depths,vec4 stpq,ivec2 size){                                    \
    \n            float result = 0.0;                                                                      \
    \n            int   count = 0;                                                                         \
    \n            for(int x=-size.x; x<=size.x; x++){                                                      \
    \n                for(int y=-size.y; y<=size.y; y++){                                                  \
    \n                    count++;                                                                         \
    \n                    result += textureProjOffset(depths, stpq, ivec2(x,y));/*.r;*/                    \
    \n                }                                                                                    \
    \n            }                                                                                        \
    \n            return result/count;                                                                     \
    }                                                                                                      \
    \
    \n        float PCF4(sampler2DShadow depths,vec4 stpq,ivec2 size){                                     \
    \n            float result = 0.0;                                                                      \
    \n            result += textureProjOffset(depths, stpq, ivec2(0,-1));/*.r;*/                           \
    \n            result += textureProjOffset(depths, stpq, ivec2(0,1));/*.r;*/                            \
    \n            result += textureProjOffset(depths, stpq, ivec2(1,0));/*.r;*/                            \
    \n            result += textureProjOffset(depths, stpq, ivec2(-1,0));/*.r;*/                           \
    \n            return result*.25;                                                                       \
    }                                                                                                      \
    \
    \n        float PCF(sampler2DShadow depths,vec4 stpq,ivec2 size){                                      \
    \n            return textureProj(depths, stpq);/*.r;*/                                                 \
    }                                                                                                      \
    const ivec2 pcf_size = ivec2(1,1);                                                                     \
	\n                                                                                                     \
	\n                                                                                                     \
	\n        float PCF4E_Ext(sampler2DShadow depths,vec4 stpq, float aa){                                 \
	\n            return min(PCF4E(depths, stpq, pcf_size),PCF(shadowTextureRGB, stpq * .125,pcf_size)) * aa * 0.4;  \
	\n	}                                                                                                  \
	\n	                                                                                                   \
	\n        float PCF4_Ext(sampler2DShadow depths,vec4 stpq, float aa){                                  \
	\n            return min(PCF4(depths, stpq, pcf_size),PCF(shadowTextureRGB, stpq * .125,pcf_size)) * aa * 0.4;   \
	}                                                                                                      \
	\
	\n        float PCF_Ext(sampler2DShadow depths,vec4 stpq, float aa){                                   \
	\n            return min(PCF(depths, stpq, pcf_size),PCF(shadowTextureRGB, stpq * .125,pcf_size)) * aa * 0.4;    \
	}                                                                                                      \
	)
#else

#ifdef  ORIG_EPHEMERIS
#define INCLUDE_PCF_EXT                                                                                    \
	STRINGIFY (                                                                                            \
    \n        float PCF4E(sampler2DShadow depths,vec4 stpq,ivec2 size){                                    \
    \n            float result = 0.0;                                                                      \
    \n            int   count = 0;                                                                         \
    \n            float coeff;                                                                             \
    \n            for(int x=-size.x; x<=size.x; x++){                                                      \
    \n                for(int y=-size.y; y<=size.y; y++){                                                  \
    \n                    count++;                                                                         \
    \n                    result += textureProjOffset(depths, stpq, ivec2(x,y)) * .25;/*.r;*/              \
    \n                }                                                                                    \
    \n            }                                                                                        \
	\n                                                                                                     \
    \n            result += textureProjOffset(depths, stpq, ivec2(0)) *.75;                                \
    \n            return result/count;                                                                     \
    }                                                                                                      \
    \
    \n        float PCF4(sampler2DShadow depths,vec4 stpq,ivec2 size){                                     \
    \n            float result = 0.0;                                                                      \
    \n            result += textureProjOffset(depths, stpq, ivec2(0,-1));/*.r;*/                           \
    \n            result += textureProjOffset(depths, stpq, ivec2(0,1));/*.r;*/                            \
    \n            result += textureProjOffset(depths, stpq, ivec2(1,0));/*.r;*/                            \
    \n            result += textureProjOffset(depths, stpq, ivec2(-1,0));/*.r;*/                           \
    \n            return result*.25;                                                                       \
    }                                                                                                      \
    \
    \n        float PCF(sampler2DShadow depths,vec4 stpq,ivec2 size){                                      \
    \n            return textureProj(depths, stpq);/*.r;*/                                                 \
    }                                                                                                      \
    const ivec2 pcf_size = ivec2(1,1);                                                                     \
    \n                                                                                                     \
	\n                                                                                                     \
	\n        float PCF4E_Ext(sampler2DShadow depths,vec4 stpq, float aa){                                 \
	\n            return PCF4E(depths, stpq, pcf_size) * aa * 0.4;                                         \
	\n	}                                                                                                  \
	\n	                                                                                                   \
    \n        float PCF4_Ext(sampler2DShadow depths,vec4 stpq, float aa){                                  \
	\n            return PCF4(depths, stpq, pcf_size) * aa * 0.4;                                          \
	}                                                                                                      \
	                                                                                                       \
	\n        float PCF_Ext(sampler2DShadow depths,vec4 stpq, float aa){                                   \
	\n            return PCF(depths, stpq, pcf_size) * aa * 0.4;                                           \
	}                                                                                                      \
	)
#else
#define INCLUDE_PCF_EXT                                                                                    \
    STRINGIFY (                                                                                            \
    \n        float PCF4E(sampler2DShadow depths,vec4 stpq,ivec2 size){                                    \
    \n            float result = 0.0;                                                                      \
    \n            int   count = 0;                                                                         \
    \n            float coeff;                                                                             \
    \n            for(int x=-size.x; x<=size.x; x++){                                                      \
    \n                for(int y=-size.y; y<=size.y; y++){                                                  \
    \n                    count++;                                                                         \
    \n                    result += textureProjOffset(depths, stpq, ivec2(x,y)) * .25;/*.r;*/              \
    \n                }                                                                                    \
    \n            }                                                                                        \
    \n                                                                                                     \
    \n            result += textureProjOffset(depths, stpq, ivec2(0)) *.75;                                \
    \n            return result/count;                                                                     \
    }                                                                                                      \
    \
    \n        float PCF4(sampler2DShadow depths,vec4 stpq,ivec2 size){                                     \
    \n            float result = 0.0;                                                                      \
    \n            result += textureProjOffset(depths, stpq, ivec2(0,-1));/*.r;*/                           \
    \n            result += textureProjOffset(depths, stpq, ivec2(0,1));/*.r;*/                            \
    \n            result += textureProjOffset(depths, stpq, ivec2(1,0));/*.r;*/                            \
    \n            result += textureProjOffset(depths, stpq, ivec2(-1,0));/*.r;*/                           \
    \n            return result*.25;                                                                       \
    }                                                                                                      \
    \
    \n        float PCF(sampler2DShadow depths,vec4 stpq,ivec2 size){                                      \
    \n            return textureProj(depths, stpq);/*.r;*/                                                 \
    }                                                                                                      \
    const ivec2 pcf_size = ivec2(1,1);                                                                     \
    \n                                                                                                     \
    \n                                                                                                     \
    \n        float PCF4E_Ext(sampler2DShadow depths,vec4 stpq, float aa){                                 \
    \n            return PCF4E(depths, stpq, pcf_size) * aa;                                               \
    \n	}                                                                                                  \
    \n	                                                                                                   \
    \n        float PCF4_Ext(sampler2DShadow depths,vec4 stpq, float aa){                                  \
    \n            return PCF4(depths, stpq, pcf_size) * aa;                                                \
    }                                                                                                      \
    \
    \n        float PCF_Ext(sampler2DShadow depths,vec4 stpq, float aa){                                   \
    \n            return PCF(depths, stpq, pcf_size) * aa;                                                 \
    }                                                                                                      \
    )

#endif
#endif


/*vec4  get_shadow_coords(vec4 posEye, int index)                                                   \
{                                                                                                 \
return vec4(dot( posEye, gl_EyePlaneS[index]),dot( posEye, gl_EyePlaneT[index] ),dot( posEye, gl_EyePlaneR[index]),dot( posEye, gl_EyePlaneQ[index] ) );  \
} */                                                                                                 

#define INCLUDE_SCENE_PARAM                                                                             \
     STRINGIFY (                                                                                        \
\n        uniform vec4 ambient;                                                                         \
\n        uniform vec4 diffuse;                                                                         \
\n        uniform vec4 specular;                                                                        \
\n        uniform vec4 light_vec_view;                                                                  \
     )

#define INCLUDE_UNIFORMS                                                                                \
    STRINGIFY (                                                                                         \
\n        uniform sampler2D           ViewLightMap;                                                     \
\n        uniform sampler2D           detailTex;                                                        \
\n        uniform samplerCube         envTex;                                                           \
\n        uniform sampler2DShadow     ShadowSplit0;                                                     \
\n        uniform sampler2DShadow     ShadowSplit1;                                                     \
\n        uniform sampler2DShadow     ShadowSplit2;                                                     \
\n        uniform sampler2D           ViewDecalMap;                                                     \
\n        uniform vec4                SceneFogParams;                                                   \
        )

#define INCLUDE_COMPABILITY \
     STRINGIFY(             \
        in vec4 osg_Vertex; \
        in vec3 osg_Normal; \
     )

#undef INCLUDE_COMPABILITY
#define INCLUDE_COMPABILITY

#define INCLUDE_DL                                                                                         \
      "\n #extension GL_ARB_texture_rectangle : enable"                                                    \
    STRINGIFY (                                                                                            \
\n    \
\n    /*const int nMaxLights = 300; */                                                                         \
\n    \
\n    \
\n    uniform int LightsActiveNum;                                                                         \
\n    \
\n    /*uniform vec4 LightVSPosAmbRatio[nMaxLights];*/                                                     \
\n    /*uniform vec4 LightVSDirSpecRatio[nMaxLights];*/                                                    \
\n    /*uniform vec4 LightAttenuation[nMaxLights];*/                                                       \
\n    /*uniform vec4 LightDiffuseNormalCoeff[nMaxLights]; */                                                          \
\n    uniform sampler2DRect lightsBuffer;                                                                  \
\n   \
\n   void ComputeDynamicLights( in vec3 vViewSpacePoint, in vec3 vViewSpaceNormal, in vec3 vReflVec, inout vec3 cAmbDiff, inout vec3 cSpecular ) \
\n   {                                                                                                     \
\n   int curLight = 0;                                                                                     \
\n   cAmbDiff = vec3(0.0f,0.0f,0.0f);                                                                      \
\n   cSpecular = vec3(0.0f,0.0f,0.0f);                                                                     \
\n   while (curLight < LightsActiveNum)                                                                    \
\n       {                                                                                                 \
\n                                                                                                         \
\n       vec2 coord = vec2((curLight % 4096) * 4.0, curLight / 4096);                                      \
\n                                                                                                         \
\n       vec4 curVSPosAmbRatio  = textureOffset(lightsBuffer, coord, ivec2 (0, 0));                        \
\n       vec4 curVSDirSpecRatio = textureOffset(lightsBuffer, coord, ivec2 (1, 0));                        \
\n       vec4 curAttenuation    = textureOffset(lightsBuffer, coord, ivec2 (2, 0));                        \
\n       vec4 curDiffuseNC      = textureOffset(lightsBuffer, coord, ivec2 (3, 0));                        \
\n                                                                                                         \
\n       /*vec4 curVSPosAmbRatio  = LightVSPosAmbRatio[curLight];*/                                        \
\n       /*vec4 curVSDirSpecRatio = LightVSDirSpecRatio[curLight];*/                                       \
\n       /*vec4 curAttenuation    = LightAttenuation[curLight];*/                                          \
\n       /*vec3 curDiffuseNC      = LightDiffuseNormalCoeff[curLight];*/                                   \
\n                                                                                                         \
\n       vec3  vVecToLight = curVSPosAmbRatio.xyz - vViewSpacePoint;                                       \
\n       float vDistToLightInv = inversesqrt(dot(vVecToLight, vVecToLight));                               \
\n       vec3  vDirToLight = vDistToLightInv * vVecToLight;                                                \
\n       \
\n       float fAngleDot = dot(vDirToLight, curVSDirSpecRatio.xyz);                                        \
\n       float fTotalAtt = clamp(curAttenuation.z * fAngleDot + curAttenuation.w, 0.0, 1.0);               \
\n                                                                                                         \
\n       fTotalAtt *= clamp(curAttenuation.x * vDistToLightInv + curAttenuation.y, 0.0, 1.0);              \
\n       \
\n       if (fTotalAtt != 0.0)                                                                             \
\n           {                                                                                             \
\n           \
\n           float fDiffuseDot = dot(vDirToLight, vViewSpaceNormal);                                       \
\n           cAmbDiff += (fTotalAtt * (curVSPosAmbRatio.w * smoothstep(-0.2, 0.2 , clamp(fDiffuseDot + curDiffuseNC.w * 2, 0.0, 1.0)) + clamp(fDiffuseDot, 0.0, 1.0))) * curDiffuseNC.xyz;   \
\n           \
\n           float fSpecPower = clamp(dot(vReflVec, vDirToLight), 0.0, 1.0);                               \
\n           fSpecPower *= fSpecPower;                                                                     \
\n           fSpecPower *= fSpecPower;                                                                     \
\n           cSpecular += (fTotalAtt * curVSDirSpecRatio.w * fSpecPower) * curDiffuseNC.xyz;               \
\n           }                                                                                             \
\n                                                                                                         \
\n                                                                                                         \
\n           ++curLight;                                                                                   \
\n       }                                                                                                 \
\n                                                                                                         \
\n                                                                                                         \
\n       return;                                                                                           \
\n   }                                                                                                     \
   )        

#define INCLUDE_DL2                                                                                      \
    STRINGIFY (                                                                                          \
\n                                                                                                         \
\n    const float PI = 3.14159265358979323846264;                                                          \
\n                                                                                                         \
\n   void compute_dynamic_lights( in vec3 vViewSpacePoint, in vec3 vViewSpaceNormal, in vec3 vReflVec, inout vec3 cAmbDiff, inout vec3 cSpecular ) \
\n   {                                                                                                     \
\n   int curLight = 0;                                                                                     \
\n   cAmbDiff = vec3(0.0f,0.0f,0.0f);                                                                      \
\n   cSpecular = vec3(0.0f,0.0f,0.0f);                                                                     \
\n   while (curLight < LightsActiveNum)                                                                    \
\n       {                                                                                                 \
\n       vec4 curVSPosAmbRatio  = LightVSPosAmbRatio[curLight];                                            \
\n       vec4 curVSDirSpecRatio = LightVSDirSpecRatio[curLight];                                           \
\n       vec4 curAttenuation    = LightAttenuation[curLight];                                              \
\n       vec4 curDiffuseNC      = LightDiffuseNormalCoeff[curLight];                                       \
\n                                                                                                         \
\n                                                                                                         \
\n        vec4 specular_ =  vec4(curDiffuse * curVSDirSpecRatio.w,1.0);                                    \
\n        vec3 vVecToLight = curVSPosAmbRatio.xyz - vViewSpacePoint;                                       \
\n        float vDistToLightInv = inversesqrt(dot(vVecToLight, vVecToLight));                              \
\n        vec3 vDirToLight = vDistToLightInv * vVecToLight;                                                \
\n                                                                                                         \
\n        float fAngleDot = dot(vDirToLight, curVSDirSpecRatio.xyz);                                       \
\n        float fTotalAtt = clamp(curAttenuation.z * fAngleDot + curAttenuation.w, 0.0, 1.0);              \
\n                                                                                                         \
\n        /*float*/ fTotalAtt = clamp(curAttenuation.x * vDistToLightInv + curAttenuation.y, 0.0, 1.0);    \
\n                                                                                                         \
\n        float intensity = 0.0;                                                                           \
\n        vec4 spec = vec4(0.0);                                                                           \
\n        vec3 ld = normalize(vVecToLight);                                                                \
\n        vec3 sd = normalize(vec3(-curVSDirSpecRatio.xyz));                                               \
\n                                                                                                         \
\n        if (dot(ld,sd) > cos(PI/4)) {                                                        \
\n                                                                                                         \
\n                    vec3 n = normalize(vViewSpaceNormal);                                                \
\n                    intensity = max(dot(n,ld), 0.0);                                                     \
\n                                                                                                         \
\n                        if (intensity > 0.0) {                                                           \
\n                                vec3 eye = normalize(vViewSpacePoint);                                   \
\n                                vec3 h = normalize(ld + eye);                                            \
\n                                float intSpec = max(dot(h,n), 0.0);                                      \
\n                                spec = specular_ * pow(intSpec, 1/*shininess*/);                         \
\n                            }                                                                            \
\n                }                                                                                        \
\n                                                                                                         \
\n           cAmbDiff += fTotalAtt *(intensity * curDiffuseNC.xyz + spec.rgb);                             \
\n                                                                                                         \
\n                                                                                                         \
\n           ++curLight;                                                                                   \
\n       }                                                                                                 \
\n                                                                                                         \
\n       return;                                                                                           \
\n   }                                                                                                     \
   )  

//#undef INCLUDE_DL
//#define INCLUDE_DL
#undef INCLUDE_DL2
#define INCLUDE_DL2

    }                                                                                                  

    namespace plane_mat
    {

    const char* vs = {
        "#extension GL_ARB_gpu_shader5 : enable \n"
        "//       plane_mat \n"

        INCLUDE_VS
        INCLUDE_COMPABILITY
        "\n"       
        LIGHT_MAPS
        SHADOW_INCLUDE

        STRINGIFY ( 
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

        void main()
        {
			vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
            vec4 viewpos = gl_ModelViewMatrix * gl_Vertex;
            
            //viewworld_matrix = mat4(vec4(1.0,0.0,0.0,0.0),vec4(0.0,1.0,0.0,0.0),vec4(0.0,0.0,1.0,0.0),vec4(0.0,0.0,0.0,1.0));
            viewworld_matrix = inverse(gl_ModelViewMatrix); 

            gl_Position    = gl_ModelViewProjectionMatrix *  gl_Vertex;

            v_out.tangent   = tangent;
            v_out.binormal  = binormal;
            v_out.normal    = normal;
            v_out.vnormal   = mat3(gl_ModelViewMatrix) * normal;
            v_out.viewpos   = viewpos.xyz;
            v_out.texcoord  = gl_MultiTexCoord1.xy;
            //v_out.shadow_view = get_shadow_coords(viewpos, shadowTextureUnit0);
            
            //mat4 EyePlane =  transpose(shadowMatrix0); 
            //v_out.shadow_view = vec4(dot( viewpos, EyePlane[0]),dot( viewpos, EyePlane[1] ),dot( viewpos, EyePlane[2]),dot( viewpos, EyePlane[3] ) );
            shadow_vs_main(viewpos);

            SAVE_LIGHTMAP_VARYINGS_VP(v_out, viewpos);
        }
    )
    };


    const char* fs = {
        "#extension GL_ARB_gpu_shader5 : enable \n"
		"#extension GL_ARB_gpu_shader_fp64 : enable \n"
        "//       plane_mat \n"

        INCLUDE_UNIFORMS

        STRINGIFY ( 
\n    
        // layout(early_fragment_tests) in;
\n
\n        in mat4 viewworld_matrix;
        )

        INCLUDE_FUNCS
        INCLUDE_FOG_FUNCS
        INCLUDE_VS
//		INCLUDE_PCF_EXT
"\n"
        INCLUDE_DL
"\n"
        INCLUDE_DL2
        INCLUDE_SCENE_PARAM
"\n"       
        LIGHT_MAPS
        SHADOW_INCLUDE

        STRINGIFY ( 

\n        
\n      uniform sampler2D colorTex;
\n      uniform sampler2D normalTex;
\n       
\n        
\n
\n        in block
\n        {
\n            vec2 texcoord;
\n            vec3 normal;
\n            vec3 vnormal;
\n            vec3 tangent;
\n            vec3 binormal;
\n            vec3 viewpos;
\n            vec4 shadow_view;
\n            vec4 lightmap_coord;
\n       } f_in;
\n
\n        out vec4  aFragColor;
\n        
\n        uniform float zShadow0; 
\n
\n        void main (void)
\n        {
$if 0
\n
\n            float fTexelSize=0.00137695;
\n            float fZOffSet  = -0.001954;
\n            float testZ = gl_FragCoord.z*2.0-1.0;
\n            float map0 = step(testZ, zShadow0);
\n            float shadowOrg0 = shadow2D( shadowTexture0,f_in.shadow_view.xyz+vec3(0.0,0.0,fZOffSet) ).r;
\n            float shadow0 = shadowOrg0;
\n            float term0 = map0*(1.0-shadow0); 
\n            float v = clamp(term0,0.0,1.0);
\n            float shadow = 1 - v * 0.5;;
$endif
\n
              // GET_SHADOW(f_in.viewpos, f_in);
              // #define GET_SHADOW(viewpos, in_frag)   
              // float shadow = 1.0; 
              // if(ambient.a > 0.35)
              //     shadow = PCF_Ext(shadowTexture0, f_in.shadow_view,ambient.a);
\n
\n            float shadow =  shadow_fs_main(ambient.a);
\n
\n            vec4 base = texture2D(colorTex, f_in.texcoord.xy);
\n            vec3 bump = fma(texture2D(normalTex, f_in.texcoord.xy).xyz, vec3(2.0), vec3(-1.0));
\n            //vec3 bump = texture2D(normalTex, gl_TexCoord[0].xy).xyz;
\n            //bump = normalize(bump * 2.0 - 1.0);
\n            vec3  normal       = normalize(bump.x * f_in.tangent + bump.y * f_in.binormal + bump.z * f_in.normal);
\n            vec4  dif_tex_col  = texture2D(colorTex,f_in.texcoord.xy, -1.0);
\n            float glass_factor = 1.0 - dif_tex_col.a;
\n
\n            // get dist to point and normalized to-eye vector
\n            float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
\n            float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
\n            float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
\n            vec3  to_eye          = -dist_to_pnt_rcp * f_in.viewpos;
\n
\n            vec3 view_up_vec = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
\n            float normal_world_space_z = dot(view_up_vec, normal);
\n                                
\n
\n            float incidence_dot  = dot(to_eye, normal);
\n            float pow_fr         = pow(saturate(1.0 - incidence_dot), 3.0);
\n            vec3  refl_vec_view  = -to_eye + (2.0 * incidence_dot) * normal;
\n            vec3  refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
\n            float refl_min       = fma(glass_factor, 0.275, 0.125);
\n            float half_refl_z    = 0.5 * (refl_vec_world.z + normal_world_space_z);
\n            float fresnel        = mix(refl_min, 0.97, pow_fr) * fma(half_refl_z, 0.15, fma(glass_factor, 0.6, 0.25)); 
\n
\n            float n_dot_l = shadow * saturate(dot(normal, light_vec_view.xyz));
\n            float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), 44.0) * 0.9;
\n            vec3  pure_spec_color = specular.rgb * specular_val;
\n            float spec_compose_fraction = 0.35;
\n
\n
\n            //const vec3 cube_color = texture(Env, refl_vec_world).rgb + pure_spec_color;
\n            vec3 cube_color = texture(envTex, refl_vec_world).rgb + pure_spec_color;
\n
\n
\n            vec3 non_ambient_term = diffuse.rgb * n_dot_l + spec_compose_fraction * pure_spec_color;
\n
\n          // Apply spot lights
\n            vec3 vLightsSpecAddOn;
\n            vec3 light_res;  
\n                  
\n            ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, /*vec3(0)*/f_in.normal, light_res, vLightsSpecAddOn);
\n            vec3 lightmap_color = light_res ; 
\n
\n            //GET_LIGHTMAP(f_in.viewpos, f_in);
\n
\n            float up_dot_clamped = saturate(fma(normal_world_space_z, 0.55, 0.45));
\n            non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);
\n
\n            float ao_trick = fma(up_dot_clamped, 0.4, 0.6);
\n            vec3  composed_lighting = ao_trick * ambient.rgb + non_ambient_term;
\n            vec3  day_result = mix(composed_lighting * dif_tex_col.rgb, cube_color, fresnel) + (1.0 - spec_compose_fraction) * pure_spec_color;
\n            float night_factor = step(ambient.a, 0.35);
\n            vec3  result = mix(day_result, vec3(0.90, 0.90, 0.86), night_factor * glass_factor);
\n
\n            aFragColor = vec4(apply_scene_fog(f_in.viewpos, result), 1.0); // vec4(1.0,0.0,0.0,1.0); //
        }
    )

    };

    SHADERS_GETTER(get_shader,vs, fs)

     AUTO_REG_NAME(plane, shaders::plane_mat::get_shader)

    } // ns plane_mat

    namespace rotor_mat
    {

        const char* vs = {  
            "#extension GL_ARB_gpu_shader5 : enable \n"
            "//       rotor_mat \n"

            INCLUDE_VS
            INCLUDE_COMPABILITY
            
            SHADOW_INCLUDE

            STRINGIFY ( 

            //varying   vec3  lightDir;
            out mat4 viewworld_matrix;

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 vnormal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                vec4 viewpos = gl_ModelViewMatrix * gl_Vertex;
                viewworld_matrix = inverse(gl_ModelViewMatrix);
                gl_Position = gl_ModelViewProjectionMatrix *  gl_Vertex;

                v_out.normal    = normal;
                v_out.vnormal   = mat3(gl_ModelViewMatrix) * normal;
                v_out.viewpos   = viewpos.xyz;
                v_out.texcoord  = gl_MultiTexCoord1.xy;
                //v_out.shadow_view = get_shadow_coords(viewpos, shadowTextureUnit0);
                
                //mat4 EyePlane =  transpose(shadowMatrix0); 
                //v_out.shadow_view = vec4(dot( viewpos, EyePlane[0]),dot( viewpos, EyePlane[1] ),dot( viewpos, EyePlane[2]),dot( viewpos, EyePlane[3] ) );
                shadow_vs_main(viewpos);
            }       
            )
        };


        const char* fs = {

            "#extension GL_ARB_gpu_shader5 : enable \n "
            "//       rotor_mat \n"

            INCLUDE_UNIFORMS

            STRINGIFY ( 

            in mat4 viewworld_matrix;
            )

            INCLUDE_FUNCS

            INCLUDE_FOG_FUNCS

            INCLUDE_VS
//			INCLUDE_PCF_EXT
            INCLUDE_SCENE_PARAM
            
            SHADOW_INCLUDE

            STRINGIFY ( 

            uniform sampler2D       colorTex;
            uniform sampler2D       nightTex;

            in block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 vnormal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } f_in;
            
            out vec4  aFragColor;

            void main (void)
            {
                //float shadow = 1.0; 
                //if(ambient.a > 0.35)
                //    shadow = PCF_Ext(shadowTexture0, f_in.shadow_view, ambient.a);
                float shadow =  shadow_fs_main(ambient.a);

                vec3 normal = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
                float n_dot_l = shadow * saturate(fma(dot(normal, light_vec_view.xyz), 0.5, 0.5));

                vec4 dif_tex_col = texture2D(colorTex, f_in.texcoord);
                vec3 result = (ambient.rgb + diffuse.rgb * n_dot_l) * dif_tex_col.rgb;

                aFragColor = vec4(apply_scene_fog(f_in.viewpos, result), dif_tex_col.a);

            }

            )

        };   


        SHADERS_GETTER(get_shader,vs, fs)

        AUTO_REG_NAME(rotor, shaders::rotor_mat::get_shader)

    } // ns rotor_mat

    namespace default_mat 
    {
       const char* vs = {  
           "#extension GL_ARB_gpu_shader5 : enable \n"
		   "//       default_mat \n"
           
		   INCLUDE_VS
           INCLUDE_COMPABILITY
           "\n"       
           LIGHT_MAPS
           SHADOW_INCLUDE

           STRINGIFY ( 
\n
\n           attribute vec3 tangent;
\n           attribute vec3 binormal;
\n           out mat4 viewworld_matrix;
\n
\n           out block
\n           {
\n               vec2 texcoord;
\n               vec3 normal;
\n               vec3 vnormal;
\n               vec3 tangent;
\n               vec3 binormal;
\n               vec3 viewpos;
\n               vec4 shadow_view;
\n               vec4 lightmap_coord;
\n           } v_out;
\n
\n           void main()
\n           {
\n               vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
\n               //mat3 rotation = mat3(tangent, binormal, normal);
\n               vec4 viewpos = gl_ModelViewMatrix * gl_Vertex;
\n               viewworld_matrix = inverse(gl_ModelViewMatrix);
\n               gl_Position = gl_ModelViewProjectionMatrix *  gl_Vertex;
\n
\n               v_out.tangent   = tangent;
\n               v_out.binormal  = cross(tangent, normal);  //binormal;
\n               v_out.normal    = normal;
\n               v_out.vnormal   = mat3(gl_ModelViewMatrix) * normal;
\n               v_out.viewpos   = viewpos.xyz;
\n               v_out.texcoord  = gl_MultiTexCoord1.xy;
\n               //v_out.shadow_view = get_shadow_coords(viewpos, shadowTextureUnit0);
\n               //mat4 EyePlane =  transpose(shadowMatrix0); 
\n               //v_out.shadow_view = vec4(dot( viewpos, EyePlane[0]),dot( viewpos, EyePlane[1] ),dot( viewpos, EyePlane[2]),dot( viewpos, EyePlane[3] ) );
\n               shadow_vs_main(viewpos);
\n
\n               SAVE_LIGHTMAP_VARYINGS_VP(v_out, viewpos);
\n           }
       )
       };


       const char* fs = {
       
       "#extension GL_ARB_gpu_shader5 : enable \n "
 	   "//       default_mat \n"

       INCLUDE_UNIFORMS

       STRINGIFY ( 

\n          in mat4 viewworld_matrix;
       )
       
       INCLUDE_FUNCS
       INCLUDE_FOG_FUNCS
       INCLUDE_VS
//	   INCLUDE_PCF_EXT
       INCLUDE_DL
       INCLUDE_DL2
       INCLUDE_SCENE_PARAM
"\n"      
       LIGHT_MAPS
       SHADOW_INCLUDE

       STRINGIFY ( 
\n
\n           uniform sampler2D colorTex;
\n           uniform sampler2D normalTex;
\n
\n           in block
\n           {
\n               vec2 texcoord;
\n               vec3 normal;
\n               vec3 vnormal;
\n               vec3 tangent;
\n               vec3 binormal;
\n               vec3 viewpos;
\n               vec4 shadow_view;
\n               vec4 lightmap_coord;
\n           } f_in;
\n
\n           out vec4  aFragColor;
\n
\n           void main (void)
\n           {
\n               // GET_SHADOW(f_in.viewpos, f_in);
\n               //float shadow = 1.0; 
\n               //if(ambient.a > 0.35)
\n               //    shadow = PCF_Ext(shadowTexture0, f_in.shadow_view, ambient.a);
\n               
\n               float shadow =  shadow_fs_main(ambient.a);
\n
\n               vec4 base = texture2D(colorTex, f_in.texcoord);
\n               vec3 bump = fma(texture2D(normalTex, f_in.texcoord).xyz, vec3(2.0), vec3(-1.0));
\n               //vec3 bump = texture2D(normalTex, f_in.texcoord).xyz;
\n               //bump = normalize(bump * 2.0 - 1.0);
\n               vec3  normal       = normalize(bump.x * f_in.tangent + bump.y * f_in.binormal + bump.z * f_in.normal);
\n               vec4  dif_tex_col  = texture2D(colorTex,f_in.texcoord, -1.0);
\n               float glass_factor = /*1.0 - dif_tex_col.a*/0;
\n
\n               // get dist to point and normalized to-eye vector
\n               float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
\n               float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
\n               float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
\n               vec3  to_eye          = -dist_to_pnt_rcp * f_in.viewpos;
\n
\n               vec3 view_up_vec = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
\n               float normal_world_space_z = dot(view_up_vec, normal);
\n
\n
\n               float incidence_dot  = dot(to_eye, normal);
\n               float pow_fr         = pow(saturate(1.0 - incidence_dot), 3.0);
\n               vec3  refl_vec_view  = -to_eye + (2.0 * incidence_dot) * normal;
\n               vec3  refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
\n               float refl_min       = 0.10 + glass_factor * 0.30;
\n               float half_refl_z    = 0.5 * (refl_vec_world.z + normal_world_space_z);
\n               float fresnel        = mix(refl_min, 0.6, pow_fr) * fma(half_refl_z, 0.15, fma(glass_factor, 0.6, 0.)); 
\n
\n               float n_dot_l = shadow * saturate(dot(normal, light_vec_view.xyz));
\n               float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), 44.0) * 0.9;
\n               vec3  pure_spec_color = specular.rgb * specular_val;
\n               float spec_compose_fraction = 0.35;
\n
\n
\n               // const vec3 cube_color = texture(Env, refl_vec_world).rgb + pure_spec_color;
\n               vec3 cube_color = texture(envTex, refl_vec_world).rgb + pure_spec_color;
\n
\n               vec3 non_ambient_term = diffuse.rgb * n_dot_l + spec_compose_fraction * pure_spec_color;
\n
\n             // Apply spot lights
\n             vec3 vLightsSpecAddOn;
\n             vec3 light_res;  
\n                  
\n               // ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, /*vec3(0)*/f_in.normal, light_res, vLightsSpecAddOn);
\n             // vec3 lightmap_color = light_res ; 
\n
\n             GET_LIGHTMAP(f_in.viewpos, f_in);
\n
\n               float up_dot_clamped = saturate(fma(normal_world_space_z, 0.55, 0.45));
\n               non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);
\n
\n               float ao_trick = fma(up_dot_clamped, 0.4, 0.6);
\n               vec3  composed_lighting = ao_trick * ambient.rgb + non_ambient_term;
\n               vec3  day_result = mix(composed_lighting * dif_tex_col.rgb, cube_color, fresnel) + (1.0 - spec_compose_fraction) * pure_spec_color;
\n               float night_factor = step(ambient.a, 0.35);
\n               vec3  result = mix(day_result, vec3(0.90, 0.90, 0.86), night_factor * glass_factor);
\n
\n               aFragColor = vec4(apply_scene_fog(f_in.viewpos, result), dif_tex_col.a);
\n			     aFragColor = vec4(dif_tex_col);
\n               //aFragColor = vec4(1.0,0.0,0.0,1.0);
\n           }
       )

       };   

       SHADERS_GETTER(get_shader, vs, fs)

       AUTO_REG_NAME(default, shaders::default_mat::get_shader)

    }  // ns default_mat

    namespace static_mat 
    {
       const char* vs = {  
           "#extension GL_ARB_gpu_shader5 : enable \n"
		   "//       statmat_mat \n"
           
		   INCLUDE_VS
           INCLUDE_COMPABILITY
           "\n"       
           LIGHT_MAPS
           SHADOW_INCLUDE

           STRINGIFY ( 
\n
\n           attribute vec3 tangent;
\n           attribute vec3 binormal;
\n           out mat4 viewworld_matrix;
\n
\n           out block
\n           {
\n               vec2 texcoord;
\n               vec3 normal;
\n               vec3 vnormal;
\n               vec3 tangent;
\n               vec3 binormal;
\n               vec3 viewpos;
\n               vec4 shadow_view;
\n               vec4 lightmap_coord;
\n               vec2 detail_uv;
\n           } v_out;
\n
\n           void main()
\n           {
\n               vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
\n               // mat3 rotation = mat3(tangent, binormal, normal);
\n               vec4 viewpos = gl_ModelViewMatrix * gl_Vertex;
\n               viewworld_matrix = inverse(gl_ModelViewMatrix);
\n               gl_Position = gl_ModelViewProjectionMatrix *  gl_Vertex;
\n
\n               v_out.tangent   = tangent;
\n               v_out.binormal  = binormal;//cross(normal,tangent);
\n               v_out.normal    = normal;
\n               v_out.vnormal   = mat3(gl_ModelViewMatrix) * normal;
\n               v_out.viewpos   = viewpos.xyz;
\n               v_out.texcoord  = gl_MultiTexCoord1.xy;
\n               v_out.detail_uv = gl_Vertex.xy;// * 0.03;
\n               shadow_vs_main(viewpos);
\n
\n               SAVE_LIGHTMAP_VARYINGS_VP(v_out, viewpos);
\n           }       
       )
       };


       const char* fs = {
       
       "#extension GL_ARB_gpu_shader5 : enable \n "
 	   "//       static_mat \n"

       INCLUDE_UNIFORMS

       STRINGIFY ( 

\n          in mat4 viewworld_matrix;
       )
       
       INCLUDE_FUNCS
       INCLUDE_FOG_FUNCS
       INCLUDE_VS

	   INCLUDE_DL
       INCLUDE_DL2
       INCLUDE_SCENE_PARAM
"\n"      
       LIGHT_MAPS
       SHADOW_INCLUDE

       STRINGIFY ( 
\n
\n           uniform sampler2D colorTex;
\n           uniform sampler2D normalTex;
\n           uniform vec4 eCol;
\n           uniform vec4 aCol; 
\n           uniform vec4 dCol;
\n           uniform vec4 sCol; 
\n           in block
\n           {
\n               vec2 texcoord;
\n               vec3 normal;
\n               vec3 vnormal;
\n               vec3 tangent;
\n               vec3 binormal;
\n               vec3 viewpos;
\n               vec4 shadow_view;
\n               vec4 lightmap_coord;
\n               vec2 detail_uv;
\n           } f_in;
\n
\n           out vec4  aFragColor;
\n
\n           void main (void)
\n           {
\n               float shadow =  shadow_fs_main(ambient.a);
\n
\n               // vec4 base = texture2D(colorTex, f_in.texcoord);
\n               vec3 bump = fma(texture2D(normalTex, f_in.texcoord).xyz, vec3(2.0), vec3(-1.0));
\n               //vec3 bump = texture2D(normalTex, f_in.texcoord).xyz;
\n               //bump = normalize(bump * 2.0 - 1.0);
\n               vec3  normal       = normalize(bump.x * f_in.tangent + bump.y * f_in.binormal + bump.z * f_in.normal);
\n               vec4  dif_tex_col  = dCol; // texture2D(colorTex,f_in.texcoord, -1.0);
\n               float glass_factor = 1.0 - dif_tex_col.a /*0*/;
\n
\n               // float detail_factor = tex_detail_factor(f_in.texcoord * textureSize2D(colorTex, 0), -0.02);
\n               // vec3 normal_noise = vec3(0.0);
\n               // if (detail_factor > 0.01)
\n               // {
\n               //     normal_noise = detail_factor * fma(texture2D(detailTex, f_in.detail_uv).rgb, vec3(0.6), vec3(-0.3));
\n               //     dif_tex_col.rgb = hardlight(dif_tex_col.rgb, normal_noise.ggg);
\n               // }
\n               // normal = normalize(0.8 * f_in.normal + (normal_noise.x * f_in.tangent + normal_noise.y * f_in.binormal));
\n
\n               // get dist to point and normalized to-eye vector
\n               float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
\n               float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
\n               float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
\n               vec3  to_eye          = -dist_to_pnt_rcp * f_in.viewpos;
\n
\n               vec3 view_up_vec = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
\n               float normal_world_space_z = dot(view_up_vec, normal);
\n
\n
\n               float incidence_dot  = dot(to_eye, normal);
\n               float pow_fr         = pow(saturate(1.0 - incidence_dot), 3.0);
\n               vec3  refl_vec_view  = -to_eye + (2.0 * incidence_dot) * normal;
\n               vec3  refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
\n               float refl_min       = 0.10 + glass_factor * 0.30;
\n               float half_refl_z    = 0.5 * (refl_vec_world.z + normal_world_space_z);
\n               float fresnel        = mix(refl_min, 0.6, pow_fr) * fma(half_refl_z, 0.15, fma(glass_factor, 0.6, 0.)); 
\n
\n               float n_dot_l = shadow * saturate(dot(normal, light_vec_view.xyz));
\n               float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), 44.0) * 0.9;
\n               vec3  pure_spec_color = specular.rgb * specular_val;
\n               float spec_compose_fraction = 0.35;
\n
\n
\n               // const vec3 cube_color = texture(Env, refl_vec_world).rgb + pure_spec_color;
\n               vec3 cube_color = texture(envTex, refl_vec_world).rgb + pure_spec_color;
\n
\n               vec3 non_ambient_term = diffuse.rgb * n_dot_l + spec_compose_fraction * pure_spec_color;
\n
\n             // Apply spot lights
\n             vec3 vLightsSpecAddOn;
\n              vec3 light_res;  
\n                  
\n               // ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, /*vec3(0)*/f_in.normal, light_res, vLightsSpecAddOn);
\n             // vec3 lightmap_color = light_res ; 
\n
\n             GET_LIGHTMAP(f_in.viewpos, f_in);
\n
\n               float up_dot_clamped = saturate(fma(normal_world_space_z, 0.55, 0.45));
\n               non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);
\n
\n               float ao_trick = fma(up_dot_clamped, 0.4, 0.6);
\n               vec3  composed_lighting = ao_trick * ambient.rgb + non_ambient_term;
\n               vec3  day_result = mix(composed_lighting * dif_tex_col.rgb, cube_color, fresnel) + (1.0 - spec_compose_fraction) * pure_spec_color;
\n               float night_factor = step(ambient.a, 0.35);
\n               vec3  result = mix(day_result, vec3(0.90, 0.90, 0.86), night_factor * glass_factor);
\n
\n               aFragColor = vec4(apply_scene_fog(f_in.viewpos, result), 1.0);
\n			     // aFragColor = vec4(1.0,0.0,0.0,1.0);
\n               // aFragColor = vec4(bump,1.0);    
\n           }
       )

       };   

       SHADERS_GETTER(get_shader, vs, fs)

       AUTO_REG_NAME(statmat, shaders::static_mat::get_shader)
       AUTO_REG_NAME(color  , shaders::static_mat::get_shader)

    }  // ns static_mat	
	
	

    namespace building_mat 
    {
        const char* vs = {  
			
            "#extension GL_ARB_gpu_shader5 : enable \n"
 		    "//       building_mat \n"

            INCLUDE_VS
            INCLUDE_COMPABILITY
            
            "\n"       
            LIGHT_MAPS
            SHADOW_INCLUDE

            STRINGIFY ( 
\n            
\n          out mat4 viewworld_matrix;
\n
\n            out block
\n            {
\n                vec2 texcoord;
\n                vec3 normal;
\n                vec3 vnormal;
\n                vec3 viewpos;
\n                vec4 shadow_view;
\n                vec4 lightmap_coord;
\n            } v_out;
\n             
\n            void main()
\n            {
\n                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
\n                vec4 viewpos = gl_ModelViewMatrix * gl_Vertex;
\n                viewworld_matrix = inverse(gl_ModelViewMatrix);
\n                gl_Position = gl_ModelViewProjectionMatrix *  gl_Vertex;

                v_out.normal    = normal;
                v_out.vnormal   = mat3(gl_ModelViewMatrix) * normal;
                v_out.viewpos   = viewpos.xyz;
                v_out.texcoord  = gl_MultiTexCoord1.xy;
                //v_out.shadow_view = get_shadow_coords(viewpos, shadowTextureUnit0);
                //mat4 EyePlane =  transpose(shadowMatrix0); 
                //v_out.shadow_view = vec4(dot( viewpos, EyePlane[0]),dot( viewpos, EyePlane[1] ),dot( viewpos, EyePlane[2]),dot( viewpos, EyePlane[3] ) );
                shadow_vs_main(viewpos);
                SAVE_LIGHTMAP_VARYINGS_VP(v_out, viewpos);
            }       
            )
        };


        const char* fs = { 
            
            "#extension GL_ARB_gpu_shader5 : enable \n "
 		    "//       building_mat \n"

            INCLUDE_UNIFORMS

            STRINGIFY ( 

             in mat4 viewworld_matrix;
            )

            INCLUDE_FUNCS
            INCLUDE_FOG_FUNCS
            INCLUDE_VS
//			INCLUDE_PCF_EXT
            INCLUDE_DL
            INCLUDE_DL2
            INCLUDE_SCENE_PARAM
"\n"            
            LIGHT_MAPS
            SHADOW_INCLUDE

            STRINGIFY ( 
\n
\n            uniform sampler2D           colorTex;
\n            uniform sampler2D           nightTex;
\n
\n            in block
\n            {
\n                vec2 texcoord;
\n                vec3 normal;
\n                vec3 vnormal;
\n                vec3 viewpos;
\n                vec4 shadow_view;
\n                vec4 lightmap_coord;
\n            } f_in;
\n            
\n            out vec4  aFragColor;
\n
\n            void main (void)
\n            {
\n                // GET_SHADOW(f_in.viewpos, f_in);
\n                //float shadow = 1.0; 
\n                //if(ambient.a > 0.35)
\n                //    shadow = PCF_Ext(shadowTexture0, f_in.shadow_view, ambient.a);
\n                
\n                float shadow =  shadow_fs_main(ambient.a);
\n
\n                // get dist to point and normalized to-eye vector
\n                float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
\n                float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
\n                float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
\n                vec3  to_eye          = -dist_to_pnt_rcp * f_in.viewpos;
\n
\n                vec3  normal = normalize(f_in.normal);
\n                float incidence_dot = dot(to_eye, normal);
\n                vec3  refl_vec_view = -to_eye + (2.0 * incidence_dot) * normal;
\n
\n                vec3  view_up_vec = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
\n                float normal_world_space_z = dot(view_up_vec, normal);
\n
\n                // diffuse color and glass factor (make windows color look darker)
\n                vec4 dif_tex_col = texture2D(colorTex, f_in.texcoord);
\n                dif_tex_col.rgb *= fma(dif_tex_col.a, 0.6, 0.4);
\n                float glass_factor = 1.0 - dif_tex_col.a;
\n
\n                // get diffuse and specular value
\n                float n_dot_l = ramp_up(shadow * saturate(dot(normal, light_vec_view.xyz)));
\n                float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), 10.0) * 2.0;
\n                vec3 spec_color = specular.rgb * specular_val;
\n
\n                // lightmaps
\n                vec3 non_ambient_term = n_dot_l * diffuse.rgb;
\n
\n              // Apply spot lights
\n              vec3 vLightsSpecAddOn;
\n                vec3 light_res = vec3(0.0);  
\n                  
\n                // ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, /*vec3(0)*/f_in.normal, light_res, vLightsSpecAddOn);
\n              // vec3 lightmap_color = light_res ; 
\n
\n              GET_LIGHTMAP(f_in.viewpos, f_in);
\n
\n                //    LIGHTMAP_BUILDING_HEIGHT_TRICK;
\n                float up_dot_clamped = saturate(fma(normal_world_space_z, 0.4, 0.6));
\n                non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);
\n
\n                // overall lighting
\n                vec3 light_color = ambient.rgb + non_ambient_term;
\n
\n                // apply detail texture
\n                float detail_factor = dif_tex_col.a * tex_detail_factor(f_in.texcoord * textureSize2D(colorTex, 0), -0.075);
\n                if (detail_factor > 0.01)
\n                    dif_tex_col.rgb = hardlight(dif_tex_col.rgb, detail_factor * fma(texture2D(detailTex, f_in.texcoord * 9.73f).rrr, vec3(0.5), vec3(-0.25)));
\n
\n                vec3 day_result = light_color * dif_tex_col.rgb;
\n                vec3 night_tex = vec3(0.0f,0.0f,0.0f); // I'm not sure
\n                if (glass_factor > 0.25)
\n                {
\n                   vec3 refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
\n                   refl_vec_world.z = abs(refl_vec_world.z);
\n                   float fresnel = saturate(fma(pow(1.0 - incidence_dot, 2.0), 0.65, 0.35)) * fma(refl_vec_world.z, 0.15, 0.85);
\n                   vec3 cube_color = texture(envTex, refl_vec_world).rgb;
\n                   day_result = mix(day_result, cube_color, glass_factor * fresnel) + spec_color * glass_factor;
\n                   night_tex = texture2D(nightTex, f_in.texcoord).rgb;
\n                }

                float night_factor = step(ambient.a, 0.35);
                vec3 result = mix(day_result, night_tex,  night_factor * glass_factor ); // 
               
                aFragColor = vec4(apply_scene_fog(f_in.viewpos, result), 1.0);
            }
            )

        };   

        SHADERS_GETTER(get_shader,vs, fs)

        AUTO_REG_NAME(building, shaders::building_mat::get_shader)

    }  // ns building_mat    

    

    namespace tree_mat 
    {
        const char* vs = {  
			
            "#extension GL_ARB_gpu_shader5 : enable \n"
 		    "//       tree_mat \n"

            INCLUDE_VS
            INCLUDE_COMPABILITY
            SHADOW_INCLUDE
            STRINGIFY ( 

            //varying   vec3  lightDir;
            out mat4 viewworld_matrix;

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 vnormal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                vec4 viewpos = gl_ModelViewMatrix * gl_Vertex;
                viewworld_matrix = inverse(gl_ModelViewMatrix);

                gl_Position = gl_ModelViewProjectionMatrix *  gl_Vertex;

                v_out.normal    = normal;
                v_out.vnormal   = mat3(gl_ModelViewMatrix) * normal;
                v_out.viewpos   = viewpos.xyz;
                v_out.texcoord  = gl_MultiTexCoord1.xy;
                //v_out.shadow_view = get_shadow_coords(viewpos, shadowTextureUnit0);
                //mat4 EyePlane =  transpose(shadowMatrix0); 
                //v_out.shadow_view = vec4(dot( viewpos, EyePlane[0]),dot( viewpos, EyePlane[1] ),dot( viewpos, EyePlane[2]),dot( viewpos, EyePlane[3] ) );
                shadow_vs_main(viewpos);
            }       
            )
        };


        const char* fs = {
            
            "#extension GL_ARB_gpu_shader5 : enable \n "
  		    "//       tree_mat \n"

            INCLUDE_UNIFORMS
            
            STRINGIFY ( 

             in mat4 viewworld_matrix;
            )
            
            INCLUDE_FUNCS
            INCLUDE_FOG_FUNCS
            INCLUDE_VS
//			INCLUDE_PCF_EXT
            INCLUDE_SCENE_PARAM
            SHADOW_INCLUDE
            STRINGIFY ( 

            uniform sampler2D       colorTex;
            uniform sampler2D       nightTex;

            in block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 vnormal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } f_in;
            
            out vec4  aFragColor;

            void main (void)
            {
                // GET_SHADOW(f_in.viewpos, f_in);
                //float shadow = 1.0; 
                //if(ambient.a > 0.35)
                //    shadow = PCF_Ext(shadowTexture0, f_in.shadow_view, ambient.a);
                
                float shadow =  shadow_fs_main(ambient.a);

                vec3 normal = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
                float n_dot_l = shadow * saturate(fma(dot(normal, light_vec_view.xyz), 0.5, 0.5));

                vec4 dif_tex_col = texture2D(colorTex, f_in.texcoord);
                vec3 result = (ambient.rgb + diffuse.rgb * n_dot_l) * dif_tex_col.rgb;

                aFragColor = vec4(apply_scene_fog(f_in.viewpos, result), dif_tex_col.a);

            }

            )

        };   

        SHADERS_GETTER(get_shader,vs, fs)

        AUTO_REG_NAME(tree, shaders::tree_mat::get_shader)

    }  // ns tree_mat


	namespace tree_inst_mat 
	{
		const char* vs = {  

			"#extension GL_ARB_gpu_shader5 : enable \n"
			"//       tree_inst_mat \n"

			INCLUDE_VS
			INCLUDE_COMPABILITY
			SHADOW_INCLUDE

			STRINGIFY ( 
\n			
\n			uniform sampler2DRect instanceMatrixTexture;
\n
\n			out mat4 viewworld_matrix;
\n
\n			out block
\n			{
\n				vec2 texcoord;
\n				vec3 normal;
\n				vec3 vnormal;
\n				vec3 viewpos;
\n				vec4 shadow_view;
\n				vec4 lightmap_coord;
\n			} v_out;
\n
\n			void main()
\n			{
\n				vec2 instanceCoord = vec2((gl_InstanceID % 4096) * 4.0, gl_InstanceID / 4096);
\n
\n				mat4 instanceModelMatrix = mat4(vec4(textureOffset(instanceMatrixTexture, instanceCoord, ivec2 (0, 0)).xyz,0.0),
\n					vec4(textureOffset(instanceMatrixTexture, instanceCoord, ivec2 (1, 0)).xyz,0.0),
\n					vec4(textureOffset(instanceMatrixTexture, instanceCoord, ivec2 (2, 0)).xyz,0.0),
\n					vec4(textureOffset(instanceMatrixTexture, instanceCoord, ivec2 (3, 0)).xyz,1.0)
\n					);
\n				
\n				mat3 normalMatrix = mat3(instanceModelMatrix[0][0], instanceModelMatrix[0][1], instanceModelMatrix[0][2],
\n					instanceModelMatrix[1][0], instanceModelMatrix[1][1], instanceModelMatrix[1][2],
\n					instanceModelMatrix[2][0], instanceModelMatrix[2][1], instanceModelMatrix[2][2]);
\n
\n
\n				vec3 normal = normalize(gl_NormalMatrix * normalMatrix * gl_Normal);
\n				vec4 viewpos = gl_ModelViewMatrix * instanceModelMatrix * gl_Vertex;
\n				viewworld_matrix = inverse(gl_ModelViewMatrix);
\n
\n				gl_Position = gl_ModelViewProjectionMatrix * instanceModelMatrix *  gl_Vertex;
\n
\n				v_out.normal    = normal;
\n				v_out.vnormal   = mat3(gl_ModelViewMatrix) * normal;
\n				v_out.viewpos   = viewpos.xyz;
\n				v_out.texcoord  = gl_MultiTexCoord1.xy;
\n				shadow_vs_main(viewpos);
\n			}       
			)
		};


    	SHADERS_GETTER(get_shader, vs, shaders::tree_mat::fs)

		AUTO_REG_NAME(treeinst, shaders::tree_inst_mat::get_shader)

	}  // ns tree_inst_mat

    namespace ground_mat 
    {
        const char* vs = {
			
            "#extension GL_ARB_gpu_shader5 : enable \n"
 		    "//       ground_mat \n"

            INCLUDE_VS
            INCLUDE_COMPABILITY
            INCLUDE_UNIFORMS
            "\n"       
            LIGHT_MAPS
            SHADOW_INCLUDE

            STRINGIFY ( 
\n
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
                vec2 detail_uv;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                vec4 viewpos = gl_ModelViewMatrix * gl_Vertex;
                viewworld_matrix = inverse(gl_ModelViewMatrix);
                gl_Position = gl_ModelViewProjectionMatrix *  gl_Vertex;

                
                v_out.normal    = normal;
                v_out.vnormal   = mat3(gl_ModelViewMatrix) * normal;
                v_out.tangent   = tangent;
                v_out.binormal  = binormal;
                 
                v_out.viewpos   = viewpos.xyz;
                v_out.detail_uv = gl_Vertex.xy * 0.03;
                v_out.texcoord  = gl_MultiTexCoord1.xy;
                // v_out.shadow_view = get_shadow_coords(viewpos, shadowTextureUnit0);
                
                //vec4 EyePlane_S = shadowMatrix[0] * refMatrix;
                //vec4 EyePlane_T = shadowMatrix[1] * refMatrix;
                //vec4 EyePlane_P = shadowMatrix[2] * refMatrix;
                //vec4 EyePlane_Q = shadowMatrix[3] * refMatrix;
                
                //mat4 EyePlane =  transpose(shadowMatrix0); 
                //v_out.shadow_view = vec4(dot( viewpos, EyePlane[0]),dot( viewpos, EyePlane[1] ),dot( viewpos, EyePlane[2]),dot( viewpos, EyePlane[3] ) );
                shadow_vs_main(viewpos);

                SAVE_LIGHTMAP_VARYINGS_VP(v_out, viewpos);
            }       
            )
        };


        const char* fs = {
            
            "#extension GL_ARB_gpu_shader5 : enable \n "
  		    "//       ground_mat \n"

            INCLUDE_UNIFORMS

            STRINGIFY ( 

           in mat4 viewworld_matrix;
            )
            
            INCLUDE_FUNCS
            INCLUDE_FOG_FUNCS
            INCLUDE_VS
//			INCLUDE_PCF_EXT
            INCLUDE_DL
            INCLUDE_DL2
            INCLUDE_SCENE_PARAM
"\n"           
            LIGHT_MAPS
            SHADOW_INCLUDE

            STRINGIFY ( 
\n
\n          uniform sampler2D       colorTex;

            uniform sampler2D       nightTex;    // testing purpose only
            uniform sampler2D       normalTex;

            in block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 vnormal;
                vec3 tangent;
                vec3 binormal;
                vec3 viewpos;
                vec2 detail_uv;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } f_in;
            
            out vec4  aFragColor;
            uniform float zShadow0; 

            void main (void)
            {
$if 0 
\n
                float fTexelSize=0.00137695;
                float fZOffSet  = -0.001954;
                float testZ = gl_FragCoord.z*2.0-1.0;
                float map0 = step(testZ, zShadow0);
                float shadowOrg0 = shadow2D( shadowTexture0,f_in.shadow_view.xyz+vec3(0.0,0.0,fZOffSet) ).r;
                float shadow0 = shadowOrg0;
                float term0 = map0*(1.0-shadow0); 
                float v = clamp(term0,0.0,1.0);
                float shadow = 1 - v * 0.5;;
$endif
\n

                // GET_SHADOW(f_in.viewpos, f_in);
                //float shadow = 1.0; 
                //if(ambient.a > 0.35)
                //    shadow = PCF4_Ext(shadowTexture0, f_in.shadow_view, ambient.a); 
\n               
                float shadow =  shadow_fs_main(ambient.a);

\n
\n                float rainy_value = 0.666 * specular.a;
\n
\n                // test for 
\n                // vec3 dif_tex_col = mix(texture2D(nightTex, f_in.texcoord).rgb,texture2D(colorTex, f_in.texcoord).rgb,texture2D(normalTex, f_in.texcoord).r);
\n                vec3 dif_tex_col = texture2D(colorTex, f_in.texcoord).rgb;
\n                dif_tex_col *= fma(dif_tex_col, vec3(rainy_value,rainy_value,rainy_value)/*rainy_value.xxx*/, vec3(1.0 - rainy_value));
\n                float detail_factor = tex_detail_factor(f_in.texcoord * textureSize2D(colorTex, 0), -0.02);
\n                vec3 normal_noise = vec3(0.0);
\n                if (detail_factor > 0.01)
\n                {
\n                    normal_noise = detail_factor * fma(texture2D(detailTex, f_in.detail_uv).rgb, vec3(0.6), vec3(-0.3));
\n                    dif_tex_col = hardlight(dif_tex_col, normal_noise.ggg);
\n                }
\n
\n                vec3 normal = normalize(0.8 * f_in.normal + (normal_noise.x * f_in.tangent + normal_noise.y * f_in.binormal));
\n                float n_dot_l = saturate(dot(normal, light_vec_view.xyz));
\n
\n
                // get dist to point and normalized to-eye vector
\n                float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
\n                float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
\n                float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
\n                vec3 to_pnt = dist_to_pnt_rcp * f_in.viewpos;
\n
\n                // reflection vector
\n                float incidence_dot = dot(-to_pnt, normal);
\n                vec3 refl_vec_view = fma(normal, vec3(2.0 * incidence_dot), to_pnt);
\n
\n                // specular
\n                float specular_val = pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), fma(rainy_value, 3.0, 3.0)) * fma(rainy_value, 0.8, 0.3);
\n                vec3 specular_color = specular_val * specular.rgb;
\n
\n                // Apply spot lights
\n                vec3 vLightsSpecAddOn;
                  vec3 light_res;  
                  
                  // ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, /*vec3(0)*/f_in.normal, light_res, vLightsSpecAddOn);
\n                // vec3 lightmap_color = light_res ; 

\n                GET_LIGHTMAP(f_in.viewpos, f_in);
\n                LIGHTMAP_SHADOW_TRICK(shadow);
\n                vec3 non_ambient_term = max(lightmap_color, shadow * (diffuse.rgb * n_dot_l + specular_color));
\n
\n                // result
\n                vec3 result = (ambient.rgb + non_ambient_term) * dif_tex_col.rgb;
\n                // reflection when rain
\n                if (rainy_value >= 0.01)
\n                {
\n                    vec3 refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
\n                    float fresnel = saturate(fma(pow(1.0 - incidence_dot, 5.0), 0.25, 0.05));
\n                    vec3 cube_color = texture(envTex, refl_vec_world).rgb;
\n                    result = mix(result, lightmap_color + cube_color + specular_color, fresnel * rainy_value);
\n                }
\n
\n
\n                aFragColor = vec4(apply_scene_fog(f_in.viewpos, result), 1.0);
\n            }
            )

        };   

        SHADERS_GETTER(get_shader,vs, fs)

        AUTO_REG_NAME(ground, shaders::ground_mat::get_shader)
        AUTO_REG_NAME(sea, shaders::ground_mat::get_shader)
        AUTO_REG_NAME(mountain, shaders::ground_mat::get_shader)

    }  // ns ground_mat

    namespace sea_mat
    {

        const char* vs = {

            "#extension GL_ARB_gpu_shader5 : enable \n"
 		    "//       sea_mat \n"

            STRINGIFY ( 
            
                //in vec2 fragCoord;

                attribute vec3 tangent;
                attribute vec3 binormal;
                out       mat4 viewworld_matrix;
                out       vec2 fragCoord;

                void main()
                {
                    vec3 normal      = normalize(gl_NormalMatrix * gl_Normal);
                    vec4 viewpos     = gl_ModelViewMatrix * gl_Vertex;
                    viewworld_matrix = inverse(gl_ModelViewMatrix);
                    gl_Position      = gl_ModelViewProjectionMatrix *  gl_Vertex;
                    fragCoord        = vec2(gl_Position.xy);
                    gl_Position.z = 0;
                }       
            )
        };

        const char* fs =  STRINGIFY(
            \n#include "data/materials/sea/sea.frag"
        );
        
        SHADERS_GETTER(get_shader, vs, fs)

        //AUTO_REG_NAME(sea, shaders::sea_mat::get_shader)
    }

    namespace flame_mat
    {

        const char* vs =  STRINGIFY(
            \n#include "data/materials/flame/flame.vert"
            );

        const char* fs =  STRINGIFY(
            \n#include "data/materials/flame/flame.frag"
            );

        SHADERS_GETTER(get_shader, vs, fs)

        AUTO_REG_NAME(flame, shaders::flame_mat::get_shader)
    }

	namespace sp_lit_mat
	{

		const char* vs =  STRINGIFY(
			\n#include "data/materials/sp_lit/sp_lit.vert"
			);

		const char* fs =  STRINGIFY(
			\n#include "data/materials/sp_lit/sp_lit.frag"
			);

		SHADERS_GETTER(get_shader, vs, fs)

	    AUTO_REG_NAME(splight, shaders::sp_lit_mat::get_shader)
	}

    namespace skinning_mat
    {

        const char* vs =  STRINGIFY(
            \n#include "data/materials/misc/skinning.vert"
            );


       const char* fs = {
       
       "#extension GL_ARB_gpu_shader5 : enable \n "
       "//       skinning_mat (default_mat) \n"

       INCLUDE_UNIFORMS

       STRINGIFY ( 

\n          in mat4 viewworld_matrix;
       )
       
       INCLUDE_FUNCS
       INCLUDE_FOG_FUNCS
       INCLUDE_VS
//	   INCLUDE_PCF_EXT
       INCLUDE_DL
       INCLUDE_DL2
       INCLUDE_SCENE_PARAM
"\n"      
       LIGHT_MAPS
       SHADOW_INCLUDE

       STRINGIFY ( 
\n
\n           uniform sampler2D colorTex;
\n           uniform sampler2D normalTex;
\n
\n           in block
\n           {
\n               vec2 texcoord;
\n               vec3 normal;
\n               vec3 vnormal;
\n               vec3 tangent;
\n               vec3 binormal;
\n               vec3 viewpos;
\n               vec4 shadow_view;
\n               vec4 lightmap_coord;
\n           } f_in;
\n
\n           out vec4  aFragColor;
\n
\n           void main (void)
\n           {
\n               // GET_SHADOW(f_in.viewpos, f_in);
\n               //float shadow = 1.0; 
\n               //if(ambient.a > 0.35)
\n               //    shadow = PCF_Ext(shadowTexture0, f_in.shadow_view, ambient.a);
\n               
\n               float shadow =  shadow_fs_main(ambient.a);
\n
\n               vec4 base = texture2D(colorTex, f_in.texcoord);
\n               vec3 bump = fma(texture2D(normalTex, f_in.texcoord).xyz, vec3(2.0), vec3(-1.0));
\n               //vec3 bump = texture2D(normalTex, f_in.texcoord).xyz;
\n               //bump = normalize(bump * 2.0 - 1.0);
\n               vec3  normal       = normalize(bump.x * f_in.tangent + bump.y * f_in.binormal + bump.z * f_in.normal);
\n               vec4  dif_tex_col  = texture2D(colorTex,f_in.texcoord, -1.0);
\n               float glass_factor = /*1.0 - dif_tex_col.a*/ 0;
\n
\n               // get dist to point and normalized to-eye vector
\n               float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
\n               float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
\n               float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
\n               vec3  to_eye          = -dist_to_pnt_rcp * f_in.viewpos;
\n
\n               vec3 view_up_vec = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
\n               float normal_world_space_z = dot(view_up_vec, normal);
\n
\n
\n               float incidence_dot  = dot(to_eye, normal);
\n               float pow_fr         = pow(saturate(1.0 - incidence_dot), 3.0);
\n               vec3  refl_vec_view  = -to_eye + (2.0 * incidence_dot) * normal;
\n               vec3  refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
\n               float refl_min       = 0.10 + glass_factor * 0.30;
\n               float half_refl_z    = 0.5 * (refl_vec_world.z + normal_world_space_z);
\n               float fresnel        = mix(refl_min, 0.6, pow_fr) * fma(half_refl_z, 0.15, fma(glass_factor, 0.6, 0.)); 
\n
\n               float n_dot_l = shadow * saturate(dot(normal, light_vec_view.xyz));
\n               float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), 44.0) * 0.9;
\n               vec3  pure_spec_color = specular.rgb * specular_val;
\n               float spec_compose_fraction = 0.35;
\n
\n
\n               // const vec3 cube_color = texture(Env, refl_vec_world).rgb + pure_spec_color;
\n               vec3 cube_color = texture(envTex, refl_vec_world).rgb + pure_spec_color;
\n
\n               vec3 non_ambient_term = diffuse.rgb * n_dot_l + spec_compose_fraction * pure_spec_color;
\n
\n             // Apply spot lights
\n             vec3 vLightsSpecAddOn;
\n             vec3 light_res;  
\n                  
\n             // ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, /*vec3(0)*/f_in.normal, light_res, vLightsSpecAddOn);
\n             // vec3 lightmap_color = light_res ; 
\n
\n             GET_LIGHTMAP(f_in.viewpos, f_in);
\n
\n               float up_dot_clamped = saturate(fma(normal_world_space_z, 0.55, 0.45));
\n               non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);
\n
\n               float ao_trick = fma(up_dot_clamped, 0.4, 0.6);
\n               vec3  composed_lighting = ao_trick * ambient.rgb + non_ambient_term;
\n               vec3  day_result = composed_lighting * dif_tex_col.rgb + (1.0 - spec_compose_fraction) * pure_spec_color;
\n               float night_factor = step(ambient.a, 0.35);
\n               vec3  result = mix(day_result, vec3(0.90, 0.90, 0.86), night_factor * glass_factor);
\n
\n               aFragColor = vec4(apply_clear_fog(f_in.viewpos, result), 1.0);
\n			     //aFragColor = vec4(1.0,0.0,0.0,1.0);
\n           }
       )

       };   

        SHADERS_GETTER(get_shader, vs, fs)

        AUTO_REG_NAME(skinning, skinning_mat::get_shader)
    }

    namespace skinning_inst_mat
    {

        const char* vs =  STRINGIFY(
            \n#include "data/materials/misc/skinning_inst.vert"
            );


        SHADERS_GETTER(get_shader, vs, skinning_mat::fs)

        AUTO_REG_NAME(skininst, skinning_inst_mat::get_shader)
    }

    namespace concrete_mat 
    {
        const char* vs = {
            
            "#extension GL_ARB_gpu_shader5 : enable \n"
 		    "//       concrete_mat \n"

            INCLUDE_VS
            INCLUDE_COMPABILITY
            "\n"       
            LIGHT_MAPS
            SHADOW_INCLUDE

            STRINGIFY ( 
\n            attribute vec3 tangent;
\n            attribute vec3 binormal;
\n            out mat4 viewworld_matrix;
\n
\n            uniform mat4 decal_matrix;        
\n
\n            out block
\n            {
\n                vec2 texcoord;
\n                vec3 normal;
\n                vec3 vnormal;
\n                vec3 tangent;
\n                vec3 binormal;
\n                vec3 viewpos;
\n                vec2 detail_uv;
\n                vec4 shadow_view;
\n                //vec4 shadow_view1;
\n                //vec4 shadow_view2;
\n                vec4 lightmap_coord;
\n                vec4 decal_coord;
\n                vec4 refl_coord;
\n            } v_out;
\n            
\n                                                        
\n            uniform mat4            shadowMatrix1;                                                          
\n            uniform mat4            shadowMatrix2;  
\n
\n            void main()
\n            {
\n                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
\n                vec4 viewpos = gl_ModelViewMatrix * gl_Vertex;
\n                viewworld_matrix = inverse(gl_ModelViewMatrix);
\n                
\n                gl_Position = gl_ModelViewProjectionMatrix *  gl_Vertex;
\n
\n                v_out.normal    = normal;
\n                v_out.vnormal   = mat3(gl_ModelViewMatrix) * normal;
\n                v_out.tangent   = tangent;
\n                v_out.binormal  = binormal;
\n
\n                v_out.viewpos   = viewpos.xyz;
\n                // v_out.detail_uv = position.xy * 0.045;
\n                v_out.detail_uv = gl_Vertex.xy * 0.045; // FIXME dont no how
\n                v_out.texcoord  = gl_MultiTexCoord1.xy;
\n
\n                // SAVE_DECAL_VARYINGS_VP
\n                v_out.decal_coord = (decal_matrix * vec4(v_out.viewpos,1.0)).xyzw;
\n
\n                //v_out.shadow_view = get_shadow_coords(viewpos, shadowTextureUnit0);
\n                //mat4 EyePlane =  transpose(shadowMatrix0); 
\n                //v_out.shadow_view = vec4(dot( viewpos, EyePlane[0]),dot( viewpos, EyePlane[1] ),dot( viewpos, EyePlane[2]),dot( viewpos, EyePlane[3] ) );
\n                
\n                //mat4 EyePlane1 =  transpose(shadowMatrix1); 
\n                //v_out.shadow_view1 = vec4(dot( viewpos, EyePlane1[0]),dot( viewpos, EyePlane1[1] ),dot( viewpos, EyePlane1[2]),dot( viewpos, EyePlane1[3] ) );
\n
\n                //mat4 EyePlane2 =  transpose(shadowMatrix2); 
\n                //v_out.shadow_view2 = vec4(dot( viewpos, EyePlane2[0]),dot( viewpos, EyePlane2[1] ),dot( viewpos, EyePlane2[2]),dot( viewpos, EyePlane2[3] ) );
\n                
\n                // Compute reflection texture coordinates
\n                v_out.refl_coord = vec4(dot( gl_Position, vec4 ( 0.5, 0, 0, 0.5 ) ),dot( gl_Position, vec4 ( 0, 0.5, 0, 0.5 ) ),/*(viewworld_matrix * viewpos)*/gl_Position.z, gl_Position.w);
\n
\n                shadow_vs_main(viewpos);
\n
\n                SAVE_LIGHTMAP_VARYINGS_VP(v_out, viewpos);
\n            }       
            )
        };

        const char* fs = { 
            
            "#extension GL_ARB_gpu_shader5 : enable \n "
 		    "//       concrete_mat \n"

            INCLUDE_UNIFORMS
            


            STRINGIFY ( 

\n
\n            in mat4 viewworld_matrix;
\n          )
            
              INCLUDE_FUNCS
              INCLUDE_FOG_FUNCS
              INCLUDE_VS
//			  INCLUDE_PCF_EXT
              INCLUDE_SCENE_PARAM
              INCLUDE_DL
              INCLUDE_DL2
"\n"              
              LIGHT_MAPS
              SHADOW_INCLUDE

              STRINGIFY ( 
\n
\n            uniform sampler2D colorTex;
\n			  uniform sampler2D normalTex;

\n            in block
\n            {
\n                vec2 texcoord;
\n                vec3 normal;
\n                vec3 vnormal;
\n                vec3 tangent;
\n                vec3 binormal;
\n                vec3 viewpos;
\n                vec2 detail_uv;
\n                vec4 shadow_view;
\n                /*vec4 shadow_view1;*/
\n                /*vec4 shadow_view2;*/
\n                vec4 lightmap_coord;
\n                vec4 decal_coord;
\n                vec4 refl_coord;
\n            } f_in;
\n            
\n            out vec4  aFragColor;
              
              /*uniform float zShadow0; */
              /*uniform float zShadow1;*/ 
              /*uniform float zShadow2; */
              
              /*uniform sampler2DShadow shadowTexture1;*/ 
              /*uniform sampler2DShadow shadowTexture2;*/
\n            uniform sampler2D reflectionTexture;
\n
\n            void main (void)
\n            {
$if 0
\n
\n              float testZ = gl_FragCoord.z*2.0-1.0;
\n                float map0 = step(testZ, zShadow0);
\n                float map1  = step(zShadow0,testZ)*step(testZ, zShadow1);
\n                float map2  = step(zShadow1,testZ)*step(testZ, zShadow2);
\n                float fTexelSize=0.00137695;
\n                float fZOffSet  = -0.001954;
\n
\n                float shadowOrg0 = shadow2D( shadowTexture0,f_in.shadow_view.xyz+vec3(0.0,0.0,fZOffSet) ).r;
\n                float shadow00 = shadow2D( shadowTexture0,f_in.shadow_view.xyz+vec3(-fTexelSize,-fTexelSize,fZOffSet) ).r;
\n                float shadow10 = shadow2D( shadowTexture0,f_in.shadow_view.xyz+vec3( fTexelSize,-fTexelSize,fZOffSet) ).r;
\n                float shadow20 = shadow2D( shadowTexture0,f_in.shadow_view.xyz+vec3( fTexelSize, fTexelSize,fZOffSet) ).r;
\n                float shadow30 = shadow2D( shadowTexture0,f_in.shadow_view.xyz+vec3(-fTexelSize, fTexelSize,fZOffSet) ).r;
\n                float shadow0 = ( 2.0*shadowOrg0 + shadow00 + shadow10 + shadow20 + shadow30)/6.0;
\n                float shadowOrg1 = shadow2D( shadowTexture1,f_in.shadow_view1.xyz+vec3(0.0,0.0,fZOffSet) ).r;
\n                float shadow01 = shadow2D( shadowTexture1,f_in.shadow_view1.xyz+vec3(-fTexelSize,-fTexelSize,fZOffSet) ).r;
\n                float shadow11 = shadow2D( shadowTexture1,f_in.shadow_view1.xyz+vec3( fTexelSize,-fTexelSize,fZOffSet) ).r;
\n                float shadow21 = shadow2D( shadowTexture1,f_in.shadow_view1.xyz+vec3( fTexelSize, fTexelSize,fZOffSet) ).r;
\n                float shadow31 = shadow2D( shadowTexture1,f_in.shadow_view1.xyz+vec3(-fTexelSize, fTexelSize,fZOffSet) ).r;
\n                float shadowOrg2 = shadow2D( shadowTexture2,f_in.shadow_view2.xyz+vec3(0.0,0.0,fZOffSet) ).r;
\n                float shadow02 = shadow2D( shadowTexture2,f_in.shadow_view2.xyz+vec3(-fTexelSize,-fTexelSize,fZOffSet) ).r;
\n                float shadow12 = shadow2D( shadowTexture2,f_in.shadow_view2.xyz+vec3( fTexelSize,-fTexelSize,fZOffSet) ).r;
\n                float shadow22 = shadow2D( shadowTexture2,f_in.shadow_view2.xyz+vec3( fTexelSize, fTexelSize,fZOffSet) ).r;
\n                float shadow32 = shadow2D( shadowTexture2,f_in.shadow_view2.xyz+vec3(-fTexelSize, fTexelSize,fZOffSet) ).r;
\n                float shadow2 = ( 2.0*shadowOrg2 + shadow02 + shadow12 + shadow22 + shadow32)/6.0;
\n
\n                float term0 = map0*(1.0-shadow0); 
\n                float term1 = map1*(1.0-shadow1);
\n                float term2 = map2*(1.0-shadow2);
\n
\n                float v = clamp(term0+term1+term2,0.0,1.0);
\n
\n                float shadow = 1 - v * 0.5;
\n
\n$endif
\n
                // GET_SHADOW(f_in.viewpos, f_in);
                //float shadow = 1.0; 
                //if(ambient.a > 0.35)
                //{
                //     shadow = PCF4_Ext(shadowTexture0, f_in.shadow_view, ambient.a);
                //}
\n
\n                float shadow =  shadow_fs_main(ambient.a);
\n
\n                float rainy_value = specular.a;
\n
\n                vec3 dif_tex_col = texture2D(colorTex, f_in.texcoord).rgb;
\n                vec4 decal_data = textureProj(ViewDecalMap, f_in.decal_coord).rgba; 
\n                dif_tex_col.rgb = fma(dif_tex_col.rgb, vec3(1.0 - decal_data.a), decal_data.rgb);     
\n                float tex_mix_val = rainy_value * 0.7;
\n                dif_tex_col *= fma(dif_tex_col, vec3(tex_mix_val,tex_mix_val,tex_mix_val)/*tex_mix_val.xxx*/, vec3(1.0 - tex_mix_val));
\n                float detail_factor = tex_detail_factor(f_in.texcoord * textureSize2D(colorTex, 0), -0.015);
\n                vec3 concrete_noise = vec3(0.0);
\n                if (detail_factor > 0.01)
\n                {
\n                    concrete_noise = detail_factor * fma(texture2D(detailTex, f_in.detail_uv).rgb, vec3(0.48), vec3(-0.24));
\n                    dif_tex_col = hardlight(dif_tex_col, concrete_noise.bbb);
\n                }
\n                // normalTex = noiseTex
\n				  vec4 bump = texture2D(normalTex, f_in.texcoord).xyzw;
\n 
\n                rainy_value *= step (0.5, bump.w ) * bump.w;
\n                vec2 vDistort = f_in.normal.xz * 0.025;
\n                vec4 refl_data = textureProj(reflectionTexture, f_in.refl_coord + vec4(vDistort,0.0,0.0)) * vec4(vec3(0.85), 1.0);
\n                // dif_tex_col.rgb = mix(dif_tex_col.rgb,mix(dif_tex_col.rgb,refl_data.rgb,refl_data.a),rainy_value);
                  dif_tex_col.rgb = fma(vec3(rainy_value * refl_data.a ), refl_data.rgb - dif_tex_col.rgb  ,dif_tex_col.rgb );
\n
\n                // FIXME
\n                // APPLY_DECAL(f_in, dif_tex_col);

\n
\n                // get dist to point and normalized to-eye vector
\n                float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
\n                float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
\n                float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
\n                vec3 to_pnt = dist_to_pnt_rcp * f_in.viewpos;
\n
\n                // reflection vector
\n
\n                vec3 normal = normalize(f_in.normal + (concrete_noise.x * f_in.tangent + concrete_noise.y * f_in.binormal) * (1.0 - decal_data.a));
\n                float incidence_dot = dot(-to_pnt, normal);
\n                vec3 refl_vec_view = fma(normal, vec3(2.0 * incidence_dot), to_pnt);
\n                
\n                // diffuse term
\n                float n_dot_l = shadow * saturate(dot(normal, light_vec_view.xyz)) * fma(rainy_value, -0.7, 1.0);
\n
\n                // specular
\n                float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), fma(rainy_value, 5.0, 1.5)) * fma(rainy_value, 0.9, 0.7);
\n                vec3 specular_color = specular_val * specular.rgb;
\n
\n                // Apply spot lights
\n                vec3 vLightsSpecAddOn;
\n                vec3 light_res;  
\n                  
\n                // ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, /*vec3(0)*/f_in.normal, light_res, vLightsSpecAddOn);
\n                // vec3 lightmap_color = light_res ; 
\n
\n               GET_LIGHTMAP(f_in.viewpos, f_in);
\n               LIGHTMAP_SHADOW_TRICK(shadow);
\n
\n               vec3 non_ambient_term = max(lightmap_color, diffuse.rgb * n_dot_l + specular_color);
\n
\n               // result
\n               vec3 result = (ambient.rgb + non_ambient_term) * dif_tex_col.rgb;
\n               // reflection when rain
\n               if (rainy_value >= 0.01)
\n               {
\n                   vec3 refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
\n                   float fresnel = saturate(fma(pow(1.0 - incidence_dot, 5.0), 0.45, 0.05));
\n                   vec3 cube_color = texture(envTex, refl_vec_world).rgb; // vec3(0.0);// 
\n                   result = mix(result, lightmap_color + cube_color, fresnel * rainy_value) + (fma(fresnel, 0.5, 0.5) * rainy_value) * specular_color;
\n               }
\n
\n               
\n               aFragColor = vec4(apply_scene_fog(f_in.viewpos, result), 1.0);    
\n               //aFragColor = vec4(apply_clear_fog(f_in.viewpos, result), 1.0);    
\n               //gl_FragData[0] = vec4(apply_scene_fog(f_in.viewpos, result), 1.0);
\n               // gl_FragData[1] = vec4(normal, 1.0);
\n            }
            )
 
        };   
        
        const char* vs_test = {

            STRINGIFY ( 
            vec4 Ambient;
            vec4 Diffuse;
            vec4 Specular;


            void pointLight(in int i, in vec3 normal, in vec3 eye, in vec3 ecPosition3)
            {
                float nDotVP;       // normal . light direction
                float nDotHV;       // normal . light half vector
                float pf;           // power factor
                float attenuation;  // computed attenuation factor
                float d;            // distance from surface to light source
                vec3  VP;           // direction from surface to light position
                vec3  halfVector;   // direction of maximum highlights

                // Compute vector from surface to light position
                VP = vec3 (gl_LightSource[i].position) - ecPosition3;

                // Compute distance between surface and light position
                d = length(VP);

                // Normalize the vector from surface to light position
                VP = normalize(VP);

                // Compute attenuation
                attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +
                    gl_LightSource[i].linearAttenuation * d +
                    gl_LightSource[i].quadraticAttenuation * d * d);

                halfVector = normalize(VP + eye);

                nDotVP = max(0.0, dot(normal, VP));
                nDotHV = max(0.0, dot(normal, halfVector));

                if (nDotVP == 0.0)
                {
                    pf = 0.0;
                }
                else
                {
                    pf = pow(nDotHV, gl_FrontMaterial.shininess);

                }
                Ambient  += gl_LightSource[i].ambient * attenuation;
                Diffuse  += gl_LightSource[i].diffuse * nDotVP * attenuation;
                Specular += gl_LightSource[i].specular * pf * attenuation;
            }

            void directionalLight(in int i, in vec3 normal)
            {
                float nDotVP;         // normal . light direction
                float nDotHV;         // normal . light half vector
                float pf;             // power factor

                nDotVP = max(0.0, dot(normal, normalize(vec3 (gl_LightSource[i].position))));
                nDotHV = max(0.0, dot(normal, vec3 (gl_LightSource[i].halfVector)));

                if (nDotVP == 0.0)
                {
                    pf = 0.0;
                }
                else
                {
                    pf = pow(nDotHV, gl_FrontMaterial.shininess);

                }
                Ambient  += gl_LightSource[i].ambient;
                Diffuse  += gl_LightSource[i].diffuse * nDotVP;
                Specular += gl_LightSource[i].specular * pf;
            }

            vec3 fnormal(void)
            {
                //Compute the normal 
                vec3 normal = gl_NormalMatrix * gl_Normal;
                normal = normalize(normal);
                return normal;
            }

            void flight(in vec3 normal, in vec4 ecPosition, float alphaFade)
            {
                vec4 color;
                vec3 ecPosition3;
                vec3 eye;

                ecPosition3 = (vec3 (ecPosition)) / ecPosition.w;
                eye = vec3 (0.0, 0.0, 1.0);

                // Clear the light intensity accumulators
                Ambient  = vec4 (0.0);
                Diffuse  = vec4 (0.0);
                Specular = vec4 (0.0);

                pointLight(0, normal, eye, ecPosition3);

                pointLight(1, normal, eye, ecPosition3);

                directionalLight(2, normal);

                color = gl_FrontLightModelProduct.sceneColor +
                    Ambient  * gl_FrontMaterial.ambient +
                    Diffuse  * gl_FrontMaterial.diffuse;
                color += Specular * gl_FrontMaterial.specular;
                color = clamp( color, 0.0, 1.0 );
                gl_FrontColor = color;

                gl_FrontColor.a *= alphaFade;
            }


            void main (void)
            {
                vec3  transformedNormal;
                float alphaFade = 1.0;

                // Eye-coordinate position of vertex, needed in various calculations
                vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;

                // Do fixed functionality vertex transform
                gl_Position = gl_ModelViewProjectionMatrix *  gl_Vertex;//ftransform();
                transformedNormal = fnormal();
                flight(transformedNormal, ecPosition, alphaFade);
            }
            )
        };
        
        const char* vs_test2 = { 
            INCLUDE_VS
            STRINGIFY ( 
            uniform mat4  shadowMatrix0;                                \n 
            uniform mat4  refMatrix;

            void main (void)
            {
               vec4 posEye    =  gl_ModelViewMatrix * gl_Vertex;
               gl_Position    =  gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;                        
               gl_TexCoord[0] =  gl_MultiTexCoord1;                  
               gl_FrontColor  =  gl_Color;                          
               gl_BackColor   =  gl_Color;                          

               //gl_TexCoord[1].s = dot( posEye, gl_EyePlaneS[5] );
               //gl_TexCoord[1].t = dot( posEye, gl_EyePlaneT[5] );
               //gl_TexCoord[1].p = dot( posEye, gl_EyePlaneR[5] );
               //gl_TexCoord[1].q = dot( posEye, gl_EyePlaneQ[5] );
               
               gl_TexCoord[1] = get_shadow_coords(posEye, 5);

               //gl_TexCoord[1] =   shadowMatrix0 * refMatrix * gl_Vertex  ;   //  shadowMatrix * gl_ModelViewMatrix *  refMatrix
               
            }
            )
        };

        const char* fs_test = { 

            INCLUDE_VS

            STRINGIFY ( 

            
            float texture2DCompare(sampler2D depths, vec2 uv, float compare){
                float depth = texture2D(depths, uv).r;
                return step(compare, depth);
             } 

            float PCF2(sampler2D depths, vec2 size, vec2 uv, float compare){
                float result = 0.0;
                for(int x=-2; x<=2; x++){
                    for(int y=-2; y<=2; y++){
                        vec2 off = vec2(x,y)/size;
                        result += texture2DCompare(depths, uv+off, compare);
                    }
                }
                return result/25.0;
             }


            void main(void)                                                         \n
            {                                                                       \n
                vec4 colorAmbientEmissive = vec4(1.0);//gl_FrontLightModelProduct.sceneColor;       \n
                vec4 color = texture2D( baseTexture, gl_TexCoord[0].xy );  // baseTextureUnit             \n

                float shadow = PCF2(shadowTexture0, gl_TexCoord[1],pcf_size); // shadowTextureUnit0
            
                float illum = luminance_crt(gl_LightSource[0].ambient + gl_LightSource[0].diffuse);

                color *= mix( colorAmbientEmissive * (1 - illum), gl_Color , shadow /*shadow2DProj( shadowTexture0, gl_TexCoord[shadowTextureUnit0]).r*/ );     \n
                gl_FragColor = color;                                                                                                \n
                //gl_FragColor = gl_TexCoord[1]; \n
                //gl_FragColor = colorAmbientEmissive*shadow; \n
            }
            )
        };

        SHADERS_GETTER(get_shader,vs, fs)
        
        AUTO_REG_NAME(concrete, shaders::concrete_mat::get_shader)

    }  // ns concrete_mat

    namespace railing_mat 
    {
        const char* vs = {  
            "#extension GL_ARB_gpu_shader5 : enable \n"
 		    "//       railing_mat \n"

            INCLUDE_VS
            INCLUDE_COMPABILITY

            "\n"       
            LIGHT_MAPS
            SHADOW_INCLUDE

            STRINGIFY ( 
\n          attribute vec3 tangent;
            attribute vec3 binormal;
            out mat4 viewworld_matrix;

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 vnormal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                vec4 viewpos = gl_ModelViewMatrix * gl_Vertex;
                viewworld_matrix = inverse(gl_ModelViewMatrix);

                gl_Position = gl_ModelViewProjectionMatrix *  gl_Vertex;

                v_out.normal    = normal;
                v_out.vnormal   = mat3(gl_ModelViewMatrix) * normal;
                v_out.viewpos   = viewpos.xyz;
                v_out.texcoord  = gl_MultiTexCoord1.xy;
                //v_out.shadow_view = get_shadow_coords(viewpos, shadowTextureUnit0);
                
                //mat4 EyePlane =  transpose(shadowMatrix0); 
                //v_out.shadow_view = vec4(dot( viewpos, EyePlane[0]),dot( viewpos, EyePlane[1] ),dot( viewpos, EyePlane[2]),dot( viewpos, EyePlane[3] ) );
                shadow_vs_main(viewpos);

                SAVE_LIGHTMAP_VARYINGS_VP(v_out, viewpos);
            }       
            )
        };


        const char* fs = {
            
            "#extension GL_ARB_gpu_shader5 : enable \n "
  		    "//       railing_mat \n"

            INCLUDE_UNIFORMS

            STRINGIFY ( 

            in mat4 viewworld_matrix;
            )

            INCLUDE_FUNCS
            INCLUDE_FOG_FUNCS
            INCLUDE_VS
//			INCLUDE_PCF_EXT
            INCLUDE_DL
            INCLUDE_DL2
            INCLUDE_SCENE_PARAM
"\n"          
            LIGHT_MAPS
            SHADOW_INCLUDE

            STRINGIFY ( 
\n
            uniform sampler2D colorTex;
\n
            in block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 vnormal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } f_in;
\n
            out vec4  aFragColor;
\n
            void main (void)
            {
                // GET_SHADOW(f_in.viewpos, f_in);
                //float shadow = 1.0; 
                //if(ambient.a > 0.35)
                //{
                //    shadow = PCF_Ext(shadowTexture0, f_in.shadow_view, ambient.a);
                //}
                
                float shadow =  shadow_fs_main(ambient.a);

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

\n              // Apply spot lights
\n              vec3 vLightsSpecAddOn;
                vec3 light_res;  
                  
                // ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, /*vec3(0)*/f_in.normal, light_res, vLightsSpecAddOn);
\n              // vec3 lightmap_color = light_res ; 

\n              GET_LIGHTMAP(f_in.viewpos, f_in);

                non_ambient_term = max(lightmap_color, non_ambient_term);

                vec4 dif_tex_col = texture2D(colorTex, f_in.texcoord);
                vec3 result = (ambient.rgb + non_ambient_term) * dif_tex_col.rgb;


                aFragColor = vec4(apply_scene_fog(f_in.viewpos, result), dif_tex_col.a);

                
            }
            )

        };   

        SHADERS_GETTER(get_shader,vs, fs)

        AUTO_REG_NAME(railing, shaders::railing_mat::get_shader)

    }  // ns railing_mat

    namespace panorama_mat 
    {
        const char* vs = {  
            
            "#extension GL_ARB_gpu_shader5 : enable \n"
  		    "//       panorama_mat \n"

            INCLUDE_COMPABILITY

            STRINGIFY ( 
            attribute vec3 tangent;
            attribute vec3 binormal;
            out mat4 viewworld_matrix;

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
            } v_out;

            void main()
            {
                vec4 viewpos = gl_ModelViewMatrix * gl_Vertex;

                gl_Position = gl_ModelViewProjectionMatrix *  gl_Vertex;
                viewworld_matrix = inverse(gl_ModelViewMatrix);
                v_out.normal = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
                v_out.viewpos   = viewpos.xyz;
                v_out.texcoord  = gl_MultiTexCoord1.xy;
            }       
            )
        };


        const char* fs = {
            
            "#extension GL_ARB_gpu_shader5 : enable \n "
   		    "//       panorama_mat \n"

            INCLUDE_UNIFORMS

            STRINGIFY ( 

            //uniform vec4    fog_params; 
            in mat4         viewworld_matrix;

            )

            INCLUDE_FUNCS

            INCLUDE_FOG_FUNCS

            INCLUDE_VS
            INCLUDE_SCENE_PARAM

            STRINGIFY ( 

            uniform sampler2D colorTex;

            in block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
            } f_in;
            
            out vec4  aFragColor;

            void main (void)
            {
                vec3 normal = normalize(f_in.normal);
                float n_dot_l = saturate(fma(dot(normal, light_vec_view.xyz), 0.75, 0.25));

                vec4 dif_tex_col = texture2D(colorTex, f_in.texcoord);
                vec3 result = (ambient.rgb + diffuse.rgb * n_dot_l) * dif_tex_col.rgb * 0.5;
                
                aFragColor = vec4(apply_scene_fog(f_in.viewpos, result), dif_tex_col.a);
            }
            )

        };   

        SHADERS_GETTER(get_shader,vs, fs)

        AUTO_REG_NAME(panorama, shaders::panorama_mat::get_shader)

    }  // ns panorama_mat

    namespace sky_fog_mat 
    {
        const char* vs = { 
            
            "#extension GL_ARB_gpu_shader5 : enable \n"
  		    "//       sky_fog_mat \n"

            INCLUDE_COMPABILITY

            STRINGIFY ( 
            attribute vec3 tangent;
            attribute vec3 binormal;
            out mat4 viewworld_matrix;

            out block
            {
                vec3 pos;
            } v_out;

            void main()
            {
                v_out.pos = gl_Vertex.xyz;
				// perform conversion to post-projective space
                viewworld_matrix = inverse(gl_ModelViewMatrix);

                vec3 vLocalSpaceCamPos = viewworld_matrix[3].xyz;
				//vec3 vLocalSpaceCamPos = gl_ModelViewMatrixInverse[3].xyz;
				gl_Position = gl_ModelViewProjectionMatrix * vec4(vLocalSpaceCamPos.xyz + gl_Vertex.xyz, 1.0);

				gl_Position.z = 0.0;
                //gl_Position.z = gl_Position.w;

            }       
            )
        };


        const char* fs = {
            
            "#extension GL_ARB_gpu_shader5 : enable  \n"
   		    "//       sky_fog_mat \n"

            INCLUDE_SCENE_PARAM

            INCLUDE_UNIFORMS

            INCLUDE_FUNCS

            STRINGIFY ( 


            //uniform vec4                fog_params;     
            uniform vec4                SkyFogParams;  
            

            in mat4 viewworld_matrix;                       
			                                            
            )

            STRINGIFY ( 
            
                            
            const float fTwoOverPi = 2.0 / 3.141593;     
            
            in block                                    
            {                                           
                vec3 pos;                               
            } f_in;                                     
			
            out vec4  aFragColor;

            void main (void)                              
            {
                // get point direction
                vec3 vPnt = normalize(f_in.pos.xyz);                

                // fog color
                float fHorizonFactor = fTwoOverPi * acos(max(vPnt.z, 0.0)); 
                //if (vPnt.z<0)
                //    discard;

                // simulate fogging here based on input density
                float fFogDensity = SkyFogParams.a; 
                float fFogHeightRamp = fFogDensity * (2.0 - fFogDensity); 
                float fFogDensityRamp = fFogDensity; 
                //float fFogFactor = mix(pow(fHorizonFactor, 40.0 - 37.0 * fFogHeightRamp), 1.0, fFogDensityRamp);
                float fFogFactor = lerp(pow(fHorizonFactor, 30.0 - 25.0 * fFogHeightRamp), 1.0, fFogDensityRamp); 
                // pow(fHorizonFactor, 35.0 - 34.0 * fFogHeightRamp); 

                // make fogging
                aFragColor = vec4(SkyFogParams.rgb, fFogFactor); 

            }
            )

        };   

        SHADERS_GETTER(get_shader,vs, fs)

        AUTO_REG_NAME(sky, shaders::sky_fog_mat::get_shader)

    }  // ns sky_fog_mat

    namespace clouds_mat 
    {
        const char* vs = {  
            "#extension GL_ARB_gpu_shader5 : enable \n"
  		    "//       clouds_mat \n"

            INCLUDE_COMPABILITY

            STRINGIFY ( 
            uniform mat4 MVP;
            out block
            {
                vec3 pos;
            } v_out;
            
            void main()
            {
                v_out.pos = gl_Vertex.xyz;
                mat4 im = inverse(gl_ModelViewMatrix * MVP);
                vec3 vLocalSpaceCamPos = im[3].xyz;//gl_ModelViewMatrixInverse[3].xyz;

               // gl_Position = gl_ModelViewProjectionMatrix   * vec4(vLocalSpaceCamPos.xyz + gl_Vertex.xyz, 1.0);
                gl_Position =  gl_ModelViewProjectionMatrix * MVP * vec4(vLocalSpaceCamPos.xyz + gl_Vertex.xyz, 1.0);
                gl_Position.z = 0.0;
                //gl_Position.z = gl_Position.w;
            }       
            )
        };


        const char* fs = {
            
            "#extension GL_ARB_gpu_shader5 : enable  \n"
  		    "//       clouds_mat \n"

            STRINGIFY ( 

            uniform sampler2D Clouds;

            // uniforms for sundisc and sunrays
            uniform vec3  frontColor;
            uniform vec3  backColor;
            uniform float density;
            
            
            )

            STRINGIFY ( 


            const float fOneOver2Pi = 0.5 / 3.141593;
            const float fTwoOverPi = 2.0 / 3.141593;   

            in block                                    
            {                                           
                vec3 pos;                               
            } f_in;                                     
            
            out vec4  aFragColor;

            void main (void)                              
            {
               // get point direction
                vec3 vPnt = normalize(f_in.pos);

                // calculate texel coords based on polar angles
                vec2 vTexCoord = vec2(fOneOver2Pi * atan(vPnt.y, vPnt.x) + 0.5, fTwoOverPi * acos(abs(vPnt.z)));

                // get clouds color
                vec4 cl_color = textureLod(Clouds, vTexCoord, 0.0);

                // make fogging
                aFragColor = vec4(cl_color.rgb * frontColor, cl_color.a * density); 

            }
            )

        };   

        SHADERS_GETTER(get_shader,vs, fs)
        
        AUTO_REG_NAME(clouds, shaders::clouds_mat::get_shader)

    }  // ns clouds_mat


	namespace lightning_mat 
	{
		const char* vs = {  
			"#extension GL_ARB_gpu_shader5 : enable \n"
  		    "//       lightning_mat \n"

			INCLUDE_COMPABILITY

			STRINGIFY ( 
			uniform mat4 MVP;
			out block
			{
				vec3 pos;
			} v_out;

			void main()
			{
				v_out.pos = gl_Vertex.xyz;
				mat4 im = inverse(gl_ModelViewMatrix * MVP);
				vec3 vLocalSpaceCamPos = im[3].xyz;

                gl_Position =  gl_ModelViewProjectionMatrix * MVP * vec4(vLocalSpaceCamPos.xyz + gl_Vertex.xyz, 1.0);
				gl_Position.z = 0.0;
			}       
			)
		};


		const char* fs = {

			"#extension GL_ARB_gpu_shader5 : enable  \n"
  		    "//       lightning_mat \n"

			STRINGIFY ( 
            


			uniform sampler2D Lightning;

			// uniforms for sundisc and sunrays
			uniform vec3  frontColor;
			uniform vec3  backColor;
			uniform float density;
            uniform vec2  flash;

			)

			STRINGIFY ( 


		    const float fOneOver2Pi = 0.5 / 3.141593;
			const float fTwoOverPi  = 2.0 / 3.141593;   
            const float fCoeff      = 20.0; 

			in block                                    
			{                                           
				vec3 pos;                               
			} f_in;                                     


			out vec4  aFragColor;

			void main (void)                              
			{
				// get point direction
				vec3 vPnt = normalize(f_in.pos);

                float fOneOver2PiMulAtan = fOneOver2Pi * atan(vPnt.y, vPnt.x);

				// calculate texel coords based on polar angles
				vec2 vTexCoord = vec2(fCoeff * fOneOver2PiMulAtan + 0.5, 1.0 - fTwoOverPi * acos(abs(vPnt.z)));

				// get clouds color
				vec4 cl_color = textureLod(Lightning, vTexCoord, 0.0);

				// make fogging
				// aFragColor = vec4(cl_color.rgb * frontColor, cl_color.a * density);
                
                //step(abs(fOneOver2PiMulAtan) + 0.5, 1.0 )
                float a = mix(1.0, 0.0, abs(0.5 * fOneOver2PiMulAtan) + 0.5);
                vec4  b = vec4(cl_color.rgb * frontColor, cl_color.a * density);

                aFragColor = vec4( a * flash.x + b * flash.y);
                

			}
			)

		};   

		const char* vs_auto = {  
			"#extension GL_ARB_gpu_shader5 : enable \n"
  		    "//       lightning_mat \n"

			INCLUDE_COMPABILITY

			STRINGIFY ( 
			uniform mat4 MVP;
			out block
			{
				vec3 pos;
                vec4 cos_time;
			} v_out;

            uniform float osg_SimulationTime;
            const float fCoeffTime = 5;
			void main()
			{
				v_out.pos = gl_Vertex.xyz;
				mat4 im = inverse(gl_ModelViewMatrix * MVP);
				vec3 vLocalSpaceCamPos = im[3].xyz;

                v_out.cos_time = cos( vec4( osg_SimulationTime,  osg_SimulationTime + 0.02,  osg_SimulationTime + 0.1, osg_SimulationTime + 0.12 ) * fCoeffTime );

				gl_Position =  gl_ModelViewProjectionMatrix * MVP * vec4(vLocalSpaceCamPos.xyz + gl_Vertex.xyz, 1.0);
				gl_Position.z = 0.0;
			}       
			)
		};


		const char* fs_auto = {

			"#extension GL_ARB_gpu_shader5 : enable  \n"
  		    "//       lightning_mat \n"

			STRINGIFY ( 
            


			uniform sampler2D Lightning;

			// uniforms for sundisc and sunrays
			uniform vec3  frontColor;
			uniform vec3  backColor;
			uniform float density;


			)

			STRINGIFY ( 


		    const float fOneOver2Pi = 0.5 / 3.141593;
			const float fTwoOverPi  = 2.0 / 3.141593;   
            const float fCoeff      = 20.0; 
            const float fCoeffLightTime = 0.98;  // cos(0.04 * fCoeffTime);/*0.98;*/
            const float fCoeffLightBackTime = 0.90; //cos(0.0902 * fCoeffTime);/*0.90;*/

			in block                                    
			{                                           
				vec3 pos;                               
                vec4 cos_time;
			} f_in;                                     

			out vec4  aFragColor;

			void main (void)                              
			{
				// get point direction
				vec3 vPnt = normalize(f_in.pos);

                float fOneOver2PiMulAtan = fOneOver2Pi * atan(vPnt.y, vPnt.x);

				// calculate texel coords based on polar angles
				vec2 vTexCoord = vec2(fCoeff * fOneOver2PiMulAtan + 0.5, 1.0 - fTwoOverPi * acos(abs(vPnt.z)));

				// get clouds color
				vec4 cl_color = textureLod(Lightning, vTexCoord, 0.0);

				// make fogging
				// aFragColor = vec4(cl_color.rgb * frontColor, cl_color.a * density);
                
                //step(abs(fOneOver2PiMulAtan) + 0.5, 1.0 )
                float a = mix(1.0, 0.0, abs(0.5 * fOneOver2PiMulAtan) + 0.5);
                vec4  b = vec4(cl_color.rgb * frontColor, cl_color.a * density);

                aFragColor = vec4( a * ( step (fCoeffLightTime, f_in.cos_time[0] ) + step (fCoeffLightTime, f_in.cos_time[2] )) ) 
                                 + b * ( step (fCoeffLightBackTime, f_in.cos_time[1] ) + step (fCoeffLightBackTime, f_in.cos_time[3] ));
                

			}
			)

		};   
		SHADERS_GETTER(get_shader,vs, fs)

	    AUTO_REG_NAME(lightning, shaders::lightning_mat::get_shader)

	}  // ns lightning_mat

    namespace  light_mat
    {

        const char* vs = { 
            INCLUDE_VS
            INCLUDE_UNIFORMS
            INCLUDE_COMPABILITY
            INCLUDE_FUNCS

            STRINGIFY ( 

            vec4 LightScreenSettings;
            
            void main (void)
            {
                LightScreenSettings = vec4(1.0,5.0,10000.0,40000.0);
                // constants here
                float ScreenClarity = LightScreenSettings.x;
                float VerticalScale = LightScreenSettings.y;
                float DistanceFadeOut = LightScreenSettings.z;
                float DistanceFog = LightScreenSettings.w;

                // position in view space
                gl_Position.xyz = gl_Vertex.xyz;

                // vis distances
                //float fDistSqr = dot(gl_Vertex.xyz, gl_Vertex.xyz);
                float fDistSqr = dot(gl_Position.xyz, gl_Position.xyz);
                float fDistInv = inversesqrt(fDistSqr);
                float fDist = fDistInv * fDistSqr;

                // get vis dist factor (1 - before 2/3*D, 0 - after 2*D)
                float fVisDistFactor = saturate(fDistInv * gl_MultiTexCoord0.y - 0.5);
                // make it be more sharp
                fVisDistFactor *= 2.0 - fVisDistFactor;

                // total fogging
                float fGlobalFogFactor = SceneFogParams.a * fDist;
                vec4 cDummy;
                float fLocalFogFactor = 0.0;//suppressVisibilityByLocalBanks(gl_Position.xyz, cDummy);
                float fTotalFogDistAtt = fGlobalFogFactor + fLocalFogFactor;

                // alpha based on fogging
                float fAlphaFogFactor = exp(-0.35 * fTotalFogDistAtt);
                float fAlphaSizeFogFactor = fAlphaFogFactor;
                fAlphaFogFactor *= 2.0 - fAlphaFogFactor;
                // size growing based on fogging
                // reflection size growing is prohibited
                float fSizeFogFactor = lerp(1.0 + 3.5 * step(VerticalScale, 1.0), 1.0, fAlphaSizeFogFactor);

                // global alpha fall-off (for reflections)
                float fAlphaDistFade = saturate(max((fDistInv * DistanceFadeOut - 1.0) * 0.25, 0.03));

                // real size, pixel screen size, magic vertical scale
                //gl_TexCoord[0].xyz = vec3(
                //    gl_MultiTexCoord0.x,
                //    ScreenClarity * fVisDistFactor * fSizeFogFactor,
                //    VerticalScale);

                // color (transp is also modulated by visible distance)
                gl_FrontColor = gl_Color;
                gl_FrontColor.a *= fVisDistFactor * fAlphaDistFade * fAlphaFogFactor;

            }
            )
        };

        const char* fs = { 

            INCLUDE_VS

            STRINGIFY ( 
            // uniform sampler2D texCulturalLight;
            out vec4  aFragColor;

            void main(void)                                                         
            {                                                                       
                aFragColor = vec4(gl_Color.rgb, 1.0);//gl_Color.a * texture2D(texCulturalLight, gl_TexCoord[0].xy).r);                                               
            }

            )
        };

        SHADERS_GETTER(get_shader,vs, fs)

        AUTO_REG_NAME(simlight , shaders::light_mat::get_shader)
    }  // ns light_mat


    namespace  spot_mat
    {

        const char* vs = { 
            
            "#extension GL_ARB_gpu_shader5 : enable \n"

            INCLUDE_VS

            INCLUDE_UNIFORMS
            
            INCLUDE_FUNCS

            STRINGIFY ( 

            uniform mat4 mvp_matrix;

            attribute vec3 position;
            attribute vec3 from_l;
            attribute vec3 l_dir;
            attribute vec3 l_color;
            attribute dist_falloff;
            attribute vec2 cone_falloff;

            out block
            {
                vec3 from_l;
                flat vec3 l_dir;
                flat vec3 l_color;
                flat vec2 dist_falloff;
                flat vec2 cone_falloff;
            } v_out;

            void main()
            {
                gl_Position = mvp_matrix * vec4(position, 1.0);

                v_out.from_l = from_l;
                v_out.l_dir = l_dir;
                v_out.l_color = l_color;
                v_out.dist_falloff = dist_falloff;
                v_out.cone_falloff = cone_falloff;
            }

            )
        };

        const char* fs = { 

            "#extension GL_ARB_gpu_shader5 : enable \n"

            INCLUDE_VS

            STRINGIFY ( 

            in block
            {
                vec3 from_l;
                flat vec3 l_dir;
                flat vec3 l_color;
                flat vec2 dist_falloff;
                flat vec2 cone_falloff;
            } f_in;

            out vec4 aFragColor; 

            void main()
            {
                // get dist falloff
                const float dist_rcp = inversesqrt(dot(f_in.from_l, f_in.from_l));
                const vec3 from_l_nrm = dist_rcp * f_in.from_l;
                const float dist_atten = clamp(fma(dist_rcp, f_in.dist_falloff.x, f_in.dist_falloff.y), 0.0, 1.0);
                // get conical falloff
                const float angle_dot = dot(from_l_nrm, f_in.l_dir);
                const float angle_atten = clamp(fma(angle_dot, f_in.cone_falloff.x, f_in.cone_falloff.y), 0.0, 1.0);
                // diffuse-like term for planar surfaces
                //const float ndotl = clamp(fma(-from_l_nrm.z, 0.35, 0.65), 0.0, 1.0);
                // write color
                const float height_packed = -f_in.from_l.z;
                const float angledist_atten = angle_atten * dist_atten;
                const float angledist_atten_ramped = angledist_atten * (2.0 - angledist_atten);
                aFragColor = vec4(f_in.l_color * (angledist_atten/* * ndotl*/), height_packed * angledist_atten_ramped);
            }

            )
        };

        SHADERS_GETTER(get_shader,vs, fs)

        AUTO_REG_NAME(spot , shaders::spot_mat::get_shader)

    }  // ns light_mat

    namespace  shadow_mat
    {

        const char* vs = { 

            "#extension GL_ARB_gpu_shader5 : enable \n"
            
            SHADOW_INCLUDE 

            STRINGIFY ( 

            //out shadow_block
            //{
            out    vec4 shadow_view0;
            out    vec4 shadow_view1;
            out    vec4 shadow_view2;
            //} shadow_out;

            uniform mat4            shadowMatrix0; 
            uniform mat4            shadowMatrix1;                                                          
            uniform mat4            shadowMatrix2;  

            void shadow_vs_main(vec4 viewpos)
            {   
                SAVE_SHADOWS_VARYINGS_VP(0, shadow_view, viewpos)
                SAVE_SHADOWS_VARYINGS_VP(1, shadow_view, viewpos)
                SAVE_SHADOWS_VARYINGS_VP(2, shadow_view, viewpos)
            }

            )
        };

        const char* fs = { 

            "#extension GL_ARB_gpu_shader5 : enable \n"

            SHADOW_INCLUDE 

            STRINGIFY ( 

            //in shadow_block
            //{
              in vec4 shadow_view0;
              in vec4 shadow_view1;
              in vec4 shadow_view2;
            //} shadow_in;

            uniform float zShadow0; 
            uniform float zShadow1; 
            uniform float zShadow2; 

            uniform sampler2DShadow shadowTexture0; 
            uniform sampler2DShadow shadowTexture1; 
            uniform sampler2DShadow shadowTexture2;

\n          float shadow_fs_main (float illum )
\n          {
\n                  float testZ = gl_FragCoord.z*2.0-1.0;
\n                  float map0  = step(testZ, zShadow0);
\n                  float map1  = step(zShadow0,testZ)*step(testZ, zShadow1);
\n                  float map2  = step(zShadow1,testZ)*step(testZ, zShadow2);
\n                  float fTexelSize=0.00137695;
\n                  float fZOffSet  = -0.001954;
\n
\n                  GENERATE_SHADOW(0,shadow_view)
\n                  GENERATE_SHADOW(1,shadow_view)
\n                  GENERATE_SHADOW(2,shadow_view)
\n
\n                 float v = clamp(term0+term1+term2,0.0,1.0);
\n
\n                 return 1 - v * 0.5;
            }

            )
        };

        SHADERS_GETTER(get_shader_pssm,vs, fs)

        const char* vs_vdsm = { 

            "#extension GL_ARB_gpu_shader5 : enable \n"

            SHADOW_INCLUDE 

            STRINGIFY ( 

            //out vdsm
            //{
            out    vec4 shadow_view0;

            //} shadow_out;

            uniform mat4            shadowMatrix0; 

            void shadow_vs_main(vec4 viewpos)
            {   
                SAVE_SHADOWS_VARYINGS_VP(0, shadow_view, viewpos)
            }

            )
        };

        const char* fs_vdsm = { 

            "#extension GL_ARB_gpu_shader5 : enable \n"

//            "float PCF4_Ext(sampler2DShadow depths,vec4 stpq, float aa); \n"   

            INCLUDE_PCF_EXT

            SHADOW_INCLUDE 

            STRINGIFY ( 

            //in  vdsm
            //{
                in vec4 shadow_view0;
            //} shadow_in;

            uniform sampler2DShadow shadowTexture0; 

\n          float shadow_fs_main (float illum )
\n          {
\n           
                    if(/*ambient.a*/illum > 0.35)
\n                  {
\n                       return PCF4E_Ext(shadowTexture0, /*shadow_in.*/shadow_view0, illum);
\n                  }
\n
\n                  return 1.0;
            }

            )
        };

        SHADERS_GETTER(get_shader_vdsm,vs_vdsm, fs_vdsm)

#ifdef  SHADOW_PSSM
        AUTO_REG_NAME(shadow , shaders::shadow_mat::get_shader_pssm)
#else       
        AUTO_REG_NAME(shadow , shaders::shadow_mat::get_shader_vdsm)
#endif

    }  // ns shadow_mat


    namespace todo
    {
        const char* uni_header = {
        "#extension GL_ARB_uniform_buffer_object : enable\n"

        "layout(std140) uniform colors0\n"
        "{\n"
        "float DiffuseCool;\n"
        "float DiffuseWarm;\n"
        "vec3  SurfaceColor;\n"
        "vec3  WarmColor;\n"
        "vec3  CoolColor;\n"
        "};\n"

        "layout (row_major, std140) uniform SceneParams           \
        {                                                         \
        mat4 projection_matrix;                               \
        mat4 viewworld_matrix;                                \
        mat4 lightmap_matrix;                                 \
        mat4 decal_matrix;                                    \
        mat4 shadow1_matrix;                                  \
        mat4 shadow2_matrix;                                  \
        vec4 shadow_split_borders;                            \
        vec4 light_vec_view;                                  \
        vec4 fog_params;                                      \
        vec4 wind_time;                                       \
        };"

        } ;
    }

}  // ns shaders
