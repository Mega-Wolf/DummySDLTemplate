#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "../helpers/backend.h"

#include "macros.h"

struct loaded_audio {
    int Frames;
    short (*Data)[2];
};

#pragma pack(push, 1)
struct wav_header {
   char RiffChunkID[4]; // "RIFF"
   int ChunkSize; // Filelength - 8
   char RiffType[4]; // "WAVE"

   char FMTChunkName[4]; // "fmt " // NOTE(Tobi): This chunk SHOULD be here; but maybe it is not
   unsigned int FMTChunkLength;
   unsigned short FormatTag; // 1 if PCM (uncompressed data)
   unsigned short ChannelCount;
   unsigned int SamplesPerSecond;
   unsigned int AverageBytesPerSecond;
   unsigned short BlockAlign;

   unsigned short BitsPerSample; // NOTE(Tobi): This only exists, if PCM

   char DataChunkName[4]; // "Data"
   unsigned int DataChunkLength; // file size - 44

};
#pragma pack(pop)

#pragma pack(push, 1)
union dummyHeader {
    struct {
        union {
            int   Ints  [512 * 1];
            short Shorts[512 * 2];
            char  Chars [512 * 4];
        } Data;
    } Align0;
    struct {
        char Dummy[1];
        union {
            int   Ints  [512 * 1];
            short Shorts[512 * 2];
            char  Chars [512 * 4];
        } Data;
    } Align1;
    struct {
        char Dummy[2];
        union {
            int   Ints  [512 * 1];
            short Shorts[512 * 2];
            char  Chars [512 * 4];
        } Data;
    } Align2;
    struct {
        char Dummy[3];
        union {
            int   Ints  [512 * 1];
            short Shorts[512 * 2];
            char  Chars [512 * 4];
        } Data;
    } Align3;
};
#pragma pack(pop)


loaded_audio LoadWav(char* filename) {
    FILE* file = fopen(filename, "rb");

    loaded_audio ret = {};
    wav_header wh;

    if (file) {
        fread(&wh, sizeof(wav_header), 1, file);

        // TODO(Tobi): Assert some stuff
        //ret.Frames = wh.
        //wh.SamplesPerSecond // 48000; 44100
        //wh.

        if (wh.ChannelCount == 2) {
            ret.Data = (short(*) [2]) malloc(wh.DataChunkLength);
            fread(ret.Data, wh.DataChunkLength, 1, file);
            ret.Frames = wh.DataChunkLength / 2 / 2; // 2 because of short; 2 channels
        } else if (wh.ChannelCount == 1) {
            ret.Data = (short(*) [2]) malloc(wh.DataChunkLength * 2);
            inc0 (sample_i,   (int) (wh.DataChunkLength / 2)) {
                fread(&ret.Data[sample_i][0], sizeof(short), 1, file);
                ret.Data[sample_i][1] = ret.Data[sample_i][0];
            }
            ret.Frames = wh.DataChunkLength / 2; // 2 because of short
        }

        fclose(file);
    }

    return ret;
}
