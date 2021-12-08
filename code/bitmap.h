#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "../helpers/backend.h"

#include "macros.h"

struct loaded_bitmap {
    int Width;
    int Height;
    color32* Data;
};

#pragma pack(push, 1)
struct bitmap_header {
    unsigned short FileType;
    unsigned int FileSize;
    unsigned short Reserved1;
    unsigned short Reserved2;
    unsigned int BitmapOffset;

    unsigned int Size;
    int Width;
    int Height;
    unsigned int Planes;
    unsigned int BitsPerPixel;

    unsigned int Compression;
    unsigned int SizeOfBitmap;
    int HorzResolution;
    int VertResolution;
    unsigned int ColorsUsed;
    unsigned int ColorsImportant;
    unsigned int RedMask;
    unsigned int GreenMask;
    unsigned int BlueMask;
};
#pragma pack(pop)

loaded_bitmap BitmapLoad(char* filename) {
    FILE* file = fopen(filename, "rb");

    loaded_bitmap ret = {};
    bitmap_header bh;

    if (file) {
        fread(&bh, sizeof(bitmap_header), 1, file);
        //Assert(bh.BitsPerPixel == 32 && bh.Compression == 3, "Wrong bitmap encoding: %s", filename);

        // TODO(Tobi): Seek set?
        fseek(file, bh.BitmapOffset - sizeof(bitmap_header), SEEK_CUR);

        ret.Width = bh.Width;
        ret.Height = bh.Height;
        ret.Data = (color32*) malloc(sizeof(color32) * bh.SizeOfBitmap);

        inc0 (y_i,   bh.Height) {
            fread(&ret.Data[bh.Width * (bh.Height - 1- y_i)], sizeof(color32), bh.Width, file);
        }

        fclose(file);
    }

    return ret;
}
