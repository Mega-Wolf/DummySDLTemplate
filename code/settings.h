#pragma once

#define PLATFORM_NO_AUDIO

#define FPS 60
#define DELTA_TIME (    1 / (float) FPS)

#define GRID_SIZE 54

#define TILES_X 20
#define TILES_Y 10

#define RIGHT_MENU_WIDTH (GRID_SIZE * 3)

#define WINDOW_WIDTH (TILES_X * GRID_SIZE + RIGHT_MENU_WIDTH)
#define WINDOW_HEIGHT (TILES_Y * GRID_SIZE)

// NOTE: x-offset; change when using a second monitor
//#define MONITOR_OFFSET 1920

#ifndef MONITOR_OFFSET
    #define MONITOR_OFFSET 0
#endif
