
#include <stdlib.h>
#include <stdio.h>

#include "settings.h"
#include "../helpers/sdl_layer.h"

#include "assets.h"

#include "main.h"
#include "intrinsics.h"

//#include "macros.h"
//#include "maths.h"
#include "drawing.h"
#include "text.h"

#include "audio.h"
#include "particles.h"

font_info DummyFontInfo;

void CollectStartPositions() {
    StartPositionsCount = 0;

    inc0 (x_i,   TILES_X) {
        if (Ground[0][x_i] & T_PATH) {
            StartPositions[StartPositionsCount].X = x_i;
            StartPositions[StartPositionsCount].Y = 0;
            ++StartPositionsCount;
        }

        if (Ground[TILES_Y - 1][x_i] & T_PATH) {
            StartPositions[StartPositionsCount].X = x_i;
            StartPositions[StartPositionsCount].Y = TILES_Y - 1;
            ++StartPositionsCount;
        }
    }

    inc (y_i,   1,    TILES_Y - 1) {
        if (Ground[y_i][0] & T_PATH) {
            StartPositions[StartPositionsCount].X = 0;
            StartPositions[StartPositionsCount].Y = y_i;
            ++StartPositionsCount;
        }

        if (Ground[y_i][TILES_X - 1] & T_PATH) {
            StartPositions[StartPositionsCount].X = TILES_X - 1;
            StartPositions[StartPositionsCount].Y = y_i;
            ++StartPositionsCount;
        }
    }
}

void InitDistanceArray() {
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

            if (Ground[y_i][x_i] & T_GOAL) {
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
                        terrain ground = Ground[y_i - 1][x_i];
                        if (y_i > 0 && !DistanceToGoal[y_i - 1][x_i] && ((ground & T_PATH) && !(ground & T_WALL))) {
                            DistanceToGoal[y_i - 1][x_i] = currentDistance + 1;
                            changedSomething = true;
                        }
                    } else {
                        terrain ground = Ground[y_i + 1][x_i];
                        if (y_i < TILES_Y - 1 && !DistanceToGoal[y_i + 1][x_i] && ((ground & T_PATH) && !(ground & T_WALL))) {
                            DistanceToGoal[y_i + 1][x_i] = currentDistance + 1;
                            changedSomething = true;
                        }
                    }

                    terrain groundLeft = Ground[y_i][x_i - 1];
                    if (x_i > 0 && !DistanceToGoal[y_i][x_i - 1] && ((groundLeft & T_PATH) && !(groundLeft & T_WALL))) {
                        DistanceToGoal[y_i][x_i - 1] = currentDistance + 1;
                        changedSomething = true;
                    }

                    terrain groundRight = Ground[y_i][x_i + 1];
                    if (x_i < TILES_X - 1 && !DistanceToGoal[y_i][x_i + 1] && ((groundRight & T_PATH) && !(groundRight & T_WALL))) {
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

    Mana = ManaStartValue;

    // TODO(Tobi): Not sure if that is needed at the moment
    Menu.Diamonds[MENU_DIAMONDS_Y / 2][1] = (diamond*) -1;

    //AudioClipStart(Music, true, 0.75f);

    DummyFontInfo = AcquireDebugFont();

    FILE* file = nullptr;
    fopen_s(&file, "assets/levels/dummy.lvl", "rb");
    if (file != nullptr) {
        fread(Ground, sizeof(Ground), 1, file);
        fclose(file);
    }

    CollectStartPositions();
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

vec2i TranslateToTopLeftPositionWithoutAnythig(vec2i triPosition) {
    bool downTriangle = (triPosition.X + triPosition.Y) % 2;

    if (downTriangle) {
        if (triPosition.X > 0) {
            return triPosition - vec2i { 0, 1 };
        } else {
            return TRANSLATE_NOTHING_FOUND;
        }
    } else {
        return triPosition;
    }
}

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

float SignLinePoint(vec2f lineA, vec2f lineB, float x, float y) {
    return (x - lineA.X) * (lineB.Y - lineA.Y) - (y - lineA.Y) * (lineB.X - lineA.X);
}

bool IsInTriangle(vec2f triPointA, vec2f triPointB, vec2f triPointC, vec2f pos) {
    float sA = SignLinePoint(triPointA, triPointB, pos.X, pos.Y);
    float sB = SignLinePoint(triPointB, triPointC, pos.X, pos.Y);
    float sC = SignLinePoint(triPointC, triPointA, pos.X, pos.Y);

    if (sA == 0) {
        if (
            triPointB.Y - triPointA.Y > 0 ||
            (triPointB.Y - triPointA.Y == 0 && triPointB.X - triPointA.X < 0)
        ) {
            sA = 1;
        }
    }

    if (sB == 0) {
        if (
            triPointC.Y - triPointB.Y > 0 ||
            (triPointC.Y - triPointB.Y == 0 && triPointC.X - triPointB.X < 0)
        ) {
            sB = 1;
        }
    }

    if (sC == 0) {
        if (
            triPointA.Y - triPointC.Y > 0 ||
            (triPointA.Y - triPointC.Y == 0 && triPointA.X - triPointC.X < 0)
        ) {
            sC = 1;
        }
    }

    // *pSA = sA;
    // *pSB = sB;
    // *pSC = sC;

    return (sA > 0) && (sB > 0) && (sC > 0);
}

bool IsInLogicalTriangle(vec2i triLogicalPosition, vec2f pos) {
    vec2f middle = TriToActualPos(triLogicalPosition);
    bool triangleIsDown = (triLogicalPosition.X + triLogicalPosition.Y) % 2;
    if (triangleIsDown) {
        vec2f topLeft  = middle + vec2f { -0.5f, - HEXAGON_H / 2 };
        vec2f topRight = middle + vec2f { +0.5f, - HEXAGON_H / 2 };
        vec2f down     = middle + vec2f {     0, + HEXAGON_H / 2 };
        return IsInTriangle(topLeft, down, topRight, pos);
    } else {
        vec2f bottomLeft  = middle + vec2f { -0.5f, + HEXAGON_H / 2 };
        vec2f bottomRight = middle + vec2f { +0.5f, + HEXAGON_H / 2 };
        vec2f up          = middle + vec2f {     0, - HEXAGON_H / 2 };
        return IsInTriangle(bottomLeft, bottomRight, up, pos);
    }
}

generation_link<monster> SelectedMonster;

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
    drawRectMenuDiamonds.Height = (MENU_DIAMONDS_Y + 1) / 2 * HEXAGON_PIXEL_HEIGHT;
    vec2i menuDiamondsMousePosition = TranslateMousePosition(&drawRectMenuDiamonds, ins);

    draw_rect drawRectMenuBuild = drawRectRightMenu;
    drawRectMenuBuild.StartY = height - 10 * HALF_HEXAGON_PIXEL_HEIGHT;
    drawRectMenuBuild.Height = 10 * HALF_HEXAGON_PIXEL_HEIGHT;
    vec2i menuBuildMousePosition = TranslateMousePosition(&drawRectMenuBuild, ins);

    draw_rect drawRectManaBar = drawRectRightMenu;
    drawRectManaBar.StartY = drawRectMenuDiamonds.StartY + drawRectMenuDiamonds.Height;
    drawRectManaBar.Height = drawRectMenuBuild.StartY - drawRectManaBar.StartY;

    vec2i mouseTilePos = MouseToTilePos(mainMousePosition);

    /// Handle Input
    {
        if (IS_KEY_PRESSED(KEY_TOGGLE_EDITOR)) {

            LevelRunning = !LevelRunning;
            if (FrameCount == 0) {
                // NOTE(Tobi): You can only edit the level before you start it; once started, you cannot edit it anymore

                IsLevelEditorActive = !IsLevelEditorActive;

                /// Save level data
                if (!IsLevelEditorActive && LevelEditorChangedSomething) {
                    LevelEditorChangedSomething = false;

                    FILE* file = nullptr;
                    fopen_s(&file, "assets/levels/dummy.lvl", "wb");
                    fwrite(Ground, sizeof(Ground), 1, file);
                    fclose(file);

                    CollectStartPositions();
                    InitDistanceArray();
                }
            }
        }

        if (IS_KEY_PRESSED(KEY_TOGGLE_SHOW_PATHFINDING)) {
            ShowPathfinding = !ShowPathfinding;
        }

        /// Level Editor
        if (IsLevelEditorActive) {
            if (ins->Mouse.Left.Down) {
                if (mouseTilePos.X >= 0 && mouseTilePos.X <TILES_X && mouseTilePos.Y >= 0 && mouseTilePos.Y < TILES_Y) {
                    Ground[mouseTilePos.Y][mouseTilePos.X] = T_PATH;
                    LevelEditorChangedSomething = true;
                }
            }

            if (ins->Mouse.Middle.Down) {
                if (mouseTilePos.X >= 0 && mouseTilePos.X < TILES_X && mouseTilePos.Y >= 0 && mouseTilePos.Y < TILES_Y) {
                    Ground[mouseTilePos.Y][mouseTilePos.X] = T_GRASS;
                    LevelEditorChangedSomething = true;
                }
            }

            if (ins->Mouse.Right.Down && ins->Mouse.Right.Toggled) {
                bool triangleIsDown = (mouseTilePos.X + mouseTilePos.Y) % 2;
                if (!triangleIsDown && mouseTilePos.X >= 0 && mouseTilePos.X < TILES_X - 2 && mouseTilePos.Y >= 0 && mouseTilePos.Y < TILES_Y - 1) {
                    if (Ground[mouseTilePos.Y][mouseTilePos.X] == T_TOWER) {
                        Ground[mouseTilePos.Y][mouseTilePos.X] = T_TRAP | T_PATH;
                        LevelEditorChangedSomething = true;

                        inc0 (y_i,   2) {
                            inc0 (x_i,   3) {
                                if (x_i == 0 && y_i == 0) { continue; }
                                Ground[mouseTilePos.Y + y_i][mouseTilePos.X + x_i] = T_PATH;
                            }
                        }
                    } else if (Ground[mouseTilePos.Y][mouseTilePos.X] & T_TRAP) {
                        Ground[mouseTilePos.Y][mouseTilePos.X] = T_GOAL | T_PATH;
                        LevelEditorChangedSomething = true;
                    } else {
                        Ground[mouseTilePos.Y][mouseTilePos.X] = T_TOWER;
                        LevelEditorChangedSomething = true;

                        inc0 (y_i,   2) {
                            inc0 (x_i,   3) {
                                if (x_i == 0 && y_i == 0) { continue; }
                                Ground[mouseTilePos.Y + y_i][mouseTilePos.X + x_i] = T_GRASS;
                            }
                        }
                    }
                }
            }
        }

        if (IS_KEY_PRESSED(KEY_MERGE)) {
            Menu.ShallMerge = !Menu.ShallMerge;
        }

        if (IS_KEY_PRESSED(KEY_WALL)) {
            Menu.WallBuildMode = !Menu.WallBuildMode;
            Menu.TowerBuildMode = false;
            Menu.TrapBuildMode = false;
        }

        if (IS_KEY_PRESSED(KEY_TOWER)) {
            Menu.TowerBuildMode = !Menu.TowerBuildMode;
            Menu.WallBuildMode = false;
            Menu.TrapBuildMode = false;
        }

        if (IS_KEY_PRESSED(KEY_TRAP)) {
            Menu.TrapBuildMode = !Menu.TrapBuildMode;
            Menu.WallBuildMode = false;
            Menu.TowerBuildMode = false;
        }

        if (ins->Mouse.WheelDelta > 0) {
            // TODO(Tobi): Consider mana (maybe) (The thing is that if I do that, I whould also have "0 - Don't buy" extra, because otherwise it's weird I feel)
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
    if (LevelRunning) {
        ++FrameCount;

        /// Menu logic
        {
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
                buildingHexTile = TranslateToTopLeftPosition(mouseTilePos, T_TOWER | T_TRAP);
                if (buildingHexTile != TRANSLATE_NOTHING_FOUND) {
                    inc_bucket (dimaond_i, diamond_, &Diamonds) {
                        if (diamond_->TilePositionTopLeft == buildingHexTile) {
                            diamondUnderCursor = diamond_;
                            break;
                        }
                    }
                }
            } else if (overValidDiamondMenuSlot && Menu.Diamonds[diamondMenuHexPos.Y][diamondMenuHexPos.X]) {
                diamondUnderCursor = Menu.Diamonds[diamondMenuHexPos.Y][diamondMenuHexPos.X];
            }

            // TODO(Tobi): Probably the Wall Build has to happen differently; or I have to decide which of the things have priority and disable other things
            if (IS_MOUSE_PRESSED(Left) && Menu.WallBuildMode && BoxContainsInEx(0, 0, TILES_X, TILES_Y, mouseTilePos.X, mouseTilePos.Y) && Mana >= CurrentWallCost) {
                vec2i topLeftBuildingPos = TranslateToTopLeftPosition(mouseTilePos, T_GOAL | T_TOWER | T_TRAP);
                bool alreadyBuilding = (Ground[mouseTilePos.Y][mouseTilePos.X] & T_WALL) || topLeftBuildingPos != TRANSLATE_NOTHING_FOUND;

                if (!alreadyBuilding) {
                    
                    /// Checking for monsters under that position
                    bool blockedByMonster = false;
                    inc_bucket(monster_i, monster_, &Monsters) {
                        if (IsInLogicalTriangle(mouseTilePos, monster_->ActualPosition)) {
                            blockedByMonster = true;
                            break;
                        }
                    }

                    /// Check if wall blocks path; clear wall again if necessary
                    if (!blockedByMonster) {
                        Ground[mouseTilePos.Y][mouseTilePos.X] |= T_WALL;
                        InitDistanceArray();
                        bool pathNowBlocked = false;
                        inc0 (startPos_i,   StartPositionsCount) {
                            vec2i startPos = StartPositions[startPos_i];
                            if (!DistanceToGoal[startPos.Y][startPos.X]) {
                                pathNowBlocked = true;
                                break;
                            }
                        }

                        if (pathNowBlocked) {
                            // TODO(Tobi): Show info
                            Ground[mouseTilePos.Y][mouseTilePos.X] &= ~T_WALL;
                            InitDistanceArray(); // Reset the distance array; TODO(Tobi): Maybe dont overwirte in the first place
                        } else {
                            // Actually got build
                            Mana -= CurrentWallCost;
                            CurrentWallCost = CurrentWallCost * 1.3f; // TODO(Tobi): Wall cost formula

                            // I have to find a new path for all monsters now I feel;
                            // TODO(Tobi): I actually can ignore the monsters where the numbers stayed the same
                        }
                    }
                }
            }

            if (IS_MOUSE_PRESSED(Left) && Menu.TowerBuildMode && BoxContainsInEx(0, 0, TILES_X, TILES_Y, mouseTilePos.X, mouseTilePos.Y) && Mana >= CurrentTowerCost) {
                vec2i topLeft = TranslateToTopLeftPositionWithoutAnythig(mouseTilePos);

                /// Checking if there already is something there
                // NOTE(Tobi): It is enough to check all up or down triangles only
                bool alreadyBuildingHere = 
                    TranslateToTopLeftPosition(topLeft + vec2i { 0, 0 }, T_GOAL | T_TOWER | T_TRAP) != TRANSLATE_NOTHING_FOUND ||
                    TranslateToTopLeftPosition(topLeft + vec2i { 2, 0 }, T_GOAL | T_TOWER | T_TRAP) != TRANSLATE_NOTHING_FOUND ||
                    TranslateToTopLeftPosition(topLeft + vec2i { 1, 1 }, T_GOAL | T_TOWER | T_TRAP) != TRANSLATE_NOTHING_FOUND;

                if (!alreadyBuildingHere) {
                    /// Checking for monsters under that position
                    bool blockedByMonster = false;
                    inc_bucket(monster_i, monster_, &Monsters) {
                        if (
                            IsInLogicalTriangle(topLeft + vec2i { 0, 0}, monster_->ActualPosition) ||
                            IsInLogicalTriangle(topLeft + vec2i { 1, 0}, monster_->ActualPosition) ||
                            IsInLogicalTriangle(topLeft + vec2i { 2, 0}, monster_->ActualPosition) ||
                            IsInLogicalTriangle(topLeft + vec2i { 0, 1}, monster_->ActualPosition) ||
                            IsInLogicalTriangle(topLeft + vec2i { 1, 1}, monster_->ActualPosition) ||
                            IsInLogicalTriangle(topLeft + vec2i { 2, 1}, monster_->ActualPosition)
                        ) {
                            blockedByMonster = true;
                            break;
                        }
                    }

                    /// Check if tower blocks path; clear tower again if necessary
                    terrain originalTerrain[2][3];
                    if (!blockedByMonster) {
                        inc0 (y_i,   2) {
                            inc0 (x_i,   3) {
                                originalTerrain[y_i][x_i] = Ground[topLeft.Y + y_i][topLeft.X + x_i];
                                Ground[topLeft.Y + y_i][topLeft.X + x_i] |= T_WALL;
                            }
                        }

                        InitDistanceArray();
                        bool pathNowBlocked = false;
                        inc0 (startPos_i,   StartPositionsCount) {
                            vec2i startPos = StartPositions[startPos_i];
                            if (!DistanceToGoal[startPos.Y][startPos.X]) {
                                pathNowBlocked = true;
                                break;
                            }
                        }

                        inc0 (y_i,   2) {
                            inc0 (x_i,   3) {
                                Ground[topLeft.Y + y_i][topLeft.X + x_i] = originalTerrain[y_i][x_i];
                            }
                        }

                        if (pathNowBlocked) {
                            // TODO(Tobi): Show info
                            Ground[mouseTilePos.Y][mouseTilePos.X] &= ~T_WALL;
                            InitDistanceArray(); // Reset the distance array; TODO(Tobi): Maybe dont overwrite in the first place
                        } else {
                            // Actually got build
                            Ground[topLeft.Y][topLeft.X] |= T_TOWER;

                            Mana -= CurrentTowerCost;
                            CurrentTowerCost = CurrentTowerCost * 1.3f; // TODO(Tobi): Tower cost formula

                            // I have to find a new path for all monsters now I feel;
                            // TODO(Tobi): I actually can ignore the monsters where the numbers stayed the same
                        }
                    }
                }
            }

            if (IS_MOUSE_PRESSED(Left) && Menu.TrapBuildMode && BoxContainsInEx(0, 0, TILES_X, TILES_Y, mouseTilePos.X, mouseTilePos.Y) && Mana >= CurrentTrapCost) {
                vec2i topLeft = TranslateToTopLeftPositionWithoutAnythig(mouseTilePos);

                /// Checking if there already is something there
                // NOTE(Tobi): It is enough to check all up or down triangles only
                bool canBuildHere = 
                    Ground[topLeft.Y + 0][topLeft.X + 0] == T_PATH &&
                    Ground[topLeft.Y + 0][topLeft.X + 1] == T_PATH &&
                    Ground[topLeft.Y + 0][topLeft.X + 2] == T_PATH &&
                    Ground[topLeft.Y + 1][topLeft.X + 0] == T_PATH &&
                    Ground[topLeft.Y + 1][topLeft.X + 1] == T_PATH &&
                    Ground[topLeft.Y + 1][topLeft.X + 2] == T_PATH;


                if (canBuildHere) {
                    Ground[topLeft.Y][topLeft.X] |= T_TRAP;

                    Mana -= CurrentTrapCost;
                    CurrentTrapCost = CurrentTrapCost * 1.3f; // TODO(Tobi): Tower cost formula

                    // I have to find a new path for all monsters now I feel;
                    // TODO(Tobi): I actually can ignore the monsters where the numbers stayed the same
                }
            }

            if (IS_MOUSE_PRESSED(Left)) {
                /// Buying
                float manaBuyCost = ManaCalcBuyCost(Menu.SelectedBuyingLevel);
                if (Mana >= manaBuyCost) {
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

                                Mana -= manaBuyCost;
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

                    if (diamondUnderCursor && Mana > ManaMergeCost) {
                        Mana -= ManaMergeCost;
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
                    // Put back to where it came from

                    // NOTE(Tobi): I have to take out that assertion since it will also happen when I cannot afford merging
                    // Assert(!diamondUnderCursor || diamondUnderCursor == Menu.DragDrop.Diamond, "How can there be a diamond under cursor, if I can't place something down");

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

            if (IS_KEY_PRESSED(KEY_LEVEL_UP) && diamondUnderCursor) {

                int countBefore = 0;
                inc0 (color_i,   DC_AMOUNT) {
                    countBefore += diamondUnderCursor->ColorsCount[color_i];
                }
                // TODO(Tobi): This works only if I don't have mixed things
                // However, for other stuff, I will need a new formula
                // (oh I would just have to store how much mana a diamond is worth at the moment and then act on that)
                int levelBefore = FindFirstSet(countBefore);
                float manaBuyCost = ManaCalcBuyCost(levelBefore);
                float manaUpgradeCost = manaBuyCost + ManaMergeCost;

                if (diamondUnderCursor && Mana >= manaUpgradeCost) {
                    Mana -= manaUpgradeCost;

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

        if (diamondUnderCursor) {
            if (IS_KEY_PRESSED(KEY_PRIO_GOAL)) {
                diamondUnderCursor->Priority = SP_CLOSEST_TO_GOAL;
            }

            if (IS_KEY_PRESSED(KEY_PRIO_LEAST_HP)) {
                diamondUnderCursor->Priority = SP_LEAST_HEALTH;
            }

            if (IS_KEY_PRESSED(KEY_PRIO_RANDOM)) {
                diamondUnderCursor->Priority = SP_RANDOM;
            }
        }

        /// Click on Monster
        if (IS_MOUSE_PRESSED(Left)){
            SelectedMonster = {};
            if (BoxContainsInEx(0, 0, TILES_X, TILES_Y, mouseTilePos.X, mouseTilePos.Y)) {
                inc_bucket (monster_i, monster_, &Monsters) {
                    // TODO(Tobi): Possibility to cycle through monsters or other pressed stuff
                    if (CirclesDoIntersect(monster_->ActualPosition, monster_->Radius, mainMousePosition / (float) HEXAGON_A, 0)) {
                        SelectedMonster = GenerationLinkCreate(monster_);
                        break;
                    }
                }
            }
        }

        /// Update Mana
        if (FrameCount % 60 == 0) {
            Mana += ManaGainPerSecond;
        }

        /// Update Monsters
        {
            inc_bucket (monster_i, monster_, &Monsters) {
                Assert(monster_->Health, "Monster should be dead");

                /// React to effects
                if (monster_->PoisonFrames > 0) {
                    float reduceAmount = monster_->PoisonAmount / monster_->PoisonFrames;
                    monster_->Health -= reduceAmount;
                    monster_->PoisonAmount -= reduceAmount;

                    if (monster_->Health <= 0) {
                        monster_->Health = 0; // TODO(Tobi): Do I need that?
                        BucketListRemove(&Monsters, monster_);
                    }

                    --monster_->PoisonFrames;
                }

                /// Pathfinding and Movement
                {
                    // TODO(Tobi): In the future I might update this directly when settign a new wal/tower etc.
                    // NOTE(Tobi): This is kind of  a fix at the moment; since this will be made again after the wiggle movement
                    // If the goal is not walkable anymore; I have to set a new goal
                    // The code here is almost like below, but it uses OldPosition everywhere instead of GoalPosition
                    if (!DistanceToGoal[monster_->GoalPosition.Y][monster_->GoalPosition.X]) {
                        bool triangleIsDown = (monster_->OldPosition.X + monster_->OldPosition.Y) % 2;
                        int closestDistance = 9999999;
                        vec2i deltaPos = { 0, 0 };

                        if (triangleIsDown) {
                            if (monster_->OldPosition.Y > 0 && DistanceToGoal[monster_->OldPosition.Y - 1][monster_->OldPosition.X]) {
                                int proposedDistance = DistanceToGoal[monster_->OldPosition.Y - 1][monster_->OldPosition.X];
                                closestDistance = proposedDistance;
                                deltaPos = vec2i { 0, -1 };
                            }
                        } else {
                            if (monster_->OldPosition.Y < TILES_Y - 1 && DistanceToGoal[monster_->OldPosition.Y + 1][monster_->OldPosition.X]) {
                                int proposedDistance = DistanceToGoal[monster_->OldPosition.Y + 1][monster_->OldPosition.X];
                                closestDistance = proposedDistance;
                                deltaPos = vec2i { 0, +1 };
                            }
                        }

                        int sameAmount = 1;
                        
                        if (monster_->OldPosition.X > 0 && DistanceToGoal[monster_->OldPosition.Y][monster_->OldPosition.X - 1]) {
                            int proposedDistance = DistanceToGoal[monster_->OldPosition.Y][monster_->OldPosition.X - 1];
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
                        
                        if (monster_->OldPosition.X < TILES_X - 1 && DistanceToGoal[monster_->OldPosition.Y][monster_->OldPosition.X + 1]) {
                            int proposedDistance = DistanceToGoal[monster_->OldPosition.Y][monster_->OldPosition.X + 1];
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

                        monster_->GoalPosition = monster_->OldPosition + deltaPos;
                    }

                    if (monster_->MovementT >= 1.0f) {
                        monster_->MovementT -= 1.0f;

                        int distanceToGoal = DistanceToGoal[monster_->GoalPosition.Y][monster_->GoalPosition.X];
                        if (distanceToGoal == 1) {
                            // TODO(Tobi): Cause damage to the orb (banishment cost)
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

                        monster_->OldPosition = monster_->GoalPosition;
                        monster_->GoalPosition += deltaPos;
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
                    newMonster->Armor = monster_wave_->Prototype.MaxArmor;
                    newMonster->Magic = monster_wave_->Prototype.MaxMagic;
                    newMonster->Mana = monster_wave_->Prototype.Mana;

                    MonsterSetToStartingPosition(newMonster);
                }
            }
        }

        /// Update Diamonds
        inc_bucket (diamond_i, diamond_, &Diamonds) {
            // TODO(Tobi): Cooldown frames has to work differently for the traps

            if (diamond_->CooldownFrames > 0) {
                --diamond_->CooldownFrames;
                continue;
            }

            // Right menu or being dragged
            if (!diamond_->IsInField) { continue; }

            float diamondRange = diamond_->RangeRadius;

            // TODO(Tobi): Check several different buildings here
            if (Ground[diamond_->TilePositionTopLeft.Y][diamond_->TilePositionTopLeft.X] & T_TRAP) {
                diamondRange = 1.0f;
            }

            /// Find target monster
            monster* target = nullptr;
            {
                monster* selectedMonster = GenerationLinkResolve(SelectedMonster);
                if (selectedMonster) {
                    if (CirclesDoIntersect(selectedMonster->ActualPosition, selectedMonster->Radius, diamond_->ActualPosition, diamondRange)) {
                        target = selectedMonster;
                    }
                }
            
                if (!target) {
                    switch (diamond_->Priority) {
                        case SP_CLOSEST_TO_GOAL: {
                            float closestDistanceToGoal = 999999999.0f;
                            inc_bucket (monster_i, monster_, &Monsters) {
                                Assert(monster_->Health > 0, "Monster should be inactive");

                                if (CirclesDoIntersect(monster_->ActualPosition, monster_->Radius, diamond_->ActualPosition, diamondRange)) {
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
                        } break;
                        case SP_LEAST_HEALTH: {
                            float leastHealth = 999999999.0f;
                            inc_bucket (monster_i, monster_, &Monsters) {
                                Assert(monster_->Health > 0, "Monster should be inactive");

                                if (CirclesDoIntersect(monster_->ActualPosition, monster_->Radius, diamond_->ActualPosition, diamondRange)) {
                                    if (monster_->Health < leastHealth) {
                                        leastHealth = monster_->Health;
                                        target = monster_;
                                    }
                                }
                            }
                        } break;
                        case SP_RANDOM: {
                            // NOTE(Tobi): I iterate over the whole thing twice; first I count how many I would hit and then again to grab one; this could be made easier by copying out the ones I hit and then just grabbing one

                            int monstersInRange = 0;
                            inc_bucket (monster_i, monster_, &Monsters) {
                                Assert(monster_->Health > 0, "Monster should be inactive");
                                if (CirclesDoIntersect(monster_->ActualPosition, monster_->Radius, diamond_->ActualPosition, diamondRange)) {
                                    ++monstersInRange;
                                }
                            }
                            if (monstersInRange == 0) { break; }

                            int chosenIndex = rand() % monstersInRange;

                            int chosen_i = 0;
                            inc_bucket (monster_i, monster_, &Monsters) {
                                Assert(monster_->Health > 0, "Monster should be inactive");
                                if (CirclesDoIntersect(monster_->ActualPosition, monster_->Radius, diamond_->ActualPosition, diamondRange)) {
                                    if (chosen_i == chosenIndex) {
                                        target = monster_;
                                        break;
                                    }
                                    ++chosen_i;
                                }
                            }
                        } break;
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
                newProjectile->Damage = diamond_->Damage; // TODO(Tobi): Adjust in trap
                newProjectile->Speed = PROJECTILE_SPEED;
                newProjectile->Target = GenerationLinkCreate(target);

                // TODO(Tobi): The effect multiplier for traps won't work like that in the future
                int colorFactor = 1;
                if (Ground[diamond_->TilePositionTopLeft.Y][diamond_->TilePositionTopLeft.X] & T_TRAP) {
                    // NOTE(Tobi): Double effects if in trap + only half damage
                    colorFactor = 2;
                    newProjectile->Damage /= 2.0f;
                }

                // TODO(Tobi): Are the colours even important for projectiles if I give them their damage and their effect anyway?
                inc0 (color_i,   DC_AMOUNT) {
                    newProjectile->ColorsCount[color_i] = diamond_->ColorsCount[color_i] * colorFactor;
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
                vec2f delta = target->ActualPosition - projectile_->Position;
                float distanceSq = delta.X * delta.X + delta.Y * delta.Y;

                // TODO(Tobi): This only checks midpoint-midpoint; not edge-edge
                if (CirclesDoIntersect(target->ActualPosition, target->Radius, projectile_->Position, projectile_->Speed)) {

                    /// Assign effects
                    // NOTE(Tobi): The effects are assigned before the health reduction so that the mana steal is correct (although almost pointless)
                    {
                        /// Poison (Green)
                        // TODO(Tobi): Maybe poison should work more similar to slow and wounded (below)
                        if (projectile_->ColorsCount[DC_GREEN] != 0) {
                            // TODO(Tobi): This is just adding up the poison damage just like in GC; do I want to copy that
                            target->PoisonAmount += sqrtf(1.0f + (projectile_->ColorsCount[DC_GREEN] - 1) /100.0f) * DIAMOND_LEVEL_1_POISON;
                            target->PoisonFrames = DIAMOND_EFFECT_LENGTH;
                        }

                        // TODO(Tobi): How will adding slow effects (or wound effects) actually work?; I only set a multiplier and that is valid for some amount of time; but what happens if the refresh happens with a different value?
                        // I could just store a length for different values and take the highest one

                        /// Slow (Blue)
                        // if (projectile_->ColorsCount[DC_BLUE] != 0) {
                        //     // TODO(Tobi): Maybe the length also grows and not just the modifier
                        //     // TODO(Tobi): FORMULA target->SlowFactor += sqrtf(1.0f + (projectile_->ColorsCount[DC_GREEN] - 1) /100.0f) * DIAMOND_LEVEL_1_POISON;
                        //     target->SlowFrames = DIAMOND_EFFECT_LENGTH;
                        // }

                        // /// Red (Wounded)
                        // if (projectile_->ColorsCount[DC_BLUE] != 0) {
                        //     // TODO(Tobi): Maybe the length also grows and not just the modifier
                        //     // TODO(Tobi): FORMULA target->WoundedFactor += sqrtf(1.0f + (projectile_->ColorsCount[DC_GREEN] - 1) /100.0f) * DIAMOND_LEVEL_1_POISON;
                        //     target->WoundedFrames = DIAMOND_EFFECT_LENGTH;
                        // }

                        /// Magic reduce (Cyan)
                        if (projectile_->ColorsCount[DC_AQUA] != 0) {
                            float magicReduce = projectile_->ColorsCount[DC_AQUA] * DIAMOND_LEVEL_1_MAGIC_REDUCE;
                            target->Magic = AtLeast(target->Magic - magicReduce, 0.0f);
                        }

                        /// Armor reduce (Purple)
                        if (projectile_->ColorsCount[DC_PURPLE] != 0) {
                            float armorReduce = projectile_->ColorsCount[DC_PURPLE] * DIAMOND_LEVEL_1_ARMOR_REDUCE;
                            target->Armor = AtLeast(target->Armor - armorReduce, 0.0f);
                        }

                        /// Yellow (Mana Steal)
                        if (projectile_->ColorsCount[DC_YELLOW] != 0) {
                            // TODO(Tobi): This is waaaaaaaaaaaaay too much
                            float manaSteal = projectile_->ColorsCount[DC_YELLOW] * DIAMOND_LEVEL_1_MANA_STEAL;
                            manaSteal = AtMost(manaSteal, target->Health);
                            Mana += manaSteal;
                        }
                    }

                    target->Health -= projectile_->Damage;
                    if (target->Health <= 0) {
                        Mana += target->Mana;
                        target->Health = 0;
                        BucketListRemove(&Monsters, target);
                        AudioClipStart(Sounds.Death, false, 0.7f);
                        ParticleEffectStartWorld(&drawRectMain, 16, Particles.Smoke, target->ActualPosition.X, target->ActualPosition.Y, COL32_RGBA(100, 80, 80, 160));
                        // TODO(Tobi): The monster has been killed; do something
                    } else {
                        AudioClipStart(Sounds.Hit, false, 0.2f);
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

    }
    
    /// Rendering
    {
        // TODO(Tobi): The screen shake in combination with the right draw rects is still kind of off

        /// Clear screen
        {
            color32 clearColor;
            if (!LevelRunning) {
                clearColor = COL32_RGB(0, 0, 192);
            } else {
                clearColor = BLACK;
            }
            inc0 (i,   height * width) {
                array[i] = clearColor;
            }
        }

        /// Render Grass / Path / Wall
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
                    if (Ground[y_i][x_i] & (T_WALL)) {
                        // Wall
                        if (triangleIsDown) {
                            DrawScreenBitmap(&drawRectMain, x_i * HALF_HEXAGON_PIXEL_WIDTH / 2, y_i * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.GrassDown, DARK_GREY);
                        } else {
                            DrawScreenBitmap(&drawRectMain, x_i * HALF_HEXAGON_PIXEL_WIDTH / 2 - evenLineOffset, y_i * HALF_HEXAGON_PIXEL_HEIGHT, Sprites.GrassUp, DARK_GREY);
                        }
                    } else if (!(Ground[y_i][x_i] & (T_PATH | T_GOAL))) {
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
                switch (Ground[y_i][x_i] & ~T_PATH) {
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
        if (!LevelRunning) {
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

        /// Render Mana Bar
        {

            float manaPercentage = Mana / 1000.0f;
            DrawScreenRectangle(&drawRectManaBar, 0, 0, 5 * HALF_HEXAGON_PIXEL_WIDTH, drawRectManaBar.Height, COL32_RGB(64, 32,   0));
            DrawScreenRectangle(&drawRectManaBar, 0, RoundFloat32ToInt32((1.0f - manaPercentage) * drawRectManaBar.Height), 5 * HALF_HEXAGON_PIXEL_WIDTH, RoundFloat32ToInt32(manaPercentage * drawRectManaBar.Height), ORANGE);

            char dummy[128];
            snprintf(dummy, ArrayCount(dummy), "%d", (int) Mana);
            int textWidth = TextGetRenderSize(&DummyFontInfo, dummy);
            TextRenderScreen(&drawRectManaBar, &DummyFontInfo, (5 * HALF_HEXAGON_PIXEL_WIDTH - textWidth) / 2, (drawRectManaBar.Height - DummyFontInfo.FontSize) / 2, dummy, WHITE);
        }

        /// Render Diamonds in Traps
        inc_bucket (diamond_i, diamond_, &Diamonds) {
            // NOTE(Tobi): I will definitely not have that separated in the future

            if (diamond_ == Menu.DragDrop.Diamond) { continue; }
            if (!diamond_->IsInField) { continue; }
            if (!(Ground[diamond_->TilePositionTopLeft.Y][diamond_->TilePositionTopLeft.X] & T_TRAP)) { continue; }

            int frame = FrameCount % 30 / 10;
            color32 trapRenderColor = diamond_->MixedColor;
            trapRenderColor.Alpha = 128; // TODO(Tobi): Maybe change their brightness instead; that way, the darker ones aren't effected that intensly
            DrawWorldBitmap(&drawRectMain, diamond_->ActualPosition.X, diamond_->ActualPosition.Y, Sprites.Cogwheels[frame], trapRenderColor);
            
            int count = 0;
            inc0 (color_i,   DC_AMOUNT) {
                count += diamond_->ColorsCount[color_i];
            }
            char dummy[5];
            snprintf(dummy, 5, "%d", count);
            TextRenderWorld(&drawRectMain, &DummyFontInfo, diamond_->ActualPosition.X + 0.475f, diamond_->ActualPosition.Y + 0.225f, dummy, WHITE);

            /// Render Diamond Range
            // (Which is 1 now in a trap; TODO(Tobi): Make constant)
            DrawWorldCircle(&drawRectMain, diamond_->ActualPosition.X, diamond_->ActualPosition.Y, 1.0f, YELLOW);

            /// Render Diamond Cooldown
            if (diamond_->CooldownFrames) {
                DrawWorldRectangle(&drawRectMain, diamond_->ActualPosition.X - 0.5f, diamond_->ActualPosition.Y + 0.5f, diamond_->CooldownFrames / (float)diamond_->MaxCooldown, 1 / 6.0f, GREEN);
            }
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

            /// Render Monster Healthbar and effect sprites
            {
                monster_->WoundedFrames = 1;
                monster_->SlowFrames = 1;

                int effects = 0;
                if (monster_->PoisonFrames > 0) {
                    ++effects;
                }
                if (monster_->WoundedFrames > 0) {
                    ++effects;
                }
                if (monster_->SlowFrames > 0) {
                    ++effects;
                }

                if (monster_->Health < monster_->MaxHealth || effects > 0) {
                    DrawWorldRectangle(&drawRectMain, monster_->ActualPosition.X - 0.5f, monster_->ActualPosition.Y + 0.5f, monster_->Health / monster_->MaxHealth, 1 / 6.0f, RED);
                    DrawWorldRectangle(&drawRectMain, monster_->ActualPosition.X - 0.5f + monster_->Health / monster_->MaxHealth, monster_->ActualPosition.Y + 0.5f, 1.0f - monster_->Health / monster_->MaxHealth, 1 / 6.0f, BLACK);
                }

                int drawnEffects = 0;
                float xOffsetEffectSprites = 0.15f * (effects - 1);
                if (monster_->PoisonFrames) {
                    DrawWorldBitmap(&drawRectMain, monster_->ActualPosition.X + - xOffsetEffectSprites + 0.3f * drawnEffects, monster_->ActualPosition.Y + 0.8f, Sprites.PoisonTex, WHITE);
                    ++drawnEffects;
                }

                if (monster_->WoundedFrames) {
                    DrawWorldBitmap(&drawRectMain, monster_->ActualPosition.X + - xOffsetEffectSprites + 0.3f * drawnEffects, monster_->ActualPosition.Y + 0.8f, Sprites.BloodTex, WHITE);
                    ++drawnEffects;
                }

                if (monster_->SlowFrames) {
                    DrawWorldBitmap(&drawRectMain, monster_->ActualPosition.X + - xOffsetEffectSprites + 0.3f * drawnEffects, monster_->ActualPosition.Y + 0.8f, Sprites.IceTex, WHITE);
                    ++drawnEffects;
                }
            }
        }

        /// Render Diamonds in Towers
        inc_bucket (diamond_i, diamond_, &Diamonds) {
            // NOTE(Tobi): Will I really kep it like that?

            if (diamond_ == Menu.DragDrop.Diamond) { continue; }

            if (diamond_->IsInField && Ground[diamond_->TilePositionTopLeft.Y][diamond_->TilePositionTopLeft.X] & T_TRAP) { continue; }

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

        /// Render selected Monster popup
        {
            monster* selectedMonster = GenerationLinkResolve(SelectedMonster);
            if (selectedMonster) {
                float monsterContextMenuLeft = selectedMonster->ActualPosition.X + drawRectMain.StartX / (float) HEXAGON_A;
                float monsterContextMenuTop = selectedMonster->ActualPosition.Y + drawRectMain.StartY / (float) HEXAGON_A;

                int lines = 3;
                if (selectedMonster->PoisonAmount > 0) {
                    ++lines;
                }
                if (selectedMonster->Armor > 0) {
                    ++lines;
                }
                if (selectedMonster->Magic > 0) {
                    ++lines;
                }

                DrawWorldRectangleAlpha(&drawRectAll, monsterContextMenuLeft, monsterContextMenuTop, 4.5f, lines * DummyFontInfo.FontSize / (float) HEXAGON_A, COL32_RGBA(0, 0, 0, 160));

                char dummy[32];
                snprintf(dummy, ArrayCount(dummy), "Health: %.2f / %.2f", selectedMonster->Health, selectedMonster->MaxHealth);
                TextRenderWorld(&drawRectAll, &DummyFontInfo, monsterContextMenuLeft, monsterContextMenuTop, dummy, WHITE);

                // TODO(Tobi): Modify with slow factor
                snprintf(dummy, ArrayCount(dummy), "Speed: %.2f", selectedMonster->Speed);
                TextRenderWorld(&drawRectAll, &DummyFontInfo, monsterContextMenuLeft, monsterContextMenuTop + DummyFontInfo.FontSize / (float) HEXAGON_A, dummy, WHITE);

                snprintf(dummy, ArrayCount(dummy), "Mana: %.2f", selectedMonster->Mana);
                TextRenderWorld(&drawRectAll, &DummyFontInfo, monsterContextMenuLeft, monsterContextMenuTop + 2 * DummyFontInfo.FontSize / (float) HEXAGON_A, dummy, WHITE);

                int linesAsOfYet = 3;
                if (selectedMonster->Armor > 0) {
                    snprintf(dummy, ArrayCount(dummy), "Armor: %.2f", selectedMonster->Armor);
                    TextRenderWorld(&drawRectAll, &DummyFontInfo, monsterContextMenuLeft, monsterContextMenuTop + linesAsOfYet * DummyFontInfo.FontSize / (float) HEXAGON_A, dummy, PURPLE);
                    ++linesAsOfYet;
                }
                if (selectedMonster->Magic > 0) {
                    snprintf(dummy, ArrayCount(dummy), "Magic: %.2f", selectedMonster->Magic);
                    TextRenderWorld(&drawRectAll, &DummyFontInfo, monsterContextMenuLeft, monsterContextMenuTop + linesAsOfYet * DummyFontInfo.FontSize / (float) HEXAGON_A, dummy, AQUA);
                    ++linesAsOfYet;
                }
                if (selectedMonster->PoisonFrames > 0) {
                    snprintf(dummy, ArrayCount(dummy), "Poisoned: %.2f", selectedMonster->PoisonAmount);
                    TextRenderWorld(&drawRectAll, &DummyFontInfo, monsterContextMenuLeft, monsterContextMenuTop + linesAsOfYet * DummyFontInfo.FontSize / (float) HEXAGON_A, dummy, GREEN);
                    ++linesAsOfYet;
                }

                // TODO(Tobi): Show wounded
            }
        }

        /// Render mouse-over highlight
        if (diamondUnderCursor && Menu.DragDrop.Diamond != diamondUnderCursor) {

            float diamondContextMenuLeft;
            float diamondContextMenuTop;
            if (!diamondUnderCursor->IsInField) {
                // TODO(Tobi): Quite a bit difficult to position it right, when I don't know how much space is needed
                vec2f diamondPosition = HexToActualPos(diamondUnderCursor->TilePositionTopLeft);
                diamondContextMenuLeft = diamondPosition.X + drawRectMenuDiamonds.StartX / (float) HEXAGON_A - 4.5f;
                diamondContextMenuTop = diamondPosition.Y + drawRectMenuDiamonds.StartY / (float) HEXAGON_A;
            } else {
                vec2f diamondPosition = TriToActualPos(diamondUnderCursor->TilePositionTopLeft) + TOP_LEFT_TO_CENTRE_OFFSET;
                diamondContextMenuLeft = diamondPosition.X + drawRectMain.StartX / (float) HEXAGON_A;
                diamondContextMenuTop = diamondPosition.Y + drawRectMain.StartY / (float) HEXAGON_A;
            }

            DrawWorldRectangleAlpha(&drawRectAll, diamondContextMenuLeft, diamondContextMenuTop, 4.5f, 2 * DummyFontInfo.FontSize / (float) HEXAGON_A, COL32_RGBA(0, 0, 0, 160));

            char dummy[32];
            snprintf(dummy, ArrayCount(dummy), "Damage: %.2f", diamondUnderCursor->Damage);
            TextRenderWorld(&drawRectAll, &DummyFontInfo, diamondContextMenuLeft, diamondContextMenuTop, dummy, WHITE);

            snprintf(dummy, ArrayCount(dummy), "Cooldown Frames: %d", diamondUnderCursor->MaxCooldown);
            TextRenderWorld(&drawRectAll, &DummyFontInfo, diamondContextMenuLeft, diamondContextMenuTop + DummyFontInfo.FontSize / (float) HEXAGON_A, dummy, WHITE);

            // TODO(Tobi): Print the effects stuff

            if (diamondUnderCursor->IsInField) {
                DrawWorldCircle(&drawRectMain, diamondUnderCursor->ActualPosition.X, diamondUnderCursor->ActualPosition.Y, 1.0f, RED);
            } else {
                DrawWorldCircle(&drawRectMenuDiamonds, diamondUnderCursor->ActualPosition.X, diamondUnderCursor->ActualPosition.Y, 1.0f, BLUE);
            }
        }

        /// Render wall highlight
        if (Menu.WallBuildMode) {
            if (BoxContainsInEx(0, 0, TILES_X, TILES_Y, mouseTilePos.X, mouseTilePos.Y)) {
                vec2f middle = TriToActualPos(mouseTilePos);
                bool triangleIsDown = (mouseTilePos.X + mouseTilePos.Y) % 2;
                if (triangleIsDown) {
                    middle += vec2f { 0, 1 / 3.0f };
                    vec2f topLeft  = middle + vec2f { -0.5f, - HEXAGON_H / 2 };
                    vec2f topRight = middle + vec2f { +0.5f, - HEXAGON_H / 2 };
                    vec2f down     = middle + vec2f {     0, + HEXAGON_H / 2 };
                    DrawWorldLineThick(&drawRectMain, topLeft, topRight, 1, WHITE);
                    DrawWorldLineThick(&drawRectMain, topRight, down, 1, WHITE);
                    DrawWorldLineThick(&drawRectMain, down, topLeft, 1, WHITE);
                } else {
                    vec2f bottomLeft  = middle + vec2f { -0.5f, + HEXAGON_H / 2 };
                    vec2f bottomRight = middle + vec2f { +0.5f, + HEXAGON_H / 2 };
                    vec2f up          = middle + vec2f {     0, - HEXAGON_H / 2 };
                    DrawWorldLineThick(&drawRectMain, bottomLeft, bottomRight, 1, WHITE);
                    DrawWorldLineThick(&drawRectMain, bottomRight, up, 1, WHITE);
                    DrawWorldLineThick(&drawRectMain, up, bottomLeft, 1, WHITE);
                }
            }
        }

        // Render hex highlight
        if (Menu.TowerBuildMode || Menu.TrapBuildMode) {
            vec2i tilePos = TranslateToTopLeftPositionWithoutAnythig(mouseTilePos);
            if (tilePos != TRANSLATE_NOTHING_FOUND) {
                vec2f actualPos = TriToActualPos(tilePos);
                
                vec2f topLeftPos     = actualPos + vec2f {  0.0f, -2 / 3.0f * HEXAGON_H };
                vec2f topRightPos    = actualPos + vec2f {  1.0f, -2 / 3.0f * HEXAGON_H };
                vec2f rightPos       = actualPos + vec2f {  1.5f, +1 / 3.0f * HEXAGON_H };
                vec2f bottomLeftPos  = actualPos + vec2f {  0.0f, +4 / 3.0f * HEXAGON_H };
                vec2f bottomRightPos = actualPos + vec2f {  1.0f, +4 / 3.0f * HEXAGON_H };
                vec2f leftPos        = actualPos + vec2f { -0.5f, +1 / 3.0f * HEXAGON_H };

                DrawWorldLineThick(&drawRectMain, topLeftPos, topRightPos, 3, WHITE);
                DrawWorldLineThick(&drawRectMain, topRightPos, rightPos, 3, WHITE);
                DrawWorldLineThick(&drawRectMain, rightPos, bottomRightPos, 3, WHITE);
                DrawWorldLineThick(&drawRectMain, bottomRightPos, bottomLeftPos, 3, WHITE);
                DrawWorldLineThick(&drawRectMain, bottomLeftPos, leftPos, 3, WHITE);
                DrawWorldLineThick(&drawRectMain, leftPos, topLeftPos, 3, WHITE);
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
}
