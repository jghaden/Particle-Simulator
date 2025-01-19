#version 330 core

in vec2 fragCoord;                 // Position from vertex shader
uniform vec2 circleCenter;         // Circle center in NDC
uniform float radius;              // Circle radius in NDC
uniform float outlineThickness;    // Outline thickness in NDC
uniform vec4 fillColor;            // Fill color for the circle
uniform vec4 outlineColor;         // Outline color for the circle

out vec4 FragColor;                // Output color

void main() {
    float dist = distance(fragCoord, circleCenter); // Distance from center

    if (dist > radius + outlineThickness) {
        discard; // Outside the circle and outline
    } else if (dist > radius) {
        FragColor = outlineColor; // Outline region
    } else {
        FragColor = fillColor;    // Inside the circle
    }
}
