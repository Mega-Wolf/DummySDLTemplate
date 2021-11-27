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

int Max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int AtMost(int a, int b) {
    return Min(a, b);
}

int AtLeast(int a, int b) {
    return Max(a, b);
}

int Clamp(int value, int min, int max) {
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

