#version 330 core

in vec2 TexCoord;              // Texture coordinates passed from the vertex shader
out vec4 FragColor;            // Final fragment color

uniform sampler2D text;        // Font texture (glyph texture)
uniform vec3 textColor;        // Text color passed as a uniform

void main()
{
    // Sample the alpha channel from the texture
    float alpha = texture(text, TexCoord).r;

    // Use textColor for the RGB channels and alpha from the texture
    FragColor = vec4(textColor, alpha);
}
