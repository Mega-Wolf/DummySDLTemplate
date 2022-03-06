#version 330 core


in vec2 TexCoord;

uniform vec4 Color;

uniform sampler2D texture1;
uniform sampler2D texture2;

out vec4 FragColor;

void main() {
    if (TexCoord.x < 0) {
        FragColor = Color;
        return;
    }

    bool isPath = texture(texture1, TexCoord).b > 0.5;
    float actualPath = texture(texture1, TexCoord).r;
    float pathValue = texture(texture1, TexCoord).r + 1 * (texture(texture2, TexCoord * 1).r - 0.5); // +  2 * (texture(texture2, TexCoord * 5).r - 0.5);

    // if (pathValue > 0.45 && pathValue <= 0.5) {
    //     FragColor = vec4(0, 0, 0, 1);
    // } else

    if (actualPath > 0.05 && pathValue > 0.7) {
        //FragColor = Color * 
        //FragColor = vec4(0, 1, 0, 1);
        FragColor = vec4(0.5, 0.5, 0.5, 1) * texture(texture2, TexCoord * 4);
    } else if (actualPath > 0.9) {
        //FragColor = vec4(0, 1, 0, 1);
        FragColor = vec4(0.6, 0.6, 0.6, 1) * texture(texture2, TexCoord * 4);
    } else {
        //FragColor = vec4(actualPath, 0, 0, 1);
        //FragColor = vec4(1,1,1,1);
        FragColor = vec4(0.2, 0.6, 0.3, 1) * texture(texture2, TexCoord * 16);
    }

    //FragColor = vec4(0, 0.5, 0, 1) * texture(texture2, TexCoord * 5);
}
