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
    unsigned int Generation;

    vec2i GoalPosition;
    vec2i OldPosition;
    float Radius;
    float Speed;
    float Health;
    float MaxHealth;

    float MovementT;
    vec2f ActualPosition;
};

struct diamond {
    vec2f TilePosition;

    float RangeRadius;
    int CooldownFrames;
    int MaxCooldown;
};

struct projectile {
    vec2f Position;
    monster* Target;
    unsigned int TargetGeneration;
    float Speed;
    float Damage;

    vec2f BuildupDelta;
    int BuildupFrames;
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

#define MONSTER_COUNT_MAX 100
int MonsterListEnd; // TODO(Tobi): Decrease MonsterListEnd if we have deleted the last monster
monster Monsters[MONSTER_COUNT_MAX];

int DiamondCount;
diamond DiamondList[32 * 32];

int DistanceToGoal[32][32];

int ProjectileCount;
projectile Projectiles[32 * 32];
