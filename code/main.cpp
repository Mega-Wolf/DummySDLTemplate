#include "settings.h"
#include "macros.h"
#include "maths.h"

#include <stdlib.h>
#include <stdio.h>

#include "../helpers/sdl_layer.h"

// Global Variables
color32* Array;
int ArrayWidth;
int ArrayHeight;

int FrameCount;

enum terrain {
    T_GRASS,
    T_PATH,
    T_TOWER,
};

terrain Ground[32][32];

bool IsLevelEditorActive;

int StartPathX = -1;
int StartPathY = -1;

struct monster {
    vec2i GoalPosition;
    vec2i OldPosition;
    int Radius;
    float Speed;

    float MovementT;
    vec2f ActualPosition;
};

int SpawnedMonsterAmount;
monster Monsters[10];

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

    inc0 (y_i,   1,    32 - 1) {
        if (Ground[y_i][0] == T_PATH) {
            StartPathX = 0;
            StartPathY = y_i;
        }

        if (Ground[y_i][32 - 1] == T_PATH) {
            StartPathX = 32 - 1;
            StartPathY = y_i;
        }
    }
}

void DrawRectangle(int x, int y, int width, int height, color32 col) {
    inc (y_i,   AtLeast(y, 0),    y_i < AtMost(y + height, ArrayHeight)) {
        inc (x_i,   AtLeast(x, 0),    x_i < AtMost(x + width, ArrayWidth)) {
            Array[Index2D(x_i, y_i, ArrayWidth)] = col;
        }
    }
}

void DrawCircle(int x, int y, int radius, color32 col) {
    inc (y_i,   AtLeast(y - radius, 0),    y_i <= AtMost(y + radius, ArrayHeight)) {
        inc (x_i,   AtLeast(x - radius, 0),    x_i <= AtMost(x + radius, ArrayWidth)) {
            float diffX = (float) (x_i - x);
            float diffY = (float) (y_i - y);

            if (diffX * diffX + diffY * diffY <= radius * radius) {
                Array[Index2D(x_i, y_i, ArrayWidth)] = col;
            }
        }
    }
}

void DrawBlock(int x, int y, color32 col) {
    DrawRectangle(x * GRID_SIZE + 1, y * GRID_SIZE + 1, GRID_SIZE - 2, GRID_SIZE - 2, col);
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

            if (ins->Mouse.Right.Down) {
                int xCell = ins->Mouse.PosX / GRID_SIZE;
                int yCell = ins->Mouse.PosY / GRID_SIZE;

                if (xCell >= 0 && xCell < 32 && yCell >= 0 && yCell < 32) {
                    Ground[yCell][xCell] = T_TOWER;
                }
            }
        }
    }

    // Logic Update
    //if (IS_KEY_PRESSED(Space)) {
    {
        ++FrameCount;

        // Update Monsters
        {
            inc0 (monster_i,   SpawnedMonsterAmount) {
                monster* monster_ = &Monsters[monster_i];

                if (monster_->MovementT >= 1.0f) {
                    monster_->MovementT -= 1.0f;

                    int deltaX = 0;
                    int deltaY = 0;

                    // NOTE(Tobi): This assumes monsters can only walk vertical/horizontal

                    if (monster_->GoalPosition.Y > 0 && Ground[monster_->GoalPosition.Y - 1][monster_->GoalPosition.X] == T_PATH && monster_->GoalPosition.Y - 1 != monster_->OldPosition.Y) {
                        deltaY = -1;
                    } else if (monster_->GoalPosition.X > 0 && Ground[monster_->GoalPosition.Y][monster_->GoalPosition.X - 1] == T_PATH && monster_->GoalPosition.X - 1 != monster_->OldPosition.X) {
                        deltaX = -1;
                    } else if (monster_->GoalPosition.Y < 32 - 2 && Ground[monster_->GoalPosition.Y + 1][monster_->GoalPosition.X] == T_PATH && monster_->GoalPosition.Y + 1 != monster_->OldPosition.Y) {
                        deltaY = +1;
                    } else if (monster_->GoalPosition.X < 32 - 2 && Ground[monster_->GoalPosition.Y][monster_->GoalPosition.X + 1] == T_PATH && monster_->GoalPosition.X + 1 != monster_->OldPosition.X) {
                        deltaX = +1;
                    }

                    if (deltaX == 0 && deltaY == 0) {
                        // TODO(Tobi): Monster reached goal
                    }

                    monster_->OldPosition.X = monster_->GoalPosition.X;
                    monster_->OldPosition.Y = monster_->GoalPosition.Y;
                    monster_->GoalPosition.X += deltaX;
                    monster_->GoalPosition.Y += deltaY;
                } else {
                    monster_->MovementT += monster_->Speed;
                }

                monster_->ActualPosition.X = (1 - monster_->MovementT) * monster_->OldPosition.X + monster_->MovementT * monster_->GoalPosition.X;
                monster_->ActualPosition.Y = (1 - monster_->MovementT) * monster_->OldPosition.Y + monster_->MovementT * monster_->GoalPosition.Y;
            }

            if (SpawnedMonsterAmount < 10 && FrameCount % 30 == 0) {
                monster* monster_ = &Monsters[SpawnedMonsterAmount];
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

                monster_->Radius = rand() % 6 + 5;
                monster_->Speed = 1 / (float) (rand() % 11 + 10);
                ++SpawnedMonsterAmount;
            }
        }
    }

    // Render
    {
        if (IsLevelEditorActive) {
            DrawRectangle(0, 0, width, height, COL32_RGB(0, 0, 192));
        } else {
            DrawRectangle(0, 0, width, height, BLACK);
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
                    default: {
                        col = PINK;
                    } break;
                }
                DrawBlock(x_i, y_i, col);
            }
        }

        // Render Monsters
        inc0 (monster_i,   SpawnedMonsterAmount) {
            monster* monster_ = &Monsters[monster_i];
            DrawCircle((int) (GRID_SIZE * monster_->ActualPosition.X) + GRID_SIZE / 2, (int) (GRID_SIZE * monster_->ActualPosition.Y) + GRID_SIZE / 2, monster_->Radius, BLUE);
        }
    }
}

void Exit() {
    FILE* file = fopen("assets/levels/dummy.lvl", "wb");
    fwrite(Ground, sizeof(Ground), 1, file);
    fclose(file);
}
