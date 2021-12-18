#pragma once

#include "../helpers/backend.h"

#include "maths.h"
#include "bitmap.h"
#include "waveform.h"

enum terrain {
    T_GRASS = 1,
    T_PATH = 2,
    T_TOWER = 4,
    T_GOAL = 8,
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

    float MovementT;
    vec2f ActualPosition;
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

int FrameCount;

terrain Ground[TILES_Y][TILES_X];

bool IsLevelEditorActive = true; // TODO(Tobi): This is done so the game is paused at the beginning

int StartPositionsCount;
vec2i StartPositions[TILES_Y * 2 + (TILES_X - 2) * 2];

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
        color32 Color;

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
            7 / (float)GRID_SIZE,
            1 / 16.0f,
            50.0f,
            RED,
        },
    },
    {
        20,
        0,
        25,
        0,
        {
            4 / (float)GRID_SIZE,
            1 / 10.0f,
            10.0f,
            GREEN,
        },
    },
    {
        20,
        0,
        35,
        0,
        {
            7 / (float)GRID_SIZE,
            1 / 16.0f,
            50.0f,
            BLUE,
        },
    },
    {
        30,
        0,
        20,
        0,
        {
            4 / (float)GRID_SIZE,
            1 / 10.0f,
            10.0f,
            YELLOW,
        },
    }
};

int MonsterWaveFrames;
int MonsterWaveSpeedEnd;

#define MONSTER_WAVE_FAST_SPEED 7

#define KEY_MERGE F4
#define KEY_LEVEL_UP F3
#define KEY_BUY F2

#define KEY_SPEED_WAVE F5

loaded_audio Music;
loaded_audio SoundHit;
loaded_audio SoundDeath;

int ShakeFrames;

#define PATH_BMP_UP 1
#define PATH_BMP_DOWN 2
#define PATH_BMP_LEFT 4
#define PATH_BMP_RIGHT 8
loaded_bitmap BitmapsPath[16];

loaded_bitmap BitmapTower;
loaded_bitmap BitmapGoal;

loaded_bitmap BitmapsSmoke[16];
