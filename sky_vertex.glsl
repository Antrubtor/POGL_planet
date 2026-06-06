#version 450

in vec2 position;
out vec2 vTexCoord;

void main()
{
    vTexCoord = position * 0.5 + 0.5;
    gl_Position = vec4(position, 0.999, 1.0);
}
