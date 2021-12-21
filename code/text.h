#pragma once

#include "../helpers/backend.h"

#include "maths.h"
#include "drawing.h"
#include "bitmap.h"

struct letter_info {
    // id

    int X;
    int Y;
    int Width;
    int XOffset;
    int XAdvance;
};

struct font_info {
    loaded_bitmap Bitmap;
    int FontSize;
    letter_info LetterInfo[96];
};

// NOTE(Tobi): This one is just copied at the beginning
// TODO(Tobi): Probably still do this differently!
// TODO(Tobi): Provide more fonts here (The assets folder at least holds images for 10, 12, 16, 24); maybe provide 20 and something big like 32 etc.
#if 1
font_info debugFont = {
    {},
    16,
    {
        {254, 68,  1,  0, 4},
        {28,  136, 2,  0, 3},
        {169, 119, 4,  0, 5},
        {170, 34,  8,  0, 7},
        {49,  102, 6,  0, 7},
        {157, 0,   11, 0, 12},
        {51,  34,  9,  0, 9},
        {37,  136, 2,  0, 2},
        {159, 119, 4,  0, 4},
        {12,  136, 3,  0, 4},
        {148, 119, 5,  0, 5},
        {215, 34,  8,  0, 8},
        {67,  136, 2,  1, 4},
        {20,  136, 3,  0, 4},
        {58,  136, 2,  0, 4},
        {219, 119, 4,  0, 4},
        {56,  102, 6,  0, 7},
        {179, 119, 4,  0, 7},
        {63,  102, 6,  0, 7},
        {70,  102, 6,  0, 7},
        {77,  102, 6,  0, 7},
        {84,  102, 6,  0, 7},
        {91,  102, 6,  0, 7},
        {98,  102, 6,  0, 7},
        {105, 102, 6,  0, 7},
        {112, 102, 6,  0, 7},
        {52,  136, 2,  0, 4},
        {49,  136, 2,  0, 4},
        {126, 68,  7,  0, 8},
        {134, 68,  7,  0, 8},
        {142, 68,  7,  0, 8},
        {119, 102, 6,  0, 7},
        {84,  0,   12, 0, 13},
        {97,  0,   11, -1, 9},
        {90,  51,  8,  0, 9},
        {99,  51,  8,  0, 9},
        {108, 51,  8,  0, 9},
        {117, 51,  8,  0, 9},
        {96,  85,  7,  0, 8},
        {91,  34,  9,  0, 10},
        {126, 51,  8,  0, 9},
        {46,  136, 2,  0, 3},
        {142, 119, 5,  0, 6},
        {141, 34,  9,  0, 9},
        {80,  85,  7,  0, 7},
        {245, 17,  10, 0, 11},
        {144, 51,  8,  0, 9},
        {21,  34,  9,  0, 10},
        {0,   51,  8,  0, 9},
        {41,  34,  9,  0, 10},
        {61,  34,  9,  0, 9},
        {153, 51,  8,  0, 9},
        {8,   85,  7,  0, 7},
        {162, 51,  8,  0, 9},
        {169, 0,   11, -1, 9},
        {28,  0,   13, 0, 13},
        {238, 68,  7,  0, 7},
        {171, 51,  8,  0, 9},
        {174, 68,  7,  0, 7},
        {244, 119, 3,  0, 4},
        {189, 119, 4,  0, 4},
        {4,   136, 3,  0, 4},
        {136, 119, 5,  0, 5},
        {230, 68,  7,  0, 7},
        {252, 119, 3,  0, 4},
        {126, 102, 6,  0, 7},
        {28,  119, 6,  0, 7},
        {133, 102, 6,  0, 7},
        {140, 102, 6,  0, 7},
        {147, 102, 6,  0, 7},
        {194, 119, 4,  0, 3},
        {154, 102, 6,  0, 7},
        {161, 102, 6,  0, 7},
        {64,  136, 2,  0, 3},
        {236, 119, 3,  -1, 3},
        {168, 102, 6,  0, 7},
        {34,  136, 2,  0, 3},
        {0,   34,  10, 0, 11},
        {175, 102, 6,  0, 7},
        {182, 102, 6,  0, 7},
        {189, 102, 6,  0, 7},
        {196, 102, 6,  0, 7},
        {199, 119, 4,  0, 4},
        {203, 102, 6,  0, 7},
        {248, 119, 3,  0, 4},
        {210, 102, 6,  0, 7},
        {118, 119, 5,  0, 5},
        {81,  34,  9,  0, 9},
        {217, 102, 6,  0, 7},
        {224, 102, 6,  0, 7},
        {231, 102, 6,  0, 7},
        {240, 119, 3,  0, 4},
        {43,  136, 2,  0, 3},
        {251, 34,  4,  0, 4},
        {40,  85,  7,  0, 8},
    }
};
#endif

#if 0
font_info debugFont = {
    {},
    24,
    {
        {509, 125, 1, 0, 6},
        {139, 150, 3, 1, 6},
        {420, 125, 7, 0, 7},
        {313, 50, 12, 0, 12},
        {66, 125, 10, 0, 12},
        {172, 0, 18, 0, 19},
        {452, 25, 14, 0, 14},
        {131, 150, 3, 0, 4},
        {7, 150, 6, 0, 7},
        {0, 150, 6, 0, 7},
        {334, 125, 8, 0, 8},
        {194, 75, 11, 0, 12},
        {135, 150, 3, 1, 6},
        {495, 125, 6, 0, 7},
        {159, 150, 3, 1, 6},
        {42, 150, 6, 0, 6},
        {242, 75, 11, 0, 12},
        {436, 125, 7, 1, 12},
        {266, 75, 11, 0, 12},
        {264, 100, 11, 0, 12},
        {24, 100, 11, 0, 12},
        {252, 100, 11, 0, 12},
        {240, 100, 11, 0, 12},
        {228, 100, 11, 0, 12},
        {216, 100, 11, 0, 12},
        {192, 100, 11, 0, 12},
        {119, 150, 3, 1, 6},
        {151, 150, 3, 1, 6},
        {156, 100, 11, 0, 12},
        {144, 100, 11, 0, 12},
        {132, 100, 11, 0, 12},
        {120, 100, 11, 0, 12},
        {132, 0, 20, 0, 21},
        {102, 25, 15, -1, 13},
        {482, 50, 12, 1, 14},
        {467, 25, 14, 0, 15},
        {215, 50, 13, 1, 15},
        {352, 50, 12, 1, 14},
        {36, 100, 11, 1, 13},
        {118, 25, 15, 0, 16},
        {201, 50, 13, 0, 14},
        {147, 150, 3, 1, 6},
        {297, 125, 9, 0, 11},
        {229, 50, 13, 1, 14},
        {77, 125, 10, 1, 12},
        {69, 25, 16, 0, 17},
        {243, 50, 13, 0, 14},
        {230, 25, 15, 0, 16},
        {0, 75, 12, 1, 14},
        {262, 25, 15, 0, 16},
        {257, 50, 13, 1, 15},
        {271, 50, 13, 0, 14},
        {456, 50, 12, 0, 12},
        {285, 50, 13, 0, 14},
        {299, 50, 13, 0, 13},
        {110, 0, 21, 0, 21},
        {482, 25, 14, 0, 14},
        {497, 25, 14, 0, 14},
        {417, 50, 12, 0, 13},
        {68, 150, 5, 0, 6},
        {481, 125, 6, 0, 6},
        {74, 150, 5, 0, 6},
        {307, 125, 8, 0, 8},
        {169, 75, 12, 0, 12},
        {80, 150, 4, 0, 7},
        {410, 75, 11, 0, 12},
        {422, 100, 10, 0, 11},
        {99, 125, 10, 0, 11},
        {110, 125, 10, 0, 11},
        {374, 75, 11, 0, 12},
        {428, 125, 7, 0, 6},
        {121, 125, 10, 0, 11},
        {132, 125, 10, 0, 11},
        {143, 150, 3, 0, 5},
        {90, 150, 4, -1, 4},
        {143, 125, 10, 0, 10},
        {123, 150, 3, 0, 4},
        {278, 25, 15, 0, 16},
        {154, 125, 10, 0, 11},
        {290, 75, 11, 0, 12},
        {176, 125, 10, 0, 11},
        {187, 125, 10, 0, 11},
        {412, 125, 7, 0, 7},
        {198, 125, 10, 0, 11},
        {56, 150, 5, 0, 6},
        {209, 125, 10, 0, 11},
        {75, 50, 13, -1, 11},
        {228, 0, 17, -1, 15},
        {494, 75, 11, -1, 10},
        {334, 100, 10, 0, 11},
        {345, 100, 10, 0, 9},
        {467, 125, 6, 0, 7},
        {508, 50, 3, 1, 6},
        {396, 125, 7, 0, 7},
        {302, 75, 11, 0, 12},
    }
};
#endif

// Debug font (Arial16)
font_info AcquireDebugFont() {
    font_info fi = debugFont;
    fi.Bitmap = BitmapLoad("assets/fonts/Arial16.bmp");
    return fi;
}

int TextGetRenderSize(font_info* fontInfo, char* text) {
    int width = 0;
    for (char* c = text; *c; c++) {
        width += fontInfo->LetterInfo[*c - ' '].XAdvance;
    }
    return width;
}

int TextGetRenderSizeWithCount(font_info* fontInfo, char* text, int amount) {
    int width = 0;
    inc0 (i,   amount) {
        char c = text[i];
        //Assert(c, "'TextGetRenderSizeWithCount' wanted to check the null character");
        width += fontInfo->LetterInfo[c - ' '].XAdvance;
    }
    return width;
}

// TODO(Tobi): width and height
// TODO(Tobi): I might actually want to specify a fontSize and the font info doesn't hold the fontSize itself (it will be more like a true type or something)
int TextRenderScreen(draw_rect* drawRect, font_info* fontInfo, int left, int top, char* text, color32 color/*, color32 backgroundColor*/) {
 
    // TODO(Tobi): This is just a dummy
    // Can I directly render here; I think later this will just create tiles with the texture on it right?
    // Those are then rendered sometime else

    // IMPORTANT(Tobi): Don't do that here, I will call functions that already do that translation so I would do that twice
    // left += drawArea->X;
    // top += drawArea->Y;

    int x = left;
    int oldX = x;
    for (char* c = text; *c; c++) {
        letter_info* letterInfo = &fontInfo->LetterInfo[*c - ' '];
        // color randomColor = {};
        // randomColor.Red = rand() % 255;
        // randomColor.Blue = rand() % 255;
        // randomColor.Green = rand() % 255;
        //ShapesBox(x, top, width, fontSize, col);

        //ShapesBox(drawArea, oldX, top, (x - oldX) + (float) letterInfo->XOffset, (float) fontInfo->FontSize, backgroundCol);

        DrawScreenBMPText(
            drawRect,
            x + letterInfo->XOffset, top,
            letterInfo->X, letterInfo->Y,
            letterInfo->Width, fontInfo->FontSize,
            color, /*backgroundColor, */
            &fontInfo->Bitmap);

        oldX = x + letterInfo->XOffset + letterInfo->Width;

        x += letterInfo->XAdvance;
    }

    //ShapesBox(drawArea, oldX, top, x - oldX, (float) fontInfo->FontSize, backgroundCol);

    return x - left;
}

int TextRenderWorld(draw_rect* drawRect, font_info* fontInfo, float left, float top, char* text, color32 color/*, color32 backgroundColor*/) {
    return TextRenderScreen(drawRect, fontInfo, (int) (GRID_SIZE * left) + GRID_SIZE / 2, (int) (GRID_SIZE * top) + GRID_SIZE / 2, text, color/*, backgroundColor*/);
}
