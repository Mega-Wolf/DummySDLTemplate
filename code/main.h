#pragma once

#include "../helpers/backend.h"

#include "maths.h"
#include "bitmap.h"

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
    bool Inactive;

    vec2f TilePosition;

    float RangeRadius;
    int CooldownFrames;
    int MaxCooldown; // Make these float?
    float Damage;

    int ColorsCount[6];

    color32 MixedColor; // NOTE(Tobi): This is only used so I don't have to calculalte it every frame; it is completely redundant though
};

struct projectile {
    vec2f Position;
    monster* Target;
    unsigned int TargetGeneration;
    float Speed;
    float Damage;
    color32 Color;

    vec2f Direction;

    vec2f BuildupDelta;
    int BuildupFrames;
};

// Global Variables
color32* Array;
int ArrayWidth;
int ArrayHeight;

int FrameCount;

terrain Ground[TILES_Y][TILES_X];

bool IsLevelEditorActive;

int StartPathX = -1;
int StartPathY = -1;

#define MONSTER_COUNT_MAX 100
int MonsterListEnd; // TODO(Tobi): Decrease MonsterListEnd if we have deleted the last monster
monster Monsters[MONSTER_COUNT_MAX];

int DiamondCount;
diamond DiamondList[TILES_Y * TILES_X];

int DistanceToGoal[TILES_Y][TILES_X];

int ProjectileCount;
projectile Projectiles[TILES_Y * TILES_X];

loaded_bitmap Cogwheels[3];
loaded_bitmap MonsterSprites[8];

loaded_bitmap IconBuy;
loaded_bitmap IconLevelUp;
loaded_bitmap IconMerge;

color32 DiamondColors[6] = {
    RED,
    GREEN,
    AQUA,
    PURPLE,
    ORANGE,
    YELLOW
};

#define MENU_DIAMONDS_Y 3
#define MENU_DIAMONDS_X 3
#define MENU_OFFSET_TILES_Y 3
#define MENU_OFFSET_Y (GRID_SIZE * MENU_OFFSET_TILES_Y)
#define MENU_OFFSET_X (GRID_SIZE * TILES_X)

#define DRAG_DROP_POSITION vec2f { 99999.0f, 99999.0f }

// TODO(Tobi): This doesn't fit with the animation since the bar will now be bigger instead of going slower
// TODO(Tobi): I don't set the socketing cooldown to the exchanged diamond
#define SOCKETING_COOLDOWN 200;
#define DIAMOND_LEVEL_1_RANGE 4.0f
#define DIAMOND_LEVEL_1_COOLDOWN 60
#define DIAMOND_LEVEL_1_DAMAGE 10.0f

#define PROJECTILE_SPEED 0.4f

struct menu_data {
    bool ShallMerge;
    bool ShallLevelUp;
    bool ShallBuy;

    struct {
        diamond* Diamond;
        vec2i Origin;
    } DragDrop;
};

menu_data Menu;

#define KEY_MERGE F4
#define KEY_LEVEL_UP F3
#define KEY_BUY F2
