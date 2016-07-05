	#version 430 compatibility  
	#extension GL_ARB_gpu_shader5 : enable  
    
	vec3 light_dir = normalize(vec3(-0.79f, 0.39f, 0.47f));  
    
	uniform vec2 Settings;  
    
	out block 
    {         
		centroid out vec2 texcoord;  
		centroid out float fade;  
		out float light;  
    } v_out;    
	    
	void main(void)  
	{  
	    float w            = dot(transpose(gl_ModelViewProjectionMatrix)[3], vec4(gl_Vertex.xyz, 1.0f));   
	    float pixel_radius = w * Settings.x;   
	    float radius       = max(Settings.y, pixel_radius);   
	    v_out.fade         = Settings.y / radius;   
	    vec3 position      = gl_Vertex.xyz + radius * gl_Normal.xyz;     
        v_out.texcoord     = vec2( gl_MultiTexCoord0.x * (0.02f / radius), gl_MultiTexCoord0.y)   ;   
	    v_out.light        = dot(light_dir,gl_Normal.xyz) * 0.5f + 0.5f;   
	    gl_Position        = gl_ModelViewProjectionMatrix * vec4(position, 1.0f) ; 
	} ;