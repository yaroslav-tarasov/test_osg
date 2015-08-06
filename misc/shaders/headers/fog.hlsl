    
    float fog_decay_factor( const in vec3 view_pos )                                                 
    {                                                                                                
        return exp(-/*fog_params*/SceneFogParams.a * dot(view_pos, view_pos));                       
    }    

    vec3 apply_scene_fog( const in vec3 view_pos, const in vec3 color )                              
    {                                                                                                
        vec3 view_vec_fog = (mat3(viewworld_matrix) * view_pos) * vec3(1.0, 1.0, 0.8);               
        return mix(/*textureCube*/texture(envTex, view_vec_fog).rgb, color, fog_decay_factor(view_vec_fog));    
        /*return mix(textureLod(envTex, view_vec_fog, 3.0).rgb, color, fog_decay_factor(view_vec_fog));*/   
    }                                                                                                
                                                                                                     
    vec3 apply_clear_fog( const in vec3 view_pos, const in vec3 color )                              
    {                                                                                                
        return mix(/*fog_params*/SceneFogParams.rgb, color, fog_decay_factor(view_pos));             
    }       
                                                                                                        
