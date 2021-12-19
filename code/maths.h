#pragma once

int Index2D(int x, int y, int width) {
    return y * width + x;
}

int Min(int a, int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

float Min(float a, float b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

int Max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

float Max(float a, float b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int AtMost(int a, int b) {
    return Min(a, b);
}

float AtMost(float a, float b) {
    return Min(a, b);
}

int AtLeast(int a, int b) {
    return Max(a, b);
}

float AtLeast(float a, float b) {
    return Max(a, b);
}

int Clamp(int value, int min, int max) {
    return AtMost(AtLeast(value, min), max);
}

float Clamp(float value, float min, float max) {
    return AtMost(AtLeast(value, min), max);
}

struct vec2i {
    int X;
    int Y;
};

struct vec2f {
    float X;
    float Y;
};

vec2f Normalised(vec2f value) {
    // TODO(Tobi): Assert not zero
    vec2f ret = {};
    float length = sqrtf(value.X * value.X + value.Y * value.Y);
    ret.X = value.X / length;
    ret.Y = value.Y / length;
    return ret;
}
