#pragma once

#include "settings.h"

#define UNUSED_PARAM(param) (param);

union color32 {
    unsigned int Value;
    struct {
        unsigned char Blue;
        unsigned char Green;
        unsigned char Red;
        unsigned char Alpha;
    };
};

#define COL32_RGB(red, green, blue)         { (unsigned int) ((((unsigned char)     255) << 24) | (((unsigned char) (red)) << 16) | (((unsigned char) (green)) << 8) | (((unsigned char) (blue))) ) }
#define COL32_RGBA(red, green, blue, alpha) { (unsigned int) ((((unsigned char) (alpha)) << 24) | (((unsigned char) (red)) << 16) | (((unsigned char) (green)) << 8) | (((unsigned char) (blue))) ) }

const color32 RED         = COL32_RGB(255,   0,   0);
const color32 DARK_GREEN  = COL32_RGB(  0, 128,   0);
const color32 GREEN       = COL32_RGB(  0, 255,   0);
const color32 YELLOW      = COL32_RGB(255, 255,   0);
const color32 BLUE        = COL32_RGB(  0,   0, 255);
const color32 AQUA        = COL32_RGB(  0, 255, 255);
const color32 ORANGE      = COL32_RGB(255, 127,   0);
const color32 PINK        = COL32_RGB(255,   0, 255);
const color32 PURPLE      = COL32_RGB(127,   0, 127);
const color32 BLACK       = COL32_RGB(  0,   0,   0);
const color32 DARK_GREY   = COL32_RGB( 64,  64,  64);
const color32 LIGHT_GREY  = COL32_RGB(160, 160, 160);
const color32 WHITE       = COL32_RGB(255, 255, 255);
const color32 C_TRANSPARENT = { 0 };

struct button {
    bool Down;
    bool Toggled;
};

#define PLATFORM_KEYBOARD_BUTTON_COUNT 20
union keyboard {
    struct {
        button Up;
        button Right;
        button Down;
        button Left;
        button Space;
        button Enter;
        button Backspace;
        button Escape;

        button F1;
        button F2;
        button F3;
        button F4;
        button F5;
        button F6;
        button F7;
        button F8;
        button F9;
        button F10;
        button F11;
        button F12;
    };
    button Buttons[PLATFORM_KEYBOARD_BUTTON_COUNT];
};

#define PLATFORM_MOUSE_BUTTON_COUNT 3
struct mouse {
    int PosX;
    int PosY;
    int WheelDelta;
    union {
        struct {
            button Left;
            button Right;
            button Middle;
        };
        button Buttons[PLATFORM_MOUSE_BUTTON_COUNT];
    };
};

struct inputs {
    keyboard Keyboard;
    mouse Mouse;
};

bool Running;
inputs Inputs;

// TODO(Tobi): I don't know whether SDL does some weird things with SDL_GetPerformanceFrequency ans do I didn't know if I can cache it or not
//unsigned long long GlobalPerfCountFrequency;
