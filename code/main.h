#pragma once

#include "../helpers/backend.h"

#include "bucket_list.h"

#include "maths.h"
#include "diamond.h"
#include "bitmap.h"
#include "waveform.h"

enum terrain {
    T_GRASS = 0,
    T_PATH = 1,
    T_TOWER = 2,
    T_GOAL = 4,
    T_TRAP = 8,
    T_WALL = 16,
};

int FrameCount;

terrain Ground[TILES_Y][TILES_X];

bool IsLevelEditorActive = true; // TODO(Tobi): This is done so the game is paused at the beginning

int StartPositionsCount;
vec2i StartPositions[TILES_Y * 2 + (TILES_X - 2) * 2];

#define MONSTER_COUNT_MAX 100
monster _MonsterList[MONSTER_COUNT_MAX];
bucket_list<monster> Monsters = BucketListInit(ArrayCount(_MonsterList), _MonsterList);

int DistanceToGoal[TILES_Y][TILES_X];

#define MENU_DIAMONDS_Y 7
#define MENU_DIAMONDS_X 3 // TODO(Tobi): Remove
#define MENU_DIAMONDS_PAIRS_X 1
#define MENU_OFFSET_TILES_Y 3
#define MENU_OFFSET_Y (GRID_SIZE * MENU_OFFSET_TILES_Y)
#define MENU_OFFSET_X (GRID_SIZE * TILES_X)

#define DRAG_DROP_POSITION vec2i { 99999, 99999 }

struct menu_data {
    int SelectedBuyingLevel;

    bool ShallMerge;

    bool WallBuildMode;
    bool TowerBuildMode;
    bool TrapBuildMode;

    diamond* Diamonds[(MENU_DIAMONDS_Y + 1) / 2][MENU_DIAMONDS_X];

    struct {
        diamond* Diamond;
        vec2i OriginTopLeft;
        bool WasInField;
    } DragDrop;
};

menu_data Menu;

struct monster_wave {
    int FullCount;
    int AlreadyReleased;
    int DelayFrames;
    int ActualStartFrame;

    struct {
        float Radius; // TODO(Tobi): This is not used anymore
        float Speed;
        float MaxHealth;
        float MaxArmor;
        float MaxMagic;
        color32 Color;
        float Mana;

        //newMonster->Radius = (rand() % 6 + 5) / (float)GRID_SIZE;
        //newMonster->Speed = 1 / (float) (rand() % 11 + 10);
    } Prototype;

};

#define WAVE_COUNT 4

#define WAVE_FRAME_LENGTH (FPS * 20)

monster_wave MonsterWaves[WAVE_COUNT] = {
    {
        10,
        0,
        40,
        0,
        {
            18 / (float)GRID_SIZE,
            1 / 16.0f,
            50.0f,
            50.0f,
            50.0f,
            RED,
            25.0f,
        },
    },
    {
        20,
        0,
        25,
        0,
        {
            8 / (float)GRID_SIZE,
            1 / 10.0f,
            10.0f,
            50.0f,
            50.0f,
            GREEN,
            30.0f,
        },
    },
    {
        20,
        0,
        35,
        0,
        {
            21 / (float)GRID_SIZE,
            1 / 16.0f,
            50.0f,
            500.0f,
            0,
            BLUE,
            35.0f,
        },
    },
    {
        30,
        0,
        20,
        0,
        {
            9 / (float)GRID_SIZE,
            1 / 10.0f,
            10.0f,
            0,
            2000.0f,
            YELLOW,
            40.0f,
        },
    }
};

int MonsterWaveFrames;
int MonsterWaveSpeedEnd;

#define MONSTER_WAVE_FAST_SPEED 7

#define KEY_TOGGLE_EDITOR F1
#define KEY_BUY F2
#define KEY_LEVEL_UP F3
#define KEY_MERGE F4
#define KEY_SPEED_WAVE F5
#define KEY_TOGGLE_SHOW_PATHFINDING F6
#define KEY_WALL F7
#define KEY_TOWER F8
#define KEY_TRAP F9

bool ShowPathfinding;

int ShakeFrames;

#define SQRT_3 1.73205080757f
#define HEXAGON_H (SQRT_3 / 2)
