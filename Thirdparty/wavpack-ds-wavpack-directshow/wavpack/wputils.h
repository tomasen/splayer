////////////////////////////////////////////////////////////////////////////
//                           **** WAVPACK ****                            //
//                  Hybrid Lossless Wavefile Compressor                   //
//              Copyright (c) 1998 - 2005 Conifer Software.               //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// wputils.h

#ifndef WPUTILS_H
#define WPUTILS_H

#include <sys/types.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include <stdlib.h>
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;
typedef __int64 int64_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
typedef __int8  int8_t;
typedef float float32_t;
#else
#include <inttypes.h>
#endif

typedef unsigned char	uchar;

#if !defined(__GNUC__) || defined(WIN32)
typedef unsigned short	ushort;
typedef unsigned int	uint;
#endif

// This header file contains all the definitions required to use the
// functions in "wputils.c" to read and write WavPack files and streams.
#include "wavpack_local.h"

typedef struct {
  int32_t (*read_bytes)(void *id, void *data, int32_t bcount);
  uint32_t (*get_pos)(void *id);
  int (*set_pos_abs)(void *id, uint32_t pos);
  int (*set_pos_rel)(void *id, int32_t delta, int mode);
  int (*push_back_byte)(void *id, int c);
  uint32_t (*get_length)(void *id);
  int (*can_seek)(void *id);
} stream_reader;


#endif
