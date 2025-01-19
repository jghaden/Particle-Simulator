#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;

uniform mat4 MVP;

out vec3 ourColor;

void main()
{
    gl_Position = MVP * vec4(aPos, 0.0, 1.0);
    gl_PointSize = 8.0;
    ourColor = aColor;
}
