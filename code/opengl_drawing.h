#pragma once

#include "../helpers/opengl.h"

// TODO(Tobi): The screenshake is not that easy anymore with the scissor test stuff I think

color4f color32_to_color4f(color32 col) {
    color4f colFloat = {
        col.Red / 255.0f,
        col.Green / 255.0f,
        col.Blue / 255.0f,
        col.Alpha / 255.0f
    };

    return colFloat;
}

void DrawScreenRectangle(draw_rect* drawRect, int x, int y, int width, int height, color32 col, int z) {
    x += Camera.X;
    y += Camera.Y;

    color4f colFloat = color32_to_color4f(col);

    RendererSetDrawRect(drawRect);
    RendererScreenRect(HMM_Translate((float) (x + drawRect->StartX), (float) (y + drawRect->StartY), (float) z) * HMM_Scale((float)width, (float)height, 1), colFloat);
}

// Negative border width/height means border goes inside
void DrawScreenBorder(draw_rect* drawRect, int x, int y, int width, int height, int borderWidth, int borderHeight, color32 col, int z) {
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

    DrawScreenRectangle(drawRect, x, y, width, borderHeight, col, z);
    DrawScreenRectangle(drawRect, x, y + height - borderHeight, width, borderHeight, col, z);

    DrawScreenRectangle(drawRect, x, y, borderWidth, height, col, z);
    DrawScreenRectangle(drawRect, x + width - borderWidth, y, borderWidth, height, col, z);
}

void DrawScreenDisc(draw_rect* drawRect, int x, int y, int radius, color32 col, int z) {
    x += Camera.X;
    y += Camera.Y;

    color4f colFloat = color32_to_color4f(col);

    RendererSetDrawRect(drawRect);
    RendererScreenDisc(HMM_Translate((float) (x + drawRect->StartX - radius), (float) (y + drawRect->StartY - radius), (float) z) * HMM_Scale((float) 2 * radius, (float) 2 * radius, 1), colFloat);
}

void DrawScreenCircle(draw_rect* drawRect, int x, int y, int radius, color32 col, int z) {
    x += Camera.X;
    y += Camera.Y;

    color4f colFloat = color32_to_color4f(col);

    RendererSetDrawRect(drawRect);
    RendererScreenCircle(HMM_Translate((float) (x + drawRect->StartX - radius), (float) (y + drawRect->StartY - radius), (float) z) * HMM_Scale((float) 2 * radius, (float) 2 * radius, 1), colFloat);
}

void DrawScreenBitmap(draw_rect* drawRect, int x, int y, sprite_data spriteData, color32 col, int z) {
    x += Camera.X;
    y += Camera.Y;

    color4f colFloat = color32_to_color4f(col);

    // TODO(Tobi): In the future, I will have to provide the shader on my own
    RendererSetDrawRect(drawRect);
    RendererScreenSprite(HMM_Translate((float) (x + drawRect->StartX), (float) (y + drawRect->StartY), (float) z), &spriteData.Sprite, OGLData.BasicAlphaCutoffShader, colFloat);
}


void DrawBackgroundSprite(draw_rect* drawRect, sprite_data spriteData, sprite_data spriteData2, int z) {
    RendererSetDrawRect(drawRect);
    RendererScreenSprite2(HMM_Translate((float) (drawRect->StartX + Camera.X), (float) (drawRect->StartY + Camera.Y), (float) z), &spriteData.Sprite, &spriteData2.Sprite, OGLData.BackgroundShader);
}

/*
void DrawScreenBMPText(draw_rect* drawRect, int x, int y, int bmpX, int bmpY, int bmpWidth, int bmpHeight, color32 color, sprite_data* sprite) {
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
*/

void DrawScreenLineThick(draw_rect* drawRect, int startX, int startY, int endX, int endY, float thickness, color32 col) { 
    // TODO(Tobi): Implement DrawScreenLineThick
    UNUSED_PARAM(drawRect);
    UNUSED_PARAM(startX);
    UNUSED_PARAM(startY);
    UNUSED_PARAM(endX);
    UNUSED_PARAM(endY);
    UNUSED_PARAM(thickness);
    UNUSED_PARAM(col);
}

void DrawScreenRectangleAlpha(draw_rect* drawRect, int x, int y, int width, int height, color32 col, int z) {
    x += Camera.X;
    y += Camera.Y;

    color4f colFloat = color32_to_color4f(col);

    RendererSetDrawRect(drawRect);
    RendererScreenRectAlpha(HMM_Translate((float) (x + drawRect->StartX), (float) (y + drawRect->StartY), (float) z) * HMM_Scale((float)width, (float)height, 1), colFloat);
}

// ##########################

void DrawWorldLineThick(draw_rect* drawRect, float startX, float startY, float endX, float endY, int pixelThickness, color32 col) {
    int intStartX = RoundFloatToInt(HEXAGON_A * startX);
    int intStartY = RoundFloatToInt(HEXAGON_A * startY);
    int intEndX = RoundFloatToInt(HEXAGON_A * endX);
    int intEndY = RoundFloatToInt(HEXAGON_A * endY);

    DrawScreenLineThick(drawRect, intStartX, intStartY, intEndX, intEndY, (float) pixelThickness, col);
}

void DrawWorldLineThick(draw_rect* drawRect, vec2f start, vec2f end, int pixelThickness, color32 col) {
    DrawWorldLineThick(drawRect, start.X, start.Y, end.X, end.Y, pixelThickness, col);
}

void DrawWorldRectangle(draw_rect* drawRect, float x, float y, float width, float height, color32 col, int z) {
    DrawScreenRectangle(drawRect, RoundFloatToInt(HEXAGON_A * x), RoundFloatToInt(HEXAGON_A * y), RoundFloatToInt(HEXAGON_A * width), RoundFloatToInt(HEXAGON_A * height), col, z);
}

void DrawWorldBitmap(draw_rect* drawRect, float x, float y, sprite_data spriteData, color32 col, int z) {
    // TODO(Tobi): Not cast to int for non-pixel-perfectness? 
    DrawScreenBitmap(drawRect, RoundFloatToInt(HEXAGON_A * x) - spriteData.LoadedBitmap.Width / 2, RoundFloatToInt(HEXAGON_A * y) - spriteData.LoadedBitmap.Height / 2, spriteData, col, z);
}

void DrawWorldDisc(draw_rect* drawRect, float x, float y, float radius, color32 col, int z) {
    DrawScreenDisc(drawRect, RoundFloatToInt(HEXAGON_A * x), RoundFloatToInt(HEXAGON_A * y), RoundFloatToInt(HEXAGON_A * radius), col, z);
}

void DrawWorldCircle(draw_rect* drawRect, float x, float y, float radius, color32 col, int z) {
    DrawScreenCircle(drawRect, RoundFloatToInt(HEXAGON_A * x), RoundFloatToInt(HEXAGON_A * y), RoundFloatToInt(HEXAGON_A * radius), col, z);
}

void DrawWorldRectangleAlpha(draw_rect* drawRect, float x, float y, float width, float height, color32 col, int z) {
    DrawScreenRectangleAlpha(drawRect, RoundFloatToInt(HEXAGON_A * x), RoundFloatToInt(HEXAGON_A * y), RoundFloatToInt(HEXAGON_A * width), RoundFloatToInt(HEXAGON_A * height), col, z);
}
