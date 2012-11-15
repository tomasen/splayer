// ----------------------------------------------------------------------------
// WavPack lib for Matroska
// ----------------------------------------------------------------------------
// Copyright christophe.paris@free.fr
// Parts by David Bryant http://www.wavpack.com
// Distributed under the BSD Software License
// ----------------------------------------------------------------------------

#include "..\wavpack\wputils.h"
#include "wavpack_common.h"

// ----------------------------------------------------------------------------

#if defined(_WIN32)

#ifdef _DEBUG

#include <stdio.h>
#include <tchar.h>

void DebugLog(const char *pFormat,...) {
    char szInfo[2000];
    
    // Format the variable length parameter list
    va_list va;
    va_start(va, pFormat);
    
    _vstprintf(szInfo, pFormat, va);
    lstrcat(szInfo, TEXT("\r\n"));
    OutputDebugString(szInfo);
    
    va_end(va);
}

#endif

#endif

// ----------------------------------------------------------------------------