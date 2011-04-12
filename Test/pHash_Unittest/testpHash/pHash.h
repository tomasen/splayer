/*

    pHash, the open source perceptual hash library
    Copyright (C) 2008-2009 Aetilius, Inc.
    All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Evan Klinger - eklinger@phash.org
    D Grant Starkweather - dstarkweather@phash.org

*/

#ifndef _PHASH_H
#define _PHASH_H

//#define cimg_debug 0
//#define cimg_display 0

#include "pHash-config.h"
#include <limits.h>
#include <math.h>
//#include <dirent.h>
#include "unistd.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <malloc.h>

#define __STDC_CONSTANT_MACROS

//#include <stdint.h>
//#include "CImg.h"

#ifdef HAVE_PTHREAD
//#include <pthread.h>
#endif

#ifndef __GLIBC__
//#include <sys/param.h>
//#include <sys/sysctl.h>
#endif

//using namespace cimg_library;
using namespace std;

#define SQRT_TWO 1.4142135623730950488016887242097

#ifndef ULLONG_MAX
#define ULLONG_MAX 18446744073709551615ULL
#endif

#define ROUNDING_FACTOR(x) (((x) >= 0) ? 0.5 : -0.5)

#if defined( _MSC_VER) || defined(_BORLANDC_)
typedef unsigned __int64 ulong64;
typedef signed __int64 long64;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed int int16_t;
typedef unsigned int uint16_t;
typedef signed long int int32_t;
typedef unsigned long int uint32_t;
typedef signed long long int int64_t;
typedef unsigned long long int uint64_t;
#else
typedef unsigned long long ulong64;
typedef signed long long long64;
#endif

#ifdef __cplusplus
extern "C" {
#endif

//const int MaxFileSize = (1<<30); /* 1GB file size limit (for mvp files) */
//const off_t HeaderSize = 64;     /* header size for mvp file */

//const char mvptag[] = "pHashMVPfile2009";

//typedef enum ph_mvp_retcode {
//    PH_SUCCESS = 0,   /* success */
//    PH_ERRPGSIZE,     /* page size error */
//    PH_ERRSMPGSIZE,
//    PH_ERRFILEOPEN,       /* file open err */
//    PH_ERRFILETRUNC,
//    PH_ERRFILESEEK,
//    PH_ERRMMAP,        /* mmap'ing error */
//    PH_ERRMSYNC,       /* msync error */
//    PH_ERRTRUNC,       /* error truncating file */
//    PH_ERRSAVEMVP,      /* could not save mvp file */
//    PH_ERRARG,   /* general arg err*/
//    PH_ERRNULLARG,   /* null arg */
//    PH_ERRMEM,       /* general memory error  */
//    PH_ERRMEMALLOC,  /* mem alloc error */
//    PH_ERRNTYPE,      /* unrecognized node type */
//    PH_ERRCAP,     /* more results found than can be supported in ret array */
//    PH_ERRFILETYPE,  /*unrecognized file type  */
//    PH_ERRDIST,      /*not enough variance in distances */
//}MVPRetCode;


typedef enum ph_hashtype {
    BYTEARRAY   = 1,          /* refers to bitwidth of the hash value */
    UINT16ARRAY = 2,
    UINT32ARRAY = 4,
    UINT64ARRAY = 8,
}HashType;

//typedef struct ph_file_offset {
//    off_t offset;
//    uint8_t fileno;
//} FileIndex;


/* structure for a single hash */
typedef struct ph_datapoint {
    char *id;
    void *hash;
    float *path;
    uint32_t hash_length;
    uint8_t hash_type;
}DP;

//typedef struct ph_slice {
//    DP **hash_p;
//    int n;
//    void *hash_params;
//} slice;

/* call back function for mvp tree functions - to performa distance calc.'s*/
//typedef float (*hash_compareCB)(DP *pointA, DP *pointB);

//typedef struct ph_mvp_file {
//    char *filename;   /* name of db to use */
//    char *buf;
//    off_t file_pos;
//    int fd;
//    uint8_t filenumber;
//    uint8_t nbdbfiles;
//    uint8_t branchfactor; /*branch factor of tree, M(=2)*/

    /*length of path to store distances from vantage points in the struct data_point.
      used when querying or constructing the tree, P(=5) */
//    uint8_t pathlength;

//    uint8_t leafcapacity; /*maximum number of data points to a leaf, K(=25) */

    /*size of page size to store a leaf of the tree structure.
      Must be >= system page size.  Might need to increase above
      the system page size to fit all the data points in a leaf.
    */
 //   off_t pgsize;
 //   HashType hash_type;

    /*callback function to use to calculate the distance between 2 datapoints */
 //   hash_compareCB hashdist;

//} MVPFile ;


/* convenience function to set var's of mvp tree */
//void ph_mvp_init(MVPFile *m);

/*! /brief Radon Projection info
 */
//typedef struct ph_projections {
//    CImg<uint8_t> *R;           //contains projections of image of angled lines through center
//    int *nb_pix_perline;        //the head of int array denoting the number of pixels of each line
//    int size;                   //the size of nb_pix_perline
//}Projections;

/*! /brief feature vector info
 */
//typedef struct ph_feature_vector {
//    double *features;           //the head of the feature array of double's
//    int size;                   //the size of the feature array
//}Features;

/*! /brief Digest info
 */
//typedef struct ph_digest {
//    char *id;                   //hash id
//    uint8_t *coeffs;            //the head of the digest integer coefficient array
//    int size;                   //the size of the coeff array
//} Digest;


/* variables for textual hash */
//const int KgramLength = 50;
//const int WindowLength = 100;
//const int delta = 1;

#define ROTATELEFT(x, bits)  (((x)<<(bits)) | ((x)>>(64-bits)))

//typedef struct ph_hash_point {
//    ulong64 hash;
//    off_t index; /*pos of hash in orig file */
//} TxtHashPoint;

//typedef struct ph_match{
//    off_t first_index; /* offset into first file */
//    off_t second_index; /* offset into second file */
//    uint32_t length;    /*length of match between 2 files */
//} TxtMatch;

#ifdef HAVE_PTHREAD
int ph_num_threads();
#endif

/* /brief alloc a single data point
 *  allocates path array, does nto set id or path
 */
 //DP* ph_malloc_datapoint(int hashtype);

/** /brief free a datapoint and its path
 *
 */
//void ph_free_datapoint(DP *dp);

/*! /brief copyright information
 */
const char* ph_about();

/*! /brief radon function
 *  Find radon projections of N lines running through the image center for lines angled 0
 *  to 180 degrees from horizontal.
 *  /param img - CImg src image
 *  /param  N  - int number of angled lines to consider.
 *  /param  projs - (out) Projections struct
 *  /return int value - less than 0 for error
 */
//int ph_radon_projections(const CImg<uint8_t> &img,int N,Projections &projs);

/*! /brief feature vector
 *         compute the feature vector from a radon projection map.
 *  /param  projs - Projections struct
 *  /param  fv    - (out) Features struct
 *  /return int value - less than 0 for error
*/
//int ph_feature_vector(const Projections &projs,Features &fv);

/*! /brief dct
 *  Compute the dct of a given vector
 *  /param R - vector of input series
 *  /param D - (out) the dct of R
 *  /return  int value - less than 0 for error
*/
//int ph_dct(const Features &fv, Digest &digest);

/*! /brief cross correlation for 2 series
 *  Compute the cross correlation of two series vectors
 *  /param x - Digest struct
 *  /param y - Digest struct
 *  /param pcc - double value the peak of cross correlation
 *  /param threshold - double value for the threshold value for which 2 images
 *                     are considered the same or different.
 *  /return - int value - 1 (true) for same, 0 (false) for different, < 0 for error
 */

//int ph_crosscorr(const Digest &x,const Digest &y,double &pcc, double threshold = 0.90);

/*! /brief image digest
 *  Compute the image digest for an image given the input image
 *  /param img - CImg object representing an input image
 *  /param sigma - double value for the deviation for a gaussian filter function
 *  /param gamma - double value for gamma correction on the input image
 *  /param digest - (out) Digest struct
 *  /param N      - int value for the number of angles to consider.
 *  /return       - less than 0 for error
 */
//int _ph_image_digest(const CImg<uint8_t> &img,double sigma, double gamma,Digest &digest,int N=180);

/*! /brief image digest
 *  Compute the image digest given the file name.
 *  /param file - string value for file name of input image.
 *  /param sigma - double value for the deviation for gaussian filter
 *  /param gamma - double value for gamma correction on the input image.
 *  /param digest - Digest struct
 *  /param N      - int value for number of angles to consider
 */
//int ph_image_digest(const char *file, double sigma, double gamma, Digest &digest,int N=180);


/*! /brief compare 2 images
 *  /param imA - CImg object of first image
 *  /param imB - CImg object of second image
 *  /param pcc   - (out) double value for peak of cross correlation
 *  /param sigma - double value for the deviation of gaussian filter
 *  /param gamma - double value for gamma correction of images
 *  /param N     - int number for the number of angles of radon projections
 *  /param theshold - double value for the threshold
 *  /return int 0 (false) for different images, 1 (true) for same image, less than 0 for error
 */
//int _ph_compare_images(const CImg<uint8_t> &imA,const CImg<uint8_t> &imB,double &pcc, double sigma = 3.5, double gamma = 1.0,int N=180,double threshold=0.90);

/*! /brief compare 2 images
 *  Compare 2 images given the file names
 *  /param file1 - char string of first image file
 *  /param file2 - char string of second image file
 *  /param pcc   - (out) double value for peak of cross correlation
 *  /param sigma - double value for deviation of gaussian filter
 *  /param gamma - double value for gamma correction of images
 *  /param N     - int number for number of angles
 *  /return int 0 (false) for different image, 1 (true) for same images, less than 0 for error
 */
//int ph_compare_images(const char *file1, const char *file2,double &pcc, double sigma = 3.5, double gamma=1.0, int N=180,double threshold=0.90);

/*! /brief return dct matrix, C
 *  Return DCT matrix of sqare size, N
 *  /param N - int denoting the size of the square matrix to create.
 *  /return CImg<double> size NxN containing the dct matrix
 */
//static CImg<float>* ph_dct_matrix(const int N);

/*! /brief compute dct robust image hash
 *  /param file string variable for name of file
 *  /param hash of type ulong64 (must be 64-bit variable)
 *  /return int value - -1 for failure, 1 for success
 */
//int ph_dct_imagehash(const char* file,ulong64 &hash);

#ifdef HAVE_PTHREAD
//DP** ph_dct_image_hashes(char *files[], int count, int threads = 0);
#endif
//static CImgList<uint8_t>* ph_getKeyFramesFromVideo(const char *filename);

//ulong64* ph_dct_videohash(const char *filename, int &Length);

// DP** ph_dct_video_hashes(char *files[], int count, int threads = 0);

// double ph_dct_videohash_dist(ulong64 *hashA, int N1, ulong64 *hashB, int N2, int threshold=21);

/* ! /brief dct video robust hash
 *   Compute video hash based on the dct of normalized video 32x32x64 cube
 *   /param file name of file
 *   /param hash ulong64 value for hash value
 *   /return int value - less than 0 for error
 */
//int ph_hamming_distance(const ulong64 hash1,const ulong64 hash2);

/** /brief create a list of datapoint's directly from a directory of image files
 *  /param dirname - path and name of directory containg all image file names
 *  /param capacity - int value for upper limit on number of hashes
 *  /param count - number of hashes created (out param)
 *  /return pointer to a list of DP pointers (NULL for error)
 */

// DP** ph_read_imagehashes(const char *dirname,int capacity, int &count);

/** /brief create MH image hash for filename image
*   /param filename - string name of image file
*   /param N - (out) int value for length of image hash returned
*   /param alpha - int scale factor for marr wavelet (default=2)
*   /param lvl   - int level of scale factor (default = 1)
*   /return uint8_t array
**/
// uint8_t* ph_mh_imagehash(const char *filename, int &N, float alpha=2.0f, float lvl = 1.0f);

/** /brief count number bits set in given byte
*   /param val - uint8_t byte value
*   /return int value for number of bits set
**/
//int ph_bitcount8(uint8_t val);

/** /brief compute hamming distance between two byte arrays
 *  /param hashA - byte array for first hash
 *  /param lenA - int length of hashA
 *  /param hashB - byte array for second hash
 *  /param lenB - int length of hashB
 *  /return double value for normalized hamming distance
 **/
//double ph_hammingdistance2(uint8_t *hashA, int lenA, uint8_t *hashB, int lenB);

/** /brief get all the filenames in specified directory
 *  /param dirname - string value for path and filename
 *  /param cap - int value for upper limit to number of files
 *  /param count - int value for number of file names returned
 *  /return array of pointers to string file names (NULL for error)
 **/

//char** ph_readfilenames(const char *dirname,int &count);

/** /brief read a datapoint (aux function)
 *   /param m current MVPFile struct containing state information
 *   /return DP* the read datapoint struct
 **/
//DP* ph_read_datapoint(MVPFile *m);

/** /brief get size of a datapoint in bytes (aux. function)
 *  /param m MVPFile struct
 *  /param dp DP struct
 *  /return int number of bytes the datapoint will use in the file.
 **/
//int ph_sizeof_dp(DP *dp,MVPFile *m);

/**  /brief save datapoint to file (aux. function)
 *   /param new_dp - DP struct of dp to be saved.
 *   /param m - MVPFil
 *   /return off_t file offset of newly written dp.
 **/
//off_t ph_save_datapoint(DP *new_dp, MVPFile *m);

/** /brief mmap memory to filenumber/offset
 *  /param filenumber - uint8_t number of file to map
 *  /param offset - off_t offset into new file
 *  /param m - MVPFile
 *  /return MVPFile - ptr to new struct containing the mmap info
 **/
//MVPRetCode _ph_map_mvpfile(uint8_t filenumber, off_t offset, MVPFile *m, MVPFile *m2);

/** /brief unmap/map from m2 to m
 *  /param filenumber - uint8_t filenumber of m2
 *  /param orig_pos   = off_t offset into original file in m.
 *  /return void
 **/
//MVPRetCode _ph_unmap_mvpfile(uint8_t filenumber, off_t orig_pos, MVPFile *m, MVPFile *m2);

/**
 * callback function for dct image hash use in mvptree structure.
 */
//float hammingdistance(DP *pntA, DP *pntB);

/** /brief aux function to query
 *  /param m - MVPFile state information
 *  /param query - DP of datapoint to query
 *  /param knearest - int capacity of results array.
 *  /param radius - float value of radius of values to consider
 *  /param results - DP array of points of result
 *  /param nbfound - int number of results found (out)
 *  /param level - int value to track recursion depth.
 *  /return MVPRetCode
**/
//static MVPRetCode _ph_query_mvptree(MVPFile *m, DP *query, int knearest, float radius,
//                float threshold, DP **results, int &nbfound, int level);

/**  /brief query mvptree function
 *   /param m - MVPFile file state info
 *   /param query - DP* item to query for
 *   /param knearest - int capacity of results array
 *   /param radius  - float radius to consider in query
 *   /param results - DP** list of pointers to results found
 *   /param nbfound -  int number of results found (out)
 **/
//MVPRetCode ph_query_mvptree(MVPFile *m, DP *query, int knearest, float radius,
//		float threshold,   DP **results, int &nbfound);

/** /brief save dp points to a file (aux func)
 *  /param m - MVPFile state information of file
 *  /param points - DP** list of points to add
 *  /param nbpoints - int length of points array
 *  /param saveall_flag - int  1 indicates used by save, 0 indicates used by add function
 *  /param level - int track recursion level
 *  /return FileIndex* - fileno and offset into file.
**/
//static MVPRetCode _ph_save_mvptree(MVPFile *m, DP **points, int nbpoints, int saveall_flag, int level,FileIndex *pOffset);

/** /brief save points to mvp file
 *  /param m - MVPFile state info of file
 *  /param points - DP** list of points to add
 *  /param nbpoints - int number of points
 *  /return MVPRetCode - ret code
**/

//MVPRetCode ph_save_mvptree(MVPFile *m, DP **points, int nbpoints);

/**  /brief add points to mvp file (aux function)
 *   /param m - MVPFile state information of file
 *   /param new_dp - datapoint to add
 *   /param level - int track recursion level
 *   /return MVPRetCode
 **/
//static MVPRetCode _ph_add_mvptree(MVPFile *m, DP *new_dp, int level);

/** /brief add a list of points to mvp file
    /param m - MVPFile state information of file.
    /param points - DP** list of points to add
    /param nbpoints - int number of points
    /return int - number of points added, neg for error
**/
//MVPRetCode ph_add_mvptree(MVPFile *m, DP **points, int nbpoints, int &nbsaved);

/** /brief textual hash for file
 *  /param filename - char* name of file
 *  /param nbpoints - int length of array of return value (out)
 *  /return TxtHashPoint* array of hash points with respective index into file.
 **/
//TxtHashPoint* ph_texthash(const char *filename, int *nbpoints);

/** /brief compare 2 text hashes
 *  /param hash1 -TxtHashPoint
 *  /param N1 - int length of hash1
 *  /param hash2 - TxtHashPoint
 *  /param N2 - int length of hash2
 *  /param nbmatches - int number of matches found (out)
 *  /return TxtMatch* - list of all matches
 **/
//TxtMatch* ph_compare_text_hashes(TxtHashPoint *hash1, int N1, TxtHashPoint *hash2, int N2, int *nbmatches);

/* random char mapping for textual hash */

// static const ulong64 textkeys[256] = {
//     15498727785010036736LLU,
//     7275080914684608512LLU,
//     14445630958268841984LLU,
//     14728618948878663680LLU,
//     16816925489502355456LLU,
//     3644179549068984320LLU,
//     6183768379476672512LLU,
//     14171334718745739264LLU,
//     5124038997949022208LLU,
//     10218941994323935232LLU,
//     8806421233143906304LLU,
//     11600620999078313984LLU,
//     6729085808520724480LLU,
//     9470575193177980928LLU,
//     17565538031497117696LLU,
//     16900815933189128192LLU,
//     11726811544871239680LLU,
//     13231792875940872192LLU,
//     2612106097615437824LLU,
//     11196599515807219712LLU,
//     300692472869158912LLU,
//     4480470094610169856LLU,
//     2531475774624497664LLU,
//     14834442768343891968LLU,
//     2890219059826130944LLU,
//     7396118625003765760LLU,
//     2394211153875042304LLU,
//     2007168123001634816LLU,
//     18426904923984625664LLU,
//     4026129272715345920LLU,
//     9461932602286931968LLU,
//     15478888635285110784LLU,
//     11301210195989889024LLU,
//     5460819486846222336LLU,
//     11760763510454222848LLU,
//     9671391611782692864LLU,
//     9104999035915206656LLU,
//     17944531898520829952LLU,
//     5395982256818880512LLU,
//     14229038033864228864LLU,
//     9716729819135213568LLU,
//     14202403489962786816LLU,
//     7382914959232991232LLU,
//     16445815627655938048LLU,
//     5226234609431216128LLU,
//     6501708925610491904LLU,
//     14899887495725449216LLU,
//     16953046154302455808LLU,
//     1286757727841812480LLU,
//     17511993593340887040LLU,
//     9702901604990058496LLU,
//     1587450200710971392LLU,
//     3545719622831439872LLU,
//     12234377379614556160LLU,
//     16421892977644797952LLU,
//     6435938682657570816LLU,
//     1183751930908770304LLU,
//     369360057810288640LLU,
//     8443106805659205632LLU,
//     1163912781183844352LLU,
//     4395489330525634560LLU,
//     17905039407946137600LLU,
//     16642801425058889728LLU,
//     15696699526515523584LLU,
//     4919114829672742912LLU,
//     9956820861803560960LLU,
//     6921347064588664832LLU,
//     14024113865587949568LLU,
//     9454608686614839296LLU,
//     12317329321407545344LLU,
//     9806407834332561408LLU,
//     724594440630435840LLU,
//     8072988737660780544LLU,
//     17189322793565552640LLU,
//     17170410068286373888LLU,
//     13299223355681931264LLU,
//     5244287645466492928LLU,
//     13623553490302271488LLU,
//     11805525436274835456LLU,
//     6531045381898240000LLU,
//     12688803018523541504LLU,
//     3061682967555342336LLU,
//     8118495582609211392LLU,
//     16234522641354981376LLU,
//     15296060347169898496LLU,
//     6093644486544457728LLU,
//     4223717250303000576LLU,
//     16479812286668603392LLU,
//     6463004544354746368LLU,
//     12666824055962206208LLU,
//     17643725067852447744LLU,
//     10858493883470315520LLU,
//     12125119390198792192LLU,
//     15839782419201785856LLU,
//     8108449336276287488LLU,
//     17044234219871535104LLU,
//     7349859215885729792LLU,
//     15029796409454886912LLU,
//     12621604020339867648LLU,
//     16804467902500569088LLU,
//     8900381657152880640LLU,
//     3981267780962877440LLU,
//     17529062343131004928LLU,
//     16973370403403595776LLU,
//     2723846500818878464LLU,
//     16252728346297761792LLU,
//     11825849685375975424LLU,
//     7968134154875305984LLU,
//     11429537762890481664LLU,
//     5184631047941259264LLU,
//     14499179536773545984LLU,
//     5671596707704471552LLU,
//     8246314024086536192LLU,
//     4170931045673205760LLU,
//     3459375275349901312LLU,
//     5095630297546883072LLU,
//     10264575540807598080LLU,
//     7683092525652901888LLU,
//     3128698510505934848LLU,
//     16727580085162344448LLU,
//     1903172507905556480LLU,
//     2325679513238765568LLU,
//     9139329894923108352LLU,
//     14028291906694283264LLU,
//     18165461932440551424LLU,
//     17247779239789330432LLU,
//     12625782052856266752LLU,
//     7068577074616729600LLU,
//     13830831575534665728LLU,
//     6800641999486582784LLU,
//     5426300911997681664LLU,
//     4284469158977994752LLU,
//     10781909780449460224LLU,
//     4508619181419134976LLU,
//     2811095488672038912LLU,
//     13505756289858273280LLU,
//     2314603454007345152LLU,
//     14636945174048014336LLU,
//     3027146371024027648LLU,
//     13744141225487761408LLU,
//     1374832156869656576LLU,
//     17526325907797573632LLU,
//     968993859482681344LLU,
//     9621146180956192768LLU,
//     3250512879761227776LLU,
//     4428369143422517248LLU,
//     14716776478503075840LLU,
//     13515088420568825856LLU,
//     12111461669075419136LLU,
//     17845474997598945280LLU,
//     11795924440611553280LLU,
//     14014634185570910208LLU,
//     1724410437128159232LLU,
//     2488510261825110016LLU,
//     9596182018555641856LLU,
//     1443128295859159040LLU,
//     1289545427904888832LLU,
//     3775219997702356992LLU,
//     8511705379065823232LLU,
//     15120377003439554560LLU,
//     10575862005778874368LLU,
//     13938006291063504896LLU,
//     958102097297932288LLU,
//     2911027712518782976LLU,
//     18446625472482639872LLU,
//     3769197585969971200LLU,
//     16416784002377056256LLU,
//     2314484861370368000LLU,
//     18406142768607920128LLU,
//     997186299691532288LLU,
//     16058626086858129408LLU,
//     1334230851768025088LLU,
//     76768133779554304LLU,
//     17027619946340810752LLU,
//     10955377032724217856LLU,
//     3327281022130716672LLU,
//     3009245016053776384LLU,
//     7225409437517742080LLU,
//     16842369442699542528LLU,
//     15120706693719130112LLU,
//     6624140361407135744LLU,
//     10191549809601544192LLU,
//     10688596805580488704LLU,
//     8348550798535294976LLU,
//     12680060080016588800LLU,
//     1838034750426578944LLU,
//     9791679102984388608LLU,
//     13969605507921477632LLU,
//     5613254748128935936LLU,
//     18303384482050211840LLU,
//     10643238446241415168LLU,
//     16189116753907810304LLU,
//     13794646699404165120LLU,
//     11601340543539347456LLU,
//     653400401306976256LLU,
//     13794528098177253376LLU,
//     15370538129509318656LLU,
//     17070184403684032512LLU,
//     16109012959547621376LLU,
//     15329936824407687168LLU,
//     18067370711965499392LLU,
//     13720894972696199168LLU,
//     16664167676175712256LLU,
//     18144138845745053696LLU,
//     12301770853917392896LLU,
//     9172800635190378496LLU,
//     3024675794166218752LLU,
//     15311015869971169280LLU,
//     16398210081298055168LLU,
//     1420301171746144256LLU,
//     11984978489980747776LLU,
//     4575606368995639296LLU,
//     11611850981347688448LLU,
//     4226831221851684864LLU,
//     12924157176120868864LLU,
//     5845166987654725632LLU,
//     6064865972278263808LLU,
//     4269092205395705856LLU,
//     1368028430456586240LLU,
//     11678120728997134336LLU,
//     4125732613736366080LLU,
//     12011266876698001408LLU,
//     9420493409195393024LLU,
//     17920379313140531200LLU,
//     5165863346527797248LLU,
//     10073893810502369280LLU,
//     13268163337608232960LLU,
//     2089657402327564288LLU,
//     8697334149066784768LLU,
//     10930432232036237312LLU,
//     17419594235325186048LLU,
//     8317960787322732544LLU,
//     6204583131022884864LLU,
//     15637017837791346688LLU,
//     8015355559358234624LLU,
//     59609911230726144LLU,
//     6363074407862108160LLU,
//     11040031362114387968LLU,
//     15370625789791830016LLU,
//     4314540415450611712LLU,
//     12460332533860532224LLU,
//     8908860206063026176LLU,
//     8890146784446251008LLU,
//     5625439441498669056LLU,
//     13135691436504645632LLU,
//     3367559886857568256LLU,
//     11470606437743329280LLU,
//     753813335073357824LLU,
//     7636652092253274112LLU,
//     12838634868199915520LLU,
//     12431934064070492160LLU,
//     11762384705989640192LLU,
//     6403157671188365312LLU,
//     3405683408146268160LLU,
//     11236019945420619776LLU,
//     11569021017716162560LLU
// };

#ifdef __cplusplus
}
#endif

#endif
