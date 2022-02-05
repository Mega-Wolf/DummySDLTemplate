#version 330 core


in vec2 TexCoord;

uniform vec4 Color;
uniform sampler2D texture1;

out vec4 FragColor;

void main() {
    FragColor = Color * texture(texture1, TexCoord);
}