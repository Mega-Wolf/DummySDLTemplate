#pragma once

#define PLATFORM_NO_AUDIO

#define FPS 60
#define DELTA_TIME (    1 / (float) FPS)

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

// NOTE: x-offset; change when using a second monitor
//#define MONITOR_OFFSET 1922

#ifndef MONITOR_OFFSET
    #define MONITOR_OFFSET 0
#endif