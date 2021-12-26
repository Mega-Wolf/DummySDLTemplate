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

// TODO(Tobi): Something is off with my y positions

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

int RoundFloatToInt(float value) {
    return (int) (value + 0.5f);
}

void DrawScreenLine(draw_rect* drawRect, int startX, int startY, int endX, int endY, color32 col) {

    startX += Camera.X;
    startY += Camera.Y;
    endX += Camera.X;
    endY += Camera.Y;

    int diffX = endX - startX;
    int diffY = endY - startY;

    int absX = abs(diffX);
    int absY = abs(diffY);

    if (absY < absX) {
        if (startX < endX) {
            float yDiff =  (float) (endY - startY) / (endX - startX);
            for (int x_i = startX; x_i <= endX; ++x_i) {
                int yOffset = RoundFloatToInt((x_i - startX) * yDiff);
                int y = startY + yOffset;
                TrySetPixel(drawRect, x_i, y, col);
            }
        } else {
            float yDiff = (float) (startY - endY) / (startX - endX);
            for (int x_i = endX; x_i <= startX; ++x_i) {
                int yOffset = RoundFloatToInt((x_i - endX) * yDiff);
                int y = endY + yOffset;
                TrySetPixel(drawRect, x_i, y, col);
            }
        }
    } else {
        if (startY < endY) {
            float xDiff =  (float) (endX - startX) / (endY - startY);
            for (int y_i = startY; y_i <= endY; ++y_i) {
                int xOffset = RoundFloatToInt((y_i - startY) * xDiff);
                int x = startX + xOffset;
                TrySetPixel(drawRect, x, y_i, col);
            }
        } else {
            float xDiff = (float) (startX - endX) / (startY - endY);
            for (int y_i = endY; y_i <= startY; ++y_i) {
                int xOffset = RoundFloatToInt((y_i - endY) * xDiff);
                int x = endX + xOffset;
                TrySetPixel(drawRect, x, y_i, col);
            }
        }
    }
}

/*
void DrawScreenLineThick(draw_rect* drawRect, int startX, int startY, int endX, int endY, int thickness, color32 col) {

    int absX = abs(startX - endX);
    int absY = abs(startY - endY);

    if (absX == 0 && absY == 0) { return; }

    vec2f ortho = { (float) (endY - startY), (float) -(endX - startX) };
    vec2f orthoNormalised = Normalised(ortho);

    int offsetBeforeX = -9999999;
    int offsetBeforeY = -9999999;
    incIn (thickness_i,   -thickness,    thickness) {
        int offsetX = RoundFloatToInt(orthoNormalised.X * thickness_i);
        int offsetY = RoundFloatToInt(orthoNormalised.Y * thickness_i);

        if (offsetX == offsetBeforeX && offsetY == offsetBeforeY) { continue; }

        offsetBeforeX = offsetX;
        offsetBeforeY = offsetY;

        DrawScreenLine(drawRect, startX + offsetX, startY + offsetY, endX + offsetX, endY + offsetY, col);
    }


    // if (absX > absY) {
    //     incIn (thickness_i,   -thickness,    thickness) {
    //         DrawScreenLine(drawRect, startX, startY + thickness_i, endX, endY + thickness_i, col);
    //     }
    // } else {
    //     incIn (thickness_i,   -thickness,    thickness) {
    //         DrawScreenLine(drawRect, startX + thickness_i, startY, endX + thickness_i, endY, col);
    //     }
    // }
}
*/

void DrawScreenLineThick(draw_rect* drawRect, int startX, int startY, int endX, int endY, float thickness, color32 col) { 
    int dx = abs(endX - startX);
    int sx = startX < endX ? 1 : -1;
    int dy = abs(endY - startY);
    int sy = startY < endY ? 1 : -1;
    int err = dx - dy;
    int e2;
    int x2;
    int y2;                          /* error value e_xy */
    float ed = dx + dy == 0 ? 1 : sqrtf((float)dx*dx+(float)dy*dy);

    float wantedRed   = col.Red   / 255.0f;
    float wantedGreen = col.Green / 255.0f;
    float wantedBlue  = col.Blue  / 255.0f;
   
    for (thickness = (thickness + 1) / 2; ; ) {                                   /* pixel loop */
        {
            float strength = 1 - AtLeast((abs(err - dx + dy) / ed - thickness +1), 0.0f);
            color32 resultColor;
            resultColor.Red =   (unsigned char) RoundFloatToInt(wantedRed * strength * 255);
            resultColor.Green = (unsigned char) RoundFloatToInt(wantedGreen * strength * 255);
            resultColor.Blue =  (unsigned char) RoundFloatToInt(wantedBlue * strength * 255);
            // TODO(Tobi): This ignores alpha, but I could just change alpha instead of everythign else
            TrySetPixel(drawRect, startX, startY, resultColor);
        }
        e2 = err;
        x2 = startX;
        if (2 * e2 >= -dx) {                                           /* x step */
            for (e2 += dy, y2 = startY; e2 < ed * thickness && (endY != y2 || dx > dy); e2 += dx) {
                float strength = 1 - AtLeast(255 * (abs(e2) / ed - thickness + 1), 0.0f);
                color32 resultColor;
                resultColor.Red =   (unsigned char) RoundFloatToInt(wantedRed * strength * 255);
                resultColor.Green = (unsigned char) RoundFloatToInt(wantedGreen * strength * 255);
                resultColor.Blue =  (unsigned char) RoundFloatToInt(wantedBlue * strength * 255);
                TrySetPixel(drawRect, startX, y2 += sy, resultColor);
            }
            if (startX == endX) { break; }
            e2 = err;
            err -= dy;
            startX += sx; 
        }
        if (2*e2 <= dy) {                                            /* y step */
            for (e2 = dx - e2; e2 < ed * thickness && (endX != x2 || dx < dy); e2 += dy) {
                float strength = 1 - AtLeast(255 * (abs(e2) / ed - thickness + 1), 0.0f);
                color32 resultColor;
                resultColor.Red =   (unsigned char) RoundFloatToInt(wantedRed * strength * 255);
                resultColor.Green = (unsigned char) RoundFloatToInt(wantedGreen * strength * 255);
                resultColor.Blue =  (unsigned char) RoundFloatToInt(wantedBlue * strength * 255);
                TrySetPixel(drawRect, x2 += sx, startY, resultColor);
            }
            if (startY == endY) { break; }
            err += dx;
            startY += sy; 
        }
    }
}

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

void DrawScreenBMPText(draw_rect* drawRect, int x, int y, int bmpX, int bmpY, int bmpWidth, int bmpHeight, color32 color, /*color32 backgroundColor,*/ loaded_bitmap* bitmap) {
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

void DrawWorldLine(draw_rect* drawRect, float startX, float startY, float endX, float endY, color32 col) {
    DrawScreenLine(drawRect, RoundFloatToInt(HEXAGON_A * startX), RoundFloatToInt(HEXAGON_A * startY), RoundFloatToInt(HEXAGON_A * endX), RoundFloatToInt(HEXAGON_A * endY), col);
}

void DrawWorldLineThick(draw_rect* drawRect, float startX, float startY, float endX, float endY, int pixelThickness, color32 col) {
    int intStartX = RoundFloatToInt(HEXAGON_A * startX);
    int intStartY = RoundFloatToInt(HEXAGON_A * startY);
    int intEndX = RoundFloatToInt(HEXAGON_A * endX);
    int intEndY = RoundFloatToInt(HEXAGON_A * endY);

    DrawScreenLineThick(drawRect, intStartX, intStartY, intEndX, intEndY, (float) pixelThickness, col);
}

void DrawWorldRectangle(draw_rect* drawRect, float x, float y, float width, float height, color32 col) {
    DrawScreenRectangle(drawRect, RoundFloatToInt(HEXAGON_A * x), RoundFloatToInt(HEXAGON_A * y), RoundFloatToInt(HEXAGON_A * width), RoundFloatToInt(HEXAGON_A * height), col);
}

void DrawWorldBorder(draw_rect* drawRect, float x, float y, float width, float height, float borderWidth, float borderHeight, color32 col) {
    DrawScreenBorder(drawRect, RoundFloatToInt(HEXAGON_A * x), RoundFloatToInt(HEXAGON_A * y), RoundFloatToInt(HEXAGON_A * width), (int) (HEXAGON_A * height), (int) (HEXAGON_A * borderWidth), (int) (HEXAGON_A * borderHeight), col);
}

void DrawWorldBitmap(draw_rect* drawRect, float x, float y, loaded_bitmap bitmap, color32 wantedColor) {
    DrawScreenBitmap(drawRect, RoundFloatToInt(HEXAGON_A * x) - (bitmap.Width) / 2, RoundFloatToInt(HEXAGON_A * y) - (bitmap.Height) / 2, bitmap, wantedColor);
}

void DrawWorldDisc(draw_rect* drawRect, float x, float y, float radius, color32 col) {
    DrawScreenDisc(drawRect, RoundFloatToInt(HEXAGON_A * x), RoundFloatToInt(HEXAGON_A * y), RoundFloatToInt(HEXAGON_A * radius), col);
}

void DrawWorldCircle(draw_rect* drawRect, float x, float y, float radius, color32 col) {
    DrawScreenCircle(drawRect, RoundFloatToInt(HEXAGON_A * x), RoundFloatToInt(HEXAGON_A * y), RoundFloatToInt(HEXAGON_A * radius), col);
}
