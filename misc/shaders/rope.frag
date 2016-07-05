	#version 430 compatibility  
	#extension GL_ARB_gpu_shader5 : enable  
   
    in block 
    {         
		centroid in vec2 texcoord;  
		centroid in float fade;  
		in float light;  
    } v_in;
	        
	uniform sampler2D colorTex;  
	out vec4 FragColor;    
	 
	void main(void)  
	{  
	 
	    FragColor = vec4(texture2D(colorTex, v_in.texcoord.xy).xyz * v_in.light, v_in.fade);
	} ;