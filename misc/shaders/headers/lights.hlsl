    
 #define GET_LIGHTMAP(viewpos, in_frag)   \                                                                        
  /*const*/ float height_world_lm = in_frag.lightmap_coord.z;  \                                                 
  /*const*/ vec4 lightmap_data = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; \                       
  /*const*/ float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.4, 0.75), 0.0, 1.0); \
			vec3 lightmap_color = lightmap_data.rgb * lightmap_height_fade;                                              
                                                                                                               
 #define LIGHTMAP_VARYINGS  \                                                                                    
     vec4 lightmap_coord                                                                                          
                                                                                                                  
 #define SAVE_LIGHTMAP_VARYINGS_VP(out_vert, view_pos_4) \                                                       
     out_vert.lightmap_coord.xyw = (lightmap_matrix * view_pos_4).xyw;  \                                        
     out_vert.lightmap_coord.z = (viewworld_matrix * view_pos_4).z;  \                                             
                                                                                                                  
 #define GET_LIGHTMAP_ZTRICK(in_frag)  \                                                                         
     /*const*/ float height_world_lm = in_frag.lightmap_coord.z;   \                                             
     /*const*/ vec4 lightmap_data = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; \                   
     /*const*/ float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.075, 1.5), 0.0, 1.0); \   
     vec3 lightmap_color = lightmap_data.rgb * lightmap_height_fade;                                              
                                                                                                                  
 #define LIGHTMAP_SHADOW_TRICK(shadow_force) \                                                                   
     /*const*/ float lm_shadow_omit_coef = clamp(fma(lightmap_data.w, -0.2, 1.6), 0.0, 1.0); \                    
     lightmap_color *= fma(mix(shadow_force, 1.0, lm_shadow_omit_coef), 0.7, 0.3);                                
                                                                                                                  
 #define LIGHTMAP_BUILDING_HEIGHT_TRICK \                                                                        
     lightmap_color *= clamp(fma(height_world_lm, -2.5, 1.25), 0.0, 1.0);                                    