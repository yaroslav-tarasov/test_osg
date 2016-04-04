#extension GL_ARB_gpu_shader5 : enable
//       flame_mat 

//in vec2 fragCoord;

attribute vec3 tangent;
attribute vec3 binormal;
out       mat4 viewworld_matrix;
out       vec2 fragCoord;

void main()
{
    vec3 normal      = normalize(gl_NormalMatrix * gl_Normal);
    vec4 viewpos     = gl_ModelViewMatrix * gl_Vertex;
    viewworld_matrix = inverse(gl_ModelViewMatrix);
    gl_Position      = gl_ModelViewProjectionMatrix *  gl_Vertex;
    fragCoord        = vec2(gl_Vertex.zx);
	//fragCoord        = viewpos.xy;
    // gl_Position.z = 0;
}       
