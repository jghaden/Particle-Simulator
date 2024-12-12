#version 330 core

in vec3 ourColor;

out vec4 FragColor;

void main()
{
    // Calculate distance from center of the point
    vec2 coord = gl_PointCoord - vec2(0.5);
    float distance = length(coord);
    if (distance > 0.5)  // Outside the radius
    {
        discard;
    }

    FragColor = vec4(ourColor, 1.0);
}
