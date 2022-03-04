// IMPORTANT(Tobi): The profiler depends on the OS version. However, I did not want to root the
// calls from here through the platform layer, since this adds an additional  function call,
// which might be bad in tight loops (although there, I probably shouldn't profile anyway)
// Looking at it again, if I #ifdef out the profiling; I can now ignroe th function call altogether, which would have not been possible in the other way

// TODO(Tobi): Implement the Unix versions

// TODO(Tobi): This profiling tool is not the best; and one has to be careful not to make an error
// One could probably actually use a C++ profiling "object" that gets destroyed when going out of scope so one can avoid the loop hassle and the forgotten PRE_RETURNS
// Also, this could even allow meta-programming stuff where I insert the object directly after the function header gets written

#pragma once

#include "../code/macros.h"

struct profiler_info {
    char* Name;
    double Time;
    int Calls;
    double Max;
};

//MakeList(ProfilerInfo*, profiler_info)

// TODO(Tobi): For now, this gets deleted when reloading the game, which might be what I want
struct global_profiler_info {
    double InverseFrequency;
    long long RecordedDummyTimestamp;

    profiler_info Data[16];
    int Count;
};

static global_profiler_info GlobalProfilerInfo;

void ProfilerInit() {
    #ifndef NPROFILE
    long long dummyFreq;
    QueryPerformanceFrequency((LARGE_INTEGER *)&dummyFreq);
    GlobalProfilerInfo.InverseFrequency = (1.0 / dummyFreq);
    #endif
}

void ProfilerClearData() {
    #ifndef NPROFILE
    inc0 (profilerInfo_i,   GlobalProfilerInfo.Count) {
        GlobalProfilerInfo.Data[profilerInfo_i].Calls = 0;
        GlobalProfilerInfo.Data[profilerInfo_i].Max = 0;
        GlobalProfilerInfo.Data[profilerInfo_i].Time = 0;
    }
    #endif
}

void ProfilerShowRecorededData(char* buffer, size_t bufferCount, int x = 0) {
    #ifndef NPROFILE
    size_t bufferAmount = 0;

    bufferAmount += snprintf(&buffer[bufferAmount], bufferCount - bufferAmount, "Profiled data:\n");
    inc0 (profilerInfo_i,   GlobalProfilerInfo.Count) {
        profiler_info* profilerInfo = &GlobalProfilerInfo.Data[profilerInfo_i];
        int divider = (x != 0) ? x : profilerInfo->Calls;
        if (profilerInfo->Calls == 0) {
            bufferAmount += snprintf(&buffer[bufferAmount], bufferCount - bufferAmount, "%s has not been called\n", profilerInfo->Name);
        } else {
            bufferAmount += snprintf(&buffer[bufferAmount], bufferCount - bufferAmount, "%-30.30s: (%d)   %f  --> Avg: %f\n", profilerInfo->Name, profilerInfo->Calls, profilerInfo->Time, profilerInfo->Time / divider);
        }
    }
    #endif
}

// Simple functions for profiling something one-self

inline long long ProfilerGetTimestamp() {
    long long ret;
    QueryPerformanceCounter((LARGE_INTEGER *) &ret);
    return ret;
}

inline double ProfilerElapsedSeconds(long long start, long long end) {
    return ((end - start) * GlobalProfilerInfo.InverseFrequency);
}

// Even simpler API; one doesn't even have to store the start time one-self

inline void ProfilerStart() {
    GlobalProfilerInfo.RecordedDummyTimestamp = ProfilerGetTimestamp();
}

inline double ProfilerEnd() {
    long long end = ProfilerGetTimestamp();
    return ProfilerElapsedSeconds(GlobalProfilerInfo.RecordedDummyTimestamp, end);
}

inline double ProfielrEndOverwrite() {
    long long end = ProfilerGetTimestamp();
    double ret = ProfilerElapsedSeconds(GlobalProfilerInfo.RecordedDummyTimestamp, end);
    GlobalProfilerInfo.RecordedDummyTimestamp = end;
    return ret;
}

// The things below are the full crazy profiling things

#if defined(NPROFILE)
#define PROFILER_WATCH_FUNCTION(name)
#define PROFILER_PRE_RETURN
#else
#define PROFILER_WATCH_FUNCTION(name)                                   \
    static profiler_info* pf = nullptr;                                  \
    if (!pf) {                                                          \
        pf = &GlobalProfilerInfo.Data[GlobalProfilerInfo.Count++];   \
        Assert(GlobalProfilerInfo.Count < ArrayCount(GlobalProfilerInfo.Data), "Too many profiling data sets"); \
        *pf = {};                                                       \
        pf->Name = name;                                                \
    }                                                                   \
	long long startTime;                                                    \
	long long endTime;                                                      \
	double timeDifference;                                              \
    ++pf->Calls;                                                        \
	QueryPerformanceCounter((LARGE_INTEGER *)&startTime);               \
    defer(, (QueryPerformanceCounter((LARGE_INTEGER *)&endTime), timeDifference = ((endTime - startTime) * GlobalProfilerInfo.InverseFrequency), pf->Time += timeDifference, pf->Max = MAX(pf->Max, timeDifference)))

#define PROFILER_WATCH_FUNCTION_IDS(name, id_ID, MAX_ID)                    \
    static profiler_info* pf[MAX_ID] = {}                                    \
    if (!pf[id_ID]) {                                                       \
        pf[id_ID] = &GlobalProfilerInfo.Data[GlobalProfilerInfo.Count++];\
        Assert(GlobalProfilerInfo.Count < ArrayCount(GlobalProfilerInfo.Data), "Too many profiling data sets"); \
        *pf[id_ID] = {};                                                    \
        pf[id_ID]->Name = name;                                             \
    }                                                                       \
	long long startTime;                                                        \
	long long endTime;                                                          \
	double timeDifference;                                                  \
    ++pf[id_ID]->Calls;                                                     \
	QueryPerformanceCounter((LARGE_INTEGER *)&startTime);                   \
    defer(, (QueryPerformanceCounter((LARGE_INTEGER *)&endTime), timeDifference = ((endTime - startTime) * GlobalProfilerInfo.InverseFrequency), pf[id_ID]->Time += timeDifference, pf[id_ID]->Max = MAX(pf[id_ID]->Max, timeDifference)))

#define PROFILER_PRE_RETURN \
    QueryPerformanceCounter((LARGE_INTEGER *)&endTime); \
    timeDifference = ((endTime - startTime) * GlobalProfilerInfo.InverseFrequency); \
    pf->Time += timeDifference; \
    pf->Max = MAX(pf->Max, timeDifference);
#endif
