
#include <stdlib.h>
#include <stdio.h>

#include "assets.h"

#include "settings.h"
#include "../helpers/sdl_layer.h"

#include "main.h"

//#include "macros.h"
//#include "maths.h"
#include "drawing.h"
#include "text.h"

#include "audio.h"
#include "particles.h"

font_info DummyFontInfo;

void InitDistanceArray() {

    /// Find start positions
    StartPositionsCount = 0;
    inc0 (x_i,   TILES_X) {
        if (Ground[0][x_i] == T_PATH) {
            StartPositions[StartPositionsCount].X = x_i;
            StartPositions[StartPositionsCount].Y = 0;
            ++StartPositionsCount;
        }

        if (Ground[TILES_Y - 1][x_i] == T_PATH) {
            StartPositions[StartPositionsCount].X = x_i;
            StartPositions[StartPositionsCount].Y = TILES_Y - 1;
            ++StartPositionsCount;
        }
    }

    inc (y_i,   1,    TILES_Y - 1) {
        if (Ground[y_i][0] == T_PATH) {
            StartPositions[StartPositionsCount].X = 0;
            StartPositions[StartPositionsCount].Y = y_i;
            ++StartPositionsCount;
        }

        if (Ground[y_i][TILES_X - 1] == T_PATH) {
            StartPositions[StartPositionsCount].X = TILES_X - 1;
            StartPositions[StartPositionsCount].Y = y_i;
            ++StartPositionsCount;
        }
    }

    /// Reset DistanceToGoal
    inc0 (y_i,   TILES_Y) {
        inc0 (x_i,   TILES_X) {
            DistanceToGoal[y_i][x_i] = 0;
        }
    }

    /// Set DistanceToGoal at the goal fields
    inc0 (y_i,   TILES_Y) {
        inc0 (x_i,   TILES_X) {
            bool triangleIsDown = (x_i + y_i) % 2;
            if (triangleIsDown) { continue; }

            if (Ground[y_i][x_i] == T_GOAL) {
                DistanceToGoal[y_i    ][x_i    ] = 1;
                DistanceToGoal[y_i    ][x_i + 1] = 1;
                DistanceToGoal[y_i    ][x_i + 2] = 1;
                DistanceToGoal[y_i + 1][x_i    ] = 1;
                DistanceToGoal[y_i + 1][x_i + 1] = 1;
                DistanceToGoal[y_i + 1][x_i + 2] = 1;
            }
        }
    }

    /// Fill DistanceToGoal
    // TODO(Tobi): I am not sure how good using ints is here
    int currentDistance = 0;
    bool changedSomething = true;
    while (changedSomething) {
        changedSomething = false;
        ++currentDistance;
        inc0 (y_i,   TILES_Y) {
            inc0 (x_i,   TILES_X) {
                bool triangleIsDown = (x_i + y_i) % 2;

                if (DistanceToGoal[y_i][x_i] == currentDistance) {
                    if (triangleIsDown) {
                        if (y_i > 0 && !DistanceToGoal[y_i - 1][x_i] && Ground[y_i - 1][x_i] == T_PATH) {
                            DistanceToGoal[y_i - 1][x_i] = currentDistance + 1;
                            changedSomething = true;
                        }
                    } else {
                        if (y_i < TILES_Y - 1 && !DistanceToGoal[y_i + 1][x_i] && Ground[y_i + 1][x_i] == T_PATH) {
                            DistanceToGoal[y_i + 1][x_i] = currentDistance + 1;
                            changedSomething = true;
                        }
                    }

                    if (x_i > 0 && !DistanceToGoal[y_i][x_i - 1] && Ground[y_i][x_i - 1] == T_PATH) {
                        DistanceToGoal[y_i][x_i - 1] = currentDistance + 1;
                        changedSomething = true;
                    }

                    if (x_i < TILES_X - 1 && !DistanceToGoal[y_i][x_i + 1] && Ground[y_i][x_i + 1] == T_PATH) {
                        DistanceToGoal[y_i][x_i + 1] = currentDistance + 1;
                        changedSomething = true;
                    }
                }
            }
        }
    }
}

void Init() {
    AssetsInit();

    //AudioClipStart(Music, true, 0.75f);

    DummyFontInfo = AcquireDebugFont();

    FILE* file = fopen("assets/levels/dummy.lvl", "rb");
    if (file != nullptr) {
        fread(Ground, sizeof(Ground), 1, file);
        fclose(file);
    }

    InitDistanceArray();
}

void MonsterSetToStartingPosition(monster* mon) {
    int startPositionIndex = rand() % StartPositionsCount;
    mon->GoalPosition = StartPositions[startPositionIndex];
    mon->OldPosition = mon->GoalPosition;

    if (mon->OldPosition.X == 0) {
        mon->OldPosition.X = -1;
    }

    if (mon->OldPosition.X == TILES_X - 1) {
        mon->OldPosition.X = TILES_X;
    }

    if (mon->OldPosition.Y == 0) {
        mon->OldPosition.Y = -1;
    }

    if (mon->OldPosition.Y == TILES_Y - 1) {
        mon->OldPosition.Y = TILES_Y;
    }

    mon->ActualPosition.X = (float) mon->OldPosition.X;
    mon->ActualPosition.Y = (float) mon->OldPosition.Y;
}

vec2i TranslateMousePosition(draw_rect* drawRect, inputs* ins) {
    vec2i ret;
    ret.X = ins->Mouse.PosX - drawRect->StartX;
    ret.Y = ins->Mouse.PosY - drawRect->StartY;
    return ret;
}

vec2f HexToActualPos(vec2i hexPosition) {
    bool triangleIsDown = (hexPosition.X + hexPosition.Y) % 2;
    bool oddLine = hexPosition.Y % 2;

    vec2f floatPos;

    if (triangleIsDown) {
        floatPos.Y = hexPosition.Y + 1 / 3.0f;
    } else {
        floatPos.Y = hexPosition.Y + 2 / 3.0f;
    }

    floatPos.X = (float) hexPosition.X;

    #define SQRT_3 1.73205080757f
    #define HEXAGON_H (SQRT_3 / 2)
    vec2f ret = { (floatPos.X + 1) * 0.5f, floatPos.Y * HEXAGON_H };
    return ret;
}

vec2i MouseToTilePos(vec2i mousePos) {
    vec2i tilePos;
    tilePos.Y = mousePos.Y / HALF_HEXAGON_PIXEL_HEIGHT;
    bool oddLine = tilePos.Y % 2;

    int evenTileIndex = mousePos.X / HALF_HEXAGON_PIXEL_WIDTH;
    int oddTileIndex  = (mousePos.X + (HALF_HEXAGON_PIXEL_WIDTH / 2)) / HALF_HEXAGON_PIXEL_WIDTH;

    int deltaY = mousePos.Y - HALF_HEXAGON_PIXEL_HEIGHT * tilePos.Y;
    float yRelative = deltaY / (float) HALF_HEXAGON_PIXEL_HEIGHT;

    if (oddLine) {
        // NOTE(Tobi): The even tile index is down

        if (evenTileIndex == oddTileIndex) {
            // NOTE(Tobi): Up - Down
            int deltaX = mousePos.X - HALF_HEXAGON_PIXEL_WIDTH * oddTileIndex;
            float xRelative = (float)deltaX / (HALF_HEXAGON_PIXEL_WIDTH / 2);
            bool isOnRight = xRelative > yRelative;
            tilePos.X = 2 * evenTileIndex + isOnRight - 1;
        } else {
            // NOTE(Tobi): Down - Up
            int deltaX = mousePos.X + (HALF_HEXAGON_PIXEL_WIDTH / 2) - HALF_HEXAGON_PIXEL_WIDTH * oddTileIndex;
            float xRelative = (float)deltaX / ((HALF_HEXAGON_PIXEL_WIDTH + 1) / 2);
            bool isOnRight = xRelative > 1 - yRelative;
            tilePos.X = 2 * evenTileIndex + isOnRight;
        }
    } else {
        // NOTE(Tobi): The even tile index is up

        if (evenTileIndex == oddTileIndex) {
            // NOTE(Tobi): Down - Up
            int deltaX = mousePos.X - HALF_HEXAGON_PIXEL_WIDTH * oddTileIndex;
            float xRelative = (float)deltaX / ((HALF_HEXAGON_PIXEL_WIDTH + 1) / 2);
            bool isOnRight = xRelative > 1 - yRelative;
            tilePos.X = 2 * evenTileIndex + isOnRight - 1;
        } else {
            // NOTE(Tobi): Up - Down
            int deltaX = mousePos.X + (HALF_HEXAGON_PIXEL_WIDTH / 2) - HALF_HEXAGON_PIXEL_WIDTH * oddTileIndex;
            float xRelative = (float)deltaX / ((HALF_HEXAGON_PIXEL_WIDTH) / 2);
            bool isOnRight = xRelative > yRelative;
            tilePos.X = 2 * evenTileIndex + isOnRight;
        }
    }

    return tilePos;
}

void Update(color32* array, int width, int height, inputs* ins) {

    if (ShakeFrames > 0) {
        --ShakeFrames;
        Camera.X = rand() % 9 - 4;
        Camera.Y = rand() % 9 - 4;
    } else {
        Camera.X = 0;
        Camera.Y = 0;
    }

    draw_rect drawRectAll = {};
    drawRectAll.ArrayData = array;
    drawRectAll.ArrayWidth = width;
    drawRectAll.Width = width;
    drawRectAll.Height = height;
    drawRectAll.StartX = 0;
    drawRectAll.StartY = 0;

    draw_rect drawRectWaveStones = {};
    drawRectWaveStones.ArrayData = array;
    drawRectWaveStones.ArrayWidth = width;
    drawRectWaveStones.Width = MONSTER_STONE_BAR_WIDTH;
    drawRectWaveStones.Height = height;
    drawRectWaveStones.StartX = 0;
    drawRectWaveStones.StartY = 0;

    draw_rect drawRectMain = {};
    drawRectMain.ArrayData = array;
    drawRectMain.ArrayWidth = width;
    drawRectMain.Width = (TRIANGLE_PAIRS_X + 1) * HALF_HEXAGON_PIXEL_WIDTH;
    drawRectMain.Height = height;
    drawRectMain.StartX = drawRectWaveStones.Width;
    drawRectMain.StartY = 0;
    vec2i mainMousePosition = TranslateMousePosition(&drawRectMain, ins);

    draw_rect drawRectRightMenu = {};
    drawRectRightMenu.ArrayData = array;
    drawRectRightMenu.ArrayWidth = width;
    drawRectRightMenu.Width = MENU_DIAMONDS_X * GRID_SIZE;
    drawRectRightMenu.Height = height;
    drawRectRightMenu.StartX = drawRectMain.StartX + drawRectMain.Width;
    drawRectRightMenu.StartY = 0;
    vec2i rightMenuMousePosition = TranslateMousePosition(&drawRectRightMenu, ins);

    draw_rect drawRectMenuDiamonds = drawRectRightMenu;
    drawRectMenuDiamonds.StartY = MENU_OFFSET_Y;
    drawRectMenuDiamonds.Height = MENU_DIAMONDS_Y * GRID_SIZE;
    vec2i menuDiamondsMousePosition = TranslateMousePosition(&drawRectMenuDiamonds, ins);

    vec2i mouseTilePos = MouseToTilePos(mainMousePosition);
    vec2i menuTilePos = MouseToTilePos(menuDiamondsMousePosition);

    /// Handle Input
    {
        if (IS_KEY_PRESSED(KEY_TOGGLE_EDITOR)) {
            IsLevelEditorActive = !IsLevelEditorActive;

            if (!IsLevelEditorActive) {
                InitDistanceArray();
            }
        }

        if (IS_KEY_PRESSED(KEY_TOGGLE_SHOW_PATHFINDING)) {
            ShowPathfinding = !ShowPathfinding;
        }

        /// "Level Editor"
        if (IsLevelEditorActive) {
            if (ins->Mouse.Left.Down) {
                if (mouseTilePos.X >= 0 && mouseTilePos.X <TILES_X && mouseTilePos.Y >= 0 && mouseTilePos.Y < TILES_Y) {
                    Ground[mouseTilePos.Y][mouseTilePos.X] = T_PATH;
                }
            }

            if (ins->Mouse.Middle.Down) {
                if (mouseTilePos.X >= 0 && mouseTilePos.X < TILES_X && mouseTilePos.Y >= 0 && mouseTilePos.Y < TILES_Y) {
                    Ground[mouseTilePos.Y][mouseTilePos.X] = T_GRASS;
                }
            }

            if (ins->Mouse.Right.Down && ins->Mouse.Right.Toggled) {
                bool triangleIsDown = (mouseTilePos.X + mouseTilePos.Y) % 2;
                if (!triangleIsDown && mouseTilePos.X >= 0 && mouseTilePos.X < TILES_X && mouseTilePos.Y >= 0 && mouseTilePos.Y < TILES_Y) {
                    if (Ground[mouseTilePos.Y][mouseTilePos.X] == T_TOWER) {
                        Ground[mouseTilePos.Y][mouseTilePos.X] = T_GOAL;
                    } else {
                        Ground[mouseTilePos.Y][mouseTilePos.X] = T_TOWER;
                    }
                }
            }
        }

        /// Buy / Level up / Merge etc.
        if (IS_KEY_PRESSED(KEY_BUY)) {
            Menu.ShallBuy = !Menu.ShallBuy;
            if (Menu.ShallBuy) {
                Menu.ShallLevelUp = false;
                Menu.ShallMerge = false;
            }
        }
        if (IS_KEY_PRESSED(KEY_LEVEL_UP)) {
            Menu.ShallLevelUp = !Menu.ShallLevelUp;
            if (Menu.ShallLevelUp) {
                Menu.ShallBuy = false;
                Menu.ShallMerge = false;
            }
        }
        if (IS_KEY_PRESSED(KEY_MERGE)) {
            Menu.ShallMerge = !Menu.ShallMerge;
            if (Menu.ShallMerge) {
                Menu.ShallBuy = false;
                Menu.ShallLevelUp = false;
            }
        }

        /// Wave Stone Speed
        if (IS_KEY_PRESSED(KEY_SPEED_WAVE)) {
            ++MonsterWaveSpeedEnd;
        }
    }

    /// Logic Update
    if (!IsLevelEditorActive) {
        ++FrameCount;

        /// Update Monsters
        {
            inc0 (monster_i,   MonsterListEnd) {
                monster* monster_ = &Monsters[monster_i];
                if (!monster_->Health) { continue; }

                /// React to effects
                {
                    if (monster_->PoisonSpeed) {
                        monster_->Health -= monster_->PoisonSpeed;
                        monster_->PoisonSpeed = AtLeast(monster_->PoisonSpeed - MONSTER_POISON_DECREASE_PER_FRAME, 0.0f);
                    }
                }

                /// Pathfinding and Movement
                {
                    if (monster_->MovementT >= 1.0f) {
                        monster_->MovementT -= 1.0f;

                        int distanceToGoal = DistanceToGoal[monster_->GoalPosition.Y][monster_->GoalPosition.X];
                        if (distanceToGoal == 1) {
                            // TODO(Tobi): Cause damage to the orb
                            ShakeFrames = 15;

                            // NOTE(Tobi): I do that, so that projectiles etc. will not move towards it
                            ++monster_->Generation;
                            MonsterSetToStartingPosition(monster_);

                            // NOTE(Tobi): This is just what has been in the else-path
                            monster_->MovementT += monster_->Speed;
                        }

                        bool triangleIsDown = (monster_->GoalPosition.X + monster_->GoalPosition.Y) % 2;
                        int closestDistance = 9999999;
                        vec2i deltaPos = { 0, 0 };

                        if (triangleIsDown) {
                            if (monster_->GoalPosition.Y > 0 && DistanceToGoal[monster_->GoalPosition.Y - 1][monster_->GoalPosition.X]) {
                                int proposedDistance = DistanceToGoal[monster_->GoalPosition.Y - 1][monster_->GoalPosition.X];
                                closestDistance = proposedDistance;
                                deltaPos = vec2i { 0, -1 };
                            }
                        } else {
                            if (monster_->GoalPosition.Y < TILES_Y - 1 && DistanceToGoal[monster_->GoalPosition.Y + 1][monster_->GoalPosition.X]) {
                                int proposedDistance = DistanceToGoal[monster_->GoalPosition.Y + 1][monster_->GoalPosition.X];
                                closestDistance = proposedDistance;
                                deltaPos = vec2i { 0, +1 };
                            }
                        }

                        int sameAmount = 1;
                        
                        if (monster_->GoalPosition.X > 0 && DistanceToGoal[monster_->GoalPosition.Y][monster_->GoalPosition.X - 1]) {
                            int proposedDistance = DistanceToGoal[monster_->GoalPosition.Y][monster_->GoalPosition.X - 1];
                            if (proposedDistance < closestDistance) {
                                closestDistance = proposedDistance;
                                deltaPos = vec2i { -1, 0 };
                                sameAmount = 1;
                            } else if (proposedDistance == closestDistance) {
                                ++sameAmount;
                                if (rand() < RAND_MAX / sameAmount) {
                                    deltaPos = vec2i { -1, 0 };
                                }
                            }
                        }
                        
                        if (monster_->GoalPosition.X < TILES_X - 1 && DistanceToGoal[monster_->GoalPosition.Y][monster_->GoalPosition.X + 1]) {
                            int proposedDistance = DistanceToGoal[monster_->GoalPosition.Y][monster_->GoalPosition.X + 1];
                            if (proposedDistance < closestDistance) {
                                closestDistance = proposedDistance;
                                deltaPos = vec2i { +1, 0 };
                                sameAmount = 1;
                            } else if (proposedDistance == closestDistance) {
                                ++sameAmount;
                                if (rand() < RAND_MAX / sameAmount) {
                                    deltaPos = vec2i { +1, 0 };
                                }
                            }
                        }

                        monster_->OldPosition.X = monster_->GoalPosition.X;
                        monster_->OldPosition.Y = monster_->GoalPosition.Y;
                        monster_->GoalPosition.X += deltaPos.X;
                        monster_->GoalPosition.Y += deltaPos.Y;
                    } else {
                        monster_->MovementT += monster_->Speed;
                    }

                    // TODO(Tobi): That whole thing will probably move away from lerping movement anyway at one point

                    vec2f actualOldPos = HexToActualPos(monster_->OldPosition);
                    vec2f actualNewPos = HexToActualPos(monster_->GoalPosition);

                    monster_->ActualPosition.X = (1 - monster_->MovementT) * actualOldPos.X + monster_->MovementT * actualNewPos.X;
                    monster_->ActualPosition.Y = (1 - monster_->MovementT) * actualOldPos.Y + monster_->MovementT * actualNewPos.Y;
                }
            }

            /// Update Monster Waves
            {
                int nextWave = MonsterWaveFrames / WAVE_FRAME_LENGTH + 1;

                if (MonsterWaveSpeedEnd >= nextWave) {
                    MonsterWaveFrames += MONSTER_WAVE_FAST_SPEED;
                } else {
                    ++MonsterWaveFrames;
                }

                inc0 (wave_i,   WAVE_COUNT) {
                    monster_wave* monster_wave_ = &MonsterWaves[wave_i];

                    int startWaveFrame = WAVE_FRAME_LENGTH * wave_i;
                    if (startWaveFrame > MonsterWaveFrames) { continue; }

                    if (!monster_wave_->ActualStartFrame) {
                        // This is now the last started wave
                        monster_wave_->ActualStartFrame = FrameCount;

                        // NOTE(Tobi): By this, I keep the MonsterWaveSpeedEnd always at a known value, so I can later just increase it
                        if (MonsterWaveSpeedEnd < wave_i) {
                            MonsterWaveSpeedEnd = wave_i;
                        }
                    }

                    if (monster_wave_->AlreadyReleased == monster_wave_->FullCount) { continue; }

                    // TODO(Tobi): When the timeline runs faster, monsters spawn with the same speed; is that what I want?
                    int timeOffsetFromStart = FrameCount - monster_wave_->ActualStartFrame;
                    if (timeOffsetFromStart % monster_wave_->DelayFrames != 0) {continue; }
                    
                    monster* newMonster = nullptr;
                    inc0 (monster_i,   MonsterListEnd) {
                        monster* monster_ = &Monsters[monster_i];
                        if (!monster_->Health) {
                            newMonster = monster_;
                            break;
                        }
                    }

                    if (!newMonster) {
                        if (MonsterListEnd < MONSTER_COUNT_MAX) {
                            newMonster = &Monsters[MonsterListEnd++];
                        }
                    }

                    if (newMonster) {
                        ++monster_wave_->AlreadyReleased;
                        unsigned int lastGeneration = newMonster->Generation;
                        *newMonster = {};
                        newMonster->Generation = lastGeneration + 1;
                        newMonster->Radius = monster_wave_->Prototype.Radius;
                        newMonster->Speed = monster_wave_->Prototype.Speed;
                        newMonster->Color = monster_wave_->Prototype.Color;
                        newMonster->MaxHealth = monster_wave_->Prototype.MaxHealth;
                        newMonster->Health = newMonster->MaxHealth;

                        MonsterSetToStartingPosition(newMonster);
                    }
                }
            }
        }

        /// Update Diamonds
        inc0 (diamond_i,   DiamondCount) {
            diamond* diamond_ = &DiamondList[diamond_i];
            if (diamond_->Inactive) { continue; }

            if (diamond_->CooldownFrames > 0) {
                --diamond_->CooldownFrames;
                continue;
            }

            // Right menu or being dragged
            if (!diamond_->IsInField) { continue; }

            /// Find target monster
            // TODO(Tobi): This will target monsters outside the level since there is no DistanceToGoal set there
            int targetIndex = -1;
            float closestDistanceToGoal = 999999999.0f;
            inc0 (monster_i,   MonsterListEnd) {
                monster* monster_ = &Monsters[monster_i];
                if (!monster_->Health) { continue; }

                float deltaX = monster_->ActualPosition.X - (float) diamond_->TilePosition.X;
                float deltaY = monster_->ActualPosition.Y - (float) diamond_->TilePosition.Y;

                float distanceSq = deltaX * deltaX + deltaY * deltaY;

                float effectiveShootingRange = diamond_->RangeRadius + monster_->Radius;
                float effectiveShootingRangeSq = effectiveShootingRange * effectiveShootingRange;
                if (distanceSq <= effectiveShootingRangeSq) {
                    int newDistanceToGoal = DistanceToGoal[monster_->GoalPosition.Y][monster_->GoalPosition.X];
                    int oldDistanceToGoal;
                    if (monster_->OldPosition.X == -1 || monster_->OldPosition.X == TILES_X || monster_->OldPosition.Y == -1 || monster_->OldPosition.Y == TILES_Y) {
                        oldDistanceToGoal = newDistanceToGoal + 1;
                    } else {
                        oldDistanceToGoal = DistanceToGoal[monster_->OldPosition.Y][monster_->OldPosition.X];
                    }

                    float t = monster_->MovementT;
                    float distanceToGoal = t * newDistanceToGoal + (1 - t) *  oldDistanceToGoal;

                    if (distanceToGoal < closestDistanceToGoal) {
                        closestDistanceToGoal = distanceToGoal;
                        targetIndex = monster_i;
                    }
                }
            }

            /// Create projectile if target monster
            if (targetIndex != -1) {
                diamond_->CooldownFrames = diamond_->MaxCooldown;

                projectile* newProjectile = &Projectiles[ProjectileCount++];
                *newProjectile = {};
                newProjectile->Color = diamond_->MixedColor;
                newProjectile->Position = diamond_->TilePosition;
                newProjectile->Damage = diamond_->Damage;
                newProjectile->Speed = PROJECTILE_SPEED;
                newProjectile->Target = &Monsters[targetIndex];
                newProjectile->TargetGeneration = newProjectile->Target->Generation;
                inc0 (color_i,   6) {
                    newProjectile->ColorsCount[color_i] = diamond_->ColorsCount[color_i];
                }

                float randomAngle = (rand() / (float)(RAND_MAX + 1)) * 2 * PI;
                vec2f delta;
                delta.X = sinf(randomAngle);
                delta.Y = cosf(randomAngle);

                newProjectile->BuildupDelta.X = delta.X * (newProjectile->Speed / 8);
                newProjectile->BuildupDelta.Y = delta.Y * (newProjectile->Speed / 8);
                newProjectile->BuildupFrames = 15;
            }
        }

        /// Update Projectiles
        inc0 (projectile_i,   ProjectileCount) {
            projectile* projectile_ = &Projectiles[projectile_i];

            monster* target = projectile_->Target;
            if (!target->Health || target->Generation != projectile_->TargetGeneration) {
                Projectiles[projectile_i] = Projectiles[--ProjectileCount];
                --projectile_i;
                continue;
            }

            if (projectile_->BuildupFrames > 0) {
                --projectile_->BuildupFrames;

                projectile_->Position.X += projectile_->BuildupDelta.X;
                projectile_->Position.Y += projectile_->BuildupDelta.Y;

                projectile_->Direction = projectile_->BuildupDelta;
            } else {
                vec2f delta;
                delta.X = target->ActualPosition.X - projectile_->Position.X;
                delta.Y = target->ActualPosition.Y - projectile_->Position.Y;

                float distanceSq = delta.X * delta.X + delta.Y * delta.Y;

                float speedSq = projectile_->Speed * projectile_->Speed;

                // TODO(Tobi): This only checks midpoint-midpoint; not edge-edge
                if (distanceSq < speedSq) {
                    target->Health -= projectile_->Damage;
                    if (target->Health <= 0) {
                        target->Health = 0;
                        AudioClipStart(Sounds.Death, false, 0.7f);
                        ParticleEffectStartWorld(&drawRectMain, 16, Particles.Smoke, target->ActualPosition.X, target->ActualPosition.Y, COL32_RGBA(100, 80, 80, 160));
                        // TODO(Tobi): The monster has been killed; do something
                    } else {
                        AudioClipStart(Sounds.Hit, false, 0.2f);

                        /// Assign effects
                        target->PoisonSpeed = sqrtf(1.0f + (projectile_->ColorsCount[DC_GREEN] - 1) /100.0f) * DIAMOND_LEVEL_1_POISON;
                    }

                    Projectiles[projectile_i] = Projectiles[--ProjectileCount];
                    --projectile_i;
                    continue;
                } else {
                    float distance = sqrtf(distanceSq);
                    delta.X /= distance;
                    delta.Y /= distance;
                    delta.X *= projectile_->Speed;
                    delta.Y *= projectile_->Speed;

                    projectile_->Position.X += delta.X;
                    projectile_->Position.Y += delta.Y;

                    projectile_->Direction = delta;
                }
            }
        }

        /// Menu logic
        // TODO(Tobi): This whole menu logic is really ugly; I hope that will change at one point
        {
            bool shallDrop = false;

            if (ins->Mouse.Left.Down) {
                if (ins->Mouse.Left.Toggled) {
                    /// Clicked

                    bool didSomething = false;
                    inc0 (diamond_i,   DiamondCount) {
                        diamond* diamond_ = &DiamondList[diamond_i];
                        if (diamond_->Inactive) { continue; }

                        bool underCursor = false;
                        if (diamond_->IsInField) {
                            // Game
                            if ((int)diamond_->TilePosition.X == mouseTilePos.X && (int)diamond_->TilePosition.Y == mouseTilePos.Y) {
                                underCursor = true;
                            }
                        } else {
                            // Menu
                            if ((int)diamond_->TilePosition.X == menuTilePos.X && (int)diamond_->TilePosition.Y == menuTilePos.Y) {
                                underCursor = true;
                            }
                        }

                        if (underCursor) {
                            if (Menu.ShallLevelUp) {
                                Menu.ShallLevelUp = false;
                                // Double all the color counts
                                int count = 0;
                                inc0 (color_i,   6) {
                                    diamond_->ColorsCount[color_i] *= 2;
                                    count += diamond_->ColorsCount[color_i];
                                }
                                DiamondSetValues(diamond_, count);

                                if (diamond_->IsInField) {
                                    diamond_->CooldownFrames = SOCKETING_COOLDOWN;
                                }
                            } else {
                                /// Pick up

                                Menu.DragDrop.Diamond = diamond_;
                                Menu.DragDrop.Origin = diamond_->IsInField ? mouseTilePos : menuTilePos;
                                Menu.DragDrop.WasInField = diamond_->IsInField;

                                diamond_->TilePosition = DRAG_DROP_POSITION;
                                diamond_->IsInField = false;
                            }

                            didSomething = true;
                            break;
                        }
                    }

                    if (!didSomething && Menu.ShallBuy) {
                        bool isInGame = mouseTilePos.X < TILES_X && mouseTilePos.Y < TILES_Y;
                        bool isInMenu = menuTilePos.X >= 0 && menuTilePos.X < MENU_DIAMONDS_X && menuTilePos.Y >= 0 && menuTilePos.Y < MENU_DIAMONDS_Y;

                        if (isInMenu || (isInGame && Ground[mouseTilePos.Y][mouseTilePos.X] == T_TOWER)) {
                            diamond* newDiamond = &DiamondList[DiamondCount++];
                            *newDiamond = {};
                            newDiamond->TilePosition = isInGame ? vec2f { (float)mouseTilePos.X, (float)mouseTilePos.Y } : vec2f { (float)menuTilePos.X, (float)menuTilePos.Y };
                            newDiamond->Damage = DIAMOND_LEVEL_1_DAMAGE;
                            newDiamond->RangeRadius = DIAMOND_LEVEL_1_RANGE;
                            newDiamond->MaxCooldown = DIAMOND_LEVEL_1_COOLDOWN;

                            int randomColor = rand() % 6;
                            newDiamond->ColorsCount[randomColor] = 1;
                            newDiamond->MixedColor = DiamondColors[randomColor];

                            newDiamond->IsInField = isInGame;
                        }
                    }

                    Menu.ShallBuy = false;
                }
            } else {
                // true if Released
                shallDrop = ins->Mouse.Left.Toggled;
            }

            if (shallDrop) {
                if (Menu.DragDrop.Diamond) {

                    // Game and Menu are almost identical
                    bool canDropHere = false;
                    bool dropInField = false;
                    if (mouseTilePos.X < TILES_X && mouseTilePos.Y < TILES_Y) {
                        /// Game
                        if (Ground[mouseTilePos.Y][mouseTilePos.X] == T_TOWER) {
                            canDropHere = true;
                            dropInField = true;
                        }
                    } else {
                        /// Menu
                        if (menuTilePos.X >= 0 && menuTilePos.X < MENU_DIAMONDS_X && menuTilePos.Y >= 0 && menuTilePos.Y < MENU_DIAMONDS_Y) {
                            canDropHere = true;
                        }
                    }

                    // NOTE(Tobi): I only check that for inGame, because it doesn't matter for the menu
                    bool sameTileAsGrabbed = Menu.DragDrop.Origin.X == mouseTilePos.X && (int) Menu.DragDrop.Origin.Y == mouseTilePos.Y;
                    if (sameTileAsGrabbed) {
                        canDropHere = false;
                    }

                    if (canDropHere) {
                        diamond* exchangeDiamond = nullptr;
                        inc0(diamond_i,   DiamondCount) {
                            diamond* diamond_ = &DiamondList[diamond_i];
                            if (diamond_->Inactive) { continue; }

                            if (diamond_->IsInField) {
                                if ((int) diamond_->TilePosition.X == mouseTilePos.X && (int) diamond_->TilePosition.Y == mouseTilePos.Y) {
                                    exchangeDiamond = diamond_;
                                    break;
                                }
                            } else {
                                if ((int) diamond_->TilePosition.X == menuTilePos.X && (int) diamond_->TilePosition.Y == menuTilePos.Y) {
                                    exchangeDiamond = diamond_;
                                    break;
                                }
                            }
                        }

                        if (exchangeDiamond) {
                            if (Menu.ShallMerge) {
                                /// Merge diamonds

                                /// Merge colors
                                int red = 0;
                                int green = 0;
                                int blue = 0;
                                int count = 0;
                                inc0 (color_i,   6) {
                                    exchangeDiamond->ColorsCount[color_i] += Menu.DragDrop.Diamond->ColorsCount[color_i];
                                    red += exchangeDiamond->ColorsCount[color_i] * DiamondColors[color_i].Red;
                                    green += exchangeDiamond->ColorsCount[color_i] * DiamondColors[color_i].Green;
                                    blue += exchangeDiamond->ColorsCount[color_i] * DiamondColors[color_i].Blue;
                                    count += exchangeDiamond->ColorsCount[color_i];
                                }
                                exchangeDiamond->MixedColor = COL32_RGB(red / count, green / count, blue / count);

                                DiamondSetValues(exchangeDiamond, count);

                                Menu.DragDrop.Diamond->Inactive = true;
                            } else {
                                /// Actually exchange
                                exchangeDiamond->TilePosition = vec2f { (float) Menu.DragDrop.Origin.X, (float) Menu.DragDrop.Origin.Y };
                                exchangeDiamond->IsInField = Menu.DragDrop.WasInField;
                                if (exchangeDiamond->IsInField) {
                                    exchangeDiamond->CooldownFrames = SOCKETING_COOLDOWN;
                                }
                            }
                        }

                        Menu.DragDrop.Diamond->TilePosition = dropInField ? vec2f { (float) mouseTilePos.X, (float) mouseTilePos.Y } : vec2f { (float) menuTilePos.X, (float) menuTilePos.Y };
                        Menu.DragDrop.Diamond->CooldownFrames = SOCKETING_COOLDOWN;
                        Menu.DragDrop.Diamond->IsInField = dropInField;
                    }

                    if (!canDropHere) {
                        /// Move back to where it was
                        Menu.DragDrop.Diamond->TilePosition = vec2f { (float)Menu.DragDrop.Origin.X, (float) Menu.DragDrop.Origin.Y };
                        Menu.DragDrop.Diamond->IsInField = Menu.DragDrop.WasInField;
                    }

                    Menu.DragDrop.Diamond = nullptr;

                    Menu.ShallBuy = false;
                    Menu.ShallMerge = false;
                    Menu.ShallLevelUp = false;
                }
            }
        }
    }

    /// Rendering
    {
        /// Clear screen
        if (IsLevelEditorActive) {
            DrawScreenRectangle(&drawRectAll, 0, 0, width, height, COL32_RGB(0, 0, 192));
        } else {
            DrawScreenRectangle(&drawRectAll, 0, 0, width, height, BLACK);
        }

        /// Render Grass / Path
        inc0 (y_i,   TILES_Y) {
            int evenLineOffset = 1 - (y_i % 2);
            inc0 (x_i,   TILES_X) {
                bool triangleIsDown = (x_i + y_i) % 2;
                if (ShowPathfinding) {
                    color32 col;
                    if (!DistanceToGoal[y_i][x_i]) {
                        col = WHITE;
                    } else {
                        int r = AtMost(5 * DistanceToGoal[y_i][x_i], 255);
                        int g = Clamp(5 * DistanceToGoal[y_i][x_i] - 255, 0, 255);
                        int b = 0;
                        col = COL32_RGB(r, g, b);
                    }
                    if (triangleIsDown) {
                        DrawScreenBitmap(&drawRectMain, x_i * HALF_HEXAGON_PIXEL_WIDTH / 2, y_i * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.WhiteDown, col);
                    } else {
                        DrawScreenBitmap(&drawRectMain, x_i * HALF_HEXAGON_PIXEL_WIDTH / 2 - evenLineOffset, y_i * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.WhiteUp, col);
                    }
                } else {
                    if (!(Ground[y_i][x_i] & (T_PATH | T_GOAL))) {
                        // Grass
                        if (triangleIsDown) {
                            DrawScreenBitmap(&drawRectMain, x_i * HALF_HEXAGON_PIXEL_WIDTH / 2, y_i * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.GrassDown, WHITE);
                        } else {
                            DrawScreenBitmap(&drawRectMain, x_i * HALF_HEXAGON_PIXEL_WIDTH / 2 - evenLineOffset, y_i * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.GrassUp, WHITE);
                        }
                    } else {
                        // Path
                        int pathBMPIndex = 0;

                        if (triangleIsDown) {
                            if (y_i == 0 || Ground[y_i - 1][x_i] & (T_PATH | T_GOAL)) {
                                pathBMPIndex += TRI_DOWN_TOP;
                            }
                            if (x_i == 0 || Ground[y_i][x_i - 1] & (T_PATH | T_GOAL)) {
                                pathBMPIndex += TRI_DOWN_LEFT;
                            }
                            if (x_i == TILES_X - 1 || Ground[y_i][x_i + 1] & (T_PATH | T_GOAL)) {
                                pathBMPIndex += TRI_DOWN_RIGHT;
                            }

                            DrawScreenBitmap(&drawRectMain, x_i * HALF_HEXAGON_PIXEL_WIDTH / 2, y_i * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.GrassDown, WHITE);
                            DrawScreenBitmap(&drawRectMain, x_i * HALF_HEXAGON_PIXEL_WIDTH / 2, y_i * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.PathDown[pathBMPIndex], WHITE);
                        } else {
                            if (y_i == TILES_Y - 1 || Ground[y_i + 1][x_i] & (T_PATH | T_GOAL)) {
                                pathBMPIndex += TRI_UP_BOTTOM;
                            }
                            if (x_i == 0 || Ground[y_i][x_i - 1] & (T_PATH | T_GOAL)) {
                                pathBMPIndex += TRI_UP_LEFT;
                            }
                            if (x_i == TILES_X - 1 || Ground[y_i][x_i + 1] & (T_PATH | T_GOAL)) {
                                pathBMPIndex += TRI_UP_RIGHT;
                            }

                            DrawScreenBitmap(&drawRectMain, x_i * HALF_HEXAGON_PIXEL_WIDTH / 2 - evenLineOffset, y_i * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.GrassUp, WHITE);
                            DrawScreenBitmap(&drawRectMain, x_i * HALF_HEXAGON_PIXEL_WIDTH / 2 - evenLineOffset, y_i * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.PathUp[pathBMPIndex], WHITE);
                        }
                    }
                }
            }
        }

        /// Render buildings
        inc0 (y_i,   TILES_Y) {
            int oddLineOffset = y_i % 2;
            int y = y_i * HALF_HEXAGON_PIXEL_HEIGHT;
            inc0 (x_i,   TILES_X) {
                bool triangleIsDown = (x_i + y_i) % 2;
                if (triangleIsDown) { continue; }

                int x = x_i / 2 * HALF_HEXAGON_PIXEL_WIDTH + oddLineOffset * (HALF_HEXAGON_PIXEL_WIDTH / 2);
                loaded_bitmap* buildingBitmap = nullptr;
                color32 buildingColor = WHITE;
                switch (Ground[y_i][x_i]) {
                    case T_TOWER: {
                        buildingBitmap = &Sprites.Tower;
                    } break;
                    case T_GOAL: {
                        buildingBitmap = &Sprites.Goal;
                        buildingColor = RED;
                    } break;
                    case T_TRAP: {
                        buildingBitmap = &Sprites.Trap;
                    } break;
                }

                if (buildingBitmap) {
                    DrawScreenBitmap(&drawRectMain, x, y, *buildingBitmap, buildingColor);
                }
            }
        }

        /// Draw lines in edit mode
        #if 0
        if (IsLevelEditorActive) {
            inc0 (x_i,   TILES_X - 1) {
                DrawScreenRectangle(&drawRectMain, GRID_SIZE * (x_i + 1), 0, 1, drawRectMain.Height, BLUE);
            }

            inc0 (y_i,   TILES_Y - 1) {
                DrawScreenRectangle(&drawRectMain, 0, GRID_SIZE * (y_i + 1), drawRectMain.Width, 1, BLUE);
            }
        }
        #endif

        /// Render right menu
        inc0 (y_i,   MENU_DIAMONDS_Y) {
            inc0 (x_i,   MENU_DIAMONDS_X) {
                DrawBlock(&drawRectMenuDiamonds, x_i, y_i, DARK_GREY);
            }
        }

        /// Render Monsters
        inc0 (monster_i,   MonsterListEnd) {
            monster* monster_ = &Monsters[monster_i];
            if (!monster_->Health) { continue; }

            #if 1
                // TODO(Tobi): I need new sprites otherwise
                // TODO(Tobi): Do I want to have hex pos and actual pos separated
                DrawWorldDisc(&drawRectMain, monster_->ActualPosition.X, monster_->ActualPosition.Y, monster_->Radius, BLUE);
            #else
                int directionInt = 0;
                {
                    //vec2i deltaPosition = monster_->GoalPosition - monster_->OldPosition;
                    int deltaX = monster_->GoalPosition.X - monster_->OldPosition.X;
                    int deltaY = monster_->GoalPosition.Y - monster_->OldPosition.Y;
                    if (deltaY == -1) {
                        directionInt = 0;
                    } else if (deltaX == +1) {
                        directionInt = 2;
                    } else if (deltaY == +1) {
                        directionInt = 4;
                    } else if (deltaX == -1) {
                        directionInt = 6;
                    }
                }

                DrawWorldBitmap(&drawRectMain, monster_->ActualPosition.X, monster_->ActualPosition.Y, MonsterSprites[directionInt], monster_->Color);
            #endif

            /// Render Monster Healthbar
            if (monster_->Health < monster_->MaxHealth) {
                DrawWorldRectangle(&drawRectMain, monster_->ActualPosition.X - 0.5f, monster_->ActualPosition.Y + 0.5f, monster_->Health / monster_->MaxHealth, 1 / 6.0f, RED);
                DrawWorldRectangle(&drawRectMain, monster_->ActualPosition.X - 0.5f + monster_->Health / monster_->MaxHealth, monster_->ActualPosition.Y + 0.5f, 1.0f - monster_->Health / monster_->MaxHealth, 1 / 6.0f, BLACK);
            }
        }

        /// Render Diamonds
        #if 0
        inc0 (diamond_i,   DiamondCount) {
            diamond* diamond_ = &DiamondList[diamond_i];
            if (diamond_->Inactive) { continue; }

            int frame;
            draw_rect* drawRect;
            if (diamond_->IsInField) {
                frame = FrameCount % 30 / 10;
                drawRect = &drawRectMain;
            } else {
                frame = 0;
                drawRect = &drawRectMenuDiamonds;
            }
            DrawWorldBitmap(drawRect, diamond_->TilePosition.X, diamond_->TilePosition.Y, Cogwheels[frame], diamond_->MixedColor);
            //  COL32_RGB(40, 20, 170)
            
            int count = 0;
            inc0 (color_i,   6) {
                count += diamond_->ColorsCount[color_i];
            }
            char dummy[5];
            snprintf(dummy, 5, "%d", count);
            TextRenderWorld(drawRect, &DummyFontInfo, diamond_->TilePosition.X + 0.2f, diamond_->TilePosition.Y + 0.2f, dummy, BLACK, WHITE);

            if (diamond_->IsInField) {
                /// Render Diamond Range
                DrawWorldCircle(&drawRectMain, diamond_->TilePosition.X, diamond_->TilePosition.Y, diamond_->RangeRadius, YELLOW);

                /// Render Diamond Cooldown
                if (diamond_->CooldownFrames) {
                    DrawWorldRectangle(&drawRectMain, diamond_->TilePosition.X - 0.5f, diamond_->TilePosition.Y + 0.5f, diamond_->CooldownFrames / (float)diamond_->MaxCooldown, 1 / 6.0f, GREEN);
                }

            }
        }
        #endif

        /// Render Projectiles
        inc0 (projectile_i,   ProjectileCount) {
            projectile* projectile_ = &Projectiles[projectile_i];

            // Drawing the projectile a bit oval; super not worth it; don't do in the series
            vec2f direction = projectile_->Direction;
            float length = sqrtf(direction.X * direction.X + direction.Y * direction.Y);
            direction.X /= length;
            direction.Y /= length;
            direction.X /= (float) GRID_SIZE;
            direction.Y /= (float) GRID_SIZE;
            DrawWorldDisc(&drawRectMain, projectile_->Position.X - direction.X, projectile_->Position.Y - direction.Y, 4 / (float)GRID_SIZE, projectile_->Color);
            DrawWorldDisc(&drawRectMain, projectile_->Position.X + direction.X, projectile_->Position.Y + direction.Y, 4 / (float)GRID_SIZE, projectile_->Color);
            DrawWorldDisc(&drawRectMain, projectile_->Position.X - direction.X, projectile_->Position.Y - direction.Y, 2 / (float)GRID_SIZE, WHITE);
            DrawWorldDisc(&drawRectMain, projectile_->Position.X + direction.X, projectile_->Position.Y + direction.Y, 2 / (float)GRID_SIZE, WHITE);
        }

        ParticlesUpdate();

        /// Render Monster Stones
        inc0 (wave_i,   WAVE_COUNT) {
            int startWaveFrame = WAVE_FRAME_LENGTH * wave_i;
            if (startWaveFrame < MonsterWaveFrames) { continue; }

            DrawScreenRectangle(&drawRectWaveStones, 2, (startWaveFrame - MonsterWaveFrames) * MONSTER_STONE_BAR_HEIGHT / WAVE_FRAME_LENGTH + 2, MONSTER_STONE_BAR_WIDTH - 4, MONSTER_STONE_BAR_HEIGHT - 4, MonsterWaves[wave_i].Prototype.Color);

            color32 borderColor = wave_i <= MonsterWaveSpeedEnd ? RED : LIGHT_GREY;
            DrawScreenBorder(&drawRectWaveStones, 0, (startWaveFrame - MonsterWaveFrames) * MONSTER_STONE_BAR_HEIGHT / WAVE_FRAME_LENGTH, MONSTER_STONE_BAR_WIDTH, MONSTER_STONE_BAR_HEIGHT, -2, -2, borderColor);

            char* numbers[] = {
                "1", "2", "3", "4", "5", "6", "7", "8", "9"
            };
            TextRenderScreen(&drawRectWaveStones, &DummyFontInfo, 8, (startWaveFrame - MonsterWaveFrames) * MONSTER_STONE_BAR_HEIGHT / WAVE_FRAME_LENGTH + 2, numbers[wave_i], BLACK);
        }

        /// Render Menu
        {
            DrawWorldBitmap(&drawRectRightMenu, 0.5f, 2, Sprites.IconBuy, WHITE);
            DrawWorldBitmap(&drawRectRightMenu, 2.5f, 2, Sprites.IconLevelUp, WHITE);
            DrawWorldBitmap(&drawRectRightMenu, 4.5f, 2, Sprites.IconMerge, WHITE);

            if (Menu.ShallBuy) {
                //DrawWorldBorder(&drawRectRightMenu, 0.5f - 1.0f, 2 - 1.0f, 2.0f, 2.0f, -2.0f / GRID_SIZE, -2.0f / GRID_SIZE, YELLOW);
                DrawWorldCircle(&drawRectRightMenu, 0.5f, 2, 1, YELLOW);
            }

            if (Menu.ShallLevelUp) {
                DrawWorldCircle(&drawRectRightMenu, 2.5f, 2, 1, YELLOW);
            }

            if (Menu.ShallMerge) {
                DrawWorldCircle(&drawRectRightMenu, 4.5f, 2, 1, YELLOW);
            }
        }

        /// Render drag-drop diamond
        if (Menu.DragDrop.Diamond) {
            loaded_bitmap bitmap = Cogwheels[0];

            DrawScreenBitmap(&drawRectAll, ins->Mouse.PosX - bitmap.Width / 2, ins->Mouse.PosY - bitmap.Height / 2, bitmap, Menu.DragDrop.Diamond->MixedColor);

            int count = 0;
            inc0 (color_i,   6) {
                count += Menu.DragDrop.Diamond->ColorsCount[color_i];
            }

            char dummy[5];
            snprintf(dummy, 5, "%d", count);
            TextRenderScreen(&drawRectAll, &DummyFontInfo, ins->Mouse.PosX - bitmap.Width / 2 + HALF_HEXAGON_PIXEL_HEIGHT, ins->Mouse.PosY - bitmap.Height / 2 + HALF_HEXAGON_PIXEL_HEIGHT, dummy, BLACK);
        }

        #if 0

        #define SQRT_3 1.73205080757f
        #define HEXAGON_HEIGHT 54.0f
        #define HALF_HEXAGON_HEIGHT (HEXAGON_HEIGHT / 2)
        #define HEXAGON_A (HALF_HEXAGON_HEIGHT * 2 / SQRT_3)

        inc0 (y_i,   TRIANGLE_LINES_Y) {
            DrawScreenLine(&drawRectMain, 0, HALF_HEXAGON_HEIGHT * y_i, drawRectMain.Width, HALF_HEXAGON_HEIGHT * y_i, RED);
        }

        // Top Right -> Bottom Left
        inc0 (x_i,   TRIANGLE_PAIRS_X + 1 + TRIANGLE_LINE_PAIRS_Y) {
            DrawScreenLine(&drawRectMain, HEXAGON_A / 2 + HEXAGON_A * x_i, 0, -(TRIANGLE_LINES_Y - 1) * HEXAGON_A / 2 + HEXAGON_A * x_i, TRIANGLE_LINES_Y * HALF_HEXAGON_HEIGHT /*drawRectMain.Height*/, RED);
        }

        // Top Left -> Bottom Right
        inc (x_i,   -TRIANGLE_LINE_PAIRS_Y,   TRIANGLE_PAIRS_X + 1) {
            DrawScreenLine(&drawRectMain, HEXAGON_A / 2 + HEXAGON_A * x_i, 0, (TRIANGLE_LINES_Y + 1) * HEXAGON_A / 2 + HEXAGON_A * x_i, TRIANGLE_LINES_Y * HALF_HEXAGON_HEIGHT /*drawRectMain.Height*/, RED);
        }

        DrawScreenLine(&drawRectMain, 0, TRIANGLE_LINES_Y * HALF_HEXAGON_HEIGHT, (1 + TRIANGLE_PAIRS_X) * HEXAGON_A, TRIANGLE_LINES_Y * HALF_HEXAGON_HEIGHT, BLACK);
        DrawScreenLine(&drawRectMain, (1 + TRIANGLE_PAIRS_X) * HEXAGON_A, 0, (1 + TRIANGLE_PAIRS_X) * HEXAGON_A, TRIANGLE_LINES_Y * HALF_HEXAGON_HEIGHT, BLACK);

        DrawScreenBitmap(&drawRectMain, 100, 100 - HALF_HEXAGON_PIXEL_HEIGHT, Sprites.PathUp[0], WHITE);
        //DrawScreenBitmap(&drawRectMain, 100, 100 - HALF_HEXAGON_PIXEL_HEIGHT, Sprites.PathUp[TRI_UP_RIGHT + TRI_UP_BOTTOM], WHITE);
        DrawScreenBitmap(&drawRectMain, 100 + HALF_HEXAGON_PIXEL_WIDTH / 2 + 1, 100 - HALF_HEXAGON_PIXEL_HEIGHT, Sprites.PathDown[0], WHITE);
        //DrawScreenBitmap(&drawRectMain, 100 + HALF_HEXAGON_PIXEL_WIDTH / 2 + 1, 100 - HALF_HEXAGON_PIXEL_HEIGHT, Sprites.PathDown[TRI_DOWN_LEFT + TRI_DOWN_RIGHT], WHITE);
        DrawScreenBitmap(&drawRectMain, 100 + HALF_HEXAGON_PIXEL_WIDTH, 100 - HALF_HEXAGON_PIXEL_HEIGHT, Sprites.PathUp[0], WHITE);
        //DrawScreenBitmap(&drawRectMain, 100 + HALF_HEXAGON_PIXEL_WIDTH, 100 - HALF_HEXAGON_PIXEL_HEIGHT, Sprites.PathUp[TRI_UP_LEFT + TRI_UP_BOTTOM], WHITE);

        DrawScreenBitmap(&drawRectMain, 100, 100, Sprites.PathDown[0], WHITE);
        //DrawScreenBitmap(&drawRectMain, 100, 100, Sprites.PathDown[TRI_DOWN_RIGHT + TRI_DOWN_TOP], WHITE);
        DrawScreenBitmap(&drawRectMain, 100 + HALF_HEXAGON_PIXEL_WIDTH / 2, 100, Sprites.PathUp[0], WHITE);
        //DrawScreenBitmap(&drawRectMain, 100 + HALF_HEXAGON_PIXEL_WIDTH / 2, 100, Sprites.PathUp[TRI_UP_LEFT + TRI_UP_RIGHT], WHITE);
        DrawScreenBitmap(&drawRectMain, 100 + HALF_HEXAGON_PIXEL_WIDTH, 100, Sprites.PathDown[0], WHITE);
        //DrawScreenBitmap(&drawRectMain, 100 + HALF_HEXAGON_PIXEL_WIDTH, 100, Sprites.PathDown[TRI_DOWN_LEFT + TRI_DOWN_TOP], WHITE);
        #endif

    }
}

void Exit() {
    FILE* file = fopen("assets/levels/dummy.lvl", "wb");
    fwrite(Ground, sizeof(Ground), 1, file);
    fclose(file);
}
