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

    const int SIZE = 15;
    const float STD_DEV = 7.5;

    bool horizontal = Color.r > 0.5;

    /// Create the filter
    float kernel[SIZE + SIZE + 1];
    float sumOfValues = 0;
    incIn (i,   -SIZE,    +SIZE) {
        float value = 1/sqrt(2 * PI * STD_DEV * STD_DEV) * exp(-(i * i) / (2 * STD_DEV * STD_DEV));
        sumOfValues += value;
        kernel[i + SIZE] = value;
    }

    inc0 (i,   SIZE + SIZE + 1) {
        kernel[i] /= sumOfValues;
    }

    float pathValue = texture(texture1, TexCoord).r;

    vec2 texOffset = 1.0 / textureSize(texture1, 0);

    /// Apply filter
    float result = 0;
    if (horizontal) {
        incIn (x_i,   -SIZE,    +SIZE) {
            result += kernel[x_i + SIZE] * texture(texture1, TexCoord + vec2(texOffset.x * x_i, 0)).r;
        }
    } else {
        incIn (y_i,   -SIZE,    +SIZE) {
            result += kernel[y_i + SIZE] * texture(texture1, TexCoord + vec2(0, texOffset.y * y_i)).r;
        }
    }

    FragColor = vec4(result, result, result, 1);
}
