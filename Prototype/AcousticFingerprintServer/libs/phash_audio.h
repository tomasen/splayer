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

#ifndef PHASH_AUDIO_H
#define PHASH_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>



#if defined(win32)
#define SEPARATOR "\\"
#elif defined(unix)
#define SEPARATOR "/"
#else
#define SEPARATOR "/"
#endif

#if defined(BUILD_DLL)
#define PHASH_EXPORT __declspec(dllexport)
#elif defined(BUILD_EXE)
#define PHASH_EXPORT __declspec(dllimport)
#else
#define PHASH_EXPORT
#endif

/* macro to toggle bit position b in uint32 type */
#define TOGGLE_BIT(word,b)     (0x00000001 << b)^word


typedef struct hash_st_info {
  unsigned int sr;
  unsigned int framelength;
  double *window;
  double **wts;
} AudioHashStInfo;


#ifndef JUST_AUDIOHASH


/* type to use for an AudioIndex object to pass to functions */ 

typedef void* AudioIndex;

/* data to store in the table for each hash frame */ 

typedef struct table_val_t {
  uint32_t id; /*  id of the audio signal  */ 
  uint32_t pos; /* frame number position in stream  */ 
} TableValue;


/*  open_audioindex                                                                */
/*                                                                                 */
/*  open index by given path/filename and init with nb buckets                     */
/*                                                                                 */
/*  PARAMS idx_file - string for the name of the file                              */

/*         add      - int denoting whether adding to the index or querying         */

/*         nbbuckets- int value for number of buckets (only for first time)        */

/*  RETURN the AudioIndex ptr   (NULL on failure)                                  */ 

PHASH_EXPORT
AudioIndex open_audioindex(const char *idx_file, int add, int nbbuckets);

/* merge_audioindex */
/* merge the entries in src_idxfile into the entries in dst_idxfile*/
/* PARAMS dst_idxfile - the main index file                        */
/* PARAMS src_idxfile - the tmp file containing new entries        */
/* RETURN int - 0 on success, -1 on error                          */    

PHASH_EXPORT
int merge_audioindex(const char *dst_idxfile, const char *src_idxfile);

/* close_audioindex                                                        */
/*                                                                         */
/* close the audio index                                                   */ 

/* PARAMS audioindex - ptr to the index                                    */
/*        add - int denoting whether used for adding o querying must match */
/*                  the value used in opening                              */

/* RETURN int value (0 on success,  less than 0 on error)                  */

PHASH_EXPORT
int close_audioindex(AudioIndex audioindex, int add);



/* insert_into_audioindex                                     */
/*                                                            */
/* insert the hash from an audio unit into the index */
/*                                                            */
/* PARAMS audio_index - ptr to the audio index                */
/*        id          - id that is unique to the audio unit   */
/*        hash        - ptr to the hash array                 */
/*        nbframes    - length of the hash array              */ 
/* RETURN int  value (0 on success, less tahn 0 on error)     */ 

PHASH_EXPORT
int insert_into_audioindex(AudioIndex audio_index, uint32_t id, uint32_t *hash, int nbframes);



/* stat_audioindex                                                                      */
/*                                                                                      */
/* retrieve info on size of index table - number buckets and entries                    */
/*                                                                                      */
/* PARAMS audio_index - ptr to an opened index                                          */
/*        nbbuckets   - ptr to integer for function to fill in with nb buckets in table */
/*        nbentries   - ptr to integer for function to fill in with nb entries in table */
/*                      (both int ptr's can be null, in which case nothing returned     */ 
/* RETURN return 0                                                                      */

PHASH_EXPORT
int stat_audioindex(AudioIndex audio_index, int *nbbuckets, int *nbentries);



/* flush_audioindex                                                      */
/*                                                                       */
/* flush the audio index to storage                                      */
/*                                                                       */
/* PARAMS audio_index - ptr to opened audio index                        */
/*        filename - string holding the name of the file to write to     */
/* RETURN int value - 0 on success, less than 0 on error                 */

PHASH_EXPORT
int flush_audioindex(AudioIndex audio_index, const char *filename);



/* grow_audioindex                                                          */
/*                                                                          */
/* grow the index to the number entries                                     */
/*                                                                          */
/* PARAMS audio_index - ptr to an opened index                              */
/*        load        - float value load limit on table nbentries/nbbuckets */
/* RETURN int value - 0 for success, less than 0 on error                   */ 

PHASH_EXPORT
int grow_audioindex(AudioIndex audio_index, const float load);


/* readfilenames                                                                           */
/*                                                                                         */
/* read file names from given directory. The resulting array of strings                    */
/* contains all the entries in the given directory, even the subdirectories, except for    */
/* . and .. , since there appears to be no consistent  way to check for file types.        */
/*                                                                                         */
/* PARAMS dirname - char string with name of directory                                     */
/*        nbfiles - int ptr to be filled in by function with the number                    */
/*                  of files retrieved.                                                    */
/* RETURN char**  - ptr to nbfiles ptrs (char*), returns NULL on failure                   */


#endif /* JUST_AUDIOHASH */

PHASH_EXPORT
char** readfilenames(const char *dirname, unsigned int *nbfiles);


/* ph_free */
/* use to free any memory allocated by the library */
/* PARAMS ptr - void ptr to block of memory        */
/* RETURN void                                     */ 

PHASH_EXPORT
void ph_free(void *ptr);

/* ph_hashst_free */ 
/* free member variables allocated for audiohash function */
/* PARAMS ptr - pointer to AudioHashStInfo struct */
/* RETURN void */

PHASH_EXPORT
void ph_hashst_free(AudioHashStInfo *ptr);


/* audiohash                                                                                */ 
/*                                                                                          */
/* calculate audio hash from signal                                                         */
/*                                                                                          */
/* PARAMS buf - float array for signal                                                      */ 
/*        buflen - signal length                                                            */
/*        sr    - int val for sampling rate of signal (e.g. 44100)                          */
/*        phash - ptr to uint32 to which will be allocated the hash                         */
/*        coeffs - ptr to 2d double array  to which will be allocated the coeff's           */
/*                  for each frame. Returned array will be nbframes x nbcoeffs              */
/*                  Can be NULL                                                             */ 
/*        nbcoeffs - ptr to int to which will be assigned the number of coeffs calculated   */
/*                   for each frame. Can be NULL                                            */
/*        nbframes - ptr to int to which will be assigned the number of frames for signal   */
/*        P        - int val for number of bit positions for which to calculate the most    */
/*                   probably of being flipped by an similar signal. Set to 0 to disable    */
/*                   this feature                                                           */
/*        bit_toggles - 2d array of indices indicating the bit indices that are most likely */
/*                      to be flipped in any perceptually similar signal;                   */
/*                     will be size nbframesXP in size.  Pass in NULL value to disable.     */
/*        minB   - ptr to double which will be assigned the mininum coeff. May be NULL.     */
/*        maxB   - ptr to double which will be assigned the maximum coeff. May be NULL.     */
/*        hash_st - ptr to AudioHashStInfo holding state information for audio hash function */
/*                  Must not be null.                                                        */ 
/*                  (call ph_free_hashst function when done calling audiohash function      */ 
/* RETURN int value - 0 for success, less than 0 on failure.                                */ 


PHASH_EXPORT
int audiohash(float *buf, uint32_t **phash, double ***coeffs, uint8_t ***bit_toggles,\
		unsigned int *nbcoeffs, unsigned int *nbframes, double *minB, double *maxB,\
	      unsigned int buflen, unsigned int P, int sr, AudioHashStInfo *hash_st);

/* lookupaudiohash                                                                               */
/* PARAMS index_table - ptr to an opened index                                                   */
/*        hash        - ptr to an audio hash to look up                                          */
/*        toggles     - 2d array nbframesXP  of bit indices to toggle.  Increases number         */
/*                      of candidate hashes to look up. Calculated from audio hash function      */
/*        nbframes    - length of hash array                                                     */
/*        P           - number of bits to toggle in considering candidate hash values for lookup */
/*        blocksize   - segments of hash to evaluate at a time                                  */ 
/*        threshold   - confidence threshold representing the percentage of matches a retrieved */
/*                      id must meet before being considered a match                            */
/*        id          - ptr to id of audio track, filled in by function                         */
/*        cs          - ptr to confidence score, filled in by function                          */
/* RETURN int value - 0 on success, less than 0 on failure                                      */ 


#ifndef JUST_AUDIOHASH

PHASH_EXPORT
int lookupaudiohash(AudioIndex index_table, uint32_t *hash, uint8_t **toggles, int nbframes,\
                    int P, int blocksize, float threshold, uint32_t *id, float *cs);


#endif /* JUST_AUDIOHASH */

#ifdef __cplusplus
}
#endif

#endif /* PHASH_AUDIO_H */ 
