#ifndef ASI_TYPES_Hxx
#define ASI_TYPES_Hxx

#if defined(_WINDLL) || defined(WIN32)

typedef __int8             i8;
typedef unsigned __int8    u8;
typedef __int16            i16;
typedef unsigned __int16   u16;
typedef __int32            i32;
typedef unsigned __int32   u32;
typedef __int64            i64;
typedef unsigned __int64   u64;

#else // Linux

#include <inttypes.h>

typedef int8_t             i8;
typedef uint8_t            u8;
typedef int16_t            i16;
typedef uint16_t           u16;
typedef int32_t            i32;
typedef uint32_t           u32;
typedef int64_t            i64;
typedef uint64_t           u64;

#endif

typedef int                BOOL;
typedef unsigned char      byte;
typedef unsigned short     DACTYPE; // type for DAC value

#endif