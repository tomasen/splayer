/*
    Audio Scout - audio content indexing software
    Copyright (C) 2010  D. Grant Starkweather & Evan Klinger
    
    Audio Scout is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    D. Grant Starkweather - dstarkweather@phash.org
    Evan Klinger          - eklinger@phash.org
*/

#ifndef _AUDIODATA_H
#define _AUDIODATA_H

#include <stdint.h>
#include "pHashAudioConfig.h"

#if defined(BUILD_DLL)
#define AUDIODATA_EXPORT __declspec(dllexport)
#elif defined(BUILD_EXE)
#define AUDIODATA_EXPORT __declspec(dllimport)
#else  
#define AUDIODATA_EXPORT
#endif


typedef void* AudioDataDB;

typedef struct  ametadata_t {
  char *composer;
  char *title1, *title2, *title3;
  char *tpe1, *tpe2, *tpe3, *tpe4;

  char *date;
  int year;
  char *album;
  char *genre;
  int duration;
  int partofset;

} AudioMetaData;

/* errors for readaudio function */
/* if value of error is < 1000, it is an error from mpg123 (look in mpg123.h for error code) */
/* if value is 0 or >= 1000, then follow this enum */ 

enum ph_phashaudio_error {
  PHERR_SUCCESS = 0, 
  PHERR_NULLARG = 1000,
  PHERR_NOBUF =1001,
  PHERR_BADSR = 1002,
  PHERR_NOBUFALLOCD = 1003,
  PHERR_SRCCONTXT = 1004,
  PHERR_SRCPROC = 1005,
  PHERR_SNDFILEOPEN = 1006,
  PHERR_MEMALLOC = 1007,
  PHERR_NOSAMPLES = 1008,
  PHERR_NOFORMAT = 1009,
  PHERR_NOENCODING = 1010,
  PHERR_MP3NEW = 1011,
};
/**
 * init_mdata
 * initialize AudioMetaData struct to all zeros
 **/ 

AUDIODATA_EXPORT
void init_mdata(AudioMetaData *mdata);

/**
 * free_mdata
 * release all memory allocated to fields and zero out.
 **/ 

AUDIODATA_EXPORT
void free_mdata(AudioMetaData *mdata);

/**
 * readaudio function
 * read audio samples into buffer 
 * PARAM filename - char string of filename to read 
 * PARAM sr - int value of desired sample rate of signal
 * PARAM sigbuf - buffer for the signal (can be null)
 * PARAM buflen - length of sigbuf buffer,  will be changed to the length of the returned buffer
 * PARAM nbsecs - float for number of seconds to take from file - use 0.0 for the whole file.
 * PARAM mdata - ptr to AudioMetaData struct to be filled in by function, can be NULL.
 * PARAM error - ptr to int value of error code (0 for success)
 * RETURN float buffer for signal, NULL if error
 **/

AUDIODATA_EXPORT
float* readaudio(const char *filename, const int sr, float *sigbuf, unsigned int *buflen,\
		 const float nbsecs, AudioMetaData *mdata, int *error);

/**
 * open_audiodata_db
 * open the connection
 * PARAM context - a context for zeromq
 * PARAM addr - string constant denoting the network address of the database connection
 * RETURN - AudioDataDB type (int file descriptor of file )
 **/
AUDIODATA_EXPORT
AudioDataDB open_audiodata_db(void *ctx, const char *addr);


/**
 * close_audiodata_db - closes connection/file
 * PARAM mdatastore - ptr to metadata store (int file descriptor ) 
 * RETURN 0 for success, neg for error
 **/
AUDIODATA_EXPORT
int close_audiodata_db(AudioDataDB mdatastore);

/**
 * store_audiodata func
 * PARAM mdatastore - ptr to AudioDataDB (int file descriptor ) 
 * PARAM mdata_inline - char string (null terminated) 
 * PARAM id - ptr to uint32_t for unique id to be returned (can be NULL, but then no id will be returend
 * /RETURN int value - 0 for success, less than 0 for error 
 **/
AUDIODATA_EXPORT
int store_audiodata(AudioDataDB mdatastore, char *mdata_inline, uint32_t *id);

/**
 * retrieve_audiodata
 * PARAM mdatastore - ptr to AudioDataDB, or int file descriptor
 * PARAM id - unique id for the metadata to retrieve
 * RETURN - char* - null-terminated string containing the metadata information (NULL for error)
 **/
AUDIODATA_EXPORT
char* retrieve_audiodata(AudioDataDB mdatastore, uint32_t id);

AUDIODATA_EXPORT
int metadata_to_inlinestr(AudioMetaData *mdata, char *str, int len);

#endif
