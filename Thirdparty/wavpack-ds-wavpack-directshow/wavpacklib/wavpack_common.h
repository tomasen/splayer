// ----------------------------------------------------------------------------
// WavPack lib for Matroska
// ----------------------------------------------------------------------------
// Copyright christophe.paris@free.fr
// Parts by David Bryant http://www.wavpack.com
// Distributed under the BSD Software License
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#ifndef WAVPACK_COMMON_H_
#define WAVPACK_COMMON_H_
//-----------------------------------------------------------------------------

// All integers are little endian.

// or-values for "flags"

#define WV_BYTES_STORED    3    // 1-4 bytes/sample
#define WV_MONO_FLAG       4    // not stereo
#define WV_HYBRID_FLAG     8    // hybrid mode
#define WV_JOINT_STEREO    0x10 // joint stereo
#define WV_CROSS_DECORR    0x20 // no-delay cross decorrelation
#define WV_HYBRID_SHAPE    0x40 // noise shape (hybrid mode only)
#define WV_FLOAT_DATA      0x80 // ieee 32-bit floating point data

#define WV_INT32_DATA      0x100  // special extended int handling
#define WV_HYBRID_BITRATE  0x200  // bitrate noise (hybrid mode only)
#define WV_HYBRID_BALANCE  0x400  // balance noise (hybrid stereo mode only)

#define WV_INITIAL_BLOCK   0x800   // initial block of multichannel segment
#define WV_FINAL_BLOCK     0x1000  // final block of multichannel segment

#define WV_SHIFT_LSB      13
#define WV_SHIFT_MASK     (0x1fL << WV_SHIFT_LSB)

#define WV_MAG_LSB        18
#define WV_MAG_MASK       (0x1fL << WV_MAG_LSB)

#define WV_SRATE_LSB      23
#define WV_SRATE_MASK     (0xfL << WV_SRATE_LSB)

#define WV_IGNORED_FLAGS  0x18000000  // reserved, but ignore if encountered
#define WV_NEW_SHAPING    0x20000000  // use IIR filter for negative shaping
#define WV_UNKNOWN_FLAGS  0xC0000000  // also reserved, but refuse decode if
                                      //  encountered

//-----------------------------------------------------------------------------

#define constrain(x,y,z) (((y) < (x)) ? (x) : ((y) > (z)) ? (z) : (y))

//-----------------------------------------------------------------------------

#if defined(_WIN32)

// ----- WIN32 ----->
#include <windows.h>
#include <mmreg.h>
#define wp_alloc(__length) GlobalAlloc(GMEM_ZEROINIT, __length)
#define wp_realloc(__mem,__length) GlobalReAlloc(__mem,__length,GMEM_MOVEABLE)
#define wp_free(__dest) GlobalFree(__dest)
#define wp_memcpy(__buff1,__buff2,__length) CopyMemory(__buff1,__buff2,__length)
#define wp_memclear(__dest,__length) ZeroMemory(__dest,__length)
#define wp_memcmp(__buff1,__buff2,__length) memcmp(__buff1,__buff2,__length)

typedef unsigned __int64 uint64;

#ifdef _DEBUG
void DebugLog(const char *pFormat,...);
#else
#define DebugLog
#endif
// <----- WIN32 -----

#else

    // Add your os definition here
    
#endif

//-----------------------------------------------------------------------------
#endif WAVPACK_COMMON_H_
//-----------------------------------------------------------------------------
