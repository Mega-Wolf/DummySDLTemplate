
#include "settings.h"
#include "../helpers/sdl_layer.h"

#include "main.h"

//#include "macros.h"
//#include "maths.h"
#include "drawing.h"

#include <stdlib.h>
#include <stdio.h>

void InitDistanceArrayAndFillTowers() {
    DiamondCount = 0;
    inc0 (y_i,   32) {
        inc0 (x_i,   32) {
            if (Ground[y_i][x_i] == T_TOWER) {
                diamond* newDiamond = &DiamondList[DiamondCount++];
                *newDiamond = {};
                newDiamond->TilePosition = vec2f { (float)x_i, (float)y_i };
                newDiamond->RangeRadius = 4.0f;
                newDiamond->MaxCooldown = 60;
            }

            if (Ground[y_i][x_i] == T_GOAL) {
                DistanceToGoal[y_i][x_i] = 1;
            } else {
                DistanceToGoal[y_i][x_i] = 0;
            }
        }
    }

    int currentDistance = 0;
    bool changedSomething = true;
    while (changedSomething) {
        changedSomething = false;
        ++currentDistance;
        inc0 (y_i,   32) {
            inc0 (x_i,   32) {
                if (DistanceToGoal[y_i][x_i] == currentDistance) {
                    if (y_i > 0 && !DistanceToGoal[y_i - 1][x_i] && Ground[y_i - 1][x_i] == T_PATH) {
                        DistanceToGoal[y_i - 1][x_i] = currentDistance + 1;
                        changedSomething = true;
                    }

                    if (y_i < 32 - 1 - 1 && !DistanceToGoal[y_i + 1][x_i] && Ground[y_i + 1][x_i] == T_PATH) {
                        DistanceToGoal[y_i + 1][x_i] = currentDistance + 1;
                        changedSomething = true;
                    }

                    if (x_i > 0 && !DistanceToGoal[y_i][x_i - 1] && Ground[y_i][x_i - 1] == T_PATH) {
                        DistanceToGoal[y_i][x_i - 1] = currentDistance + 1;
                        changedSomething = true;
                    }

                    if (x_i < 32 - 1 - 1 && !DistanceToGoal[y_i][x_i + 1] && Ground[y_i][x_i + 1] == T_PATH) {
                        DistanceToGoal[y_i][x_i + 1] = currentDistance + 1;
                        changedSomething = true;
                    }
                }
            }
        }
    }
}

void Init() {
    FILE* file = fopen("assets/levels/dummy.lvl", "rb");
    if (file != nullptr) {
        fread(Ground, sizeof(Ground), 1, file);
        fclose(file);
    }

    inc0 (x_i,   32) {
        if (Ground[0][x_i] == T_PATH) {
            StartPathX = x_i;
            StartPathY = 0;
        }

        if (Ground[32 - 1][x_i] == T_PATH) {
            StartPathX = x_i;
            StartPathY = 32 - 1;
        }
    }

    inc (y_i,   1,    32 - 1) {
        if (Ground[y_i][0] == T_PATH) {
            StartPathX = 0;
            StartPathY = y_i;
        }

        if (Ground[y_i][32 - 1] == T_PATH) {
            StartPathX = 32 - 1;
            StartPathY = y_i;
        }
    }

    InitDistanceArrayAndFillTowers();
}

void DrawWorldRectangle(float x, float y, float width, float height, color32 col) {
    DrawScreenRectangle((int) (GRID_SIZE * x) + GRID_SIZE / 2, (int) (GRID_SIZE * y) + GRID_SIZE / 2, (int) (GRID_SIZE * width), (int) (GRID_SIZE * height), col);
}

void DrawWorldDisc(float x, float y, float radius, color32 col) {
    DrawScreenDisc((int) (GRID_SIZE * x) + GRID_SIZE / 2, (int) (GRID_SIZE * y) + GRID_SIZE / 2, (int) (GRID_SIZE * radius), col);
}

void DrawWorldCircle(float x, float y, float radius, color32 col) {
    DrawScreenCircle((int) (GRID_SIZE * x) + GRID_SIZE / 2, (int) (GRID_SIZE * y) + GRID_SIZE / 2, (int) (GRID_SIZE * radius), col);
}

void DrawBlock(int x, int y, color32 col) {
    // NOTE(Tobi): I don't know whether I like this function anymore; should I just call DrawWorldRectangle?
    DrawScreenRectangle((int) (GRID_SIZE * x) + 1, (int) (GRID_SIZE * y) + 1, 32 - 2, 32 - 2, col);
}

void Update(color32* array, int width, int height, inputs* ins) {
    // TODO(Tobi): remove when using them; just here to prevent warnings
    UNUSED_PARAM(height);
    UNUSED_PARAM(ins);

    Array = array;
    ArrayWidth = width;
    ArrayHeight = height;

    // Handle Input
    {
        if (IS_KEY_PRESSED(F1)) {
            IsLevelEditorActive = !IsLevelEditorActive;

            if (!IsLevelEditorActive) {
                InitDistanceArrayAndFillTowers();
            }
        }

        // "Level Editor"
        if (IsLevelEditorActive) {
            if (ins->Mouse.Left.Down) {
                int xCell = ins->Mouse.PosX / GRID_SIZE;
                int yCell = ins->Mouse.PosY / GRID_SIZE;

                if (xCell >= 0 && xCell < 32 && yCell >= 0 && yCell < 32) {
                    Ground[yCell][xCell] = T_PATH;
                }
            }

            if (ins->Mouse.Middle.Down) {
                int xCell = ins->Mouse.PosX / GRID_SIZE;
                int yCell = ins->Mouse.PosY / GRID_SIZE;

                if (xCell >= 0 && xCell < 32 && yCell >= 0 && yCell < 32) {
                    Ground[yCell][xCell] = T_GRASS;
                }
            }

            if (ins->Mouse.Right.Down && ins->Mouse.Right.Toggled) {
                int xCell = ins->Mouse.PosX / GRID_SIZE;
                int yCell = ins->Mouse.PosY / GRID_SIZE;

                if (xCell >= 0 && xCell < 32 && yCell >= 0 && yCell < 32) {
                    if (Ground[yCell][xCell] == T_TOWER) {
                        Ground[yCell][xCell] = T_GOAL;
                    } else {
                        Ground[yCell][xCell] = T_TOWER;
                    }
                }
            }
        }
    }

    // Logic Update
    //if (IS_KEY_PRESSED(Space)) {
    if (!IsLevelEditorActive) {
        ++FrameCount;

        // Update Monsters
        {
            // Movement and pathfinding
            int firstDeadMonster = -1;
            inc0 (monster_i,   MonsterListEnd) {
                monster* monster_ = &Monsters[monster_i];
                if (!monster_->Health) {
                    if (firstDeadMonster == -1) {
                        firstDeadMonster = monster_i;
                    }
                    continue;
                }

                if (monster_->MovementT >= 1.0f) {
                    monster_->MovementT -= 1.0f;

                    int distanceToGoal = DistanceToGoal[monster_->GoalPosition.Y][monster_->GoalPosition.X];
                    if (distanceToGoal == 1) {
                        // TODO(Tobi): Reached goal; what do I do now
                        // Fow now, I just delete myself

                        monster_->Health = 0;
                        continue;
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
                    
                    if (monster_->GoalPosition.Y < 32 - 2 && DistanceToGoal[monster_->GoalPosition.Y + 1][monster_->GoalPosition.X]) {
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
                    
                    if (monster_->GoalPosition.X < 32 - 2 && DistanceToGoal[monster_->GoalPosition.Y][monster_->GoalPosition.X + 1]) {
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

            // Spawn monsters
            if (FrameCount % 30 == 0) {
                if (firstDeadMonster == -1) {
                    if (MonsterListEnd < MONSTER_COUNT_MAX) {
                        firstDeadMonster = MonsterListEnd++;
                    }
                }

                if (firstDeadMonster != -1) {
                    monster* monster_ = &Monsters[firstDeadMonster];
                    unsigned int lastGeneration = monster_->Generation;
                    *monster_ = {};
                    monster_->Generation = lastGeneration + 1;
                    monster_->MaxHealth = 50;
                    monster_->Health = monster_->MaxHealth;

                    monster_->GoalPosition.X = StartPathX;
                    monster_->GoalPosition.Y = StartPathY;
                    monster_->OldPosition.X = StartPathX;
                    monster_->OldPosition.Y = StartPathY;

                    if (monster_->OldPosition.X == 0) {
                        monster_->OldPosition.X = -1;
                    }

                    if (monster_->OldPosition.X == 32 - 1) {
                        monster_->OldPosition.X = 32;
                    }

                    if (monster_->OldPosition.Y == 0) {
                        monster_->OldPosition.Y = -1;
                    }

                    if (monster_->OldPosition.Y == 32 - 1) {
                        monster_->OldPosition.Y = 32;
                    }

                    monster_->ActualPosition.X = (float) monster_->OldPosition.X;
                    monster_->ActualPosition.Y = (float) monster_->OldPosition.Y;

                    monster_->Radius = (rand() % 6 + 5) / 32.0f;
                    monster_->Speed = 1 / (float) (rand() % 11 + 10);
                }
            }
        }

        // Update Diamonds
        inc0 (diamond_i,   DiamondCount) {
            diamond* diamond_ = &DiamondList[diamond_i];

            if (diamond_->CooldownFrames > 0) {
                --diamond_->CooldownFrames;
                continue;
            }

            int targetIndex = -1;
            int closestDistanceToGoal = 999999999;

            inc0 (monster_i,   MonsterListEnd) {
                monster* monster_ = &Monsters[monster_i];
                if (!monster_->Health) { continue; }

                float deltaX = monster_->ActualPosition.X - (float) diamond_->TilePosition.X;
                float deltaY = monster_->ActualPosition.Y - (float) diamond_->TilePosition.Y;

                float distanceSq = deltaX * deltaX + deltaY * deltaY;

                float effectiveShootingRange = diamond_->RangeRadius + monster_->Radius;
                float effectiveShootingRangeSq = effectiveShootingRange * effectiveShootingRange;
                if (distanceSq <= effectiveShootingRangeSq) {
                    // TODO(Tobi): This might break when I will use diagonal movement
                    int tileX = (int) (monster_->ActualPosition.X + 0.5f);
                    int tileY = (int) (monster_->ActualPosition.Y + 0.5f);
                    int distanceToGoal = DistanceToGoal[tileY][tileX];

                    if (distanceToGoal < closestDistanceToGoal) {
                        closestDistanceToGoal = distanceToGoal;
                        targetIndex = monster_i;
                    }
                }
            }

            if (targetIndex != -1) {
                diamond_->CooldownFrames = diamond_->MaxCooldown;

                projectile* newProjectile = &Projectiles[ProjectileCount++];
                *newProjectile = {};
                newProjectile->Position = diamond_->TilePosition;
                newProjectile->Damage = 10;
                newProjectile->Speed = 0.4f;
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

        // Update Projectiles
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
                        // TODO(Tobi): The monster has been killed; do something
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
                }
            }   
        }
    }

    // Render
    {
        if (IsLevelEditorActive) {
            DrawScreenRectangle(0, 0, width, height, COL32_RGB(0, 0, 192));
        } else {
            DrawScreenRectangle(0, 0, width, height, BLACK);
        }

        // Render terrain
        inc0 (y_i,   32) {
            inc0 (x_i,   32) {
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
                DrawBlock(x_i, y_i, col);
            }
        }

        // Render Monsters
        inc0 (monster_i,   MonsterListEnd) {
            monster* monster_ = &Monsters[monster_i];
            if (!monster_->Health) { continue; }

            DrawWorldDisc(monster_->ActualPosition.X, monster_->ActualPosition.Y, monster_->Radius, BLUE);

            // Render Monster Healthbar
            if (monster_->Health < monster_->MaxHealth) {
                DrawWorldRectangle(monster_->ActualPosition.X - 0.5f, monster_->ActualPosition.Y + 0.5f, AtMost(monster_->Health / monster_->MaxHealth, 1.0f), 1 / 6.0f, RED);
            }
        }

        // Render Diamonds
        inc0 (diamond_i,   DiamondCount) {
            diamond* diamond_ = &DiamondList[diamond_i];
            DrawWorldDisc(diamond_->TilePosition.X, diamond_->TilePosition.Y, 10 / 32.0f, COL32_RGB(40, 20, 170));

            // Render Diamond Range
            DrawWorldCircle(diamond_->TilePosition.X, diamond_->TilePosition.Y, diamond_->RangeRadius, YELLOW);

            // Render Diamond Cooldown
            if (diamond_->CooldownFrames) {
                DrawWorldRectangle(diamond_->TilePosition.X - 0.5f, diamond_->TilePosition.Y + 0.5f, diamond_->CooldownFrames / (float)diamond_->MaxCooldown, 1 / 6.0f, GREEN);
            }
        }

        // Render Projetiles
        inc0 (projectile_i,   ProjectileCount) {
            projectile* projectile_ = &Projectiles[projectile_i];
            DrawWorldDisc(projectile_->Position.X, projectile_->Position.Y, 4 / 32.0f, RED);
        }

    }
}

void Exit() {
    FILE* file = fopen("assets/levels/dummy.lvl", "wb");
    fwrite(Ground, sizeof(Ground), 1, file);
    fclose(file);
}
