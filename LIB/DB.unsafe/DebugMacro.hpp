
#ifndef __AT_DEBUG_MACRO_HPP__
#define __AT_DEBUG_MACRO_HPP__

#include <cstdio>

#ifdef _DEBUG_MACRO
    #define DEBUG_PRINT(fmt, args...) \
        do { printf("%-16s:%-4d|" fmt, __FILE__, __LINE__, ##args); } while(0)
#else
    #define DEBUG_PRINT(fmt, args...)
#endif

#endif // __AT_DEBUG_MACRO_HPP__