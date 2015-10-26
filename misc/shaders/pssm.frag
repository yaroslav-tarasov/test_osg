uniform sampler2D baseTexture; 
uniform float enableBaseTexture; 
uniform vec2 ambientBias;
uniform sampler2DShadow shadowTexture0; 
uniform float zShadow0; 
uniform sampler2DShadow shadowTexture1; 
uniform float zShadow1; 
uniform sampler2DShadow shadowTexture2; 
uniform float zShadow2; 

#define GENERATE_SHADOW(ind) \
    float shadowOrg#ind = shadow2D( shadowTexture#ind,gl_TexCoord[#ind + 1].xyz+vec3(0.0,0.0,fZOffSet) ).r;                 \
    float shadow0#ind = shadow2D( shadowTexture#ind,gl_TexCoord[#ind + 1].xyz+vec3(-fTexelSize,-fTexelSize,fZOffSet) ).r;   \ 
    float shadow1#ind = shadow2D( shadowTexture#ind,gl_TexCoord[#ind + 1].xyz+vec3( fTexelSize,-fTexelSize,fZOffSet) ).r;   \
    float shadow2#ind = shadow2D( shadowTexture#ind,gl_TexCoord[#ind + 1].xyz+vec3( fTexelSize, fTexelSize,fZOffSet) ).r;   \
    float shadow3#ind = shadow2D( shadowTexture#ind,gl_TexCoord[#ind + 1].xyz+vec3(-fTexelSize, fTexelSize,fZOffSet) ).r;   \
    float shadow#ind = ( 2.0*shadowOrg#ind + shadow0#ind + shadow1#ind + shadow2#ind + shadow3#ind)/6.0;                    \    
    float term#ind = map#ind*(1.0-shadow#ind); 


void main(void)
{
    float testZ = gl_FragCoord.z*2.0-1.0;
    float map0 = step(testZ, zShadow0);
    float map1  = step(zShadow0,testZ)*step(testZ, zShadow1);
    float map2  = step(zShadow1,testZ)*step(testZ, zShadow2);
    float fTexelSize=0.00137695;
    float fZOffSet  = -0.001954;
    
#if 0 
    float shadowOrg0 = shadow2D( shadowTexture0,gl_TexCoord[1].xyz+vec3(0.0,0.0,fZOffSet) ).r;
    float shadow00 = shadow2D( shadowTexture0,gl_TexCoord[1].xyz+vec3(-fTexelSize,-fTexelSize,fZOffSet) ).r;
    float shadow10 = shadow2D( shadowTexture0,gl_TexCoord[1].xyz+vec3( fTexelSize,-fTexelSize,fZOffSet) ).r;
    float shadow20 = shadow2D( shadowTexture0,gl_TexCoord[1].xyz+vec3( fTexelSize, fTexelSize,fZOffSet) ).r;
    float shadow30 = shadow2D( shadowTexture0,gl_TexCoord[1].xyz+vec3(-fTexelSize, fTexelSize,fZOffSet) ).r;
    float shadow0 = ( 2.0*shadowOrg0 + shadow00 + shadow10 + shadow20 + shadow30)/6.0;
    float shadowOrg1 = shadow2D( shadowTexture1,gl_TexCoord[2].xyz+vec3(0.0,0.0,fZOffSet) ).r;
    float shadow01 = shadow2D( shadowTexture1,gl_TexCoord[2].xyz+vec3(-fTexelSize,-fTexelSize,fZOffSet) ).r;
    float shadow11 = shadow2D( shadowTexture1,gl_TexCoord[2].xyz+vec3( fTexelSize,-fTexelSize,fZOffSet) ).r;
    float shadow21 = shadow2D( shadowTexture1,gl_TexCoord[2].xyz+vec3( fTexelSize, fTexelSize,fZOffSet) ).r;
    float shadow31 = shadow2D( shadowTexture1,gl_TexCoord[2].xyz+vec3(-fTexelSize, fTexelSize,fZOffSet) ).r;
    float shadow1 = ( 2.0*shadowOrg1 + shadow01 + shadow11 + shadow21 + shadow31)/6.0;
    float shadowOrg2 = shadow2D( shadowTexture2,gl_TexCoord[3].xyz+vec3(0.0,0.0,fZOffSet) ).r;
    float shadow02 = shadow2D( shadowTexture2,gl_TexCoord[3].xyz+vec3(-fTexelSize,-fTexelSize,fZOffSet) ).r;
    float shadow12 = shadow2D( shadowTexture2,gl_TexCoord[3].xyz+vec3( fTexelSize,-fTexelSize,fZOffSet) ).r;
    float shadow22 = shadow2D( shadowTexture2,gl_TexCoord[3].xyz+vec3( fTexelSize, fTexelSize,fZOffSet) ).r;
    float shadow32 = shadow2D( shadowTexture2,gl_TexCoord[3].xyz+vec3(-fTexelSize, fTexelSize,fZOffSet) ).r;
    float shadow2 = ( 2.0*shadowOrg2 + shadow02 + shadow12 + shadow22 + shadow32)/6.0;
#endif
    
    GENERATE_SHADOW(0)
    GENERATE_SHADOW(1)
    GENERATE_SHADOW(2)

#if 0     
    float term0 = map0*(1.0-shadow0); 
    float term1 = map1*(1.0-shadow1);
    float term2 = map2*(1.0-shadow2);
#endif
    
    float v = clamp(term0+term1+term2,0.0,1.0);
    vec4 color    = gl_Color; 
    vec4 texcolor = texture2D(baseTexture,gl_TexCoord[0].st); 
    float enableBaseTextureFilter = enableBaseTexture*(1.0 - step(texcolor.x+texcolor.y+texcolor.z+texcolor.a,0.0)); 
    vec4 colorTex = color*texcolor;
    // gl_FragColor.rgb = (((color*(ambientBias.x+1.0)*(1.0-enableBaseTextureFilter)) + colorTex*(1.0+ambientBias.x)*enableBaseTextureFilter)*(1.0-ambientBias.y*v)).rgb; 
    gl_FragColor.rgb =  (ambientBias.x+1.0) * mix (color,colorTex*(1.0-ambientBias.y*v),enableBaseTextureFilter).rgb;
    // gl_FragColor.a = (color*(1.0-enableBaseTextureFilter) + colorTex*enableBaseTextureFilter).a; 
    gl_FragColor.a = mix (color,colorTex,enableBaseTextureFilter).a;
}