#pragma once

#include <math.h>

#include "../helpers/backend.h"

#include "maths.h"

enum diamond_color {
    DC_WHITE,
    DC_YELLOW,
    DC_GREEN,
    DC_AQUA,
    DC_BLUE,
    DC_PURPLE,
    DC_RED,
    DC_BLACK,

    DC_AMOUNT
};

enum shooting_priority {
    SP_CLOSEST_TO_GOAL,
    SP_LEAST_HEALTH,
    SP_RANDOM,

    SP_AMOUNT
};

color32 DiamondColors[DC_AMOUNT] = {
    WHITE,
    YELLOW,
    GREEN,
    AQUA,
    BLUE,
    PURPLE,
    RED,
    DARK_GREY,
};

struct diamond {
    unsigned int Generation;
    unsigned int LastGeneration;

    shooting_priority Priority;
    
    bool IsInField;

    vec2f ActualPosition;
    vec2i TilePositionTopLeft;

    float RangeRadius;
    int CooldownFrames;
    int MaxCooldown; // Make these float?
    float Damage;

    int ColorsCount[DC_AMOUNT];

    color32 MixedColor; // NOTE(Tobi): This is only used so I don't have to calculalte it every frame; it is completely redundant though
};

struct monster {
    unsigned int Generation;
    unsigned int LastGeneration;

    vec2i GoalPosition;
    vec2i OldPosition;
    float Radius;
    float Speed;
    float Health;
    float Armor;
    float Magic;
    color32 Color;

    float PoisonAmount;
    int PoisonFrames;

    int SlowFrames;

    int WoundedFrames;

    float MovementT;
    vec2f ActualPosition;

    float Mana; // TODO(Tobi): Do I really need that

    float MaxHealth; // TODO(Tobi): Isn't that given in the MonsterWave
    // MaxArmor; MaxMagic?

};

struct projectile {
    vec2f Position;
    
    generation_link<monster> Target;

    float Speed;
    float Damage;

    int ColorsCount[DC_AMOUNT];
    color32 Color;

    vec2f Direction;

    vec2f BuildupDelta;
    int BuildupFrames;
};

diamond _DiamondList[TILES_Y * TILES_X];
bucket_list<diamond> Diamonds = BucketListInit(ArrayCount(_DiamondList), _DiamondList);


int ProjectileCount;
projectile Projectiles[TILES_Y * TILES_X];

// TODO(Tobi): This doesn't fit with the animation since the bar will now be bigger instead of going slower
#define SOCKETING_COOLDOWN 200;
#define DIAMOND_LEVEL_1_RANGE 4.0f
#define DIAMOND_LEVEL_1_COOLDOWN 60
#define DIAMOND_LEVEL_1_DAMAGE 10.0f
#define DIAMOND_LEVEL_1_MAGIC_REDUCE 10.0f
#define DIAMOND_LEVEL_1_ARMOR_REDUCE 10.0f
#define DIAMOND_LEVEL_1_MANA_STEAL 10.0f

#define DIAMOND_EFFECT_LENGTH (3 * FPS) // NOTE(Tobi): At the moment, poison, slow and wounded all last the same amount of time

#define DIAMOND_LEVEL_1_POISON 5.0f

#define PROJECTILE_SPEED 0.4f


float Mana; // NOTE(Tobi): I have this as float at the moment, so that I can have fractions of mana (do I want that?)
float ManaMergeCost = 240.0f; // TODO(Tobi): This will probably be set once at the start of the level
float ManaDiamondLevel1Cost = 60.0f;
float ManaStartValue = 400.0f;
float ManaGainPerSecond = 3.6f;

float CurrentWallCost = 0.06f; //60; // TODO(Tobi): Correct wall cost
float CurrentTowerCost = 0.10f; //100; // TODO(Tobi): Correct tower cost
float CurrentTrapCost = 0.10f; //100; // TODO(Tobi): Correct trap cost

void DiamondSetValues(diamond* dim, int count) {
    dim->Damage = count * DIAMOND_LEVEL_1_DAMAGE;
    dim->RangeRadius = sqrtf(1.0f + (count - 1) /100.0f) * DIAMOND_LEVEL_1_RANGE;
    dim->MaxCooldown = (int) (powf(0.975f, (float) (count - 1)) * DIAMOND_LEVEL_1_COOLDOWN);
    dim->CooldownFrames = SOCKETING_COOLDOWN;
}

float ManaCalcBuyCost(int buyingLevel) {
    int numberOfMerges = buyingLevel;
    float manaMerging = numberOfMerges * ManaMergeCost;
    int numberOfLevel1s = 1 << buyingLevel;
    float manaDiamondCost = numberOfLevel1s * ManaDiamondLevel1Cost;

    float totalCost = manaMerging + manaDiamondCost;
    return totalCost;
}

