#version 330 core

#define inc(varname, start, max) for (int varname = start; varname < max; ++varname)
#define inc0(varname, max) for (int varname = 0; varname < max; ++varname)
#define incIn(varname, start, max) for (int varname = start; varname <= max; ++varname)
#define inc0In(varname, max) for (int varname = 0; varname <= max; ++varname)

#define dec(variable, start, min_comp) for (int variable = (start); --variable >= (min_comp); )
#define dec0(variable, start) for (int variable = (start); --variable >= 0; )
#define decIn(variable, start, min_comp) for (int variable = (start); variable >= (min_comp); --variable)
#define dec0In(variable, start) for (int variable = (start); variable >= 0; --variable)

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

vec4 lerp(vec4 a, vec4 b, float t) {
    return vec4(
        a.r + t * (b.r - a.r),
        a.g + t * (b.g - a.g),
        a.b + t * (b.b - a.b),
        a.a + t * (b.a - a.a)
    );
}

float rev_lerp(float v, float a, float b) {
    return (v - a) / (b - a);
}

float remap(float oldValue, float oldA, float oldB, float newA, float newB) {
    float t = rev_lerp(oldValue, oldA, oldB);
    return lerp(newA, newB, t);
}

#define PI 3.141592

in vec2 TexCoord;

uniform vec4 Color;
uniform sampler2D texture1;

uniform Time {
    float time;
};

out vec4 FragColor; // TODO(Tobi): I might turn this 1d

void main() {
    float currentPercentage = 0.7;
    float barHalfWidth = 0.025;

    float randValue = (texture(texture1, TexCoord / 3 + vec2(time , 0)).r - 0.5) * barHalfWidth;

    float barDist = abs(TexCoord.y - currentPercentage + randValue);

    float barStrength = clamp(remap(barDist, 0, barHalfWidth, 1, 0), 0, 1);

    //FragColor = lerp(Color, vec4(1), barStrength);
    FragColor = vec4(time != 0, time, time, Color.a);
}
