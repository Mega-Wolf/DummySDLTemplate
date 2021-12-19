#pragma once

#include <math.h>

#include "../helpers/backend.h"

#include "maths.h"

enum diamond_color {
    DC_RED,
    DC_GREEN,
    DC_AQUA,
    DC_PURPLE,
    DC_ORANGE,
    DC_YELLOW
};

color32 DiamondColors[6] = {
    RED,
    GREEN,
    AQUA,
    PURPLE,
    ORANGE,
    YELLOW
};

struct diamond {
    bool Inactive;
    bool IsInField;

    vec2f TilePosition;

    float RangeRadius;
    int CooldownFrames;
    int MaxCooldown; // Make these float?
    float Damage;

    int ColorsCount[6];

    color32 MixedColor; // NOTE(Tobi): This is only used so I don't have to calculalte it every frame; it is completely redundant though
};

struct monster {
    unsigned int Generation;

    vec2i GoalPosition;
    vec2i OldPosition;
    float Radius;
    float Speed;
    float Health;
    float MaxHealth;
    color32 Color;

    float PoisonSpeed;

    float MovementT;
    vec2f ActualPosition;
};

struct projectile {
    vec2f Position;
    monster* Target;
    unsigned int TargetGeneration;
    float Speed;
    float Damage;

    int ColorsCount[6];
    color32 Color;

    vec2f Direction;

    vec2f BuildupDelta;
    int BuildupFrames;
};

int DiamondCount;
diamond DiamondList[TILES_Y * TILES_X];

int ProjectileCount;
projectile Projectiles[TILES_Y * TILES_X];

// TODO(Tobi): This doesn't fit with the animation since the bar will now be bigger instead of going slower
#define SOCKETING_COOLDOWN 200;
#define DIAMOND_LEVEL_1_RANGE 4.0f
#define DIAMOND_LEVEL_1_COOLDOWN 60
#define DIAMOND_LEVEL_1_DAMAGE 10.0f

#define DIAMOND_LEVEL_1_POISON 5.0f
#define MONSTER_POISON_DECREASE_PER_FRAME 1.0f

#define PROJECTILE_SPEED 0.4f

void DiamondSetValues(diamond* dim, int count) {
    dim->Damage = count * DIAMOND_LEVEL_1_DAMAGE;
    dim->RangeRadius = sqrtf(1.0f + (count - 1) /100.0f) * DIAMOND_LEVEL_1_RANGE;
    dim->MaxCooldown = (int) (powf(0.975f, (float) (count - 1)) * DIAMOND_LEVEL_1_COOLDOWN);
    dim->CooldownFrames = SOCKETING_COOLDOWN;
}

