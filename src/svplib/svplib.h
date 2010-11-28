
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

#define SVP_LogMsg(x, ...)  Logging(x)
#define SVP_LogMsg2(...) Logging(__VA_ARGS__)
#define SVP_LogMsg3(...) Logging(__VA_ARGS__)
#define SVP_LogMsg4(...) Logging(__VA_ARGS__)
#define SVP_LogMsg5(...) Logging(__VA_ARGS__)
#define SVP_LogMsg6(...) Logging(__VA_ARGS__)

bool SVP_CanUseCoreAvcCUDA(bool useCUDA);
