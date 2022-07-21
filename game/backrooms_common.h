#pragma once

typedef char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
typedef long long i64;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;

#define CODE_BLOCK(block)

#if defined(_WIN32) 
    #define BACKROOMS_WINDOWS
#elif defined(__APPLE__)
    #error "MacOS is not supported!"
#elif defined(__linux__)
    #error "Linux is not supported!"
#endif 