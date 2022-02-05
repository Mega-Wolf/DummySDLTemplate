#pragma once

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
// #include <signal.h>
#include <string.h>
// #include <cstdint>
#include <type_traits>

#define inc(varname, start, max) for (int varname = start; varname < max; ++varname)
#define inc0(varname, max) for (int varname = 0; varname < max; ++varname)
#define incIn(varname, start, max) for (int varname = start; varname <= max; ++varname)
#define inc0In(varname, max) for (int varname = 0; varname <= max; ++varname)

#define dec(variable, start, min_comp) for (int variable = (start); --variable >= (min_comp); )
#define dec0(variable, start) for (int variable = (start); --variable >= 0; )
#define decIn(variable, start, min_comp) for (int variable = (start); variable >= (min_comp); --variable)
#define dec0In(variable, start) for (int variable = (start); variable >= 0; --variable)

#define IS_KEY_PRESSED(key) ins->Keyboard.key.Down && ins->Keyboard.key.Toggled
#define IS_MOUSE_PRESSED(key) ins->Mouse.key.Down && ins->Mouse.key.Toggled
#define IS_MOUSE_RELEASED(key) !ins->Mouse.key.Down && ins->Mouse.key.Toggled

typedef unsigned int uint32;


#define PI 3.141592f

#define Throw() \
{ \
    char* ptr = nullptr; \
    *ptr = 'X'; \
}

#if NDEBUG
inline void Assert(bool condition, char* errorMsg, ...) { __assume(condition); }
#else
inline void Assert(bool condition, char* errorMsg, ...) {
    if (!condition) {
        va_list args;
        va_start(args, errorMsg);
        fprintf(stderr, "Wrong assertion error:\n\t");
        vfprintf(stderr, errorMsg, args);
        fprintf(stderr, "\n");
        va_end(args);
        Throw();
    }
}
#endif

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

//#define       all(counterVariable, typeVariable, array) for (int counterVariable = 0; counterVariable < ArrayCount(array); ++counterVariable) for (auto* typeVariable = &array[counterVariable]; typeVariable; typeVariable = nullptr)
    //Assert(array, "Array is null");
#define all(counterVariable, typeVariable, array) \
    Assert(array, "Array is null"); \
    for (int counterVariable = 0; counterVariable < SIZE(array); ++counterVariable) \
        if (auto* typeVariable = &array[counterVariable])
    
// #define all_count(counterVariable, typeVariable, array, count) for (int counterVariable = 0; counterVariable < (count);           ++counterVariable) for (auto* typeVariable = &array[counterVariable]; typeVariable; typeVariable = nullptr)
#define all_count(counterVariable, typeVariable, array, count) \
    Assert(array, "Array is null"); \
    for (int counterVariable = 0; counterVariable < (count); ++counterVariable) \
        if (auto* typeVariable = &array[counterVariable])

#pragma region [Enum magic]

template <typename T = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator~(T lhs) {
    return static_cast<T>(
        ~static_cast<typename std::underlying_type<T>::type>(lhs)
    );
}

template <typename T = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator|(T lhs, T rhs) {
    return static_cast<T>(
        static_cast<typename std::underlying_type<T>::type>(lhs) |
        static_cast<typename std::underlying_type<T>::type>(rhs)
    );
}

template <typename T = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator&(T lhs, T rhs) {
    return static_cast<T>(
        static_cast<typename std::underlying_type<T>::type>(lhs) &
        static_cast<typename std::underlying_type<T>::type>(rhs)
    );
}

template <typename T = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator^(T lhs, T rhs) {
    return static_cast<T>(
        static_cast<typename std::underlying_type<T>::type>(lhs) ^
        static_cast<typename std::underlying_type<T>::type>(rhs)
    );
}

template <typename T = typename std::enable_if<std::is_enum<T>::value, T>::type>
void operator|=(T& lhs, T rhs) {
    lhs = static_cast<T>(
        static_cast<typename std::underlying_type<T>::type>(lhs) |
        static_cast<typename std::underlying_type<T>::type>(rhs)
    );
}

template <typename T = typename std::enable_if<std::is_enum<T>::value, T>::type>
void operator&=(T& lhs, T rhs) {
    lhs = static_cast<T>(
        static_cast<typename std::underlying_type<T>::type>(lhs) &
        static_cast<typename std::underlying_type<T>::type>(rhs)
    );
}

template <typename T = typename std::enable_if<std::is_enum<T>::value, T>::type>
void operator^=(T& lhs, T rhs) {
    lhs = static_cast<T>(
        static_cast<typename std::underlying_type<T>::type>(lhs) ^
        static_cast<typename std::underlying_type<T>::type>(rhs)
    );
}

// template <typename T = typename std::enable_if<std::is_enum<T>::value, T>::type>
// T& operator++(T& lhs) {
//     lhs = static_cast<T>(
//         ++static_cast<typename std::underlying_type<T>::type>(lhs)
//     );
//     return lhs;
// }

// template <typename T = typename std::enable_if<std::is_enum<T>::value, T>::type>
// T& operator--(T& lhs) {
//     lhs = static_cast<T>(
//         --static_cast<typename std::underlying_type<T>::type>(lhs)
//     );
//     return lhs;
// }

// template <typename T = typename std::enable_if<std::is_enum<T>::value, T>::type>
// T operator++(T& lhs, int) {
//     lhs = static_cast<T>(
//         ++static_cast<typename std::underlying_type<T>::type>(lhs)
//     );
// }

// template <typename T = typename std::enable_if<std::is_enum<T>::value, T>::type>
// T operator--(T& lhs, int) {
//     lhs = static_cast<T>(
//         --static_cast<typename std::underlying_type<T>::type>(lhs)
//     );
// }

#pragma endregion
