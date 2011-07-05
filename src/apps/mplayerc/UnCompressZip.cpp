#include "stdafx.h"
#include "UnCompressZip.h"
#include <Strings.h>

UnCompressZip::UnCompressZip(void)
{
}

UnCompressZip::~UnCompressZip(void)
{
}

unzFile UnCompressZip::unzOpen64 (const void *path)
{
  return unzOpenInternal(path, NULL, 1);
}   

unzFile UnCompressZip::unzOpenInternal (const void *path,
                               zlib_filefunc64_32_def* pzlib_filefunc64_32_def,
                               int is64bitOpenFunction)
{
    unz64_s us;
    unz64_s *s;
    ZPOS64_T central_pos;
    uLong   uL;

    uLong number_disk;          /* number of the current dist, used for
                                   spaning ZIP, unsupported, always 0*/
    uLong number_disk_with_CD;  /* number the the disk with central dir, used
                                   for spaning ZIP, unsupported, always 0*/
    ZPOS64_T number_entry_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */

    int err=UNZ_OK;

    //if (unz_copyright[0]!=' ')
        //return NULL;

    us.z_filefunc.zseek32_file = NULL;
    us.z_filefunc.ztell32_file = NULL;
    if (pzlib_filefunc64_32_def==NULL)
        fill_fopen64_filefunc(&us.z_filefunc.zfile_func64);
    else
        us.z_filefunc = *pzlib_filefunc64_32_def;
    us.is64bitOpenFunction = is64bitOpenFunction;

    us.filestream = ZOPEN64(us.z_filefunc, path, ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_EXISTING);
    if (us.filestream==NULL)
        return NULL;

    central_pos = unz64local_SearchCentralDir64(&us.z_filefunc,us.filestream);
    if (central_pos)
    {
       uLong uS;
       ZPOS64_T uL64;

       us.isZip64 = 1;

       if (ZSEEK64(us.z_filefunc, us.filestream, central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
         err=UNZ_ERRNO;

       if (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
         err=UNZ_ERRNO;

        
       if (unz64local_getLong64(&us.z_filefunc, us.filestream,&uL64)!=UNZ_OK)
         err=UNZ_ERRNO;

        
       if (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        
        if (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        
        if (unz64local_getLong(&us.z_filefunc, us.filestream,&number_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

       
        if (unz64local_getLong(&us.z_filefunc, us.filestream,&number_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        
        if (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.gi.number_entry)!=UNZ_OK)
            err=UNZ_ERRNO;

        
        if (unz64local_getLong64(&us.z_filefunc, us.filestream,&number_entry_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        if ((number_entry_CD!=us.gi.number_entry) ||
            (number_disk_with_CD!=0) ||
            (number_disk!=0))
            err=UNZ_BADZIPFILE;

        
        if (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.size_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        
        if (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.offset_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        us.gi.size_comment = 0;
    }
    else
    {
        central_pos = unz64local_SearchCentralDir(&us.z_filefunc,us.filestream);
        if (central_pos==0)
            err=UNZ_ERRNO;

        us.isZip64 = 0;

        if (ZSEEK64(us.z_filefunc, us.filestream,
                                        central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err=UNZ_ERRNO;

        
        if (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

       
        if (unz64local_getShort(&us.z_filefunc, us.filestream,&number_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        
        if (unz64local_getShort(&us.z_filefunc, us.filestream,&number_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        
        if (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.gi.number_entry = uL;

       
        if (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        number_entry_CD = uL;

        if ((number_entry_CD!=us.gi.number_entry) ||
            (number_disk_with_CD!=0) ||
            (number_disk!=0))
            err=UNZ_BADZIPFILE;

     
        if (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.size_central_dir = uL;

        if (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.offset_central_dir = uL;


        if (unz64local_getShort(&us.z_filefunc, us.filestream,&us.gi.size_comment)!=UNZ_OK)
            err=UNZ_ERRNO;
    }

    if ((central_pos<us.offset_central_dir+us.size_central_dir) &&
        (err==UNZ_OK))
        err=UNZ_BADZIPFILE;

    if (err!=UNZ_OK)
    {
        ZCLOSE64(us.z_filefunc, us.filestream);
        return NULL;
    }

    us.byte_before_the_zipfile = central_pos - (us.offset_central_dir+us.size_central_dir);
    us.central_pos = central_pos;
    us.pfile_in_zip_read = NULL;
    us.encrypted = 0;

    s=(unz64_s*)ALLOC(sizeof(unz64_s));
    if( s != NULL)
    {
        *s=us;
        Bytef buf[UNZ_BUFSIZE];
        char  name[100];
        if (unzGoToFirstFile((unzFile)s) != UNZ_OK)
          return (unzFile)UNZ_ERRNO;
        ::CreateDirectory(m_uncompresspath.GetBuffer(MAX_PATH), NULL);
        m_uncompresspath.ReleaseBuffer();
        do 
        {
          
          unzOpenCurrentFile((unzFile)s);
          uInt filelength = unzReadCurrentFile((unzFile)s, buf, UNZ_BUFSIZE);
          unz64local_GetCurrentFileInfoInternal((unzFile)s, &s->cur_file_info,
             &s->cur_file_info_internal, name,100,NULL,0,NULL,0);

          std::wstring path = m_uncompresspath.GetString();
          path += L"\\";
          std::wstring filename = Strings::StringToWString(name);
          path += filename;

          HANDLE hfile = CreateFile(path.c_str(), GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
          CloseHandle(hfile);

          FILE* file;
          _wfopen_s(&file, path.c_str(), L"wb");
          fwrite(buf, 1, filelength, file);
          fclose(file);
        }
        while(unzGoToNextFile((unzFile)s) == UNZ_OK);
       
        unzClose((unzFile)s);
    }
    return (unzFile)s;
}

ZPOS64_T UnCompressZip::unz64local_SearchCentralDir64(const zlib_filefunc64_32_def* pzlib_filefunc_def,
                                                      voidpf filestream)
{
  unsigned char* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;
  uLong uL;
  ZPOS64_T relativeOffset;

  if (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    return 0;


  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  if (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned char*)ALLOC(BUFREADCOMMENT+4);
  if (buf==NULL)
    return 0;

  uBackRead = 4;
  while (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos;
    int i;
    if (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    else
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    if (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      break;

    if (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      break;

    for (i=(int)uReadSize-3; (i--)>0;)
      if (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        uPosFound = uReadPos+i;
        break;
      }

      if (uPosFound!=0)
        break;
  }
  TRYFREE(buf);
  if (uPosFound == 0)
    return 0;


  if (ZSEEK64(*pzlib_filefunc_def,filestream, uPosFound,ZLIB_FILEFUNC_SEEK_SET)!=0)
    return 0;


  if (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
    return 0;


  if (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
    return 0;
  if (uL != 0)
    return 0;


  if (unz64local_getLong64(pzlib_filefunc_def,filestream,&relativeOffset)!=UNZ_OK)
    return 0;


  if (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
    return 0;
  if (uL != 1)
    return 0;

  if (ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET)!=0)
    return 0;


  if (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
    return 0;

  if (uL != 0x06064b50)
    return 0;

  return relativeOffset;
  return 0;
}

int UnCompressZip::unz64local_getLong (const zlib_filefunc64_32_def* pzlib_filefunc_def,
                                       voidpf filestream, uLong *pX)
{
  uLong x ;
  int i = 0;
  int err;

  err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (uLong)i;

  if (err==UNZ_OK)
    err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x |= ((uLong)i)<<8;

  if (err==UNZ_OK)
    err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x |= ((uLong)i)<<16;

  if (err==UNZ_OK)
    err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((uLong)i)<<24;

  if (err==UNZ_OK)
    *pX = x;
  else
    *pX = 0;
  return err;
}

int UnCompressZip::unz64local_getByte(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, int *pi)
{
  unsigned char c;
  int err = (int)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
  if (err==1)
  {
    *pi = (int)c;
    return UNZ_OK;
  }
  else
  {
    if (ZERROR64(*pzlib_filefunc_def,filestream))
      return UNZ_ERRNO;
    else
      return UNZ_EOF;
  }
}

int UnCompressZip::unz64local_getLong64(const zlib_filefunc64_32_def* pzlib_filefunc_def,
                                        voidpf filestream, ZPOS64_T *pX)
{
  ZPOS64_T x ;
  int i = 0;
  int err;

  err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (ZPOS64_T)i;

  if (err==UNZ_OK)
    err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x |= ((ZPOS64_T)i)<<8;

  if (err==UNZ_OK)
    err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x |= ((ZPOS64_T)i)<<16;

  if (err==UNZ_OK)
    err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x |= ((ZPOS64_T)i)<<24;

  if (err==UNZ_OK)
    err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x |= ((ZPOS64_T)i)<<32;

  if (err==UNZ_OK)
    err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x |= ((ZPOS64_T)i)<<40;

  if (err==UNZ_OK)
    err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x |= ((ZPOS64_T)i)<<48;

  if (err==UNZ_OK)
    err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x |= ((ZPOS64_T)i)<<56;

  if (err==UNZ_OK)
    *pX = x;
  else
    *pX = 0;
  return err;
}

int UnCompressZip::unz64local_getShort (const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream,
                                        uLong *pX)
{
  uLong x ;
  int i = 0;
  int err;

  err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (uLong)i;

  if (err==UNZ_OK)
    err = unz64local_getByte(pzlib_filefunc_def,filestream,&i);
  x |= ((uLong)i)<<8;

  if (err==UNZ_OK)
    *pX = x;
  else
    *pX = 0;
  return err;
}

ZPOS64_T UnCompressZip::unz64local_SearchCentralDir(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream)
{
  unsigned char* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;

  if (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    return 0;


  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  if (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned char*)ALLOC(BUFREADCOMMENT+4);
  if (buf==NULL)
    return 0;

  uBackRead = 4;
  while (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos ;
    int i;
    if (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    else
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    if (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      break;

    if (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      break;

    for (i=(int)uReadSize-3; (i--)>0;)
      if (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        uPosFound = uReadPos+i;
        break;
      }

      if (uPosFound!=0)
        break;
  }
  TRYFREE(buf);
  return uPosFound;
}

int UnCompressZip::unzGoToFirstFile (unzFile file)
{
  int err=UNZ_OK;
  unz64_s* s;
  if (file==NULL)
    return UNZ_PARAMERROR;
  s=(unz64_s*)file;
  s->pos_in_central_dir=s->offset_central_dir;
  s->num_file=0;
  err=unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
    &s->cur_file_info_internal,
    NULL,0,NULL,0,NULL,0);
  s->current_file_ok = (err == UNZ_OK);
  return err;
}

int UnCompressZip::unz64local_GetCurrentFileInfoInternal(unzFile file, unz_file_info64 *pfile_info, unz_file_info64_internal
                                                         *pfile_info_internal, char *szFileName, uLong fileNameBufferSize,
                                                         void *extraField, uLong extraFieldBufferSize, char *szComment,
                                                         uLong commentBufferSize)
{
  unz64_s* s;
  unz_file_info64 file_info;
  unz_file_info64_internal file_info_internal;
  int err=UNZ_OK;
  uLong uMagic;
  long lSeek=0;
  uLong uL;

  if (file==NULL)
    return UNZ_PARAMERROR;
  s=(unz64_s*)file;
  if (ZSEEK64(s->z_filefunc, s->filestream,
    s->pos_in_central_dir+s->byte_before_the_zipfile,
    ZLIB_FILEFUNC_SEEK_SET)!=0)
    err=UNZ_ERRNO;


  /* we check the magic */
  if (err==UNZ_OK)
  {
    if (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
      err=UNZ_ERRNO;
    else if (uMagic!=0x02014b50)
      err=UNZ_BADZIPFILE;
  }

  if (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.version) != UNZ_OK)
    err=UNZ_ERRNO;

  if (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.version_needed) != UNZ_OK)
    err=UNZ_ERRNO;

  if (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.flag) != UNZ_OK)
    err=UNZ_ERRNO;

  if (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.compression_method) != UNZ_OK)
    err=UNZ_ERRNO;

  if (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.dosDate) != UNZ_OK)
    err=UNZ_ERRNO;

  unz64local_DosDateToTmuDate(file_info.dosDate,&file_info.tmu_date);

  if (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.crc) != UNZ_OK)
    err=UNZ_ERRNO;

  if (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
    err=UNZ_ERRNO;
  file_info.compressed_size = uL;

  if (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
    err=UNZ_ERRNO;
  file_info.uncompressed_size = uL;

  if (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.size_filename) != UNZ_OK)
    err=UNZ_ERRNO;

  if (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.size_file_extra) != UNZ_OK)
    err=UNZ_ERRNO;

  if (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.size_file_comment) != UNZ_OK)
    err=UNZ_ERRNO;

  if (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.disk_num_start) != UNZ_OK)
    err=UNZ_ERRNO;

  if (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.internal_fa) != UNZ_OK)
    err=UNZ_ERRNO;

  if (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.external_fa) != UNZ_OK)
    err=UNZ_ERRNO;

  // relative offset of local header
  if (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
    err=UNZ_ERRNO;
  file_info_internal.offset_curfile = uL;

  lSeek+=file_info.size_filename;
  if ((err==UNZ_OK) && (szFileName!=NULL))
  {
    uLong uSizeRead ;
    if (file_info.size_filename<fileNameBufferSize)
    {
      *(szFileName+file_info.size_filename)='\0';
      uSizeRead = file_info.size_filename;
    }
    else
      uSizeRead = fileNameBufferSize;

    if ((file_info.size_filename>0) && (fileNameBufferSize>0))
      if (ZREAD64(s->z_filefunc, s->filestream,szFileName,uSizeRead)!=uSizeRead)
        err=UNZ_ERRNO;
    lSeek -= uSizeRead;
  }

  // Read extrafield
  if ((err==UNZ_OK) && (extraField!=NULL))
  {
    ZPOS64_T uSizeRead ;
    if (file_info.size_file_extra<extraFieldBufferSize)
      uSizeRead = file_info.size_file_extra;
    else
      uSizeRead = extraFieldBufferSize;

    if (lSeek!=0)
    {
      if (ZSEEK64(s->z_filefunc, s->filestream,lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
        lSeek=0;
      else
        err=UNZ_ERRNO;
    }

    if ((file_info.size_file_extra>0) && (extraFieldBufferSize>0))
      if (ZREAD64(s->z_filefunc, s->filestream,extraField,(uLong)uSizeRead)!=uSizeRead)
        err=UNZ_ERRNO;

    lSeek += file_info.size_file_extra - (uLong)uSizeRead;
  }
  else
    lSeek += file_info.size_file_extra;


  if ((err==UNZ_OK) && (file_info.size_file_extra != 0))
  {
    uLong acc = 0;

    // since lSeek now points to after the extra field we need to move back
    lSeek -= file_info.size_file_extra;

    if (lSeek!=0)
    {
      if (ZSEEK64(s->z_filefunc, s->filestream,lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
        lSeek=0;
      else
        err=UNZ_ERRNO;
    }

    while(acc < file_info.size_file_extra)
    {
      uLong headerId;
      uLong dataSize;

      if (unz64local_getShort(&s->z_filefunc, s->filestream,&headerId) != UNZ_OK)
        err=UNZ_ERRNO;

      if (unz64local_getShort(&s->z_filefunc, s->filestream,&dataSize) != UNZ_OK)
        err=UNZ_ERRNO;

      /* ZIP64 extra fields */
      if (headerId == 0x0001)
      {
        uLong uL;

        if(file_info.uncompressed_size == (ZPOS64_T)(unsigned long)-1)
        {
          if (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.uncompressed_size) != UNZ_OK)
            err=UNZ_ERRNO;
        }

        if(file_info.compressed_size == (ZPOS64_T)(unsigned long)-1)
        {
          if (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.compressed_size) != UNZ_OK)
            err=UNZ_ERRNO;
        }

        if(file_info_internal.offset_curfile == (ZPOS64_T)(unsigned long)-1)
        {
          /* Relative Header offset */
          if (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info_internal.offset_curfile) != UNZ_OK)
            err=UNZ_ERRNO;
        }

        if(file_info.disk_num_start == (unsigned long)-1)
        {
          /* Disk Start Number */
          if (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
            err=UNZ_ERRNO;
        }

      }
      else
      {
        if (ZSEEK64(s->z_filefunc, s->filestream,dataSize,ZLIB_FILEFUNC_SEEK_CUR)!=0)
          err=UNZ_ERRNO;
      }

      acc += 2 + 2 + dataSize;
    }
  }

  if ((err==UNZ_OK) && (szComment!=NULL))
  {
    uLong uSizeRead ;
    if (file_info.size_file_comment<commentBufferSize)
    {
      *(szComment+file_info.size_file_comment)='\0';
      uSizeRead = file_info.size_file_comment;
    }
    else
      uSizeRead = commentBufferSize;

    if (lSeek!=0)
    {
      if (ZSEEK64(s->z_filefunc, s->filestream,lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
        lSeek=0;
      else
        err=UNZ_ERRNO;
    }

    if ((file_info.size_file_comment>0) && (commentBufferSize>0))
      if (ZREAD64(s->z_filefunc, s->filestream,szComment,uSizeRead)!=uSizeRead)
        err=UNZ_ERRNO;
    lSeek+=file_info.size_file_comment - uSizeRead;
  }
  else
    lSeek+=file_info.size_file_comment;


  if ((err==UNZ_OK) && (pfile_info!=NULL))
    *pfile_info=file_info;

  if ((err==UNZ_OK) && (pfile_info_internal!=NULL))
    *pfile_info_internal=file_info_internal;

  return err;
}

void UnCompressZip::unz64local_DosDateToTmuDate (ZPOS64_T ulDosDate, tm_unz* ptm)
{
  ZPOS64_T uDate;
  uDate = (ZPOS64_T)(ulDosDate>>16);
  ptm->tm_mday = (uInt)(uDate&0x1f) ;
  ptm->tm_mon =  (uInt)((((uDate)&0x1E0)/0x20)-1) ;
  ptm->tm_year = (uInt)(((uDate&0x0FE00)/0x0200)+1980) ;

  ptm->tm_hour = (uInt) ((ulDosDate &0xF800)/0x800);
  ptm->tm_min =  (uInt) ((ulDosDate&0x7E0)/0x20) ;
  ptm->tm_sec =  (uInt) (2*(ulDosDate&0x1f)) ;
}

int UnCompressZip::unzOpenCurrentFile (unzFile file)
{
  return unzOpenCurrentFile3(file, NULL, NULL, 0, NULL);
}

int UnCompressZip::unzOpenCurrentFile3 (unzFile file, int* method, int* level, int raw, const char* password)
{
  int err=UNZ_OK;
  uInt iSizeVar;
  unz64_s* s;
  file_in_zip64_read_info_s* pfile_in_zip_read_info;
  ZPOS64_T offset_local_extrafield;  /* offset of the local extra field */
  uInt  size_local_extrafield;    /* size of the local extra field */

  if (file==NULL)
    return UNZ_PARAMERROR;
  s=(unz64_s*)file;
  if (!s->current_file_ok)
    return UNZ_PARAMERROR;

  if (s->pfile_in_zip_read != NULL)
    unzCloseCurrentFile(file);

  if (unz64local_CheckCurrentFileCoherencyHeader(s,&iSizeVar, &offset_local_extrafield,&size_local_extrafield)!=UNZ_OK)
    return UNZ_BADZIPFILE;

  pfile_in_zip_read_info = (file_in_zip64_read_info_s*)ALLOC(sizeof(file_in_zip64_read_info_s));
  if (pfile_in_zip_read_info==NULL)
    return UNZ_INTERNALERROR;

  pfile_in_zip_read_info->read_buffer=(char*)ALLOC(UNZ_BUFSIZE);
  pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
  pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
  pfile_in_zip_read_info->pos_local_extrafield=0;
  pfile_in_zip_read_info->raw=raw;

  if (pfile_in_zip_read_info->read_buffer==NULL)
  {
    TRYFREE(pfile_in_zip_read_info);
    return UNZ_INTERNALERROR;
  }

  pfile_in_zip_read_info->stream_initialised=0;

  if (method!=NULL)
    *method = (int)s->cur_file_info.compression_method;

  if (level!=NULL)
  {
    *level = 6;
    switch (s->cur_file_info.flag & 0x06)
    {
      case 6 : *level = 1; break;
      case 4 : *level = 2; break;
      case 2 : *level = 9; break;
    }
   }

  if ((s->cur_file_info.compression_method!=0) &&

      (s->cur_file_info.compression_method!=Z_BZIP2ED) &&

      (s->cur_file_info.compression_method!=Z_DEFLATED))

      err=UNZ_BADZIPFILE;

  pfile_in_zip_read_info->crc32_wait=s->cur_file_info.crc;
  pfile_in_zip_read_info->crc32=0;
  pfile_in_zip_read_info->total_out_64=0;
  pfile_in_zip_read_info->compression_method = s->cur_file_info.compression_method;
  pfile_in_zip_read_info->filestream=s->filestream;
  pfile_in_zip_read_info->z_filefunc=s->z_filefunc;
  pfile_in_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

  pfile_in_zip_read_info->stream.total_out = 0;

  if ((s->cur_file_info.compression_method==Z_BZIP2ED) && (!raw))
    pfile_in_zip_read_info->raw=1;
  else if ((s->cur_file_info.compression_method==Z_DEFLATED) && (!raw))
  {
    pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
    pfile_in_zip_read_info->stream.zfree = (free_func)0;
    pfile_in_zip_read_info->stream.opaque = (voidpf)0;
    pfile_in_zip_read_info->stream.next_in = 0;
    pfile_in_zip_read_info->stream.avail_in = 0;

    err=inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS);
    if (err == Z_OK)
      pfile_in_zip_read_info->stream_initialised=Z_DEFLATED;
    else
    {
      TRYFREE(pfile_in_zip_read_info);
      return err;
    }
  }
  pfile_in_zip_read_info->rest_read_compressed = s->cur_file_info.compressed_size ;
  pfile_in_zip_read_info->rest_read_uncompressed = s->cur_file_info.uncompressed_size ;

  pfile_in_zip_read_info->pos_in_zipfile = s->cur_file_info_internal.offset_curfile + 
    SIZEZIPLOCALHEADER + iSizeVar;

  pfile_in_zip_read_info->stream.avail_in = (uInt)0;

  s->pfile_in_zip_read = pfile_in_zip_read_info;
  s->encrypted = 0;
  
  return UNZ_OK;

}

int UnCompressZip::unzCloseCurrentFile (unzFile file)
{
  int err=UNZ_OK;

  unz64_s* s;
  file_in_zip64_read_info_s* pfile_in_zip_read_info;
  if (file==NULL)
    return UNZ_PARAMERROR;
  s=(unz64_s*)file;
  pfile_in_zip_read_info=s->pfile_in_zip_read;

  if (pfile_in_zip_read_info==NULL)
    return UNZ_PARAMERROR;


  if ((pfile_in_zip_read_info->rest_read_uncompressed == 0) &&
    (!pfile_in_zip_read_info->raw))
  {
    if (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait)
      err=UNZ_CRCERROR;
  }


  TRYFREE(pfile_in_zip_read_info->read_buffer);
  pfile_in_zip_read_info->read_buffer = NULL;
  if (pfile_in_zip_read_info->stream_initialised == Z_DEFLATED)
    inflateEnd(&pfile_in_zip_read_info->stream);

  pfile_in_zip_read_info->stream_initialised = 0;
  TRYFREE(pfile_in_zip_read_info);

  s->pfile_in_zip_read=NULL;

  return err;
}

int UnCompressZip::unz64local_CheckCurrentFileCoherencyHeader(unz64_s* s, uInt* piSizeVar, ZPOS64_T * poffset_local_extrafield,
                                                              uInt  * psize_local_extrafield)
{
  uLong uMagic,uData,uFlags;
    uLong size_filename;
    uLong size_extra_field;
    int err=UNZ_OK;

    *piSizeVar = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    if (ZSEEK64(s->z_filefunc, s->filestream,s->cur_file_info_internal.offset_curfile +
                                s->byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_SET)!=0)
        return UNZ_ERRNO;


    if (err==UNZ_OK)
    {
        if (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        else if (uMagic!=0x04034b50)
            err=UNZ_BADZIPFILE;
    }

    if (unz64local_getShort(&s->z_filefunc, s->filestream,&uData) != UNZ_OK)
        err=UNZ_ERRNO;
/*
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
        err=UNZ_BADZIPFILE;
*/
    if (unz64local_getShort(&s->z_filefunc, s->filestream,&uFlags) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unz64local_getShort(&s->z_filefunc, s->filestream,&uData) != UNZ_OK)
        err=UNZ_ERRNO;
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.compression_method))
        err=UNZ_BADZIPFILE;

    if ((err==UNZ_OK) && (s->cur_file_info.compression_method!=0) &&

                         (s->cur_file_info.compression_method!=Z_BZIP2ED) &&

                         (s->cur_file_info.compression_method!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

    if (unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* date/time */
        err=UNZ_ERRNO;

    if (unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* crc */
        err=UNZ_ERRNO;
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.crc) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    if (unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* size compr */
        err=UNZ_ERRNO;
    else if (uData != 0xFFFFFFFF && (err==UNZ_OK) && (uData!=s->cur_file_info.compressed_size) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    if (unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* size uncompr */
        err=UNZ_ERRNO;
    else if (uData != 0xFFFFFFFF && (err==UNZ_OK) && (uData!=s->cur_file_info.uncompressed_size) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    if (unz64local_getShort(&s->z_filefunc, s->filestream,&size_filename) != UNZ_OK)
        err=UNZ_ERRNO;
    else if ((err==UNZ_OK) && (size_filename!=s->cur_file_info.size_filename))
        err=UNZ_BADZIPFILE;

    *piSizeVar += (uInt)size_filename;

    if (unz64local_getShort(&s->z_filefunc, s->filestream,&size_extra_field) != UNZ_OK)
        err=UNZ_ERRNO;
    *poffset_local_extrafield= s->cur_file_info_internal.offset_curfile +
                                    SIZEZIPLOCALHEADER + size_filename;
    *psize_local_extrafield = (uInt)size_extra_field;

    *piSizeVar += (uInt)size_extra_field;

    return err;
}

int UnCompressZip::unzReadCurrentFile(unzFile file, voidp buf, unsigned len)
{
  int err=UNZ_OK;
  uInt iRead = 0;
  unz64_s* s;
  file_in_zip64_read_info_s* pfile_in_zip_read_info;
  if (file==NULL)
    return UNZ_PARAMERROR;
  s=(unz64_s*)file;
  pfile_in_zip_read_info=s->pfile_in_zip_read;

  if (pfile_in_zip_read_info==NULL)
    return UNZ_PARAMERROR;


  if ((pfile_in_zip_read_info->read_buffer == NULL))
        return UNZ_END_OF_LIST_OF_FILE;
  if (len==0)
    return 0;

  pfile_in_zip_read_info->stream.next_out = (Bytef*)buf;

  pfile_in_zip_read_info->stream.avail_out = (uInt)len;

  if ((len>pfile_in_zip_read_info->rest_read_uncompressed) &&
      (!(pfile_in_zip_read_info->raw)))
    pfile_in_zip_read_info->stream.avail_out = (uInt)pfile_in_zip_read_info->rest_read_uncompressed;

  if ((len>pfile_in_zip_read_info->rest_read_compressed+
      pfile_in_zip_read_info->stream.avail_in) &&
      (pfile_in_zip_read_info->raw))
    pfile_in_zip_read_info->stream.avail_out = (uInt)pfile_in_zip_read_info->rest_read_compressed+
      pfile_in_zip_read_info->stream.avail_in;

  while (pfile_in_zip_read_info->stream.avail_out>0)
  {
    if ((pfile_in_zip_read_info->stream.avail_in==0) &&
        (pfile_in_zip_read_info->rest_read_compressed>0))
    {
      uInt uReadThis = UNZ_BUFSIZE;
      if (pfile_in_zip_read_info->rest_read_compressed<uReadThis)
        uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed;
      if (uReadThis == 0)
        return UNZ_EOF;
      if (ZSEEK64(pfile_in_zip_read_info->z_filefunc,
                  pfile_in_zip_read_info->filestream,
                  pfile_in_zip_read_info->pos_in_zipfile +
                  pfile_in_zip_read_info->byte_before_the_zipfile,
                  ZLIB_FILEFUNC_SEEK_SET)!=0)
        return UNZ_ERRNO;
      if (ZREAD64(pfile_in_zip_read_info->z_filefunc,
          pfile_in_zip_read_info->filestream,
          pfile_in_zip_read_info->read_buffer,
          uReadThis)!=uReadThis)
          return UNZ_ERRNO;

      pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

      pfile_in_zip_read_info->rest_read_compressed-=uReadThis;

      pfile_in_zip_read_info->stream.next_in = (Bytef*)pfile_in_zip_read_info->read_buffer;
      pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis;
    }
  
    if ((pfile_in_zip_read_info->compression_method==0) || (pfile_in_zip_read_info->raw))
    {
      uInt uDoCopy,i ;

      if ((pfile_in_zip_read_info->stream.avail_in == 0) &&
          (pfile_in_zip_read_info->rest_read_compressed == 0))
        return (iRead==0) ? UNZ_EOF : iRead;

      if (pfile_in_zip_read_info->stream.avail_out <
          pfile_in_zip_read_info->stream.avail_in)
        uDoCopy = pfile_in_zip_read_info->stream.avail_out ;
      else
        uDoCopy = pfile_in_zip_read_info->stream.avail_in ;

      for (i=0;i<uDoCopy;i++)
        *(pfile_in_zip_read_info->stream.next_out+i) = *(pfile_in_zip_read_info->stream.next_in+i);

      pfile_in_zip_read_info->total_out_64 = pfile_in_zip_read_info->total_out_64 + uDoCopy;

      pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,
                                            pfile_in_zip_read_info->stream.next_out,
                                            uDoCopy);
      pfile_in_zip_read_info->rest_read_uncompressed-=uDoCopy;
      pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
      pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
      pfile_in_zip_read_info->stream.next_out += uDoCopy;
      pfile_in_zip_read_info->stream.next_in += uDoCopy;
      pfile_in_zip_read_info->stream.total_out += uDoCopy;
      iRead += uDoCopy;
    }
    else 
    {
      ZPOS64_T uTotalOutBefore,uTotalOutAfter;
      const Bytef *bufBefore;
      ZPOS64_T uOutThis;
      int flush=Z_SYNC_FLUSH;

      uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
      bufBefore = pfile_in_zip_read_info->stream.next_out;

      /*
      if ((pfile_in_zip_read_info->rest_read_uncompressed ==
          pfile_in_zip_read_info->stream.avail_out) &&
          (pfile_in_zip_read_info->rest_read_compressed == 0))
        flush = Z_FINISH;
      */
      err=inflate(&pfile_in_zip_read_info->stream,flush);

      if ((err>=0) && (pfile_in_zip_read_info->stream.msg!=NULL))
        err = Z_DATA_ERROR;

      uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
      uOutThis = uTotalOutAfter-uTotalOutBefore;

      pfile_in_zip_read_info->total_out_64 = pfile_in_zip_read_info->total_out_64 + uOutThis;

      pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,bufBefore,
        (uInt)(uOutThis));

      pfile_in_zip_read_info->rest_read_uncompressed -= uOutThis;

      iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);

      if (err==Z_STREAM_END)
        return (iRead==0) ? UNZ_EOF : iRead;
      if (err!=Z_OK)
        break;
     }
   }

   if (err==Z_OK)
     return iRead;
   return err;
}

int UnCompressZip::unzGoToNextFile (unzFile  file)
{
  unz64_s* s;
  int err;

  if (file==NULL)
    return UNZ_PARAMERROR;
  s=(unz64_s*)file;
  if (!s->current_file_ok)
    return UNZ_END_OF_LIST_OF_FILE;
  if (s->gi.number_entry != 0xffff)    /* 2^16 files overflow hack */
    if (s->num_file+1==s->gi.number_entry)
      return UNZ_END_OF_LIST_OF_FILE;

  s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.size_filename +
    s->cur_file_info.size_file_extra + s->cur_file_info.size_file_comment ;
  s->num_file++;
  err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
    &s->cur_file_info_internal,
    NULL,0,NULL,0,NULL,0);
  s->current_file_ok = (err == UNZ_OK);
  return err;
}

void UnCompressZip::SetUnCompressPath(CString s)
{
  m_uncompresspath = s;
}

int UnCompressZip::unzClose (unzFile file)
{
  unz64_s* s;
  if (file==NULL)
    return UNZ_PARAMERROR;
  s=(unz64_s*)file;

  if (s->pfile_in_zip_read!=NULL)
    unzCloseCurrentFile(file);

  ZCLOSE64(s->z_filefunc, s->filestream);
  TRYFREE(s);
  return UNZ_OK;
}