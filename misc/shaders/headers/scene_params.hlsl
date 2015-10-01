        
    uniform vec4 ambient;                                                                           
    uniform vec4 diffuse;                                                                           
    uniform vec4 specular;                                                                          
    uniform vec4 light_vec_view; 

    uniform sampler2D        ViewLightMap;                                                       
    uniform sampler2D        detailTex;                                                          
    uniform samplerCube      envTex;                                                             
    uniform sampler2DShadow  ShadowSplit0;                                                       
    uniform sampler2DShadow  ShadowSplit1;                                                       
    uniform sampler2DShadow  ShadowSplit2;                                                       
    uniform sampler2D        ViewDecalMap;                                                       
    uniform vec4             SceneFogParams;     
    
    uniform sampler2D        baseTexture;                                                                     
    uniform int              baseTextureUnit;                                                                       
    uniform sampler2DShadow  shadowTexture0;                                                            
    uniform int              shadowTextureUnit0;                                                                    
    uniform mat4             shadowMatrix0;                                                                                                                                                                    

                                                                                                     
    uniform mat4             shadow0_matrix;
  