#include "SVPhash.h"
#include "MD5Checksum.h"
#include "SVPToolBox.h"
#include "..\..\include\libunrar\dll.hpp"
#include "SVPRarLib.h"

#define SVP_LogMsg5 __noop
#define SVP_LogMsg3 __noop


CSVPhash::CSVPhash(void)
{
}

CSVPhash::~CSVPhash(void)
{
}
std::wstring CSVPhash::ComputerSubFilesFileHash(
  std::vector<std::wstring>* szaSubFiles)
{
  unsigned char md5buf[16];
  for (std::vector<std::wstring>::iterator iter = szaSubFiles -> begin();
    iter != szaSubFiles -> end(); iter++)
  {
    CMD5Checksum cmd5 ;
    std::wstring szpath = *iter;
    CSVPToolBox svpTool;
    if (!svpTool.ifFileExist_STL(szpath.c_str()))
    {
      SVP_LogMsg(_T("sub file not exist for hash"));
      return _T("");
      break;
    }
    std::wstring szBuf = cmd5.GetMD5(szpath);
    if (!szBuf.empty())
    {
      //SVP_LogMsg(szBuf);
      if (iter == szaSubFiles -> begin())
        memcpy_s(md5buf, 16, cmd5.lpszMD5, 16);
      else
        for(int j = 0; j < 16; j++)
          md5buf[j] ^= cmd5.lpszMD5[j];
    }
    else
      SVP_LogMsg(_T("md5 error"));
  }
  return (LPCTSTR)HexToString(md5buf);
}
CString CSVPhash::HexToString(BYTE* lpszMD5){
	//Convert the hexadecimal checksum to a CString
	CString strMD5;
	for ( int i=0; i < 16; i++) 
	{
		CString Str;
		if (lpszMD5[i] == 0) {
			Str = CString(_T("00"));
		}
		else if (lpszMD5[i] <= 15) 	{
			Str.Format(_T("0%x"),lpszMD5[i]);
		}
		else {
			Str.Format(_T("%x"),lpszMD5[i]);
		}

		ASSERT( Str.GetLength() == 2 );
		strMD5 += Str;
	}
	ASSERT( strMD5.GetLength() == 32 );
	return strMD5;
}
CString CSVPhash::ComputerFileHash(CString szFilePath)
{
  return ComputerFileHash_STL((LPCTSTR)szFilePath).c_str();
}
std::wstring CSVPhash::ComputerFileHash_STL(std::wstring szFilePath)
{

  std::wstring szRet = L"";
  __int64 offset[4];


  CSVPRarLib svpRar;
  if(svpRar.SplitPath_STL(szFilePath))
  {
    // this is RAR path
    SVP_LogMsg5(_T(" CSVPhash::ComputerFileHash_STL this is RAR path %s "),
      svpRar.m_fnRAR);

    struct RAROpenArchiveDataEx ArchiveDataEx;
    memset(&ArchiveDataEx, 0, sizeof(ArchiveDataEx));

    ArchiveDataEx.ArcNameW = (LPTSTR)(LPCTSTR)svpRar.m_fnRAR;
    char fnA[MAX_PATH];
    size_t ConvertedChars;
    if(wcstombs_s(&ConvertedChars, fnA, MAX_PATH, svpRar.m_fnRAR,
      svpRar.m_fnRAR.GetLength()+1) == -1)
      fnA[0] = 0;
    ArchiveDataEx.ArcName = fnA;

    ArchiveDataEx.OpenMode = RAR_OM_EXTRACT;
    ArchiveDataEx.CmtBuf = 0;
    HANDLE hrar = RAROpenArchiveEx(&ArchiveDataEx);
    if(!hrar) 
      return szRet;

    struct RARHeaderDataEx HeaderDataEx;
    HeaderDataEx.CmtBuf = NULL;

    while(RARReadHeaderEx(hrar, &HeaderDataEx) == 0)
    {
      std::wstring subfn = HeaderDataEx.FileNameW;

      SVP_LogMsg5(L"Got m_fnInsideRar RAR path %s %s " , svpRar.m_fnInsideRar,
        subfn.c_str());
      if(subfn.compare((LPCTSTR)svpRar.m_fnInsideRar) == 0)
      {

        int errRar = RARExtractChunkInit(hrar, HeaderDataEx.FileName);
        if (errRar != 0)
        {
          RARCloseArchive(hrar);
          break;
        }

        __int64 ftotallen = HeaderDataEx.UnpSize;

        if (ftotallen < 8192)
        {
          //a video file less then 8k? impossible!
        }
        else
        {
          offset[3] = ftotallen - 8192;
          offset[2] = ftotallen / 3;
          offset[1] = ftotallen / 3 * 2;
          offset[0] = 4096;
          CMD5Checksum mMd5;
          char bBuf[4096];
          for(int i = 0; i < 4;i++)
          {
            RARExtractChunkSeek(hrar, offset[i], SEEK_SET);
            //hash 4k block
            int readlen = RARExtractChunk(hrar, (char*)bBuf, 4096);
            std::wstring szMD5 = mMd5.GetMD5((unsigned char*)bBuf, 4096);
            if (!szRet.empty())
              szRet.append(L";");
            szRet.append(szMD5);
          }
        }
        RARExtractChunkClose(hrar);
        break;
      }
      RARProcessFile(hrar, RAR_SKIP, NULL, NULL);
    }
    RARCloseArchive(hrar);
  }
  else
  {
    int stream;
    errno_t err;
    unsigned long timecost = GetTickCount();
    err =  _wsopen_s(&stream, szFilePath.c_str(),
      _O_BINARY|_O_RDONLY , _SH_DENYNO , _S_IREAD);
    if (!err)
    {
      __int64 ftotallen  = _filelengthi64(stream);
      if (ftotallen < 8192)
      {
        //a video file less then 8k? impossible!
      }
      else
      {
        offset[3] = ftotallen - 8192;
        offset[2] = ftotallen / 3;
        offset[1] = ftotallen / 3 * 2;
        offset[0] = 4096;
        CMD5Checksum mMd5;
        unsigned char bBuf[4096];
        for(int i = 0; i < 4;i++)
        {
          _lseeki64(stream, offset[i], 0);
          //hash 4k block
          int readlen = _read( stream, bBuf, 4096);
          std::wstring szMD5 = mMd5.GetMD5(bBuf, readlen); 
          if(!szRet.empty())
            szRet.append(L";");
          szRet.append(szMD5);
        }
      }
      _close(stream);
    }
    timecost =  GetTickCount() - timecost;
  }



  //szFilePath.Format(_T("Vid Hash Cost %d milliseconds "), timecost);
  //SVP_LogMsg(szFilePath);
  return szRet;
}
