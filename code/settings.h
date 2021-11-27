#pragma once

#define PLATFORM_NO_AUDIO

#define FPS 60
#define DELTA_TIME (    1 / (float) FPS)

#define GRID_SIZE 32

#define WINDOW_WIDTH (32 * GRID_SIZE)
#define WINDOW_HEIGHT (32 * GRID_SIZE)

// NOTE: x-offset; change when using a second monitor
//#define MONITOR_OFFSET 1920

#ifndef MONITOR_OFFSET
    #define MONITOR_OFFSET 0
#endif