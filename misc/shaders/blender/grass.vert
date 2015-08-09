// grass.vert
// blender version

#version 150 compatibility

out GroundData {
	vec2 TexCoords;
	vec3 Normal;
} grnd;

void main()
{
	grnd.Normal = gl_Normal;

	grnd.TexCoords = gl_MultiTexCoord0.st;

	gl_Position = gl_Vertex;
}