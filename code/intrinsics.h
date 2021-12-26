#pragma once

// TODO(Tobi): This is compiler and platform specific and therefore I have to do stuff here to distinguish that

#include "macros.h"

#define COMPILER_MSVC 1
// #define COMPILER_LLVM

#if !NO_HEADERS && COMPILER_MSVC
#include <intrin.h>
#endif


int FindFirstSet(unsigned int mask) {
    #ifdef COMPILER_MSVC
    {
        unsigned long index;
        _BitScanForward(&index, mask);
        return index;
    }
    #else
    {
        inc0 (i,   32) {
            if (mask & (1 << i)) {
                return i;
            }
        }

        // NOTE(Tobi): Or -1 or something
        return 0;
    }
    #endif
}

bool FindFirstSet(unsigned int* index, unsigned int mask) {
    #ifdef COMPILER_MSVC
    {
        unsigned long* ind = (unsigned long*) index;
        return _BitScanForward(ind, mask);
    }
    #else
    {
        inc0 (i,   32) {
            if (mask & (1 << i)) {
                *index = i;
                return true;
            }
        }

        return false;
    }
    #endif
}
