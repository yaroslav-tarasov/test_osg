	#version 430 compatibility  
	#extension GL_ARB_gpu_shader5 : enable  
    // 
	//    rope.vert
	//
    
	out mat4 viewworld_matrix;

	#include "scene_params.hlsl"
	#include "lights.hlsl"
	
	uniform vec2 Settings;  

	out block 
    {         
		centroid out vec2  texcoord;  
		centroid out float fade;  
		out float		   light; 
		vec3               viewpos;
	    LIGHTMAP_VARYINGS; 
    } v_out;    
	    
	void main(void)  
	{  
	    float w            = dot(transpose(gl_ModelViewProjectionMatrix)[3], vec4(gl_Vertex.xyz, 1.0f));   
	    float pixel_radius = w * Settings.x;   
	    float radius       = max(Settings.y, pixel_radius);   
	    v_out.fade         = Settings.y / radius;   
	    vec3 position      = gl_Vertex.xyz + radius * gl_Normal.xyz;     
        v_out.texcoord     = vec2( gl_MultiTexCoord0.x * (0.02f / radius), gl_MultiTexCoord0.y)   ;   
	    v_out.light        = dot(light_vec_view.xyz,gl_Normal.xyz) * 0.5f + 0.5f;  
		v_out.viewpos      = (gl_ModelViewMatrix * vec4(position, 1.0f)).xyz;
		 
		viewworld_matrix = inverse(gl_ModelViewMatrix);

	    gl_Position        = gl_ModelViewProjectionMatrix * vec4(position, 1.0f) ; 
	} ;