
#include <stdlib.h>
#include <stdio.h>

#include "settings.h"
#include "../helpers/sdl_layer.h"

#include "main.h"

//#include "macros.h"
//#include "maths.h"
#include "drawing.h"
#include "text.h"

#include "audio.h"

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
            if (Ground[y_i][x_i] == T_GOAL) {
                DistanceToGoal[y_i][x_i] = 1;
            } else {
                DistanceToGoal[y_i][x_i] = 0;
            }
        }
    }

    /// Fill DistanceToGoal
    int currentDistance = 0;
    bool changedSomething = true;
    while (changedSomething) {
        changedSomething = false;
        ++currentDistance;
        inc0 (y_i,   TILES_Y) {
            inc0 (x_i,   TILES_X) {
                if (DistanceToGoal[y_i][x_i] == currentDistance) {
                    if (y_i > 0 && !DistanceToGoal[y_i - 1][x_i] && Ground[y_i - 1][x_i] == T_PATH) {
                        DistanceToGoal[y_i - 1][x_i] = currentDistance + 1;
                        changedSomething = true;
                    }

                    if (y_i < TILES_Y - 1 && !DistanceToGoal[y_i + 1][x_i] && Ground[y_i + 1][x_i] == T_PATH) {
                        DistanceToGoal[y_i + 1][x_i] = currentDistance + 1;
                        changedSomething = true;
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
    Music = LoadWav("assets\\audio\\LevelMusic.wav");
    SoundHit = LoadWav("assets\\audio\\SplatHit.wav");
    SoundDeath = LoadWav("assets\\audio\\SplatDeathReverb.wav");

    AudioClipStart(Music, true, 0.75f);

    DummyFontInfo = AcquireDebugFont();

    Cogwheels[0] = BitmapLoad("assets\\sprites\\CogwheelSmall0.bmp");
    Cogwheels[1] = BitmapLoad("assets\\sprites\\CogwheelSmall1.bmp");
    Cogwheels[2] = BitmapLoad("assets\\sprites\\CogwheelSmall2.bmp");

    MonsterSprites[0] = BitmapLoad("assets\\sprites\\MonsterUp.bmp");
    MonsterSprites[1] = BitmapLoad("assets\\sprites\\MonsterUpRight.bmp");
    MonsterSprites[2] = BitmapLoad("assets\\sprites\\MonsterRight.bmp");
    MonsterSprites[3] = BitmapLoad("assets\\sprites\\MonsterDownRight.bmp");
    MonsterSprites[4] = BitmapLoad("assets\\sprites\\MonsterDown.bmp");
    MonsterSprites[5] = BitmapLoad("assets\\sprites\\MonsterDownLeft.bmp");
    MonsterSprites[6] = BitmapLoad("assets\\sprites\\MonsterLeft.bmp");
    MonsterSprites[7] = BitmapLoad("assets\\sprites\\MonsterUpLeft.bmp");

    IconBuy     = BitmapLoad("assets\\sprites\\Buy.bmp");
    IconLevelUp = BitmapLoad("assets\\sprites\\LevelUp.bmp");
    IconMerge   = BitmapLoad("assets\\sprites\\Merge.bmp");

    FILE* file = fopen("assets/levels/dummy.lvl", "rb");
    if (file != nullptr) {
        fread(Ground, sizeof(Ground), 1, file);
        fclose(file);
    }

    InitDistanceArray();
}

void DiamondSetValues(diamond* dim, int count) {
    dim->Damage = count * DIAMOND_LEVEL_1_DAMAGE;
    dim->RangeRadius = sqrtf(1.0f + (count - 1) /100.0f)  * DIAMOND_LEVEL_1_RANGE;
    dim->MaxCooldown = (int) (powf(0.975f, (float) (count - 1)) * DIAMOND_LEVEL_1_COOLDOWN);
    dim->CooldownFrames = SOCKETING_COOLDOWN;
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

void Update(color32* array, int width, int height, inputs* ins) {

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
    drawRectMain.Width = TILES_X * GRID_SIZE;
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

    /// Handle Input
    {
        if (IS_KEY_PRESSED(F1)) {
            IsLevelEditorActive = !IsLevelEditorActive;

            if (!IsLevelEditorActive) {
                InitDistanceArray();
            }
        }

        /// "Level Editor"
        if (IsLevelEditorActive) {
            if (ins->Mouse.Left.Down) {
                int xCell = ins->Mouse.PosX / GRID_SIZE;
                int yCell = ins->Mouse.PosY / GRID_SIZE;

                if (xCell >= 0 && xCell <TILES_X && yCell >= 0 && yCell < TILES_Y) {
                    Ground[yCell][xCell] = T_PATH;
                }
            }

            if (ins->Mouse.Middle.Down) {
                int xCell = ins->Mouse.PosX / GRID_SIZE;
                int yCell = ins->Mouse.PosY / GRID_SIZE;

                if (xCell >= 0 && xCell < TILES_X && yCell >= 0 && yCell < TILES_Y) {
                    Ground[yCell][xCell] = T_GRASS;
                }
            }

            if (ins->Mouse.Right.Down && ins->Mouse.Right.Toggled) {
                int xCell = ins->Mouse.PosX / GRID_SIZE;
                int yCell = ins->Mouse.PosY / GRID_SIZE;

                if (xCell >= 0 && xCell < TILES_X && yCell >= 0 && yCell < TILES_Y) {
                    if (Ground[yCell][xCell] == T_TOWER) {
                        Ground[yCell][xCell] = T_GOAL;
                    } else {
                        Ground[yCell][xCell] = T_TOWER;
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
            /// Pathfinding and Movement
            inc0 (monster_i,   MonsterListEnd) {
                monster* monster_ = &Monsters[monster_i];
                if (!monster_->Health) { continue; }

                if (monster_->MovementT >= 1.0f) {
                    monster_->MovementT -= 1.0f;

                    int distanceToGoal = DistanceToGoal[monster_->GoalPosition.Y][monster_->GoalPosition.X];
                    if (distanceToGoal == 1) {
                        // TODO(Tobi): Cause damage to the orb

                        // NOTE(Tobi): I do that, so that projectiles etc. will not move towards it
                        ++monster_->Generation;
                        MonsterSetToStartingPosition(monster_);

                        // NOTE(Tobi): This is just what has been in the else-path
                        monster_->MovementT += monster_->Speed;
                    }

                    // NOTE(Tobi): This assumes monsters can only walk vertical/horizontal

                    int closestDistance = 9999999;
                    vec2i deltaPos = { 0, 0 };

                    if (monster_->GoalPosition.Y > 0 && DistanceToGoal[monster_->GoalPosition.Y - 1][monster_->GoalPosition.X]) {
                        int proposedDistance = DistanceToGoal[monster_->GoalPosition.Y - 1][monster_->GoalPosition.X];
                        closestDistance = proposedDistance;
                        deltaPos = vec2i { 0, -1 };
                    }

                    int sameAmount = 1;
                    
                    if (monster_->GoalPosition.X > 0 && DistanceToGoal[monster_->GoalPosition.Y][monster_->GoalPosition.X - 1]) {
                        int proposedDistance = DistanceToGoal[monster_->GoalPosition.Y][monster_->GoalPosition.X - 1];
                        if (proposedDistance < closestDistance) {
                            closestDistance = proposedDistance;
                            deltaPos = vec2i { -1, 0 };
                            sameAmount = 1;
                        } else if (proposedDistance == closestDistance) {
                            if (rand() < RAND_MAX / 2) {
                                deltaPos = vec2i { -1, 0 };
                            }
                            sameAmount = 2;
                        }
                    }
                    
                    if (monster_->GoalPosition.Y < TILES_Y - 1 && DistanceToGoal[monster_->GoalPosition.Y + 1][monster_->GoalPosition.X]) {
                        int proposedDistance = DistanceToGoal[monster_->GoalPosition.Y + 1][monster_->GoalPosition.X];
                        if (proposedDistance < closestDistance) {
                            closestDistance = proposedDistance;
                            deltaPos = vec2i { 0, +1 };
                            sameAmount = 1;
                        } else if (proposedDistance == closestDistance) {
                            ++sameAmount;
                            if (rand() < RAND_MAX / sameAmount) {
                                deltaPos = vec2i { 0, +1 };
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

                monster_->ActualPosition.X = (1 - monster_->MovementT) * monster_->OldPosition.X + monster_->MovementT * monster_->GoalPosition.X;
                monster_->ActualPosition.Y = (1 - monster_->MovementT) * monster_->OldPosition.Y + monster_->MovementT * monster_->GoalPosition.Y;
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
                        AudioClipStart(SoundDeath, false, 0.7f);
                        // TODO(Tobi): The monster has been killed; do something
                    } else {
                        AudioClipStart(SoundHit, false, 0.2f);
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
        vec2i menuTilePos = { rightMenuMousePosition.X / GRID_SIZE, rightMenuMousePosition.Y / GRID_SIZE - MENU_OFFSET_TILES_Y };
        vec2i mouseTilePos = { mainMousePosition.X / GRID_SIZE, mainMousePosition.Y / GRID_SIZE };
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
        if (IsLevelEditorActive) {
            DrawScreenRectangle(&drawRectAll, 0, 0, width, height, COL32_RGB(0, 0, 192));
        } else {
            DrawScreenRectangle(&drawRectAll, 0, 0, width, height, BLACK);
        }

        /// Render terrain
        inc0 (y_i,   TILES_Y) {
            inc0 (x_i,   TILES_X) {
                color32 col;
                switch (Ground[y_i][x_i]) {
                    case T_GRASS: {
                        col = DARK_GREEN;
                    } break;
                    case T_PATH: {
                        col = ORANGE;
                    } break;
                    case T_TOWER: {
                        col = LIGHT_GREY;
                    } break;
                    case T_GOAL: {
                        col = PURPLE;
                    } break;
                    default: {
                        col = PINK;
                    } break;
                }
                DrawBlock(&drawRectMain, x_i, y_i, col);
            }
        }

        /// Render right menu
        inc0 (y_i,   MENU_DIAMONDS_Y) {
            inc0 (x_i,   MENU_DIAMONDS_X) {
                // TODO(Tobi): I might even split the y stuff in smaller windows as well
                DrawBlock(&drawRectMenuDiamonds, x_i, y_i, DARK_GREY);
            }
        }

        /// Render Monsters
        inc0 (monster_i,   MonsterListEnd) {
            monster* monster_ = &Monsters[monster_i];
            if (!monster_->Health) { continue; }

            //DrawWorldDisc(monster_->ActualPosition.X, monster_->ActualPosition.Y, monster_->Radius, BLUE);

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

            /// Render Monster Healthbar
            if (monster_->Health < monster_->MaxHealth) {
                DrawWorldRectangle(&drawRectMain, monster_->ActualPosition.X - 0.5f, monster_->ActualPosition.Y + 0.5f, monster_->Health / monster_->MaxHealth, 1 / 6.0f, RED);
                DrawWorldRectangle(&drawRectMain, monster_->ActualPosition.X - 0.5f + monster_->Health / monster_->MaxHealth, monster_->ActualPosition.Y + 0.5f, 1.0f - monster_->Health / monster_->MaxHealth, 1 / 6.0f, BLACK);
            }
        }

        /// Render Diamonds
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
            TextRenderScreen(&drawRectWaveStones, &DummyFontInfo, 8, (startWaveFrame - MonsterWaveFrames) * MONSTER_STONE_BAR_HEIGHT / WAVE_FRAME_LENGTH + 2, numbers[wave_i], BLACK, WHITE);
        }

        /// Render Menu
        {
            DrawWorldBitmap(&drawRectRightMenu, 0, 2, IconBuy, WHITE);
            DrawWorldBitmap(&drawRectRightMenu, 1, 2, IconLevelUp, WHITE);
            DrawWorldBitmap(&drawRectRightMenu, 2, 2, IconMerge, WHITE);

            if (Menu.ShallBuy) {
                DrawWorldBorder(&drawRectRightMenu, 0 - 0.5f, 2 - 0.5f, 1.0f, 1.0f, -2.0f / GRID_SIZE, -2.0f / GRID_SIZE, YELLOW);
            }

            if (Menu.ShallLevelUp) {
                DrawWorldBorder(&drawRectRightMenu, 1 - 0.5f, 2 - 0.5f, 1.0f, 1.0f, -2.0f / GRID_SIZE, -2.0f / GRID_SIZE, YELLOW);
            }

            if (Menu.ShallMerge) {
                DrawWorldBorder(&drawRectRightMenu, 2 - 0.5f, 2 - 0.5f, 1.0f, 1.0f, -2.0f / GRID_SIZE, -2.0f / GRID_SIZE, YELLOW);
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
            TextRenderScreen(&drawRectAll, &DummyFontInfo, ins->Mouse.PosX - bitmap.Width / 2 + 27, ins->Mouse.PosY - bitmap.Height / 2 + 27, dummy, BLACK, WHITE);
        }

    }
}

void Exit() {
    FILE* file = fopen("assets/levels/dummy.lvl", "wb");
    fwrite(Ground, sizeof(Ground), 1, file);
    fclose(file);
}
