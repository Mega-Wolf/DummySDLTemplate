
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

    // TODO(Tobi): Not sure if that is needed at the moment
    Menu.Diamonds[MENU_DIAMONDS_Y / 2][1] = (diamond*) -1;

    //AudioClipStart(Music, true, 0.75f);

    DummyFontInfo = AcquireDebugFont();

    FILE* file = fopen("assets/levels/dummy.lvl", "rb");
    if (file != nullptr) {
        fread(Ground, sizeof(Ground), 1, file);
        fclose(file);
    }

    InitDistanceArray();
}

vec2f TriToActualPos(vec2i triPosition) {
    bool triangleIsDown = (triPosition.X + triPosition.Y) % 2;

    vec2f floatPos;

    if (triangleIsDown) {
        floatPos.Y = triPosition.Y + 1 / 3.0f;
    } else {
        floatPos.Y = triPosition.Y + 2 / 3.0f;
    }

    floatPos.X = (float) triPosition.X;

    vec2f ret = { (floatPos.X + 1) * 0.5f, floatPos.Y * HEXAGON_H };
    return ret;
}

vec2f HexToActualPos(vec2i hexPosition) {
    bool oddColumn = hexPosition.X % 2;

    if (oddColumn) {
        return { 1.0f + 1.5f * hexPosition.X, (2.0f + 2.0f * hexPosition.Y) * HEXAGON_H };
    } else {
        return { 1.0f + 1.5f * hexPosition.X, (1.0f + 2.0f * hexPosition.Y) * HEXAGON_H };
    }
}

#define TRANSLATE_NOTHING_FOUND vec2i { -1, -1 }
#define TOP_LEFT_TO_CENTRE_OFFSET vec2f { 0.5f, 1 / 3.0f * HEXAGON_H };

vec2i TranslateToTopLeftPosition(vec2i triPosition, terrain compareThing) {
    bool downTriangle = (triPosition.X + triPosition.Y) % 2;

    int x = triPosition.X;
    int y = triPosition.Y;

    if (downTriangle) {
        if (y >= 1 && Ground[y - 1][x] & compareThing) {
            return vec2i { x, y - 1 };
        } else if (x >= 1 && Ground[y][x - 1] & compareThing) {
            return vec2i { x - 1, y };
        } else if (x >= 2 && y >= 1 && Ground[y - 1][x - 2] & compareThing) {
            return vec2i { x - 2, y - 1 };
        }
    } else {
        if (Ground[y][x] & compareThing) {
            return triPosition;
        } else if (x >= 1 && y >= 1 && Ground[y - 1][x - 1] & compareThing) {
            return vec2i { x - 1, y - 1 };
        } else if (x >= 2 && Ground[y][x - 2] & compareThing) {
            return vec2i { x - 2, y };
        }
    }

    return TRANSLATE_NOTHING_FOUND;
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

    mon->ActualPosition = TriToActualPos(mon->OldPosition);
}

vec2i TranslateMousePosition(draw_rect* drawRect, inputs* ins) {
    vec2i ret;
    ret.X = ins->Mouse.PosX - drawRect->StartX;
    ret.Y = ins->Mouse.PosY - drawRect->StartY;
    return ret;
}

vec2i MouseToTilePos(vec2i mousePos, bool invert = false) {
    vec2i tilePos;
    tilePos.Y = mousePos.Y / HALF_HEXAGON_PIXEL_HEIGHT;
    bool oddLine = (invert + tilePos.Y) % 2;

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
    drawRectMenuDiamonds.Height = MENU_DIAMONDS_Y * HEXAGON_PIXEL_HEIGHT;
    vec2i menuDiamondsMousePosition = TranslateMousePosition(&drawRectMenuDiamonds, ins);

    draw_rect drawRectMenuBuild = drawRectRightMenu;
    drawRectMenuBuild.StartY = height - 10 * HALF_HEXAGON_PIXEL_HEIGHT;
    drawRectMenuBuild.Height = 10 * HALF_HEXAGON_PIXEL_HEIGHT;
    vec2i menuBuildMousePosition = TranslateMousePosition(&drawRectMenuBuild, ins);

    vec2i mouseTilePos = MouseToTilePos(mainMousePosition);

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

        if (IS_KEY_PRESSED(KEY_MERGE)) {
            Menu.ShallMerge = !Menu.ShallMerge;
        }

        if (ins->Mouse.WheelDelta > 0) {
            // TODO(Tobi): Consider mana
            ++Menu.SelectedBuyingLevel;
        } else if (ins->Mouse.WheelDelta < 0) {
            Menu.SelectedBuyingLevel = AtLeast(0, Menu.SelectedBuyingLevel - 1);
        }

        /// Wave Stone Speed
        if (IS_KEY_PRESSED(KEY_SPEED_WAVE)) {
            ++MonsterWaveSpeedEnd;
        }
    }

    diamond* diamondUnderCursor = nullptr;

    /// Logic Update
    if (!IsLevelEditorActive) {
        ++FrameCount;

        /// Update Monsters
        {
            inc_bucket (monster_i, monster_, &Monsters) {
                Assert(monster_->Health, "Monster should be dead");

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

                    vec2f actualOldPos = TriToActualPos(monster_->OldPosition);
                    vec2f actualNewPos = TriToActualPos(monster_->GoalPosition);

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
                    
                    ++monster_wave_->AlreadyReleased;

                    monster* newMonster = BucketGetCleared(&Monsters);
                    newMonster->Radius = monster_wave_->Prototype.Radius;
                    newMonster->Speed = monster_wave_->Prototype.Speed;
                    newMonster->Color = monster_wave_->Prototype.Color;
                    newMonster->MaxHealth = monster_wave_->Prototype.MaxHealth;
                    newMonster->Health = newMonster->MaxHealth;

                    MonsterSetToStartingPosition(newMonster);
                }
            }
        }

        /// Update Diamonds
        inc_bucket(diamond_i, diamond_, &Diamonds) {
            if (diamond_->CooldownFrames > 0) {
                --diamond_->CooldownFrames;
                continue;
            }

            // Right menu or being dragged
            if (!diamond_->IsInField) { continue; }

            /// Find target monster
            // TODO(Tobi): This will target monsters outside the level since there is no DistanceToGoal set there
            monster* target = nullptr;
            float closestDistanceToGoal = 999999999.0f;
            inc_bucket(monster_i, monster_, &Monsters) {
                Assert(monster_->Health > 0, "Monster should be inactive");

                float deltaX = monster_->ActualPosition.X - (float) diamond_->ActualPosition.X;
                float deltaY = monster_->ActualPosition.Y - (float) diamond_->ActualPosition.Y;

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
                        target = monster_;
                    }
                }
            }

            /// Create projectile if target monster
            if (target) {
                diamond_->CooldownFrames = diamond_->MaxCooldown;

                projectile* newProjectile = &Projectiles[ProjectileCount++];
                *newProjectile = {};
                newProjectile->Color = diamond_->MixedColor;
                newProjectile->Position = diamond_->ActualPosition;
                newProjectile->Damage = diamond_->Damage;
                newProjectile->Speed = PROJECTILE_SPEED;
                newProjectile->Target = GenerationLinkCreate(target);
                inc0 (color_i,   DC_AMOUNT) {
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

            monster* target = GenerationLinkResolve(projectile_->Target);
            if (!target) {
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
                        BucketListRemove(&Monsters, target);
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
        // TODO(Tobi): I kind of ignore all the invalid stuff, so I have to come back after doing the generation thing
        {
            // TODO(Tobi): I'm changing this now, so that I will always check what diamond is currently selected
            // For the context menu, this is something I will need anyway

            vec2i menuTilePos = MouseToTilePos(menuDiamondsMousePosition);
            if (!BoxContainsInEx(0, 0, drawRectMenuDiamonds.Width, drawRectMenuDiamonds.Height, menuDiamondsMousePosition.X, menuDiamondsMousePosition.Y)) {
                menuTilePos = { -1000, -1000 };
            }

            int diamondMenuHexX = menuTilePos.X / 3; 
            if (diamondMenuHexX == 1) {
                --menuTilePos.Y;
            }

            if (menuTilePos.Y < 0) {
                menuTilePos = { -1000, -1000 };
            }

            vec2i diamondMenuHexPos = { diamondMenuHexX, menuTilePos.Y / 2 };
            bool overValidDiamondMenuSlot = BoxContainsInEx(0, 0, 3, 4, diamondMenuHexPos.X, diamondMenuHexPos.Y) && !(diamondMenuHexPos.X == 1 && diamondMenuHexPos.Y == 3);

            vec2i buildingHexTile = TRANSLATE_NOTHING_FOUND;

            /// Find the diamond underneath the cursor
            if (BoxContainsInEx(0, 0, TILES_X, TILES_Y, mouseTilePos.X, mouseTilePos.Y)) {
                buildingHexTile = TranslateToTopLeftPosition(mouseTilePos, T_TOWER);
                if (buildingHexTile != TRANSLATE_NOTHING_FOUND) {
                    inc_bucket(dimaond_i, diamond_, &Diamonds) {
                        if (diamond_->TilePositionTopLeft == buildingHexTile) {
                            diamondUnderCursor = diamond_;
                            break;
                        }
                    }
                }
            } else if (overValidDiamondMenuSlot && Menu.Diamonds[diamondMenuHexPos.Y][diamondMenuHexPos.X]) {
                diamondUnderCursor = Menu.Diamonds[diamondMenuHexPos.Y][diamondMenuHexPos.X];
            }

            if (IS_MOUSE_PRESSED(Left)) {
                /// Buying
                {
                    vec2i buildMenuTilePos = MouseToTilePos(menuBuildMousePosition, true);

                    int buildMenuTriangleMapping[10][9] = {
                        {        -1,        -1,        -1,  DC_WHITE,  DC_WHITE,  DC_WHITE,       -1,       -1,       -1 },
                        {        -1,        -1,        -1,  DC_WHITE,  DC_WHITE,  DC_WHITE,       -1,       -1,       -1 },
                        {        -1,        -1,        -1, DC_YELLOW, DC_YELLOW, DC_YELLOW,       -1,       -1,       -1 },
                        {    DC_RED,    DC_RED,    DC_RED, DC_YELLOW, DC_YELLOW, DC_YELLOW, DC_GREEN, DC_GREEN, DC_GREEN },
                        {    DC_RED,    DC_RED,    DC_RED,        -1,        -1,        -1, DC_GREEN, DC_GREEN, DC_GREEN },
                        { DC_PURPLE, DC_PURPLE, DC_PURPLE,        -1,        -1,        -1,  DC_AQUA,  DC_AQUA,  DC_AQUA },
                        { DC_PURPLE, DC_PURPLE, DC_PURPLE,   DC_BLUE,   DC_BLUE,   DC_BLUE,  DC_AQUA,  DC_AQUA,  DC_AQUA },
                        {        -1,        -1,        -1,   DC_BLUE,   DC_BLUE,   DC_BLUE,       -1,       -1,       -1 },
                        {        -1,        -1,        -1,  DC_BLACK,  DC_BLACK,  DC_BLACK,       -1,       -1,       -1 },
                        {        -1,        -1,        -1,  DC_BLACK,  DC_BLACK,  DC_BLACK,       -1,       -1,       -1 },
                    };

                    if (buildMenuTilePos.X >= 0  && buildMenuTilePos.X < 9 && buildMenuTilePos.Y >= 0  && buildMenuTilePos.Y < 10) {
                        int buildColorIndex = buildMenuTriangleMapping[buildMenuTilePos.Y][buildMenuTilePos.X];
                        if (buildColorIndex != -1) {

                            // TODO(Tobi): Wouldn't it be easier to just store them by index
                            vec2i freeSlot = { -1, -1 };
                            inc0 (y_i,   4) {
                                inc0 (x_i,   3) {
                                    // TODO(Tobi): Do I like the filling order proposed here better?
                                    //int xs[] = { 0, 2, 1 };
                                    //int x = xs[x_i];

                                    if (!Menu.Diamonds[y_i][x_i]) {
                                        freeSlot = { x_i, y_i };
                                        goto _afterMenuSlot;
                                    }
                                }
                            }
                            _afterMenuSlot:

                            if (freeSlot.X != -1) {
                                diamond* newDiamond = BucketGetCleared(&Diamonds);

                                // TODO(Tobi): This has to be changed, otherwise I can't have more than 32 levels
                                int levelCount = 1 << Menu.SelectedBuyingLevel;
                                newDiamond->ColorsCount[buildColorIndex] = levelCount;
                                DiamondSetValues(newDiamond, levelCount);
                                newDiamond->MixedColor = DiamondColors[buildColorIndex];

                                // NOTE(Tobi): Well, this is the positon relative to the menu
                                newDiamond->TilePositionTopLeft = freeSlot;
                                newDiamond->ActualPosition = HexToActualPos(freeSlot);

                                Menu.Diamonds[freeSlot.Y][freeSlot.X] = newDiamond;

                                // TODO(Tobi): Reduce mana (I can put the check before this maybe)
                            }
                        }
                    }
                }

                /// Drag-Drop
                if (diamondUnderCursor) {
                    // TODO(Tobi): Assert that I don't hold something

                    Menu.DragDrop.Diamond = diamondUnderCursor;
                    Menu.DragDrop.WasInField = diamondUnderCursor->IsInField;
                    Menu.DragDrop.OriginTopLeft = diamondUnderCursor->TilePositionTopLeft;
                    if (diamondUnderCursor->IsInField) {
                        // TODO(Tobi): If I comment out this line, diamonds will continue shooting, since they are still kind of in their slot (do I want that)
                        Menu.DragDrop.Diamond->IsInField = false;
                    } else {
                        Menu.Diamonds[diamondMenuHexPos.Y][diamondMenuHexPos.X] = nullptr;
                    }

                    // TODO(Tobi): At the moment this would actually not matter
                    diamondUnderCursor = nullptr;
                }
            }

            if (IS_MOUSE_RELEASED(Left) && Menu.DragDrop.Diamond) {

                bool canPutOrMerge = false;

                // TODO(Tobi): If I say merge and there is nothing to merge; I can't put it down at the moment; is that what I want?

                if (Menu.ShallMerge) {
                    /// Merge diamonds

                    // TODO(Tobi): Check for mana here as well
                    if (diamondUnderCursor) {
                        canPutOrMerge = true;

                        // NOTE(Tobi): diamondUnderCursor is the one surviving
                        // With other words; the Drag-Drop diamond is the one that vanishes, since this one is not listed anywhere anymore anyway (well ,maybe still in the field, but not in the menu and in the field I care about the inactive thing) 
                        
                        int count = 0;
                        int r = 0;
                        int g = 0;
                        int b = 0;
                        inc0 (color_i,   DC_AMOUNT) {
                            diamondUnderCursor->ColorsCount[color_i] += Menu.DragDrop.Diamond->ColorsCount[color_i];
                            count += diamondUnderCursor->ColorsCount[color_i];
                            r += diamondUnderCursor->ColorsCount[color_i] * DiamondColors[color_i].Red;
                            g += diamondUnderCursor->ColorsCount[color_i] * DiamondColors[color_i].Green;
                            b += diamondUnderCursor->ColorsCount[color_i] * DiamondColors[color_i].Blue;
                        }
                        diamondUnderCursor->MixedColor = COL32_RGB(r / count, g / count, b / count );
                        DiamondSetValues(diamondUnderCursor, count);

                        diamondUnderCursor->CooldownFrames = SOCKETING_COOLDOWN;

                        BucketListRemove(&Diamonds, Menu.DragDrop.Diamond);
                    }

                    Menu.ShallMerge = false;
                } else {
                    /// Place Down and replace if necessary

                    // NOTE(Tobi): These things would be equal if dropping on top of itself again
                    if (Menu.DragDrop.Diamond != diamondUnderCursor) {

                        /// Place down
                        if (buildingHexTile != TRANSLATE_NOTHING_FOUND) {
                            Menu.DragDrop.Diamond->CooldownFrames = SOCKETING_COOLDOWN;
                            Menu.DragDrop.Diamond->TilePositionTopLeft = buildingHexTile;
                            Menu.DragDrop.Diamond->ActualPosition = TriToActualPos(buildingHexTile) + TOP_LEFT_TO_CENTRE_OFFSET;
                            Menu.DragDrop.Diamond->IsInField = true;
                            canPutOrMerge = true;
                        } else if (overValidDiamondMenuSlot) {
                            Menu.DragDrop.Diamond->TilePositionTopLeft = diamondMenuHexPos;
                            Menu.DragDrop.Diamond->ActualPosition = HexToActualPos(diamondMenuHexPos);
                            Menu.DragDrop.Diamond->IsInField = false;

                            Menu.Diamonds[diamondMenuHexPos.Y][diamondMenuHexPos.X] = Menu.DragDrop.Diamond;
                            canPutOrMerge = true;
                        }

                        /// Put the thing under cursor to where I came from
                        if (diamondUnderCursor) {
                            diamondUnderCursor->IsInField = Menu.DragDrop.WasInField;
                            diamondUnderCursor->TilePositionTopLeft = Menu.DragDrop.OriginTopLeft;
                            if (diamondUnderCursor->IsInField) {
                                diamondUnderCursor->CooldownFrames = SOCKETING_COOLDOWN;
                                diamondUnderCursor->ActualPosition = TriToActualPos(diamondUnderCursor->TilePositionTopLeft) + TOP_LEFT_TO_CENTRE_OFFSET;
                            } else {                
                                diamondUnderCursor->ActualPosition = HexToActualPos(diamondUnderCursor->TilePositionTopLeft);
                                Menu.Diamonds[diamondUnderCursor->TilePositionTopLeft.Y][diamondUnderCursor->TilePositionTopLeft.X] = diamondUnderCursor;
                            }
                        }
                    }

                }

                if (!canPutOrMerge) {
                    Assert(!diamondUnderCursor || diamondUnderCursor == Menu.DragDrop.Diamond, "How can there be a diamond under cursor, if I can't place something down");
                    // Put back to where it came from

                    Menu.DragDrop.Diamond->IsInField = Menu.DragDrop.WasInField;
                    Menu.DragDrop.Diamond->TilePositionTopLeft = Menu.DragDrop.OriginTopLeft;

                    if (Menu.DragDrop.Diamond->IsInField) {
                        Menu.DragDrop.Diamond->ActualPosition = TriToActualPos(Menu.DragDrop.Diamond->TilePositionTopLeft) + TOP_LEFT_TO_CENTRE_OFFSET;
                    } else {
                        Menu.DragDrop.Diamond->ActualPosition = HexToActualPos(Menu.DragDrop.Diamond->TilePositionTopLeft);
                        Menu.Diamonds[Menu.DragDrop.Diamond->TilePositionTopLeft.Y][Menu.DragDrop.Diamond->TilePositionTopLeft.X] = Menu.DragDrop.Diamond;
                    }
                }

                Menu.DragDrop.Diamond = nullptr;
            }

            if (IS_KEY_PRESSED(KEY_LEVEL_UP)) {
                if (diamondUnderCursor) {
                    // TODO(Tobi): Check for mana

                    // NOTE(Tobi): When I double everything; I don't have to change the colors
                    int count = 0;
                    inc0 (color_i,   DC_AMOUNT) {
                        diamondUnderCursor->ColorsCount[color_i] *= 2;
                        count += diamondUnderCursor->ColorsCount[color_i];
                    }                    
                    DiamondSetValues(diamondUnderCursor, count);
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
        {
            inc0 (y_i,   MENU_DIAMONDS_Y) {
                bool oddLine = y_i % 2;
                inc0 (x_i,   MENU_DIAMONDS_PAIRS_X + 1) {
                    if (oddLine && x_i == MENU_DIAMONDS_PAIRS_X) { break; }
                    if (oddLine) {
                        DrawScreenBitmap(&drawRectMenuDiamonds, x_i * (HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH) + HALF_HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH / 2, y_i * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Tower, DARK_GREY);
                    } else {
                        DrawScreenBitmap(&drawRectMenuDiamonds, x_i * (HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH), y_i * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Tower, DARK_GREY);
                    }
                }
            }

            // Render the stuff at the bottom
            // Hard-coded, because everythign else isn't really better

            DrawScreenRectangle(&drawRectMenuBuild, 0, 0 * HALF_HEXAGON_PIXEL_HEIGHT, 5 * HALF_HEXAGON_PIXEL_WIDTH, 5 * HALF_HEXAGON_PIXEL_HEIGHT, COL32_RGB(120, 130, 30));
            DrawScreenRectangle(&drawRectMenuBuild, 0, 5 * HALF_HEXAGON_PIXEL_HEIGHT, 5 * HALF_HEXAGON_PIXEL_WIDTH, 5 * HALF_HEXAGON_PIXEL_HEIGHT, COL32_RGB(30, 20, 90));

            // Left ones
            DrawScreenBitmap(&drawRectMenuBuild, 0, 3 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Tower, WHITE);
            DrawScreenBitmap(&drawRectMenuBuild, 0, 5 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Tower, WHITE);

            DrawScreenBitmap(&drawRectMenuBuild, 0, 3 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Cogwheels[0], RED);
            DrawScreenBitmap(&drawRectMenuBuild, 0, 5 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Cogwheels[0], PURPLE);

            // Middle ones
            DrawScreenBitmap(&drawRectMenuBuild, HALF_HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH / 2, 0 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Tower, WHITE);
            DrawScreenBitmap(&drawRectMenuBuild, HALF_HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH / 2, 2 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Tower, WHITE);
            DrawScreenBitmap(&drawRectMenuBuild, HALF_HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH / 2, 6 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Tower, WHITE);
            DrawScreenBitmap(&drawRectMenuBuild, HALF_HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH / 2, 8 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Tower, WHITE);

            DrawScreenBitmap(&drawRectMenuBuild, HALF_HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH / 2, 0 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Cogwheels[0], WHITE);
            DrawScreenBitmap(&drawRectMenuBuild, HALF_HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH / 2, 2 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Cogwheels[0], YELLOW);
            DrawScreenBitmap(&drawRectMenuBuild, HALF_HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH / 2, 6 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Cogwheels[0], BLUE);
            DrawScreenBitmap(&drawRectMenuBuild, HALF_HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH / 2, 8 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Cogwheels[0], DARK_GREY);

            // Right ones
            DrawScreenBitmap(&drawRectMenuBuild, HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH, 3 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Tower, WHITE);
            DrawScreenBitmap(&drawRectMenuBuild, HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH, 5 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Tower, WHITE);

            DrawScreenBitmap(&drawRectMenuBuild, HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH, 3 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Cogwheels[0], GREEN);
            DrawScreenBitmap(&drawRectMenuBuild, HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH, 5 * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.Cogwheels[0], AQUA);

            // Level buy number in the middle
            char dummy[10];
            snprintf(dummy, ArrayCount(dummy), "%d", Menu.SelectedBuyingLevel + 1);
            int textWidth = TextGetRenderSize(&DummyFontInfo, dummy);
            DrawScreenDisc(&drawRectMenuBuild, HALF_HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH / 2 + (HEXAGON_PIXEL_WIDTH) / 2, 5 * HALF_HEXAGON_PIXEL_HEIGHT, 20, DARK_GREY);
            TextRenderScreen(&drawRectMenuBuild, &DummyFontInfo, HALF_HEXAGON_PIXEL_WIDTH + HALF_HEXAGON_PIXEL_WIDTH / 2 + (HEXAGON_PIXEL_WIDTH - textWidth) / 2, 5 * HALF_HEXAGON_PIXEL_HEIGHT - DummyFontInfo.FontSize / 2, dummy, WHITE);
        }

        /// Render Monsters
        inc_bucket (monster_i, monster_, &Monsters) {

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
        inc_bucket (diamond_i, diamond_, &Diamonds) {
            // NOTE(Tobi): Will I really kep it like that?

            if (diamond_ == Menu.DragDrop.Diamond) { continue; }

            int frame;
            draw_rect* drawRect;
            if (diamond_->IsInField) {
                frame = FrameCount % 30 / 10;
                drawRect = &drawRectMain;
            } else {
                frame = 0;
                drawRect = &drawRectMenuDiamonds;
            }
            DrawWorldBitmap(drawRect, diamond_->ActualPosition.X, diamond_->ActualPosition.Y, Sprites.Cogwheels[frame], diamond_->MixedColor);
            //  COL32_RGB(40, 20, 170)
            
            int count = 0;
            inc0 (color_i,   DC_AMOUNT) {
                count += diamond_->ColorsCount[color_i];
            }
            char dummy[5];
            snprintf(dummy, 5, "%d", count);
            TextRenderWorld(drawRect, &DummyFontInfo, diamond_->ActualPosition.X + 0.475f, diamond_->ActualPosition.Y + 0.225f, dummy, WHITE);

            if (diamond_->IsInField) {
                /// Render Diamond Range
                DrawWorldCircle(&drawRectMain, diamond_->ActualPosition.X, diamond_->ActualPosition.Y, diamond_->RangeRadius, YELLOW);

                /// Render Diamond Cooldown
                if (diamond_->CooldownFrames) {
                    DrawWorldRectangle(&drawRectMain, diamond_->ActualPosition.X - 0.5f, diamond_->ActualPosition.Y + 0.5f, diamond_->CooldownFrames / (float)diamond_->MaxCooldown, 1 / 6.0f, GREEN);
                }
            }
        }

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

        /// Render Top Menu
        {
            DrawWorldBitmap(&drawRectRightMenu, 4.5f, 2, Sprites.IconMerge, WHITE);

            if (Menu.ShallMerge) {
                DrawWorldCircle(&drawRectRightMenu, 4.5f, 2, 1, YELLOW);
            }
        }

        /// Render drag-drop diamond
        if (Menu.DragDrop.Diamond) {
            loaded_bitmap bitmap = Sprites.Cogwheels[0];

            DrawScreenBitmap(&drawRectAll, ins->Mouse.PosX - bitmap.Width / 2, ins->Mouse.PosY - bitmap.Height / 2, bitmap, Menu.DragDrop.Diamond->MixedColor);

            int count = 0;
            inc0 (color_i,   DC_AMOUNT) {
                count += Menu.DragDrop.Diamond->ColorsCount[color_i];
            }

            char dummy[5];
            snprintf(dummy, 5, "%d", count);
            TextRenderScreen(&drawRectAll, &DummyFontInfo,RoundFloat32ToInt32(ins->Mouse.PosX + 0.475f * HEXAGON_A), RoundFloat32ToInt32(ins->Mouse.PosY + 0.225f * HEXAGON_A), dummy, WHITE);
        }

        /// Render mouse-over highlight
        // TODO(Tobi): Replace with context menu
        if (diamondUnderCursor && Menu.DragDrop.Diamond != diamondUnderCursor) {
            if (diamondUnderCursor->IsInField) {
                DrawWorldCircle(&drawRectMain, diamondUnderCursor->ActualPosition.X, diamondUnderCursor->ActualPosition.Y, 1.0f, RED);
            } else {
                DrawWorldCircle(&drawRectMenuDiamonds, diamondUnderCursor->ActualPosition.X, diamondUnderCursor->ActualPosition.Y, 1.0f, BLUE);
            }
        }

        /// Draw testing lines
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
