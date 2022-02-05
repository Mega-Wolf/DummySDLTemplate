#version 330 core


in vec2 TexCoord;

uniform vec4 Color;
uniform sampler2D texture1;

out vec4 FragColor;

void main() {
    if (
        abs(TexCoord.x - 0.5) > 0.45 ||
        abs(TexCoord.y - 0.5) > 0.45
    ) {
        FragColor = Color;
    } else {
        discard;
    }
    
}