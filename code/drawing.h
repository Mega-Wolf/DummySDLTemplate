#pragma once

#include "macros.h"
#include "maths.h"

void DrawScreenRectangle(int x, int y, int width, int height, color32 col) {
    inc (y_i,   AtLeast(y, 0),    AtMost(y + height, ArrayHeight)) {
        inc (x_i,   AtLeast(x, 0),    AtMost(x + width, ArrayWidth)) {
            Array[Index2D(x_i, y_i, ArrayWidth)] = col;
        }
    }
}

void DrawScreenBitmap(int x, int y, loaded_bitmap bitmap, color32 wantedColor) {
    int startX = AtLeast(-x, 0);
    int startY = AtLeast(-y, 0);

    int endX = AtMost(bitmap.Width, ArrayWidth);
    int endY = AtMost(bitmap.Height, ArrayHeight);

    inc (y_i,   startY,    endY) {
        inc (x_i,   startX,    endX) {
            color32 bitmapColor = bitmap.Data[Index2D(x_i, y_i, bitmap.Width)];
            if (bitmapColor.Alpha > 0) {

                float bitmapRed   = bitmapColor.Red   / 255.0f;
                float bitmapGreen = bitmapColor.Green / 255.0f;
                float bitmapBlue  = bitmapColor.Blue  / 255.0f;

                float wantedRed   = wantedColor.Red   / 255.0f;
                float wantedGreen = wantedColor.Green / 255.0f;
                float wantedBlue  = wantedColor.Blue  / 255.0f;

                float resultRed   = bitmapRed   * wantedRed;
                float resultGreen = bitmapGreen * wantedGreen;
                float resultBlue  = bitmapBlue  * wantedBlue;

                color32 finalColor = {};
                finalColor.Red   = (unsigned char) (resultRed   * 255);
                finalColor.Green = (unsigned char) (resultGreen * 255);
                finalColor.Blue  = (unsigned char) (resultBlue  * 255);

                Array[Index2D(x + x_i, y + y_i, ArrayWidth)] = finalColor;
            }
        }
    }
}

void DrawScreenDisc(int x, int y, int radius, color32 col) {
    inc (y_i,   AtLeast(y - radius, 0),    AtMost(y + radius + 1, ArrayHeight)) {
        inc (x_i,   AtLeast(x - radius, 0),    AtMost(x + radius + 1, ArrayWidth)) {
            float diffX = (float) (x_i - x);
            float diffY = (float) (y_i - y);

            if (diffX * diffX + diffY * diffY <= radius * radius) {
                Array[Index2D(x_i, y_i, ArrayWidth)] = col;
            }
        }
    }
}

void TrySetPixel(int x, int y, color32 col) {
    if (x >= 0 && x < ArrayWidth && y >= 0 && y < ArrayHeight) {
        Array[Index2D(x, y, ArrayWidth)] = col;
    }
}

#define SQRT2 1.41421356237
//#define SQRT2_DIV_2 0.70710678118

void DrawScreenCircle(int x, int y, int radius, color32 col) {
    // TODO(Tobi): The inputs are integers; this means that our circle will look a bit weird

    TrySetPixel(x, y - radius, col);
    TrySetPixel(x, y + radius, col);
    TrySetPixel(x - radius, y, col);
    TrySetPixel(x + radius, y, col);

    int radiusSq = radius * radius;
    int maxRadiusToCheck = (int) (radius * (float) SQRT2 / 2);
    for (int i = 1; i <= maxRadiusToCheck; ++i) {

        int offset = (int) (sqrtf((float) (radiusSq - i * i)) + 0.5f);

        TrySetPixel(x + i, y - offset, col);
        TrySetPixel(x - i, y - offset, col);
        TrySetPixel(x + i, y + offset, col);
        TrySetPixel(x - i, y + offset, col);
        TrySetPixel(x - offset, y + i, col);
        TrySetPixel(x - offset, y - i, col);
        TrySetPixel(x + offset, y + i, col);
        TrySetPixel(x + offset, y - i, col);
    }
}
