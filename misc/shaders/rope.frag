	#version 430 compatibility  
	#extension GL_ARB_gpu_shader5 : enable  
    // 
	//    rope.frag
	//
	
	
	in mat4 viewworld_matrix;

	#include "scene_params.hlsl"
	#include "lights.hlsl"
	#include "fog.hlsl"
	

    in block 
    {         
		centroid in vec2 texcoord;  
		centroid in float fade;  
		in float light;
		vec3 viewpos;
	    LIGHTMAP_VARYINGS;   
    } f_in;
	        
	uniform sampler2D colorTex;  
	out vec4 aFragColor;    
	 
	void main(void)  
	{  
	 
	    //aFragColor = vec4(texture2D(colorTex, f_in.texcoord.xy).xyz * f_in.light, f_in.fade);
		vec3 result = texture2D(colorTex, f_in.texcoord.xy).xyz * f_in.light;
		aFragColor = vec4(apply_clear_fog(f_in.viewpos, result), f_in.fade);
	} ;