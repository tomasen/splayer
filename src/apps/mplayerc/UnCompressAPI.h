#pragma once
#undef __MACTYPES__
#include "..\..\zlib\zlib.h"

#define ZLIB_FILEFUNC_SEEK_CUR (1)
#define ZLIB_FILEFUNC_SEEK_END (2)
#define ZLIB_FILEFUNC_SEEK_SET (0)

#define ZLIB_FILEFUNC_MODE_READ      (1)
#define ZLIB_FILEFUNC_MODE_WRITE     (2)
#define ZLIB_FILEFUNC_MODE_READWRITEFILTER (3)

#define ZLIB_FILEFUNC_MODE_EXISTING (4)
#define ZLIB_FILEFUNC_MODE_CREATE   (8)

#define ZOPEN64(filefunc,filename,mode)       (call_zopen64((&(filefunc)),(filename),(mode)))
#define ZTELL64(filefunc,filestream)          (call_ztell64((&(filefunc)),(filestream)))
#define ZSEEK64(filefunc,filestream,pos,mode) (call_zseek64((&(filefunc)),(filestream),(pos),(mode)))
#define ZREAD64(filefunc,filestream,buf,size)     ((*((filefunc).zfile_func64.zread_file))   ((filefunc).zfile_func64.opaque,filestream,buf,size))
#define ZCLOSE64(filefunc,filestream)             ((*((filefunc).zfile_func64.zclose_file))  ((filefunc).zfile_func64.opaque,filestream))
#define ZERROR64(filefunc,filestream)             ((*((filefunc).zfile_func64.zerror_file))  ((filefunc).zfile_func64.opaque,filestream))

typedef unsigned long long int ZPOS64_T;

typedef voidpf (CALLBACK *open_file_func)(voidpf opaque, const char* filename, int mode);
typedef uLong  (CALLBACK *read_file_func)(voidpf opaque, voidpf stream, void* buf, uLong size);
typedef uLong  (CALLBACK *write_file_func)(voidpf opaque, voidpf stream, const void* buf, uLong size);
typedef int    (CALLBACK *close_file_func)(voidpf opaque, voidpf stream);
typedef int    (CALLBACK *testerror_file_func)(voidpf opaque, voidpf stream);
typedef long   (CALLBACK *tell_file_func)(voidpf opaque, voidpf stream);
typedef long   (CALLBACK *seek_file_func)(voidpf opaque, voidpf stream, uLong offset, int origin);
typedef ZPOS64_T (CALLBACK *tell64_file_func)(voidpf opaque, voidpf stream);
typedef long     (CALLBACK *seek64_file_func)(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin);
typedef voidpf   (CALLBACK *open64_file_func)(voidpf opaque, const void* filename, int mode);

typedef struct zlib_filefunc64_def_s
{
  open64_file_func    zopen64_file;
  read_file_func      zread_file;
  write_file_func     zwrite_file;
  tell64_file_func    ztell64_file;
  seek64_file_func    zseek64_file;
  close_file_func     zclose_file;
  testerror_file_func zerror_file;
  voidpf              opaque;
} zlib_filefunc64_def;

typedef struct zlib_filefunc64_32_def_s
{
  zlib_filefunc64_def zfile_func64;
  open_file_func      zopen32_file;
  tell_file_func      ztell32_file;
  seek_file_func      zseek32_file;
} zlib_filefunc64_32_def;

typedef struct unz_global_info64_s
{
    ZPOS64_T number_entry;         /* total number of entries in
                                     the central dir on this disk */
    uLong size_comment;         /* size of the global comment of the zipfile */
}unz_global_info64;

typedef struct tm_unz_s
{
  uInt tm_sec;            /* seconds after the minute - [0,59] */
  uInt tm_min;            /* minutes after the hour - [0,59] */
  uInt tm_hour;           /* hours since midnight - [0,23] */
  uInt tm_mday;           /* day of the month - [1,31] */
  uInt tm_mon;            /* months since January - [0,11] */
  uInt tm_year;           /* years - [1980..2044] */
} tm_unz;

typedef struct unz_file_info64_s
{
  uLong version;              /* version made by                 2 bytes */
  uLong version_needed;       /* version needed to extract       2 bytes */
  uLong flag;                 /* general purpose bit flag        2 bytes */
  uLong compression_method;   /* compression method              2 bytes */
  uLong dosDate;              /* last mod file date in Dos fmt   4 bytes */
  uLong crc;                  /* crc-32                          4 bytes */
  ZPOS64_T compressed_size;   /* compressed size                 8 bytes */
  ZPOS64_T uncompressed_size; /* uncompressed size               8 bytes */
  uLong size_filename;        /* filename length                 2 bytes */
  uLong size_file_extra;      /* extra field length              2 bytes */
  uLong size_file_comment;    /* file comment length             2 bytes */

  uLong disk_num_start;       /* disk number start               2 bytes */
  uLong internal_fa;          /* internal file attributes        2 bytes */
  uLong external_fa;          /* external file attributes        4 bytes */

  tm_unz tmu_date;
} unz_file_info64;

typedef struct unz_file_info64_internal_s
{
  ZPOS64_T offset_curfile;/* relative offset of local header 8 bytes */
} unz_file_info64_internal;

typedef struct
{
  char  *read_buffer;         /* internal buffer for compressed data */
  z_stream stream;            /* zLib stream structure for inflate */

  ZPOS64_T pos_in_zipfile;       /* position in byte on the zipfile, for fseek*/
  uLong stream_initialised;   /* flag set if stream structure is initialised*/

  ZPOS64_T offset_local_extrafield;/* offset of the local extra field */
  uInt  size_local_extrafield;/* size of the local extra field */
  ZPOS64_T pos_local_extrafield;   /* position in the local extra field in read*/
  ZPOS64_T total_out_64;

  uLong crc32;                /* crc32 of all data uncompressed */
  uLong crc32_wait;           /* crc32 we must obtain after decompress all */
  ZPOS64_T rest_read_compressed; /* number of byte to be decompressed */
  ZPOS64_T rest_read_uncompressed;/*number of byte to be obtained after decomp*/
  zlib_filefunc64_32_def z_filefunc;
  voidpf filestream;        /* io structore of the zipfile */
  uLong compression_method;   /* compression method (0==store) */
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
  int   raw;
} file_in_zip64_read_info_s;

typedef struct
{
    zlib_filefunc64_32_def z_filefunc;
    int is64bitOpenFunction;
    voidpf filestream;        /* io structore of the zipfile */
    unz_global_info64 gi;       /* public global information */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    ZPOS64_T num_file;             /* number of the current file in the zipfile*/
    ZPOS64_T pos_in_central_dir;   /* pos of the current file in the central dir*/
    ZPOS64_T current_file_ok;      /* flag about the usability of the current file*/
    ZPOS64_T central_pos;          /* position of the beginning of the central dir*/

    ZPOS64_T size_central_dir;     /* size of the central directory  */
    ZPOS64_T offset_central_dir;   /* offset of start of central directory with
                                   respect to the starting disk number */

    unz_file_info64 cur_file_info; /* public info about the current file in zip*/
    unz_file_info64_internal cur_file_info_internal; /* private info about it*/
    file_in_zip64_read_info_s* pfile_in_zip_read; /* structure about the current
                                        //file if we are decompressing it */
    int encrypted;

    int isZip64;

#    ifndef NOUNCRYPT
    unsigned long keys[3];     /* keys defining the pseudo-random sequence */
    const unsigned long* pcrc_32_tab;
#    endif
} unz64_s;

void fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def);

voidpf  call_zopen64(const zlib_filefunc64_32_def* pfilefunc,const void*filename,int mode);

ZPOS64_T call_ztell64(const zlib_filefunc64_32_def* pfilefunc,voidpf filestream);

long call_zseek64(const zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, int origin);