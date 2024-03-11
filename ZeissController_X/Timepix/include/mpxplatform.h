#ifndef MPXPLATFORM_H
#define MPXPLATFORM_H


#if defined(_WINDLL) || defined(WIN32)
/* WINDOWS */
// Windowz duck tape and rope

typedef unsigned __int8     uint8_t;
typedef __int16             int16_t;

#if defined(MPXMODULE_EXPORT)
#define MPXMODULE_API __declspec(dllexport)
#else
#define MPXMODULE_API __declspec(dllimport)
#endif


#else
#include <stdint.h>

/* LINUX and APPLE */

#define MPXMODULE_API 

typedef int SOCKET;

#   define INVALID_SOCKET -1
#   define UNREFERENCED_PARAMETER(a)

#endif

#endif
