#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

uniform mat4 MVP;
uniform float zoom;

out vec3 ourColor;

void main()
{
    gl_Position = MVP * vec4(aPos, 1.0);
    gl_PointSize = 8.0 * zoom;  // Adjust the size of the points here
    ourColor = aColor;
}
