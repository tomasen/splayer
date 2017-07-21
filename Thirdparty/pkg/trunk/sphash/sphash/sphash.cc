// SPHash.cc : Defines the exported functions for the DLL application.
//

#include "pch.h"
#include "sphash.h"
#ifdef WIN32
#include "unrar.hpp"
#endif
#include "MD5Checksum.h"

using namespace std;

// files format:
// d:\\a\\a1.ext\0d:\\a\\a2.ext\0\0
vector<wstring> get_filelist(const wchar_t* file)
{
  vector<wstring> filelist;
  wstring filepath;

  int num_null = 0;
  int char_index = 0;
  int last_char_index = 0;
  while (true)
  {
    switch (file[char_index])
    {
    case 0:
      num_null++;
      if (num_null >= 2)
        break;
      filepath.assign(&file[last_char_index], char_index - last_char_index);
      filelist.push_back(filepath);
      last_char_index = char_index+1;
      break;
    default:
      num_null = 0;
      break;
    }
    char_index++;
    if (num_null >= 2)
      break;
  }

  return filelist;
}

#ifdef WIN32
// rar file format
// rar://C:\\Project\\SAMPLE.rar?SAMPLE.mkv
int rarfile_handle(wstring &path, wstring &rarpath)
{
  wstring prefix = path.substr(0, 6);
  transform(prefix.begin(), prefix.end(), prefix.begin(), tolower);

  if (prefix != L"rar://")
    return 0;

  int pos = path.find('?');
  if (pos == path.npos)
    return 0;

  rarpath = path.substr(pos+1, path.length()-pos-1);
  path = path.substr(6, pos-6);

  return 1;
}
#endif

int file_exist(wstring &path, wstring &newpath)
{
#ifdef WIN32
  rarfile_handle(path, newpath);
  struct _stat sbuf;
  return (!_wstat(path.c_str(), &sbuf) && sbuf.st_mode & _S_IFREG);
#endif

#ifdef _MAC_
  struct stat sbuf;
  return (!stat(Utf8(path.c_str()), &sbuf) && sbuf.st_mode & S_IFREG);
#endif  
}

string bintotext_hex(unsigned char *buf, size_t buf_size)
{
  stringstream textstream;
  for (size_t i = 0; i < buf_size; i++)
    textstream << setw(2) << setfill('0') << hex << (unsigned int)buf[i];

  return textstream.str();
}
int modhash_binary(unsigned char md5buf[16], char* result_inout, int* result_len)
{
  if (result_len <= 0)
    return 1;
	
  MD5Checksum md5;
  md5.GetMD5((unsigned char*)result_inout, *result_len);
  memcpy(md5buf, md5.lpszMD5, 16);

  return 0;
}

int modhash_file(unsigned char md5buf[16], const wchar_t* file_name, char* result_inout, int* result_len)
{
  vector<wstring> filelist = get_filelist(file_name);
  for (vector<wstring>::iterator it = filelist.begin(); it != filelist.end(); it++)
  {
    MD5Checksum md5;
    wstring strbuf;
    wstring newpath, path = *it;

    if (!file_exist(path, newpath))
      return 1;

    strbuf = md5.GetMD5(path);
    if (strbuf.empty())
      continue;

    if (it == filelist.begin())
      memcpy(md5buf, md5.lpszMD5, 16);
    else 
      for (int j = 0; j < 16; j++)
        md5buf[j] ^= md5.lpszMD5[j];
  } // for filelist

  return 0;
}

int modhash_video(vector<vector<unsigned char> > &strbuf, const wchar_t* file_name, char* result_inout, int* result_len)
{
  wstring path(file_name);
  wstring rarpath;

  __int64 offset[4];
#ifdef WIN32
  if (rarfile_handle(path, rarpath))
  {
    struct RAROpenArchiveDataEx rarst;
    memset(&rarst, 0, sizeof(rarst));

    rarst.ArcNameW  = (LPWSTR)path.c_str();
    rarst.OpenMode  = RAR_OM_EXTRACT;
    rarst.CmtBuf    = 0;

    HANDLE hrar = RAROpenArchiveEx(&rarst);
    if (hrar)
    {
      struct RARHeaderDataEx HeaderDataEx;
      HeaderDataEx.CmtBuf = NULL;

      while (RARReadHeaderEx(hrar, &HeaderDataEx) == 0)
      {
        if (HeaderDataEx.Method != 0x30) // only support stored file
          break;

        wstring subfn = HeaderDataEx.FileNameW;
        if (subfn.compare(rarpath.c_str()) == 0)
        {
          int errRar = RARExtractChunkInit(hrar, HeaderDataEx.FileName);
          if (errRar != 0)
            break;

          __int64 ftotallen = HeaderDataEx.UnpSize;

          if (ftotallen >= 8192)
          {
            offset[3] = ftotallen - 8192;
            offset[2] = ftotallen / 3;
            offset[1] = ftotallen / 3 * 2;
            offset[0] = 4096;
            MD5Checksum md5;
            char buf[4096];
            for (int i = 0; i < 4; i++)
            {
              RARExtractChunkSeek(hrar, offset[i], SEEK_SET);
              //hash 4k block
              RARExtractChunk(hrar, (char*)buf, 4096);
              md5.GetMD5((unsigned char*)buf, 4096);
              vector<unsigned char> str(16);
              memcpy(&str[0], md5.lpszMD5, 16);
              strbuf.push_back(str);
            }
          }
          RARExtractChunkClose(hrar);
          break;
        }
        RARProcessFile(hrar, RAR_SKIP, NULL, NULL);
      } // while
      RARCloseArchive(hrar);
    }
  } // if (rarfile_handle(path))
#endif
  
  if (strbuf.empty())
  {
    
    int stream;
#ifdef WIN32   
    if (!_wsopen_s(&stream, path.c_str(), _O_BINARY|_O_RDONLY, _SH_DENYNO, _S_IREAD))
#endif
#ifdef _MAC_
    stream = open(Utf8(path.c_str()), O_RDONLY);
    if (stream >= 0)
#endif      
    {
      __int64 ftotallen  = 0;
#ifdef WIN32      
      ftotallen  = _filelengthi64(stream);
#endif
#ifdef _MAC_
      struct stat sbuf;
      if (!fstat(stream, &sbuf))
        ftotallen = sbuf.st_size;
#endif      
      if (ftotallen >= 8192)
      {
        offset[3] = ftotallen - 8192;
        offset[2] = ftotallen / 3;
        offset[1] = ftotallen / 3 * 2;
        offset[0] = 4096;
        MD5Checksum md5;
        unsigned char Buf[4096];
        for (int i = 0; i < 4; i++)
        {
#ifdef WIN32
          _lseeki64(stream, offset[i], 0);
#endif
#ifdef _MAC_
          lseek(stream, offset[i], 0);
#endif
#ifdef WIN32
          //hash 4k block
          int readlen = _read( stream, Buf, 4096);
#endif
#ifdef _MAC_
          int readlen = read( stream, Buf, 4096);
#endif
          md5.GetMD5(Buf, readlen);
          vector<unsigned char> str(16);
          memcpy(&str[0], md5.lpszMD5, 16);
          strbuf.push_back(str);
        }
      }
#ifdef WIN32
      _close(stream);
#endif
#ifdef _MAC_
      close(stream);
#endif
    }
  }

  return 0;
}

void algo_md5(int hash_mode, const wchar_t* file_name, 
                  char* result_inout, int* result_len)
{
  switch (hash_mode)
  {
  case HASH_MOD_BINARY_STR:
  case HASH_MOD_BINARY_BIN:
  case HASH_MOD_FILE_BIN:
  case HASH_MOD_FILE_STR:
    {
      // this is ugly, fix it later
      unsigned char md5buf[16];
      switch (hash_mode)
      {
        case HASH_MOD_BINARY_STR:
        case HASH_MOD_BINARY_BIN:
          if (modhash_binary(md5buf, result_inout, result_len) != 0)
            return;
          break;
        case HASH_MOD_FILE_BIN:
        case HASH_MOD_FILE_STR:
          if (modhash_file(md5buf, file_name, result_inout, result_len) != 0)
            return;
          break;
      }

      if (hash_mode == HASH_MOD_FILE_BIN || hash_mode == HASH_MOD_BINARY_BIN)
      {
        *result_len = 16;
        memcpy(result_inout, md5buf, *result_len);
      }
      else
      {
        string text = bintotext_hex(md5buf, sizeof(md5buf));
        text.push_back(0);
        *result_len = text.size();
        memcpy(result_inout, text.c_str(), *result_len);
      }
    }
    break;
  case HASH_MOD_VIDEO_STR:
    {
      vector<vector<unsigned char> > strbuf;
      if (modhash_video(strbuf, file_name, result_inout, result_len) != 0)
        return;

      string text;
      for (vector<vector<unsigned char> >::iterator it = strbuf.begin(); it != strbuf.end(); it++)
      {
        if (it != strbuf.begin())
          text += ';';

        text += bintotext_hex(&(*it)[0], (*it).size());
      }
      text.push_back(0);
      *result_len = text.size();
      memcpy(result_inout, text.c_str(), *result_len);
    }
    break;
  default:
    break;
  }
}

void hash_file(int hash_mode, int hash_algorithm, 
               const wchar_t* file_name, 
               char* result_inout, int* result_len)
{
  *result_len = 0;

  switch (hash_algorithm)
  {
  case HASH_ALGO_MD5:
    algo_md5(hash_mode, file_name, result_inout, result_len);
    break;
  default:
    break;
  }
}

void hash_data(int hash_mode, int hash_algorithm, 
               char* result_inout, int* result_len)
{

  switch (hash_algorithm)
  {
  case HASH_ALGO_MD5:
    algo_md5(hash_mode, NULL, result_inout, result_len);
    break;
  default:
    break;
  }
}