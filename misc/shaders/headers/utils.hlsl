                                                                              
    float saturate( const in float x )                                                               
    {                                                                                                
        return clamp(x, 0.0, 1.0);                                                                   
    }                                                                                                
                                                                                                     
    float lerp(float a, float b, float w)                                                            
    {                                                                                                
        return a + w*(b-a);                                                                          
    }                                                                                                
                                                                                                     
    vec3 hardlight( const in vec3 color, const in vec3 hl )                                          
    {                                                                                                
        vec3 hl_pos = step(vec3(0.0), hl);                                                           
        return (vec3(1.0) - hl_pos) * color * (hl + vec3(1.0)) +                                     
            hl_pos * mix(color, vec3(1.0), hl);                                                      
    }                                                                                                
                                                                                                     
    float tex_detail_factor( const in vec2 tex_c_mod, const in float coef )                          
    {                                                                                                
        vec2 grad_vec = fwidth(tex_c_mod);                                                           
        float detail_fac = exp(coef * dot(grad_vec, grad_vec));                                      
        return detail_fac * (2.0 - detail_fac);                                                      
    }                                                                                                
                                                                                                     
    float ramp_up( const in float x )                                                                
    {                                                                                                
        return x * fma(x, -0.5, 1.5);                                                                
    }                                                                                                
                                                                                                     
    float ramp_down( const in float x )                                                              
    {                                                                                                
        return x * fma(x, 0.5, 0.5);                                                                 
    }                                                                                                
                                                                                                         
    float luminance_crt( const in vec4 col )                                                         
    {                                                                                                
        const vec4 crt = vec4(0.299, 0.587, 0.114, 0.0);                                             
        return dot(col,crt);                                                                         
    }                                                                                                
                                                                                                     
                                                                                                  
    float PCF4E(sampler2DShadow depths,vec4 stpq,ivec2 size){                                    
            float result = 0.0;                                                                      
            int   count = 0;                                                                         
            for(int x=-size.x; x<=size.x; x++){                                                      
                for(int y=-size.y; y<=size.y; y++){                                                  
                    count++;                                                                         
                    result += textureProjOffset(depths, stpq, ivec2(x,y));/*.r;*/                    
                }                                                                                    
            }                                                                                        
            return result/count;                                                                     
        }                                                                                              
                                                                                                       
    float PCF4(sampler2DShadow depths,vec4 stpq,ivec2 size){                                     
            float result = 0.0;                                                                      
            result += textureProjOffset(depths, stpq, ivec2(0,-1));/*.r;*/                           
            result += textureProjOffset(depths, stpq, ivec2(0,1));/*.r;*/                            
            result += textureProjOffset(depths, stpq, ivec2(1,0));/*.r;*/                            
            result += textureProjOffset(depths, stpq, ivec2(-1,0));/*.r;*/                           
            return result*.25;                                                                       
    }                                                                                                  
                                                                                                       
    float PCF(sampler2DShadow depths,vec4 stpq,ivec2 size){                                      
            return textureProj(depths, stpq);/*.r;*/                                                 
    }                                                                                                  
    
    const ivec2 pcf_size = ivec2(1,1);                                                               
      
