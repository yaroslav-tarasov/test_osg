// grass.frag
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2D tex;

void main()
{
	vec4 color = texture2D(tex,gl_TexCoord[0].st);
	gl_FragColor = color;
}
