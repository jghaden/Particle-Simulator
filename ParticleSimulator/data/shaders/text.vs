#version 330 core

layout(location = 0) in vec2 aPosition;    // Vertex position (x, y)
layout(location = 1) in vec2 aTexCoord;    // Texture coordinates (u, v)

out vec2 TexCoord;                         // Output texture coordinate to fragment shader

uniform mat4 projection;                   // Projection matrix (orthographic)
uniform float zoom;                        // Optional zoom factor for scaling

void main()
{
    // Apply the zoom and projection matrix to the vertex position
    gl_Position = projection * vec4(aPosition * zoom, 0.0, 1.0);

    // Pass the texture coordinate to the fragment shader
    TexCoord = aTexCoord;
}
