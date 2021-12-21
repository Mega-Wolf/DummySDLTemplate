#pragma once

#include "bitmap.h"
#include "waveform.h"


/// Music

loaded_audio Music;

/// Sounds

loaded_audio SoundHit;
loaded_audio SoundDeath;


/// Sprites

loaded_bitmap BitmapWhiteUp;
loaded_bitmap BitmapWhiteDown;

loaded_bitmap BitmapTower;
loaded_bitmap BitmapTrap;
loaded_bitmap BitmapGoal;

#define TRI_DOWN_TOP 1
#define TRI_DOWN_LEFT 2
#define TRI_DOWN_RIGHT 4
loaded_bitmap BitmapsPathDown[8];
loaded_bitmap BitmapGrassDown;

#define TRI_UP_BOTTOM 1
#define TRI_UP_LEFT 2
#define TRI_UP_RIGHT 4
loaded_bitmap BitmapsPathUp[8];
loaded_bitmap BitmapGrassUp;


/// Particles

loaded_bitmap BitmapsSmoke[16];