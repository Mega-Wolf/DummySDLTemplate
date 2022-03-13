#version 330 core


in vec2 TexCoord;

uniform vec4 Color;

uniform sampler2D texture1; // Path data
uniform sampler2D texture2; // Noise

out vec4 FragColor;

void main() {
    if (TexCoord.x < 0) {
        // TODO(Tobi): This i sjust used to ignroe the color thing
        FragColor = Color;
        return;
    }

    vec3 pathData = texture(texture1, TexCoord).rgb;

    bool isPath = pathData.b > 0.5;

    if (isPath) {
        FragColor = vec4(0.5, 0.5, 0.5, 1) * texture(texture2, TexCoord * 4);
    } else {
        vec2 light = pathData.rg - vec2(0.5, 0.5);
        light = normalize(light);
        vec2 LIGHT_DIR = vec2 (-1 , -1);
        LIGHT_DIR = normalize(LIGHT_DIR);

        float lightValue = dot(light, LIGHT_DIR) + 1;

        vec3 col = vec3(0.2, 0.6, 0.3) * texture(texture2, TexCoord * 16).rgb;

        

        vec3 finalCol = col * lightValue;

        //FragColor = vec4(0.2, 0.6, 0.3, 1) * texture(texture2, TexCoord * 16) * lightValue;
        //FragColor = vec4(lightValue, lightValue, lightValue, 1 );
        FragColor = vec4(finalCol , 1);
    }
}
