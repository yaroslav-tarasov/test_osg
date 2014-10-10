varying vec4 color;

void main()
{
    gl_FragColor = clamp(color, 0.0, 1.0);
}
