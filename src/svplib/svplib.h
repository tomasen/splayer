
#pragma once


#include "targetver.h"

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <atlcoll.h>

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>

#define SVP_MIN(a, b)  (((a) < (b)) ? (a) : (b)) 
#define SVP_MAX(a, b)  (((a) > (b)) ? (a) : (b)) 

#include <logging.h>

#ifndef SVP_LogMsg
#define SVP_LogMsg(x, ...)  Logging(x)
#endif

#ifndef SVP_LogMsg2
#define SVP_LogMsg2(...) Logging(__VA_ARGS__)
#endif

#ifndef SVP_LogMsg3
#define SVP_LogMsg3(...) Logging(__VA_ARGS__)
#endif

#ifndef SVP_LogMsg4
#define SVP_LogMsg4(...) Logging(__VA_ARGS__)
#endif

#ifndef SVP_LogMsg5
#define SVP_LogMsg5(...) Logging(__VA_ARGS__)
#endif

#ifndef SVP_LogMsg6
#define SVP_LogMsg6(...) Logging(__VA_ARGS__)
#endif

#ifdef SVP_DEBUG
#define SVP_ASSERT(x)       \
    if (!(x)) {             \
        __debugbreak();     \
    }
#else
#define SVP_ASSERT(x)
#endif

bool SVP_CanUseCoreAvcCUDA(bool useCUDA);
