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

int RoundFloat32ToInt32(float in) {
    // TODO(Tobi): Intrinsic?
    return (int) (in +0.5f);
}

unsigned int RoundFloat32ToUInt32(float in) {
    // TODO(Tobi): Intrinsic?
    return (unsigned int) (in + 0.5f);
}

#pragma region [vec2i]

struct vec2f {
    union {
        struct {
            float X;
    	    float Y;
        };
        float Data[2];
    };

    // NOTE(Tobi): Is that possible somehow
    //explicit operator vec2i() { return vec2i({(int) X, (int) Y}); }
};

struct vec2i {
    union {
        struct {
            int X;
    	    int Y;
        };
        int Data[2];
    };

    // NOTE(Tobi): Ugly because of metaprogramming
    operator vec2f() { return vec2f({(float) X, (float) Y}); }
};

vec2i operator+(vec2i vec1, vec2i vec2) {
    return vec2i{ vec1.X + vec2.X, vec1.Y + vec2.Y};
}

vec2i operator-(vec2i vec1, vec2i vec2) {
    return vec2i{ vec1.X - vec2.X, vec1.Y - vec2.Y};
}

void operator+=(vec2i &vec, vec2i vec2) {
    vec.X += vec2.X;
    vec.Y += vec2.Y;
}

void operator-=(vec2i &vec, vec2i vec2) {
    vec.X -= vec2.X;
    vec.Y -= vec2.Y;
}

vec2i operator-(vec2i vec) {
    return vec2i{ -vec.X, -vec.Y };
}

vec2i operator*(vec2i vec, int scalar) {
    return { vec.X * scalar, vec.Y * scalar };
}

vec2f operator*(vec2i vec, float scalar) {
    return { vec.X * scalar, vec.Y * scalar };
}

vec2i operator*(int scalar, vec2i vec) {
    return { vec.X * scalar, vec.Y * scalar };
}

vec2f operator*(float scalar, vec2i vec) {
    return { vec.X * scalar, vec.Y * scalar };
}

vec2i operator/(vec2i vec, int scalar) {
    return { vec.X / scalar, vec.Y / scalar };
}

vec2f operator/(vec2i vec, float scalar) {
    return { vec.X / scalar, vec.Y / scalar };
}

vec2i operator%(vec2i vec, int scalar) {
    return { vec.X % scalar, vec.Y % scalar };
}

void operator*=(vec2i &vec, int scalar) {
    vec.X *= scalar;
    vec.Y *= scalar;
}

// void operator*=(vec2i &vec, float scalar) {
//     vec.X *= scalar;
//     vec.Y *= scalar;
// }

// void operator*=(int scalar, vec2i &vec) {
//     vec.X *= scalar;
//     vec.Y *= scalar;
// }

// void operator*=(float scalar, vec2i &vec) {
//     vec.X *= scalar;
//     vec.Y *= scalar;
// }

void operator/=(vec2i &vec, int scalar) {
    vec.X /= scalar;
    vec.Y /= scalar;
}

// void operator/=(vec2i &vec, float scalar) {
//     vec.X /= scalar;
//     vec.Y /= scalar;
// }

void operator%=(vec2i &vec, int scalar) {
    vec.X %= scalar;
    vec.Y %= scalar;
}

bool operator==(vec2i vec1, vec2i vec2) {
    return vec1.X == vec2.X && vec1.Y == vec2.Y;
}

bool operator!=(vec2i vec1, vec2i vec2) {
    return !(vec1.X == vec2.X && vec1.Y == vec2.Y);
}

#pragma endregion

#pragma region [vec2f]

vec2f operator+(vec2f vec1, vec2f vec2) {
    return vec2f{ vec1.X + vec2.X, vec1.Y + vec2.Y};
}

vec2f operator-(vec2f vec1, vec2f vec2) {
    return vec2f{ vec1.X - vec2.X, vec1.Y - vec2.Y};
}

void operator+=(vec2f &vec, vec2f vec2) {
    vec.X += vec2.X;
    vec.Y += vec2.Y;
}

void operator-=(vec2f &vec, vec2f vec2) {
    vec.X -= vec2.X;
    vec.Y -= vec2.Y;
}

vec2f operator-(vec2f vec) {
    return vec2f{ -vec.X, -vec.Y };
}

vec2f operator*(vec2f vec, int scalar) {
    return { vec.X * scalar, vec.Y * scalar };
}

vec2f operator*(vec2f vec, float scalar) {
    return { vec.X * scalar, vec.Y * scalar };
}

vec2f operator*(int scalar, vec2f vec) {
    return { vec.X * scalar, vec.Y * scalar };
}

vec2f operator*(float scalar, vec2f vec) {
    return { vec.X * scalar, vec.Y * scalar };
}

vec2f operator/(vec2f vec, int scalar) {
    return { vec.X / scalar, vec.Y / scalar };
}

vec2f operator/(vec2f vec, float scalar) {
    return { vec.X / scalar, vec.Y / scalar };
}

void operator*=(vec2f &vec, int scalar) {
    vec.X *= scalar;
    vec.Y *= scalar;
}

void operator*=(vec2f &vec, float scalar) {
    vec.X *= scalar;
    vec.Y *= scalar;
}

// void operator*=(int scalar, vec2f &vec) {
//     vec.X *= scalar;
//     vec.Y *= scalar;
// }

// void operator*=(float scalar, vec2f &vec) {
//     vec.X *= scalar;
//     vec.Y *= scalar;
// }


void operator/=(vec2f &vec, int scalar) {
    vec.X /= scalar;
    vec.Y /= scalar;
}

void operator/=(vec2f &vec, float scalar) {
    vec.X /= scalar;
    vec.Y /= scalar;
}

bool operator==(vec2f vec1, vec2f vec2) {
    return vec1.X == vec2.X && vec1.Y == vec2.Y;
}

bool operator!=(vec2f vec1, vec2f vec2) {
    return !(vec1.X == vec2.X && vec1.Y == vec2.Y);
}

vec2i vec2f_vec2i(vec2f vec) {
    return { RoundFloat32ToInt32(vec.X), RoundFloat32ToInt32(vec.Y) };
}


float dot(vec2f a, vec2f b) {
    return a.X * b.X + a.Y * b.Y;
}

inline vec2f rotateRet(vec2f vec, float rad) {
    float cosAlpha = cosf(rad);
    float sinAlpha = sinf(rad);
    vec2f ret;
    ret.X = cosAlpha * vec.X + -sinAlpha * vec.Y;
    ret.Y = sinAlpha * vec.X +  cosAlpha * vec.Y;
    return ret;
}

inline void rotateThis(vec2f* vec, float rad) {
    float cosAlpha = cosf(rad);
    float sinAlpha = sinf(rad);
    vec2f n;
    n.X = cosAlpha * vec->X + -sinAlpha * vec->Y;
    n.Y = sinAlpha * vec->X +  cosAlpha * vec->Y;
    *vec = n;
}

inline float Length(vec2f vec) {
    return sqrtf(vec.X * vec.X + vec.Y * vec.Y);
}

inline float LengthSq(vec2f vec) {
    return vec.X * vec.X + vec.Y * vec.Y;
}

// inline vec2f Norm(vec2f vec) {
//     Assert(vec.X != 0 || vec.Y != 0, "Norm only woks with vec != O");
//     float length = Length(vec);
//     return vec / length;
// }


#pragma endregion

vec2f Normalised(vec2f value) {
    // TODO(Tobi): Assert not zero
    vec2f ret = {};
    float length = sqrtf(value.X * value.X + value.Y * value.Y);
    ret.X = value.X / length;
    ret.Y = value.Y / length;
    return ret;
}

template<typename T>
bool BetweenInIn(T value, T min, T max) {
    Assert(min <= max, "Min is bigger than max");
    return value >= min && value <= max;
}

template<typename T>
bool BetweenInEx(T value, T min, T max) {
    Assert(min <= max, "Min is bigger than max");
    return value >= min && value < max;
}

// TODO(Tobi): Is this really the order of paramters I want to have?
template<typename T>
bool BoxContainsInIn(T left, T top, T right, T bottom, T x, T y) {
    return 
        x >= left &&
        x <= right &&
        y >= top &&
        y <= bottom;
}

template<typename T>
bool BoxContainsInEx(T left, T top, T right, T bottom, T x, T y) {
    return 
        x >= left &&
        x < right &&
        y >= top &&
        y < bottom;
}

#define LERP(x, y, t) ((t)*(y) + (1 - (t)) * (x))
#define Lerp(x, y, t) ((t)*(y) + (1 - (t)) * (x))
#define InvLerp(x, y, V) (((V) - (x)) / ((y) - (x)))

template<typename T>
T RemapLinear(T inputMin, T inputMax, T outputMin, T outputMax, T value) {
    T t = InvLerp(inputMin, inputMax, value);
    return Lerp(outputMin, outputMax, t);
}
