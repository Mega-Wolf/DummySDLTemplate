#pragma once

#define inc0(varname, max) for (int varname = 0; varname < max; ++varname)
#define inc(varname, start, max) for (int varname = start; varname < max; ++varname)

#define IS_KEY_PRESSED(key) ins->Keyboard.key.Down && ins->Keyboard.key.Toggled
