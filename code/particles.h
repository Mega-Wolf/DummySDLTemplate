#pragma once

#include "opengl_drawing.h"
#include "bitmap.h"
#include "macros.h"
#include "maths.h"

struct playing_particle_effect {
    sprite_data* Sprites;
    int SpriteCount;
    
    draw_rect* DrawRect;
    float WorldX;
    float WorldY;
    color32 Color;

    int StartFrame;
};

#define MAX_PARTICLE_EFFECTS 1024

int ParticleEffectsCount;
playing_particle_effect ParticleEffects[MAX_PARTICLE_EFFECTS];

void ParticleEffectStartWorld(draw_rect* drawRect, int spriteCount, sprite_data* sprites, float x, float y, color32 color) {
    playing_particle_effect* newParticleEffect = &ParticleEffects[ParticleEffectsCount++];
    *newParticleEffect = {};
    newParticleEffect->DrawRect = drawRect;
    newParticleEffect->Sprites  = sprites;
    newParticleEffect->SpriteCount  = spriteCount;
    newParticleEffect->WorldX = x;
    newParticleEffect->WorldY = y;
    newParticleEffect->Color = color;
    newParticleEffect->StartFrame = FrameCount;
}

void ParticlesUpdate() {
    inc0 (effect_i,   ParticleEffectsCount) {
        playing_particle_effect* ppe_ = &ParticleEffects[effect_i];
        int frameInAnimation = FrameCount - ppe_->StartFrame;
        int frameForSprites = frameInAnimation / 6;
        if (frameForSprites >= ppe_->SpriteCount) {
            ParticleEffects[effect_i--] = ParticleEffects[--ParticleEffectsCount];
            continue;
        }

        DrawWorldBitmap(ppe_->DrawRect, ppe_->WorldX, ppe_->WorldY, ppe_->Sprites[frameForSprites], ppe_->Color, DEPTH_PARTICLES);
    }
}
