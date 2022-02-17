#version 330 core


in vec2 TexCoord;

uniform vec4 Color;

out vec4 FragColor;

void main() {
    
    vec2 diff = 2 * abs(TexCoord - vec2(0.5, 0.5));
    float lengthSquared = diff.x * diff.x + diff.y * diff.y;
    
    if (lengthSquared >= 0.9 && lengthSquared <= 1) {
        FragColor = Color;
    } else {
        discard;
    }
}
