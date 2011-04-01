#include "stdafx.h"
#include "UnCompressAPI.h"

static voidpf CALLBACK fopen64_file_func(voidpf opaque, const void* filename, int mode)
{
  FILE* file = NULL;
  const char* mode_fopen = NULL;
  if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    mode_fopen = "rb";
  else
    if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
      mode_fopen = "r+b";
    else
      if (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

  if ((filename!=NULL) && (mode_fopen != NULL))
    file = fopen((const char*)filename, mode_fopen);
  return file;
}

static uLong CALLBACK fread_file_func (voidpf opaque, voidpf stream, void* buf, uLong size)
{
  uLong ret;
  ret = (uLong)fread(buf, 1, (size_t)size, (FILE*)stream);
  return ret;
}

static uLong CALLBACK fwrite_file_func (voidpf opaque, voidpf stream, const void* buf, uLong size)
{
  uLong ret;
  ret = (uLong)fwrite(buf, 1, (size_t)size, (FILE *)stream);
  return ret;
}

static ZPOS64_T CALLBACK ftell64_file_func (voidpf opaque, voidpf stream)
{
  ZPOS64_T ret;
  ret = ftell((FILE *)stream);
  return ret;
}

static long CALLBACK fseek64_file_func (voidpf  opaque, voidpf stream, ZPOS64_T offset, int origin)
{
  int fseek_origin=0;
  long ret;
  switch (origin)
  {
  case ZLIB_FILEFUNC_SEEK_CUR :
    fseek_origin = SEEK_CUR;
    break;
  case ZLIB_FILEFUNC_SEEK_END :
    fseek_origin = SEEK_END;
    break;
  case ZLIB_FILEFUNC_SEEK_SET :
    fseek_origin = SEEK_SET;
    break;
  default: return -1;
  }
  ret = 0;

  if(fseek((FILE *)stream, offset, fseek_origin) != 0)
    ret = -1;

  return ret;
}

static int CALLBACK fclose_file_func (voidpf opaque, voidpf stream)
{
  int ret;
  ret = fclose((FILE *)stream);
  return ret;
}

static int CALLBACK ferror_file_func (voidpf opaque, voidpf stream)
{
  int ret;
  ret = ferror((FILE *)stream);
  return ret;
}

void fill_fopen64_filefunc (zlib_filefunc64_def*  pzlib_filefunc_def)
{
  pzlib_filefunc_def->zopen64_file = fopen64_file_func;
  pzlib_filefunc_def->zread_file = fread_file_func;
  pzlib_filefunc_def->zwrite_file = fwrite_file_func;
  pzlib_filefunc_def->ztell64_file = ftell64_file_func;
  pzlib_filefunc_def->zseek64_file = fseek64_file_func;
  pzlib_filefunc_def->zclose_file = fclose_file_func;
  pzlib_filefunc_def->zerror_file = ferror_file_func;
  pzlib_filefunc_def->opaque = NULL;
}

voidpf call_zopen64(const zlib_filefunc64_32_def* pfilefunc,const void*filename,int mode)
{
  if (pfilefunc->zfile_func64.zopen64_file != NULL)
    return (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,mode);
  else
  {
    return (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(const char*)filename,mode);
  }
}

ZPOS64_T call_ztell64(const zlib_filefunc64_32_def* pfilefunc,voidpf filestream)
{
  if (pfilefunc->zfile_func64.zseek64_file != NULL)
    return (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
  else
  {
    uLong tell_uLong = (*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
    if ((tell_uLong) == ((uLong)-1))
      return (ZPOS64_T)-1;
    else
      return tell_uLong;
  }
}

long call_zseek64(const zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, int origin)
{
  if (pfilefunc->zfile_func64.zseek64_file != NULL)
    return (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,offset,origin);
  else
  {
    uLong offsetTruncated = (uLong)offset;
    if (offsetTruncated != offset)
      return -1;
    else
      return (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,offsetTruncated,origin);
  }
}


