                                                                                      
    const int nMaxLights = 124;                                                                           
    const float PI = 3.14159265358979323846264; 
    
    uniform int LightsActiveNum;                                                                         
    
    uniform vec4 LightVSPosAmbRatio[nMaxLights];                                                         
    uniform vec4 LightVSDirSpecRatio[nMaxLights];                                                        
    uniform vec4 LightAttenuation[nMaxLights];                                                           
    uniform vec3 LightDiffuse[nMaxLights];                                                               
    
    void ComputeDynamicLights( in vec3 vViewSpacePoint, in vec3 vViewSpaceNormal, in vec3 vReflVec, inout vec3 cAmbDiff, inout vec3 cSpecular ) 
   {                                                                                                     
   int curLight = 0;                                                                                     
   cAmbDiff = vec3(0.0f,0.0f,0.0f);                                                                      
   cSpecular = vec3(0.0f,0.0f,0.0f);                                                                     
   while (curLight < LightsActiveNum)                                                                    
       {                                                                                                 
       vec4 curVSPosAmbRatio  = LightVSPosAmbRatio[curLight];                                            
       vec4 curVSDirSpecRatio = LightVSDirSpecRatio[curLight];                                           
       vec4 curAttenuation    = LightAttenuation[curLight];                                              
       vec3 curDiffuse        = LightDiffuse[curLight];                                                  
       
       vec3 vVecToLight = curVSPosAmbRatio.xyz - vViewSpacePoint;                                        
       float vDistToLightInv = inversesqrt(dot(vVecToLight, vVecToLight));                               
       vec3 vDirToLight = vDistToLightInv * vVecToLight;                                                 
       
       float fAngleDot = dot(vDirToLight, curVSDirSpecRatio.xyz);                                        
       float fTotalAtt = clamp(curAttenuation.z * fAngleDot + curAttenuation.w, 0.0, 1.0);               
                                                                                                         
       fTotalAtt *= clamp(curAttenuation.x * vDistToLightInv + curAttenuation.y, 0.0, 1.0);              
       
       if (fTotalAtt != 0.0)                                                                             
           {                                                                                             
           
           float fDiffuseDot = dot(vDirToLight, vViewSpaceNormal);                                       
           cAmbDiff += (fTotalAtt * (curVSPosAmbRatio.w + clamp(fDiffuseDot, 0.0, 1.0))) * curDiffuse;   
           
           float fSpecPower = clamp(dot(vReflVec, vDirToLight), 0.0, 1.0);                               
           fSpecPower *= fSpecPower;                                                                     
           fSpecPower *= fSpecPower;                                                                     
           cSpecular += (fTotalAtt * curVSDirSpecRatio.w * fSpecPower) * curDiffuse;                     
           }                                                                                             
                                                                                                         
                                                                                                         
           ++curLight;                                                                                   
       }                                                                                                 
                                                                                                         
                                                                                                         
       return;                                                                                           
   }                                                                                                     
                                                                                                      
   void compute_dynamic_lights( in vec3 vViewSpacePoint, in vec3 vViewSpaceNormal, in vec3 vReflVec, inout vec3 cAmbDiff, inout vec3 cSpecular ) 
   {                                                                                                     
   int curLight = 0;                                                                                     
   cAmbDiff = vec3(0.0f,0.0f,0.0f);                                                                      
   cSpecular = vec3(0.0f,0.0f,0.0f);                                                                     
   while (curLight < LightsActiveNum)                                                                    
       {                                                                                                 
       vec4 curVSPosAmbRatio  = LightVSPosAmbRatio[curLight];                                            
       vec4 curVSDirSpecRatio = LightVSDirSpecRatio[curLight];                                           
       vec4 curAttenuation    = LightAttenuation[curLight];                                              
       vec3 curDiffuse        = LightDiffuse[curLight];                                                  
                                                                                                         
                                                                                                         
        vec4 specular_ =  vec4(curDiffuse * curVSDirSpecRatio.w,1.0);                                    
        vec3 vVecToLight = curVSPosAmbRatio.xyz - vViewSpacePoint;                                       
        float vDistToLightInv = inversesqrt(dot(vVecToLight, vVecToLight));                              
        vec3 vDirToLight = vDistToLightInv * vVecToLight;                                                
                                                                                                         
        float fAngleDot = dot(vDirToLight, curVSDirSpecRatio.xyz);                                       
        float fTotalAtt = clamp(curAttenuation.z * fAngleDot + curAttenuation.w, 0.0, 1.0);              
                                                                                                         
        /*float*/ fTotalAtt = clamp(curAttenuation.x * vDistToLightInv + curAttenuation.y, 0.0, 1.0);    
                                                                                                         
        float intensity = 0.0;                                                                           
        vec4 spec = vec4(0.0);                                                                           
        vec3 ld = normalize(vVecToLight);                                                                
        vec3 sd = normalize(vec3(-curVSDirSpecRatio.xyz));                                               
                                                                                                         
        if (dot(ld,sd) > cos(PI/4)) {                                                        
                                                                                                         
                    vec3 n = normalize(vViewSpaceNormal);                                                
                    intensity = max(dot(n,ld), 0.0);                                                     
                                                                                                         
                        if (intensity > 0.0) {                                                           
                                vec3 eye = normalize(vViewSpacePoint);                                   
                                vec3 h = normalize(ld + eye);                                            
                                float intSpec = max(dot(h,n), 0.0);                                      
                                spec = specular_ * pow(intSpec, 1/*shininess*/);                         
                            }                                                                            
                }                                                                                        
                                                                                                         
           cAmbDiff += fTotalAtt *(intensity * curDiffuse + spec.rgb);                                   
                                                                                                         
                                                                                                         
           ++curLight;                                                                                   
       }                                                                                                 
                                                                                                         
       return;                                                                                           
   } 