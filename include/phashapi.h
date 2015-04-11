#ifndef _PHASHAPI_H
#define _PHASHAPI_H

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <malloc.h>

#include "../include/stdint.h"

#define __STDC_CONSTANT_MACROS

using namespace std;

#define SQRT_TWO 1.4142135623730950488016887242097

#ifndef ULLONG_MAX
#define ULLONG_MAX 18446744073709551615ULL
#endif

#define ROUNDING_FACTOR(x) (((x) >= 0) ? 0.5 : -0.5)

#if defined( _MSC_VER) || defined(_BORLANDC_)
typedef unsigned __int64 ulong64;
typedef signed __int64 long64;
#else
typedef unsigned long long ulong64;
typedef signed long long long64;
#endif

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum ph_hashtype {
        BYTEARRAY   = 1,          /* refers to bitwidth of the hash value */
        UINT16ARRAY = 2,
        UINT32ARRAY = 4,
        UINT64ARRAY = 8,
    }HashType;

    /* structure for a single hash */
    typedef struct ph_datapoint
    {
        char *id;
        void *hash;
        float *path;
        uint32_t hash_length;
        uint8_t hash_type;
    } DP;

#define ROTATELEFT(x, bits)  (((x)<<(bits)) | ((x)>>(64-bits)))

// This function is just a test for this lib. It is nothing but a piece of rubbish.
int SampleAddInt(int i1, int i2);

/* /brief free memory space
 *
 * /param buf – memory for storing audio frames
 * /param hash – memory for hash value
 * /param cs – confidence score
 * /return nothing
 */
void ph_freemem_hash(float* buf, uint32_t* hash);
void ph_freemem_cs(double* cs);

/*  /brief count number of samples in file
 *
 *  /param filename - path and file name of audio file
 *  /param sr - sample rate conversion
 *  /param channels - channels number conversion
 *  /return int count of number of sampels, negative for error
*/

int ph_count_samples(const char *filename, int sr,int channels);


/* /brief read audio
 *
 * /param filename - path and name of audio file to read
 * /param sr - sample rate conversion
 * /param channels - nb channels to convert to (always 1) unused
 * /param buf - preallocated buffer
 * /param buflen - (in/out) param for buf length
 * /param nbsecs - float value for duration (in secs) to read from file
 * /return float* - float pointer to start of buffer - one channel of audio, NULL if error
 */
float* ph_readaudio(const char *filename, int sr, int channels, float *sigbuf, int &buflen, const float nbsecs = 0);

/* /brief audio hash calculation
 * purpose: hash calculation for each frame in the buffer.
 *          Each value is computed from successive overlapping frames of the input buffer.
 *          The value is based on the bark scale values of the frame fft spectrum. The value
 *          computed from temporal and spectral differences on the bark scale.
 *
 * /param buf - pointer to start of buffer
 * /param N   - length of buffer
 * /param sr  - sample rate on which to base the audiohash
 * /param nb_frames - (out) number of frames in audio buf and length of audiohash buffer returned
 * /return uint32 pointer to audio hash, NULL for error
*/
uint32_t* ph_audiohash(float *buf, int nbbuf, const int sr, int &nbframes);

DP **ph_audio_hashes(char *files[], int count, int sr = 8000, int channels = 1, int threads = 0);

/* /brief bit count set bits in 32bit variable
 * /param n
 * /return int number of bits set to 1, negative if error
 */
int ph_bitcount(uint32_t n);


/* /brief compare 2 hash blocks
 * /param ptr_blockA - pointer to the first block
 * /param ptr_blockB - pointer to the second block
 * /param block_size - length of both blocks to compare
 * /return double bit error rate (ber) from comparing two blocks, neg for error
 */
double ph_compare_blocks(const uint32_t *ptr_blockA,const uint32_t *ptr_blockB, const int block_size);


/* /brief distance function between two hashes
 *
 * /param hash_a - first hash
 * /param Na     - length of first hash
 * /param hash_b - second hash
 * /param Nb     - length of second hash
 * /param threshold - threshold value to compare successive blocks, 0.25, 0.30, 0.35
 * /param block_size - length of block_size, 256
 * /param Nc     - (out) length of confidence score vector
 * /return double - ptr to confidence score vector
 */
double* ph_audio_distance_ber(uint32_t *hash_a , const int Na, uint32_t *hash_b, const int Nb, const float threshold, const int block_size, int &Nc);


#ifdef __cplusplus
}
#endif

#endif

