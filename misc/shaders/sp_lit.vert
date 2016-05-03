#extension GL_ARB_gpu_shader5 : enable
//      sp_lit.vert

out       vec3 pos;
out       vec3 normal;
out       mat4 viewworld_matrix;
out       vec2 fragCoord;

void main()
{
	gl_Position        = gl_ModelViewProjectionMatrix*gl_Vertex;
	gl_TexCoord[0]     = gl_Vertex;
	gl_TexCoord[1].xyz = gl_Normal.xyz;
	gl_TexCoord[2].xy  = gl_MultiTexCoord0.xy;
	normal             = normalize(gl_NormalMatrix * gl_Normal);
	pos                = (gl_ModelViewProjectionMatrix*gl_Vertex).xyz;
}      
