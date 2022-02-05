#pragma once

#include "bitmap.h"
#include "waveform.h"
#include "../helpers/opengl.h"

// #define SPRITE_INIT_FUNCTION SPRITE_INIT_FUNCTION
// #define SPRITE_TYPE loaded_bitmap
#define SPRITE_INIT_FUNCTION CreateSprite
#define SPRITE_TYPE sprite_data

// TODO(Tobi): The sprite init funciton might need more data like what'S the centre of the sprite

#define ASSET_LIST(name, pairs)
#define ASSET_AUDIO(name, path)
#define ASSET_SPRITE(name, path)
#define ASSET_SPRITE_ARRAY(name, pathStart, size, pathEnd)
#define ASSET_SPRITE_ARRAY_NAMES(name, size, ...)
#define ASSET_SPRITE_ARRAY_BITFIELD(name, pathStart, bits, pathEnd, ...)
#define AUDIO_LIST ASSET_LIST
#define SPRITE_LIST ASSET_LIST

/// Music
#define ASSETS_MUSIC AUDIO_LIST(Music, \
    ASSET_AUDIO(Music, "assets\\audio\\Background.wav") \
)

/// Sounds
#define ASSETS_SOUNDS AUDIO_LIST(Sounds, \
    ASSET_AUDIO(Hit, "assets\\audio\\SplatHit.wav") \
    ASSET_AUDIO(Death, "assets\\audio\\SplatDeathReverb.wav") \
)

/// Sprites
#define ASSETS_SPRITES SPRITE_LIST(Sprites, \
    ASSET_SPRITE(WhiteUp, "assets\\sprites\\hexagon\\Up_White.bmp") \
    ASSET_SPRITE(WhiteDown, "assets\\sprites\\hexagon\\Down_White.bmp") \
    ASSET_SPRITE(GrassUp, "assets\\sprites\\hexagon\\Up_Grass.bmp") \
    ASSET_SPRITE(GrassDown, "assets\\sprites\\hexagon\\Down_Grass.bmp") \
    ASSET_SPRITE(Tower, "assets\\sprites\\hexagon\\Tower.bmp") \
    ASSET_SPRITE(Trap, "assets\\sprites\\hexagon\\Trap.bmp") \
    ASSET_SPRITE(Goal, "assets\\sprites\\hexagon\\Goal.bmp") \
    ASSET_SPRITE(IconBuy, "assets\\sprites\\Buy.bmp") \
    ASSET_SPRITE(IconLevelUp, "assets\\sprites\\LevelUp.bmp") \
    ASSET_SPRITE(IconMerge, "assets\\sprites\\Merge.bmp") \
    ASSET_SPRITE_ARRAY_NAMES(Cogwheels, 3, "assets\\sprites\\hexagon\\Gear0.bmp", "assets\\sprites\\hexagon\\Gear15.bmp", "assets\\sprites\\hexagon\\Gear30.bmp" ) \
    ASSET_SPRITE_ARRAY_NAMES(Monster, 8, "assets\\sprites\\MonsterUp.bmp", "assets\\sprites\\MonsterUpRight.bmp", "assets\\sprites\\MonsterRight.bmp", "assets\\sprites\\MonsterDownRight.bmp", "assets\\sprites\\MonsterDown.bmp", "assets\\sprites\\MonsterDownLeft.bmp", "assets\\sprites\\MonsterLeft.bmp", "assets\\sprites\\MonsterUpLeft.bmp") \
    ASSET_SPRITE_ARRAY_BITFIELD(PathUp, "assets\\sprites\\hexagon\\Up_", 3, ".bmp", "Bottom", "Left", "Right") \
    ASSET_SPRITE_ARRAY_BITFIELD(PathDown, "assets\\sprites\\hexagon\\Down_", 3, ".bmp", "Top", "Left", "Right") \
    ASSET_SPRITE(BloodTex, "assets\\sprites\\Blood.bmp") \
    ASSET_SPRITE(PoisonTex, "assets\\sprites\\Poison.bmp") \
    ASSET_SPRITE(IceTex, "assets\\sprites\\Ice.bmp") \
)

/// Particles
#define ASSETS_PARTICLES SPRITE_LIST(Particles, \
    ASSET_SPRITE_ARRAY(Smoke, "assets\\sprites\\Smoke", 16, ".bmp") \
)



#undef ASSET_LIST
#undef ASSET_AUDIO
#undef ASSET_SPRITE
#undef ASSET_SPRITE_ARRAY
#undef ASSET_SPRITE_ARRAY_NAMES
#undef ASSET_SPRITE_ARRAY_BITFIELD

#define ASSET_AUDIO(name, path) loaded_audio name;
#define ASSET_SPRITE(name, path) SPRITE_TYPE name;
#define ASSET_SPRITE_ARRAY(name, pathStart, size, ...) SPRITE_TYPE name[size];
#define ASSET_SPRITE_ARRAY_NAMES(name, size, ...) SPRITE_TYPE name[size];
#define ASSET_SPRITE_ARRAY_BITFIELD(name, pathStart, bits, pathEnd, ...) SPRITE_TYPE name[1 << bits];

#define ASSET_LIST(name, pairs) \
    struct { \
        pairs \
    } name; \

ASSETS_MUSIC
ASSETS_SOUNDS
ASSETS_SPRITES
ASSETS_PARTICLES

#undef ASSET_LIST
#undef ASSET_AUDIO
#undef ASSET_SPRITE
#undef ASSET_SPRITE_ARRAY
#undef ASSET_SPRITE_ARRAY_NAMES
#undef ASSET_SPRITE_ARRAY_BITFIELD
#undef AUDIO_LIST
#undef SPRITE_LIST

#define ASSET_AUDIO(name, path) { \
    *AssetSystemActiveAudio++ = LoadWav(path); \
}
#define ASSET_SPRITE(name, path) { \
    *AssetSystemActiveBitmap++ = SPRITE_INIT_FUNCTION(path); \
}

#define ASSET_SPRITE_ARRAY(name, pathStart, size, pathEnd) \
inc0 (_i,   size) { \
    char _dummy[256]; \
    snprintf(_dummy, 256, pathStart ## "%d" ## pathEnd, _i); \
    *AssetSystemActiveBitmap++ = SPRITE_INIT_FUNCTION(_dummy); \
}

#define ASSET_SPRITE_ARRAY_NAMES(name, size, ...) \
{ \
    char* _names[] = { __VA_ARGS__ }; \
    inc0 (_i,   size) { \
        *AssetSystemActiveBitmap++ = SPRITE_INIT_FUNCTION(_names[_i]); \
    } \
}

#define ASSET_SPRITE_ARRAY_BITFIELD(name, pathStart, bits, pathEnd, ...) \
inc0 (_i,   1 << bits) { \
    char* _names[] = { __VA_ARGS__ }; \
    char _dummy[256]; \
    int _count = snprintf(_dummy, 256, pathStart); \
    inc0 (_ii,   bits) { \
        if (_i & (1 << _ii)) { \
            _count += snprintf(&_dummy[_count], 256 - _count, "%s", _names[_ii]); \
        } \
    } \
    _count += snprintf(&_dummy[_count], 256 - _count, pathEnd); \
    *AssetSystemActiveBitmap++ = SPRITE_INIT_FUNCTION(_dummy); \
}

#define AUDIO_LIST(name, pairs) \
    AssetSystemStartAudioList(&name); \
    ASSET_LIST(name, pairs)

#define SPRITE_LIST(name, pairs) \
    AssetSystemStartSpriteList(&name); \
    ASSET_LIST(name, pairs)

#define ASSET_LIST(name, pairs) \
    pairs


SPRITE_TYPE* AssetSystemActiveBitmap;
loaded_audio* AssetSystemActiveAudio;

void AssetSystemStartAudioList(void* audioList) {
    AssetSystemActiveAudio = (loaded_audio*) audioList;
    AssetSystemActiveBitmap = nullptr;
}

void AssetSystemStartSpriteList(void* spriteList) {
    AssetSystemActiveAudio = nullptr;
    AssetSystemActiveBitmap = (SPRITE_TYPE*) spriteList;
}

void AssetsInit () {
    // Loading all the assets

    ASSETS_MUSIC
    ASSETS_SOUNDS
    ASSETS_SPRITES
    ASSETS_PARTICLES
}
