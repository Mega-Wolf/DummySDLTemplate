#pragma once

#include "../code/macros.h"

// TODO(Tobi): Why does that file use those fancy methods; I think they are more performant

struct debug_read_file_result {
    char* Content;
    uint32 Size;
};

debug_read_file_result DebugPlatformReadEntireTextFile(char* fileName) {
    debug_read_file_result ret = {};

    FILE* file;
    fopen_s(&file, fileName,  "rb"); // NOTE(Tobi): That will otherwise mess up size stuff
    if (!file) { return ret; }

    fseek(file, 0, SEEK_END);
    ret.Size = ftell(file);
    fseek(file, 0, SEEK_SET);

    ret.Content = (char*) malloc(ret.Size + 1);
    fread(ret.Content, ret.Size, 1, file);

    ret.Content[ret.Size] = '\0';
    ++ret.Size;

    fclose(file);

    return ret;
}

// DEBUG_PLATFORM_FREE_FILE_MEMORY(DebugPlatformFreeFileMemory) {
//     UNUSED_PARAM(thread);
//     Assert(memory != nullptr);
//     VirtualFree(memory, 0, MEM_RELEASE);
// }

// TODO(Tobi): Leaks memory
// DEBUG_PLATFORM_READ_ENTIRE_FILE(DebugPlatformReadEntireFile) {
//     debug_read_file_result result = {};

//     HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
//     if (fileHandle != INVALID_HANDLE_VALUE) {
//         LARGE_INTEGER fileSize;
//         if (GetFileSizeEx(fileHandle, &fileSize)) {
//             // NOTE(Tobi): We only allow 4GB files at the moemnt
//             uint32 truncatedSize = SafeTruncateUInt32(fileSize.QuadPart);
//             result.Contents = VirtualAlloc(0, truncatedSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
//             if (result.Contents != nullptr) {
//                 DWORD bytesRead;
//                 if (ReadFile(fileHandle, result.Contents, truncatedSize, &bytesRead, 0) && truncatedSize == bytesRead) {
//                     // NOTE(Tobi): Successfull
//                     result.ContentsSize = truncatedSize;
//                 } else {
//                     DebugPlatformFreeFileMemory(thread, result.Contents);
//                     result.Contents = nullptr;
//                 }
//             }
//         }
//         CloseHandle(fileHandle);
//     }

//     return result;

//     // TODO(Tobi): Add some logging in all the elses
// }

// DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DebugPlatformWriteEntireFile) {
//     UNUSED_PARAM(thread);

//     bool32 ret = false;
//     HANDLE fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
//     if (fileHandle != INVALID_HANDLE_VALUE) {
//         DWORD bytesWritten;
//         if (WriteFile(fileHandle, memory, memorySize, &bytesWritten, 0) && memorySize == bytesWritten) {
//             // NOTE(Tobi): It worked
//             ret = true;
//         } else {
//             // TODO(Tobi): Logging
//         }
        
//         CloseHandle(fileHandle);
//     }

//     return ret;
//     // TODO(Tobi): Add some logging in all the elses
// }