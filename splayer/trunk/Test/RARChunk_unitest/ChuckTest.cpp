// ChuckTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "..\..\include\libunrar\dll.hpp"

#include <stdio.h>

#include <string>
#include <vector>

// Convert a std::wstring to a std::string
std::string WStringToString(const std::wstring& s)
{
  char* ch;
  UINT bytes = WideCharToMultiByte(CP_ACP, 0, s.c_str(), -1, NULL, 0,
    NULL, NULL); 
  ch = new char[bytes];
  if(ch)
    bytes = WideCharToMultiByte(CP_ACP, 0, s.c_str(), -1, ch, bytes,
    NULL, NULL);
  std::string str = ch;
  delete[] ch;
  return str;
}

int compare_filebinary(FILE* filehandle, unsigned long long filepos, char* buff, unsigned long buffsize, int debug)
{
  char *fbuf = (char *)malloc(buffsize);
  _fseeki64(filehandle, filepos, SEEK_SET);
  fread_s(fbuf, buffsize, sizeof(char), buffsize, filehandle);
  int ret = memcmp(buff, fbuf, buffsize);
  
  if (debug)
  {
    for (int i = 0;i < 100;i++)
      wprintf(L"%02x", (DWORD)buff[i] & 0xff);
    wprintf(L"\n");
    for (int i = 0;i < 100;i++)
      wprintf(L"%02x", (DWORD)fbuf[i] & 0xff);
    wprintf(L"\n");


  }

  free(fbuf);
  return ret;
}

int _tmain(int argc, _TCHAR* argv[])
{
  HANDLE rar_handle = NULL;
  //std::wstring fn_rar = L"E:\\-=eMule=-\\1140746814_39741.rar";
  //std::wstring fn_rar = L"E:\\-=eMule=-\\Bride.Flight.2008.Bluray.720p.AC3.x264-CHD_新娘航班\\B2_新娘航班.part1.rar";
  //std::wstring fn_rar = L"D:\\xxxx.part1.rar";
  std::wstring fn_rar = L"E:\\-=eMule=-\\Overheard.2009.REPACK.CN.DVDRip.Xvid-XTM\\sample\\xtm-overheard.repack-sample_stored.rar";
  //std::wstring fn_rar = L"E:\\-=eMule=-\\Overheard.2009.REPACK.CN.DVDRip.Xvid-XTM\\sample\\xtm-overheard.repack-sample_s.part1.rar";
  
  std::wstring tmp_dir = L"C:\\Temp\\";
  
  struct RAROpenArchiveDataEx ArchiveDataEx;
  memset(&ArchiveDataEx, 0, sizeof(ArchiveDataEx));
  
  ArchiveDataEx.ArcNameW = (wchar_t*)fn_rar.c_str();
  //ArchiveDataEx.ArcName = "E:\\-=eMule=-\\1140746814_39741.rar";

  ArchiveDataEx.OpenMode = RAR_OM_EXTRACT;
  ArchiveDataEx.CmtBuf = 0;
  rar_handle = RAROpenArchiveEx(&ArchiveDataEx);

  const long buffsize = 1000;
  char testbuff[buffsize];

  if (rar_handle)
  {
    wprintf(L"RAROpenArchiveEx open successed\n");
    struct RARHeaderDataEx HeaderDataEx;
    HeaderDataEx.CmtBuf = NULL;
    while (RARReadHeaderEx(rar_handle, &HeaderDataEx) == 0)
    {
      // 正常解压该文件
      unsigned long long filesize = ((unsigned long long)HeaderDataEx.UnpSizeHigh << 32) + HeaderDataEx.UnpSize;
      int err = 0;
      std::wstring unp_tmpfile = tmp_dir + HeaderDataEx.FileNameW;
      err = RARProcessFileW(rar_handle, RAR_EXTRACT, (wchar_t*)tmp_dir.c_str(), HeaderDataEx.FileNameW);
      wprintf(L"RARProcessFileW to %s return %d size %lld %x %x\n", unp_tmpfile.c_str(), err, filesize, HeaderDataEx.UnpVer, HeaderDataEx.Method);
    }
    RARCloseArchive(rar_handle);
  }

  rar_handle = RAROpenArchiveEx(&ArchiveDataEx);
  if (rar_handle)
  {
    wprintf(L"RAROpenArchiveEx open successed\n");
    struct RARHeaderDataEx HeaderDataEx;
    HeaderDataEx.CmtBuf = NULL;
    while (RARReadHeaderEx(rar_handle, &HeaderDataEx) == 0)
    {
      unsigned long long filesize = ((unsigned long long)HeaderDataEx.UnpSizeHigh << 32) + HeaderDataEx.UnpSize;
      int err = 0;
      std::wstring unp_tmpfile = tmp_dir + HeaderDataEx.FileNameW;
      err = RARExtractChunkInit(rar_handle, HeaderDataEx.FileName);
      if (err != 0)
      {
        wprintf(L"RARExtractChunkInit return error %d\n", err);
        continue;
      }

      FILE* unp_filehandle = NULL;
      err = _wfopen_s(&unp_filehandle, unp_tmpfile.c_str(), L"rb");
      if (err)
      {
        wprintf(L"open extracted file fail %d %d\n", err, unp_filehandle);
        continue;
      }

      
      // 顺序测试
      int iExtractRet = 0;
      unsigned long long fpos = 0;
      do
      {
        iExtractRet = RARExtractChunk(rar_handle, (char*)testbuff, buffsize);
        // Compare 
        if (compare_filebinary(unp_filehandle, fpos, testbuff, iExtractRet, 0))
        {
          wprintf(L"Sequence compare difference found at %lld for %d\n", fpos, buffsize);
          break;
        }
        //else
        //  wprintf(L"Sequence compare is same %lld %d\n", fpos, iExtractRet);

        fpos += iExtractRet;
      } while(iExtractRet > 0);

      // 随机测试
      for (int i = 0; i < 100; i++)
      {
        unsigned long long ll_pos = rand() * filesize/RAND_MAX;
        RARExtractChunkSeek(rar_handle, ll_pos, SEEK_SET);
        RARExtractChunk(rar_handle, (char*)testbuff, buffsize);
        // Compare 
        if (compare_filebinary(unp_filehandle, ll_pos, testbuff, iExtractRet, 0))
        {
          wprintf(L"Random compare difference found at %lld\n", ll_pos);
          break;
        }
        //else
        //  wprintf(L"Random compare is same %lld\n", ll_pos);
      }
      wprintf(L"RARExtractChunk test for %s finished\n", unp_tmpfile.c_str());
    }
    RARCloseArchive(rar_handle);
  }
  wprintf(L"Test finished\n");
  scanf_s("%d");
	return 0;
}

