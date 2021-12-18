#pragma once

#include "settings.h"
#include "macros.h"
#include "maths.h"

struct draw_rect {
    int StartX;
    int StartY;
    int Width;
    int Height;

    color32* ArrayData;
    int ArrayWidth;
};

int Index2d(int x, int y, draw_rect* drawRect) {
    return Index2D(x + drawRect->StartX, y + drawRect->StartY, drawRect->ArrayWidth);
}

void TrySetPixel(draw_rect* drawRect, int x, int y, color32 col) {
    if (x >= 0 && x < drawRect->Width && y >= 0 && y < drawRect->Height) {
        drawRect->ArrayData[Index2d(x, y, drawRect)] = col;
    }
}

#define SQRT2 1.41421356237
//#define SQRT2_DIV_2 0.70710678118

vec2i Camera;

void DrawScreenRectangle(draw_rect* drawRect, int x, int y, int width, int height, color32 col) {
    x += Camera.X;
    y += Camera.Y;

    inc (y_i,   AtLeast(y, 0),    AtMost(y + height, drawRect->Height)) {
        inc (x_i,   AtLeast(x, 0),    AtMost(x + width, drawRect->Width)) {
            drawRect->ArrayData[Index2d(x_i, y_i, drawRect)] = col;
        }
    }
}

// Negative border width/height means border goes inside
void DrawScreenBorder(draw_rect* drawRect, int x, int y, int width, int height, int borderWidth, int borderHeight, color32 col) {
    // NOTE(Tobi): I don't have to offset that, since it will call DrawScreenRectangle later on, however, it kind of looks cool
    // x += Camera.X;
    // y += Camera.Y;

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

    DrawScreenRectangle(drawRect, x, y, width, borderHeight, col);
    DrawScreenRectangle(drawRect, x, y + height - borderHeight, width, borderHeight, col);

    DrawScreenRectangle(drawRect, x, y, borderWidth, height, col);
    DrawScreenRectangle(drawRect, x + width - borderWidth, y, borderWidth, height, col);
}

void DrawScreenBitmap(draw_rect* drawRect, int x, int y, loaded_bitmap bitmap, color32 wantedColor) {
    x += Camera.X;
    y += Camera.Y;

    int startX = AtLeast(-x, 0);
    int startY = AtLeast(-y, 0);

    int endX = AtMost(bitmap.Width, drawRect->Width - x);
    int endY = AtMost(bitmap.Height, drawRect->Height - y);

    inc (y_i,   startY,    endY) {
        inc (x_i,   startX,    endX) {
            color32 bitmapColor = bitmap.Data[Index2D(x_i, y_i, bitmap.Width)];
            if (bitmapColor.Alpha == 0) { continue; }

            int outputIndex = Index2d(x + x_i, y + y_i, drawRect);

            float bitmapRed   = bitmapColor.Red   / 255.0f;
            float bitmapGreen = bitmapColor.Green / 255.0f;
            float bitmapBlue  = bitmapColor.Blue  / 255.0f;
            float bitmapAlpha = bitmapColor.Alpha / 255.0f;

            float wantedRed   = wantedColor.Red   / 255.0f;
            float wantedGreen = wantedColor.Green / 255.0f;
            float wantedBlue  = wantedColor.Blue  / 255.0f;
            float wantedAlpha = wantedColor.Alpha / 255.0f;

            float resultRed   = bitmapRed   * wantedRed;
            float resultGreen = bitmapGreen * wantedGreen;
            float resultBlue  = bitmapBlue  * wantedBlue;
    	    float resultAlpha = bitmapAlpha * wantedAlpha;

            if (resultAlpha < 1.0f) {

                color32 dstColor = drawRect->ArrayData[outputIndex];
                float dstRed   = dstColor.Red   / 255.0f;
                float dstGreen = dstColor.Green / 255.0f;
                float dstBlue  = dstColor.Blue  / 255.0f;

                resultRed   = resultRed   * resultAlpha + dstRed   * (1 - resultAlpha);
                resultGreen = resultGreen * resultAlpha + dstGreen * (1 - resultAlpha);
                resultBlue  = resultBlue  * resultAlpha + dstBlue  * (1 - resultAlpha);
            }

            color32 finalColor = {};
            finalColor.Red   = (unsigned char) (resultRed   * 255);
            finalColor.Green = (unsigned char) (resultGreen * 255);
            finalColor.Blue  = (unsigned char) (resultBlue  * 255);

            drawRect->ArrayData[outputIndex] = finalColor;
        }
    }
}

void DrawScreenDisc(draw_rect* drawRect, int x, int y, int radius, color32 col) {
    x += Camera.X;
    y += Camera.Y;

    inc (y_i,   AtLeast(y - radius, 0),    AtMost(y + radius + 1, drawRect->Height)) {
        inc (x_i,   AtLeast(x - radius, 0),    AtMost(x + radius + 1, drawRect->Width)) {
            float diffX = (float) (x_i - x);
            float diffY = (float) (y_i - y);

            if (diffX * diffX + diffY * diffY <= radius * radius) {
                drawRect->ArrayData[Index2d(x_i, y_i, drawRect)] = col;
            }
        }
    }
}

void DrawScreenCircle(draw_rect* drawRect, int x, int y, int radius, color32 col) {
    x += Camera.X;
    y += Camera.Y;

    // TODO(Tobi): The inputs are integers; this means that our circle will look a bit weird

    TrySetPixel(drawRect, x, y - radius, col);
    TrySetPixel(drawRect, x, y + radius, col);
    TrySetPixel(drawRect, x - radius, y, col);
    TrySetPixel(drawRect, x + radius, y, col);

    int radiusSq = radius * radius;
    int maxRadiusToCheck = (int) (radius * (float) SQRT2 / 2);
    for (int i = 1; i <= maxRadiusToCheck; ++i) {

        int offset = (int) (sqrtf((float) (radiusSq - i * i)) + 0.5f);

        TrySetPixel(drawRect, x + i, y - offset, col);
        TrySetPixel(drawRect, x - i, y - offset, col);
        TrySetPixel(drawRect, x + i, y + offset, col);
        TrySetPixel(drawRect, x - i, y + offset, col);
        TrySetPixel(drawRect, x - offset, y + i, col);
        TrySetPixel(drawRect, x - offset, y - i, col);
        TrySetPixel(drawRect, x + offset, y + i, col);
        TrySetPixel(drawRect, x + offset, y - i, col);
    }
}

void DrawScreenBMPText(draw_rect* drawRect, int x, int y, int bmpX, int bmpY, int bmpWidth, int bmpHeight, color32 color, color32 backgroundColor, loaded_bitmap* bitmap) {
    x += Camera.X;
    y += Camera.Y;
    
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

            TrySetPixel(drawRect, x + x_i, y + y_i, resultingColor);
        }
    }
}

void DrawWorldRectangle(draw_rect* drawRect, float x, float y, float width, float height, color32 col) {
    DrawScreenRectangle(drawRect, (int) (GRID_SIZE * x) + GRID_SIZE / 2, (int) (GRID_SIZE * y) + GRID_SIZE / 2, (int) (GRID_SIZE * width), (int) (GRID_SIZE * height), col);
}

void DrawWorldBorder(draw_rect* drawRect, float x, float y, float width, float height, float borderWidth, float borderHeight, color32 col) {
    DrawScreenBorder(drawRect, (int) (GRID_SIZE * x) + GRID_SIZE / 2, (int) (GRID_SIZE * y) + GRID_SIZE / 2, (int) (GRID_SIZE * width), (int) (GRID_SIZE * height), (int) (GRID_SIZE * borderWidth), (int) (GRID_SIZE * borderHeight), col);
}

void DrawWorldBitmap(draw_rect* drawRect, float x, float y, loaded_bitmap bitmap, color32 wantedColor) {
    DrawScreenBitmap(drawRect, (int) (GRID_SIZE * x) + (GRID_SIZE - bitmap.Width) / 2, (int) (GRID_SIZE * y) + (GRID_SIZE - bitmap.Height) / 2, bitmap, wantedColor);
}

void DrawWorldDisc(draw_rect* drawRect, float x, float y, float radius, color32 col) {
    DrawScreenDisc(drawRect, (int) (GRID_SIZE * x) + GRID_SIZE / 2, (int) (GRID_SIZE * y) + GRID_SIZE / 2, (int) (GRID_SIZE * radius), col);
}

void DrawWorldCircle(draw_rect* drawRect, float x, float y, float radius, color32 col) {
    DrawScreenCircle(drawRect, (int) (GRID_SIZE * x) + GRID_SIZE / 2, (int) (GRID_SIZE * y) + GRID_SIZE / 2, (int) (GRID_SIZE * radius), col);
}

void DrawBlock(draw_rect* drawRect, int x, int y, color32 col) {
    // NOTE(Tobi): I don't know whether I like this function anymore; should I just call DrawWorldRectangle?
    DrawScreenRectangle(drawRect, (int) (GRID_SIZE * x) + 1, (int) (GRID_SIZE * y) + 1, GRID_SIZE - 2, GRID_SIZE - 2, col);
}

// void DrawWorldBMPText(float x, float y, int bmpX, int bmpY, int bmpWidth, int bmpHeight, color32 color, color32 backgroundColor, loaded_bitmap* bitmap) {
//     DrawScreenBMPText((int) (GRID_SIZE * x) + GRID_SIZE / 2, (int) (GRID_SIZE * y) + GRID_SIZE / 2, bmpX, bmpY, bmpWidth, bmpHeight, color, backgroundColor, bitmap);
// }
