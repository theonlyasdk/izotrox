#pragma once

#ifdef __ANDROID__
    #define IF_ANDROID(...) __VA_ARGS__
    #define IF_DESKTOP(...)
#else
    #define IF_ANDROID(...)
    #define IF_DESKTOP(...) __VA_ARGS__
#endif