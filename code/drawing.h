#pragma once

#include "settings.h"
#include "macros.h"
#include "maths.h"

void DrawScreenRectangle(int x, int y, int width, int height, color32 col) {
    inc (y_i,   AtLeast(y, 0),    AtMost(y + height, ArrayHeight)) {
        inc (x_i,   AtLeast(x, 0),    AtMost(x + width, ArrayWidth)) {
            Array[Index2D(x_i, y_i, ArrayWidth)] = col;
        }
    }
}

// Negative border width/height means border goes inside
void DrawScreenBorder(int x, int y, int width, int height, int borderWidth, int borderHeight, color32 col) {

    if (borderWidth > 0) {
        x -= borderWidth;
        width += borderWidth * 2;
    } else {
        borderWidth = -borderWidth;
    }

    if (borderHeight > 0) {
        y -= borderHeight;
        height += borderHeight * 2;
    } else {
        borderHeight = -borderHeight;
    }

    DrawScreenRectangle(x, y, width, borderHeight, col);
    DrawScreenRectangle(x, y + height - borderHeight, width, borderHeight, col);

    DrawScreenRectangle(x, y, borderWidth, height, col);
    DrawScreenRectangle(x + width - borderWidth, y, borderWidth, height, col);
}

void DrawScreenBitmap(int x, int y, loaded_bitmap bitmap, color32 wantedColor) {
    int startX = AtLeast(-x, 0);
    int startY = AtLeast(-y, 0);

    int endX = AtMost(bitmap.Width, ArrayWidth - x);
    int endY = AtMost(bitmap.Height, ArrayHeight - y);

    inc (y_i,   startY,    endY) {
        inc (x_i,   startX,    endX) {
            color32 bitmapColor = bitmap.Data[Index2D(x_i, y_i, bitmap.Width)];
            if (bitmapColor.Alpha > 127) {

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

// NOTE(Tobi): This supports rendering only part of the BMP; also it supoports setting a colour
// void DrawScreenBMPText(int x, int y, int bmpX, int bmpY, int bmpWidth, int bmpHeight, color32 color, color32 backgroundColor, loaded_bitmap* bitmap) {

void DrawScreenBMPText(int x, int y, int bmpX, int bmpY, int bmpWidth, int bmpHeight, color32 color, color32 backgroundColor, loaded_bitmap* bitmap) {
    // TODO(Tobi): Optimise
    // TODO(Tobi): Assert correct bitmap
    inc0 (y_i,   bmpHeight) {
        inc0 (x_i,   bmpWidth) {

            color32 pxColor = bitmap->Data[Index2D(bmpX + x_i, bmpY + y_i, bitmap->Width)];
            if (pxColor.Alpha <= 127) { continue; } // TODO(Tobi): Actually use alpha
            float alpha = pxColor.Alpha / 255.0f;
            color32 resultingColor = {};
            // TODO(Tobi): Asserts
            resultingColor.Red   = (unsigned char) (alpha * color.Red   + ((1 - alpha) * pxColor.Red) + 0.5f);
            resultingColor.Green = (unsigned char) (alpha * color.Green + ((1 - alpha) * pxColor.Green) + 0.5f);
            resultingColor.Blue  = (unsigned char) (alpha * color.Blue  + ((1 - alpha) * pxColor.Blue) + 0.5f);

            TrySetPixel(x + x_i, y + y_i, resultingColor);
        }
    }
}

void DrawWorldRectangle(float x, float y, float width, float height, color32 col) {
    DrawScreenRectangle((int) (GRID_SIZE * x) + GRID_SIZE / 2 + MONSTER_STONE_BAR_WIDTH, (int) (GRID_SIZE * y) + GRID_SIZE / 2, (int) (GRID_SIZE * width), (int) (GRID_SIZE * height), col);
}

void DrawWorldBorder(float x, float y, float width, float height, float borderWidth, float borderHeight, color32 col) {
    DrawScreenBorder((int) (GRID_SIZE * x) + GRID_SIZE / 2 + MONSTER_STONE_BAR_WIDTH, (int) (GRID_SIZE * y) + GRID_SIZE / 2, (int) (GRID_SIZE * width), (int) (GRID_SIZE * height), (int) (GRID_SIZE * borderWidth), (int) (GRID_SIZE * borderHeight), col);
}

void DrawWorldBitmap(float x, float y, loaded_bitmap bitmap, color32 wantedColor) {
    DrawScreenBitmap((int) (GRID_SIZE * x) + (GRID_SIZE - bitmap.Width) / 2 + MONSTER_STONE_BAR_WIDTH, (int) (GRID_SIZE * y) + (GRID_SIZE - bitmap.Height) / 2, bitmap, wantedColor);
}

void DrawWorldDisc(float x, float y, float radius, color32 col) {
    DrawScreenDisc((int) (GRID_SIZE * x) + GRID_SIZE / 2 + MONSTER_STONE_BAR_WIDTH, (int) (GRID_SIZE * y) + GRID_SIZE / 2, (int) (GRID_SIZE * radius), col);
}

void DrawWorldCircle(float x, float y, float radius, color32 col) {
    DrawScreenCircle((int) (GRID_SIZE * x) + GRID_SIZE / 2 + MONSTER_STONE_BAR_WIDTH, (int) (GRID_SIZE * y) + GRID_SIZE / 2, (int) (GRID_SIZE * radius), col);
}

void DrawBlock(int x, int y, color32 col) {
    // NOTE(Tobi): I don't know whether I like this function anymore; should I just call DrawWorldRectangle?
    DrawScreenRectangle((int) (GRID_SIZE * x) + 1 + MONSTER_STONE_BAR_WIDTH, (int) (GRID_SIZE * y) + 1, GRID_SIZE - 2, GRID_SIZE - 2, col);
}

// void DrawWorldBMPText(float x, float y, int bmpX, int bmpY, int bmpWidth, int bmpHeight, color32 color, color32 backgroundColor, loaded_bitmap* bitmap) {
//     DrawScreenBMPText((int) (GRID_SIZE * x) + GRID_SIZE / 2 + MONSTER_STONE_BAR_WIDTH, (int) (GRID_SIZE * y) + GRID_SIZE / 2, bmpX, bmpY, bmpWidth, bmpHeight, color, backgroundColor, bitmap);
// }