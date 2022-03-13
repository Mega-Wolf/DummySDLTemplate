#version 330 core

#define inc(varname, start, max) for (int varname = start; varname < max; ++varname)
#define inc0(varname, max) for (int varname = 0; varname < max; ++varname)
#define incIn(varname, start, max) for (int varname = start; varname <= max; ++varname)
#define inc0In(varname, max) for (int varname = 0; varname <= max; ++varname)

#define dec(variable, start, min_comp) for (int variable = (start); --variable >= (min_comp); )
#define dec0(variable, start) for (int variable = (start); --variable >= 0; )
#define decIn(variable, start, min_comp) for (int variable = (start); variable >= (min_comp); --variable)
#define dec0In(variable, start) for (int variable = (start); variable >= 0; --variable)

#define PI 3.141592

in vec2 TexCoord;

uniform vec4 Color; // TODO(Tobi): I misuse this at the moment as a horizontal bool

uniform sampler2D texture1;

out vec4 FragColor; // TODO(Tobi): I might turn this 1d

void main() {
    if (TexCoord.x < 0) {
        // TODO(Tobi): This i sjust used to ignroe the color thing
        FragColor = Color;
        return;
    }

    const int SIZE = 4;

    float pathValue = texture(texture1, TexCoord).r;

    vec2 texOffset = 1.0 / textureSize(texture1, 0);

    /// Apply filter
    vec2 result = vec2(0, 0);
    incIn (y_i,   -SIZE,    +SIZE) {
        incIn (x_i,   -SIZE,    +SIZE) {
            if (x_i == 0 && y_i == 0) { continue; }
            float value = texture(texture1, TexCoord + vec2(texOffset.x * x_i, texOffset.y * y_i)).r;
            result += (value - 0.5) / sqrt(x_i * x_i + y_i * y_i) * vec2(x_i, y_i);
        }
    }

    //result = normalize(result);

    FragColor = vec4(result + 0.5, pathValue, 1);
}
