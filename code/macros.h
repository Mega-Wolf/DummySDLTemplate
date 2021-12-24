#pragma once

#define inc0(varname, max) for (int varname = 0; varname < max; ++varname)
#define inc(varname, start, max) for (int varname = start; varname < max; ++varname)

#define inc0In(varname, max) for (int varname = 0; varname <= max; ++varname)
#define incIn(varname, start, max) for (int varname = start; varname <= max; ++varname)

#define IS_KEY_PRESSED(key) ins->Keyboard.key.Down && ins->Keyboard.key.Toggled
#define IS_MOUSE_PRESSED(key) ins->Mouse.key.Down && ins->Mouse.key.Toggled
#define IS_MOUSE_RELEASED(key) !ins->Mouse.key.Down && ins->Mouse.key.Toggled


#define PI 3.141592f