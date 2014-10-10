varying vec4 color;

void main()
{
    color = gl_Vertex;
    gl_Position = ftransform();
}
