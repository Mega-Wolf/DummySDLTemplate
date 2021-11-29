#pragma once

#include "../helpers/backend.h"

#include "maths.h"

enum terrain {
    T_GRASS,
    T_PATH,
    T_TOWER,
    T_GOAL,
};

struct monster {
    vec2i GoalPosition;
    vec2i OldPosition;
    float Radius;
    float Speed;

    float MovementT;
    vec2f ActualPosition;
};

struct diamond {
    vec2i TilePosition;

    float RangeRadius;
    int CooldownFrames;
    int MaxCooldown;
};

// Global Variables
color32* Array;
int ArrayWidth;
int ArrayHeight;

int FrameCount;

terrain Ground[32][32];

bool IsLevelEditorActive;

int StartPathX = -1;
int StartPathY = -1;

int SpawnedMonsterAmount;
monster Monsters[10];

int DiamondCount;
diamond DiamondList[32 * 32];

int DistanceToGoal[32][32];
