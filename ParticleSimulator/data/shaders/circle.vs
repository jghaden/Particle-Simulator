#version 330 core

layout(location = 0) in vec2 aPos;      // Vertex position in normalized device coordinates (NDC)
out vec2 fragCoord;                     // Pass position to fragment shader

void main() {
    fragCoord = aPos;                   // Pass position
    gl_Position = vec4(aPos, 0.0, 1.0); // Convert to clip space
}
