#shader vertex
#version 330 core

layout(location = 0) in vec4 position;

uniform mat4 u_MVP;

out vec2 TexCoords;

void main() 
{
    gl_Position = u_MVP * position;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

void main() 
{
    color = vec4(1.0, 1.0, 1.0, 1.0);
}