#pragma once
#undef __MACTYPES__
#include "..\..\zlib\zlib.h"
#include "UnCompressAPI.h"

#define UNZ_OK                          (0)
#define UNZ_END_OF_LIST_OF_FILE         (-100)
#define UNZ_ERRNO                       (Z_ERRNO)
#define UNZ_EOF                         (0)
#define UNZ_PARAMERROR                  (-102)
#define UNZ_BADZIPFILE                  (-103)
#define UNZ_INTERNALERROR               (-104)
#define UNZ_CRCERROR                    (-105)

#define ALLOC(size) (malloc(size))
#define BUFREADCOMMENT (0x400)

# define TRYFREE(p) {if (p) free(p);}

#define Z_BZIP2ED 12
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
#define UNZ_BUFSIZE (1000384)

typedef voidp unzFile;

class UnCompressZip
{
public:
  UnCompressZip(void);
  ~UnCompressZip(void);

  unzFile unzOpen64 (const void *path);

  unzFile unzOpenInternal(const void *path, zlib_filefunc64_32_def* pzlib_filefunc64_32_def,
                          int is64bitOpenFunction);

  ZPOS64_T unz64local_SearchCentralDir64(const zlib_filefunc64_32_def* pzlib_filefunc_def,
                                         voidpf filestream);

  int unz64local_getLong (const zlib_filefunc64_32_def* pzlib_filefunc_def,
                          voidpf filestream, uLong *pX);

  int unz64local_getByte(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, int *pi);

  int unz64local_getLong64(const zlib_filefunc64_32_def* pzlib_filefunc_def,
                           voidpf filestream, ZPOS64_T *pX);

  int unz64local_getShort(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream,
                         uLong *pX);

  ZPOS64_T unz64local_SearchCentralDir(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream);

  int unzGoToFirstFile (unzFile file);

  int unz64local_GetCurrentFileInfoInternal(unzFile file, unz_file_info64 *pfile_info, unz_file_info64_internal
                                            *pfile_info_internal, char *szFileName, uLong fileNameBufferSize,
                                            void *extraField, uLong extraFieldBufferSize, char *szComment,
                                            uLong commentBufferSize);

  void unz64local_DosDateToTmuDate (ZPOS64_T ulDosDate, tm_unz* ptm);

  int unzOpenCurrentFile (unzFile file);

  int unzOpenCurrentFile3 (unzFile file, int* method, int* level, int raw, const char* password);

  int unzCloseCurrentFile (unzFile file);

  int unz64local_CheckCurrentFileCoherencyHeader(unz64_s* s, uInt* piSizeVar, ZPOS64_T * poffset_local_extrafield,
                                                 uInt  * psize_local_extrafield);

  int unzReadCurrentFile  (unzFile file, voidp buf, unsigned len);

  int unzGoToNextFile (unzFile  file);

  void SetUnCompressPath(CString s);

  int unzClose (unzFile file);
private:
  CString m_uncompresspath;

};
