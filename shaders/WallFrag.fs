#version 330 core


in vec2 TexCoord;

uniform vec4 Color;

uniform sampler2D texture1;
uniform sampler2D texture2;

out vec4 FragColor;

void main() {
    FragColor = Color * texture(texture1, TexCoord);
    if (FragColor.a < 0.5) {
        // TODO(Tobi): Maybe prodive a uniform float for determining the cutoff value
        discard;
    }
}
