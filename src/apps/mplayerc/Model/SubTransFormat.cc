#include "stdafx.h"
#include <sys/stat.h>
#include "SubTransFormat.h"

#include <Strings.h>
#include "../revision.h"

#undef __MACTYPES__
#include "../../../zlib/zlib.h"
#include "unrar.hpp"
#include "../resource.h"
#include <shooterclient.key>

#include <vector>
#include <fstream>

#include "..\Controller\HashController.h"
#include <logging.h>

#define CHAR4TOINT(szBuf) \
  ( ((int)szBuf[0] & 0xff) << 24) | ( ((int)szBuf[1] & 0xff) << 16) | ( ((int)szBuf[2] & 0xff) << 8) |  szBuf[3] & 0xff

#define SVP_MIN(a, b)  (((a) < (b)) ? (a) : (b))

#define SVPATH_BASENAME 0  //Without Dot
#define SVPATH_EXTNAME 1  //With Dot
#define SVPATH_DIRNAME 2 //With Slash
#define SVPATH_FILENAME 3  //Without Dot

#define fansub_search_buf 30000
#define UNIQU_HASH_SIZE 512

int SubTransFormat::ExtractDataFromAiSubRecvBuffer_STL(std::list<std::wstring> *m_tmphandlemsgs, std::wstring szFilePath,
                                                       std::wstring tmpoutfile, std::vector<std::wstring> &szaSubDescs,
                                                       std::vector<std::wstring> &tmpfiles)
{
  FILE *sAiSubRecvBuff;
  if (_wfopen_s(&sAiSubRecvBuff, tmpoutfile.c_str(), L"rb") != 0)
    return 1;

  char szSBuff[2] = {0,0};
  int ret = 0;

  fseek(sAiSubRecvBuff, 0, SEEK_SET); // move point yo begining of file

  fread(szSBuff, sizeof(char), 1, sAiSubRecvBuff);

  int iStatCode = szSBuff[0];
  if (iStatCode <= 0)
  {
    if (iStatCode == -1)
      ret = -404;
    else
      ret = -1;
    goto releaseALL;
  }
  else
    m_tmphandlemsgs->push_back(ResStr_STL(IDS_LOG_MSG_SVPSUB_GOTMATCHED_AND_DOWNLOADING));

  for(int j = 0; j < iStatCode; j++)
  {
    int exterr = HandleSubPackage(sAiSubRecvBuff, szaSubDescs, tmpfiles);
    if(exterr)
    {
      ret = exterr;
      break;
    }
  }


releaseALL:
  fclose(sAiSubRecvBuff);
  _wremove(tmpoutfile.c_str());
  return ret;
}

int SubTransFormat::HandleSubPackage(FILE* fp, std::vector<std::wstring> &szaSubDescs,
                     std::vector<std::wstring> &tmpfiles)
{
  //Extract Package

  char szSBuff[8];
  int err;
  if ( fread(szSBuff , sizeof(char), 4, fp) < 4)
    return -1;

  int iPackageLength = CHAR4TOINT(szSBuff);

  if ( fread(szSBuff , sizeof(char), 4, fp) < 4)
    return -2;

  size_t iDescLength = CHAR4TOINT(szSBuff);

  if (iDescLength > 0)
  {
    char * szDescData = ReadToPTCharByLength(fp, iDescLength);
    if (!szDescData)
      return -4;

    // convert szDescData to Unicode and save to CString
    std::string data(szDescData);
    szaSubDescs.push_back(Strings::Utf8StringToWString(data));
    free(szDescData);
  }
  else
    szaSubDescs.push_back(L"");

  err = ExtractSubFiles(fp, tmpfiles);
  return 0;
}

char* SubTransFormat::ReadToPTCharByLength(FILE* fp, size_t length)
{
  char * szBuffData =(char *)calloc( length + 1, sizeof(char));

  if(!szBuffData)
    return 0;

  if ( fread(szBuffData , sizeof(char), length, fp) < length)
    return 0;

  return szBuffData;
}

int SubTransFormat::ExtractSubFiles(FILE* fp, std::vector<std::wstring> &tmpfiles)
{
  char szSBuff[8];
  if ( fread(szSBuff , sizeof(char), 4, fp) < 4)
    return -1;

  size_t iFileDataLength = CHAR4TOINT(szSBuff); //确认是否确实有文件下载
  if (!iFileDataLength)
    return 0;

  if ( fread(szSBuff , sizeof(char), 1, fp) < 1)
    return -2;

  int iFileCount = szSBuff[0];
  if ( iFileCount <= 0 )
    return -3;

  for (int k = 0; k < iFileCount; k++){
    if(ExtractEachSubFile(fp, tmpfiles) )
      return -4;
  }

  return 0;
}

int SubTransFormat::ExtractEachSubFile(FILE* fp, std::vector<std::wstring> &tmpfiles)
{
  // get file ext name
  char szSBuff[4096];
  if ( fread(szSBuff , sizeof(char), 4, fp) < 4)
    return -1;

  if ( fread(szSBuff , sizeof(char), 4, fp) < 4)
    return -1;

  size_t iExtLength = CHAR4TOINT(szSBuff);
  char* szExtName = ReadToPTCharByLength(fp, iExtLength);
  if(!szExtName)
    return -2;

  //get filedata length
  if ( fread(szSBuff , sizeof(char), 4, fp) < 4)
    return -1;

  size_t iFileLength = CHAR4TOINT(szSBuff);

  // gen tmp name and tmp file point
  std::wstring otmpfilename = GetTempFileName();
  if(otmpfilename.empty())
    return -5;

  FILE* fpt;
  errno_t err = _wfopen_s(&fpt, otmpfilename.c_str(), L"wb");
  if(err)
    return -4;

  // copy date to tmp file
  size_t leftoread = iFileLength;
  do{
    size_t needtoread = SVP_MIN(4096, leftoread);
    size_t accturead = fread(szSBuff , sizeof(char), needtoread, fp);
    if(accturead == 0){
      //wtf
      break;
    }
    leftoread -= accturead;
    err = fwrite(szSBuff,  sizeof(char), accturead , fpt);


  }while(leftoread > 0);
  fclose( fpt );

  std::wstring otmpfilenameraw = GetTempFileName();
  std::wstring szLogmsg, of1, of2;
  of1 = otmpfilename;
  of2 = otmpfilenameraw;
  int gzret = UnpackGZFile(of1 , of2);

  // add filename and tmp name to szaTmpFileNames
  std::wstring str;
  std::string extname(szExtName);
  str = Strings::Utf8StringToWString(extname);
  str += L"|";

  std::wstring str2 = otmpfilenameraw;
  str += str2.c_str();
  str += L";";

  tmpfiles.push_back(str);

  free(szExtName);

  return 0;
}

std::wstring SubTransFormat::GetTempFileName()
{
  wchar_t tmppath[MAX_PATH];
  if (::GetTempFileName(GetTempDir().c_str(), L"svp", NULL, tmppath) == 0)
    return L"";
  else
    return tmppath;
}

int SubTransFormat::UnpackGZFile(std::wstring fnin, std::wstring fnout)
{

  FILE* fout;
  int ret = 0;
  if ( _wfopen_s( &fout, fnout.c_str(), L"wb" ) != 0){
    return -1; //output file open error
  }

  std::string szFnin = Strings::WStringToUtf8String(fnin).c_str();

  gzFile gzfIn = gzopen( szFnin.c_str() , "rb");	
  if (gzfIn){

    char buff[4096];
    int iBuffReadLen ;
    do{
      iBuffReadLen = gzread(gzfIn, buff, 4096 );
      if(iBuffReadLen > 0 ){
        if( fwrite(buff, sizeof( char ),  iBuffReadLen, fout) <= 0 ){
          //file write error
          ret = 1;
        }
      }else if(iBuffReadLen < 0){
        ret = -3; //decompress error
        break;
      }else{
        break;
      }
    }while(1);

    fclose(fout);
    gzclose(gzfIn);
  }else{
    ret = -2; //gz file open error
  }

  return ret;
}

std::wstring SubTransFormat::GetTempDir()
{
  wchar_t lpPathBuffer[MAX_PATH];
  GetTempPath(MAX_PATH,  lpPathBuffer); 

  return lpPathBuffer;
}


int SubTransFormat::PackGZfile(std::wstring fnin, std::wstring fnout)
{

  FILE* fp;
  int ret = 0;
  if ( _wfopen_s( &fp, fnin.c_str(), L"rb" ) != 0)
    return -1; //input file open error

  std::string szFnout = Strings::WStringToUtf8String(fnout);

  gzFile gzfOut = gzopen(szFnout.c_str(), "wb9");	
  if (gzfOut){

    char buff[4096];
    int iBuffReadLen ;
    do{
      iBuffReadLen = fread( buff, sizeof( char), 4096 ,fp);
      if(iBuffReadLen > 0 ){
        if( gzwrite( gzfOut , buff,  iBuffReadLen ) <= 0 ){
          //gz file compress write error
          ret = 1;
        }
      }else if(iBuffReadLen < 0){
        ret = -3; //file read error
        break;
      }else{
        break;
      }
    }while(1);

    fclose(fp);
    gzclose(gzfOut);
  }else{
    ret = -2; //gz file open error
  }

  return ret;
}

std::wstring SubTransFormat::ExtractRarFile(std::wstring rarfn)
{
  std::wstring szRet = L"";

  struct RAROpenArchiveDataEx ArchiveDataEx;
  memset(&ArchiveDataEx, 0, sizeof(ArchiveDataEx));

  ArchiveDataEx.ArcNameW = (wchar_t*)rarfn.c_str();
  char fnA[MAX_PATH];
  if(wcstombs(fnA, (wchar_t*)rarfn.c_str(), rarfn.size()+1) == -1) fnA[0] = 0;
  ArchiveDataEx.ArcName = fnA;

  ArchiveDataEx.OpenMode = RAR_OM_EXTRACT;
  ArchiveDataEx.CmtBuf = 0;
  HANDLE hrar = RAROpenArchiveEx(&ArchiveDataEx);
  if(!hrar)
    return szRet;

  struct RARHeaderDataEx HeaderDataEx;
  HeaderDataEx.CmtBuf = NULL;
  szRet = GetTempFileName();
  szRet +=L".sub";

  //CFile m_sub (szRet.c_str(), CFile::modeCreate|CFile::modeReadWrite|CFile::typeBinary);
  std::fstream io(szRet.c_str(), std::ios::binary);

  while(RARReadHeaderEx(hrar, &HeaderDataEx) == 0)
  {
    std::wstring subfn(HeaderDataEx.FileNameW);

    if(subfn.substr(subfn.size()-4, 4) == L".sub")
    {
      char *buff = new char[HeaderDataEx.UnpSize];
      if (buff == NULL)
      {
        RARCloseArchive(hrar);
        return szRet;
      }

      int errRar = RARExtractChunkInit(hrar, HeaderDataEx.FileName);
      if (errRar != 0) {
        RARCloseArchive(hrar);
        break;
      }

      RARExtractChunk(hrar, (char*)buff, HeaderDataEx.UnpSize);

      io.seekp(std::ios::beg);
      io.write(buff, HeaderDataEx.UnpSize);
      delete [] buff;
      io.close();

      RARExtractChunkClose(hrar);
      break;
    }

    RARProcessFile(hrar, RAR_SKIP, NULL, NULL);
  }

  RARCloseArchive(hrar);
  return szRet;
}

BOOL SubTransFormat::SplitPath_STL(std::wstring fnSVPRarPath, std::wstring &fnrar, std::wstring &fninrar)
{
  std::wstring fnSVPExt = fnSVPRarPath.substr(0, 6);
  std::transform(fnSVPExt.begin(), fnSVPExt.end(), fnSVPExt.begin(), tolower);
  BOOL israr = (fnSVPExt == L"rar://");

  BOOL ret = false;
  if(israr)
  {
    int iPos = fnSVPRarPath.find('?');
    if(iPos != fnSVPRarPath.npos)
    {
      fnrar = fnSVPRarPath.substr(6, iPos - 6).c_str();
      fninrar = fnSVPRarPath.substr(iPos + 1,
        fnSVPRarPath.size() - iPos - 1).c_str();
      ret = true;
    }
  }

  return ret;
}

BOOL SubTransFormat::IfFileExist_STL(std::wstring szPathname, BOOL evenSlowDriver)
{
  std::wstring szPathExt = szPathname.substr(0, 6);
  std::transform(szPathExt.begin(), szPathExt.end(),
    szPathExt.begin(), tolower);
  if (szPathExt ==  L"rar://")
  {
    //RARTODO: 检测rar内的文件是否存在 //Done
    std::wstring fnrar, fninrar;
    if(SplitPath_STL(szPathname, fnrar, fninrar))
      szPathname = fnrar;
    else
      return false;
  }

  if (!evenSlowDriver)
  {
    switch(GetDriveType(szPathname.c_str()))
    {
    case DRIVE_REMOVABLE:
    case DRIVE_FIXED:
    case DRIVE_RAMDISK:					
      break;
    default:
      return true;
      break;
    }
  }
  struct _stat sbuf;

  return (!_wstat(szPathname.c_str(), &sbuf) && _S_IFREG & sbuf.st_mode);
}

std::wstring SubTransFormat::GetVideoFileBasename(std::wstring szVidPath, std::vector<std::wstring>* szaPathInfo)
{
  BOOL bIsRar = false;
  std::wstring fninrar, fnrar;
  if(SplitPath_STL(szVidPath.c_str(), fnrar, fninrar))
  {
    bIsRar = true;
    szVidPath = fnrar.c_str();
  }

  int posDot    = szVidPath.find_last_of(L'.');
  int posSlash  = szVidPath.find_last_of(L'\\');
  int posSlash2 = szVidPath.find_last_of(L'/');
  if (posSlash2 > posSlash)
    posSlash = posSlash2;

  if(posDot > posSlash)
  {
    if (szaPathInfo != NULL)
    {
      std::wstring szBaseName = szVidPath.substr(0, posDot);
      std::wstring szExtName  = szVidPath.substr(posDot, szVidPath.size() - posDot);
      std::transform(szExtName.begin(), szExtName.end(), szExtName.begin(), tolower);
      std::wstring szFileName = szVidPath.substr(posSlash+1, (posDot - posSlash - 1));
      std::wstring szDirName  = szVidPath.substr(0, posSlash + 1);
      szaPathInfo -> clear();
      szaPathInfo -> push_back(szBaseName); // Base Name
      if (bIsRar)
      {
        std::wstring::size_type pos = fninrar.find_last_of('.');
        if(pos != std::wstring::npos)
          szExtName = fninrar.substr(pos);//issus
      }
      szaPathInfo -> push_back(szExtName ); //ExtName
      szaPathInfo -> push_back(szDirName); //Dir Name ()
      szaPathInfo -> push_back(szFileName); // file name only
    }
    return szVidPath.substr(posDot);
  }
  return szVidPath;
}

std::wstring SubTransFormat::GetSameTmpName(std::wstring fnin)
{
  std::wstring fnout = GetTempDir() + fnin;
  _wunlink(fnout.c_str());
  if(IfFileExist_STL(fnout))
    return GetTempFileName();

  return fnout;
}

std::wstring SubTransFormat::GetSubFileByTempid_STL(size_t iTmpID, std::wstring szVidPath,
                                                    std::vector<std::wstring> szaSubDescs,
                                                    std::vector<std::wstring> tmpfiles)
{
  //get base path name
  std::vector<std::wstring> szVidPathInfo;
  std::wstring szTargetBaseName = L"";
  std::wstring szDefaultSubPath = L"";

  std::wstring StoreDir;
  GetVideoFileBasename(szVidPath, &szVidPathInfo); 

  if(StoreDir.empty() || !IfDirExist_STL(StoreDir) || 
    !IfDirWritable_STL(StoreDir))
  {
    GetAppDataPath(StoreDir);
    if (StoreDir[StoreDir.size()-1] != L'\\')
      StoreDir.append(L"\\");

    StoreDir.append(L"SVPSub");
    _wmkdir(StoreDir.c_str());
    if(StoreDir.empty() || !IfDirExist_STL(StoreDir) || 
      !IfDirWritable_STL(StoreDir))
    {
      StoreDir = GetPlayerPath_STL(L"SVPSub");
      _wmkdir(StoreDir.c_str());
      if(StoreDir.empty() || !IfDirExist_STL(StoreDir) || 
        !IfDirWritable_STL(StoreDir))
      {   
        //WTF cant create fordler ?
      }
    }
  }


  if (StoreDir[StoreDir.size() - 1] != L'\\')
    StoreDir.append(L"\\");

  GetVideoFileBasename(szVidPath, &szVidPathInfo);

  std::wstring szBasename(StoreDir);
  szBasename.append(szVidPathInfo.at(SVPATH_FILENAME).c_str());

  //set new file name
  std::vector<std::wstring> szSubfiles;
  std::wstring szXTmpdata = tmpfiles.at(iTmpID);
  Explode(szXTmpdata, L";", &szSubfiles);
  bool bIsIdxSub = FALSE;
  int ialreadyExist = 0;
  if (szXTmpdata.find(L"idx|") != szXTmpdata.npos && szXTmpdata.find(L"sub|") != szXTmpdata.npos)
    if (!IfFileExist_STL(szBasename + L".idx") && !IfFileExist_STL(szBasename + L".sub"))
      bIsIdxSub = TRUE;

  for(size_t i = 0; i < szSubfiles.size(); i++)
  {
    std::vector<std::wstring> szSubTmpDetail;
    Explode(szSubfiles[i], L"|", &szSubTmpDetail);
    if (szSubTmpDetail.size() < 2)
      continue;

    std::wstring szSource = szSubTmpDetail[1];
    std::wstring szLangExt  = L".chn"; //TODO: use correct language perm 
    if(bIsIdxSub)
      szLangExt  = L"";
    else
      szLangExt  = DetectSubFileLanguage_STL(szSource);
    if (szSubTmpDetail[0].at(0) != L'.')
      szSubTmpDetail[0] = L"." + szSubTmpDetail[0];
    std::wstring szTarget = szBasename + szLangExt + szSubTmpDetail[0];
    szTargetBaseName = szBasename + szLangExt ;

    std::wstring szSourceMD5 = HashController::GetInstance()->GetMD5Hash(szSource.c_str());
    std::wstring szTargetMD5;
    //check if target exist
    wchar_t szTmp[128];
    lstrcpy(szTmp, L"");
    int ilan = 1;
    while(IfFileExist_STL(szTarget))
    {
      //TODO: compare if its the same file
      //szTargetMD5 = Strings::StringToWString((std::string)str);
      szTargetMD5 = HashController::GetInstance()->GetMD5Hash(szTarget.c_str());
      if(szTargetMD5 == szSourceMD5)
      {
        // TODO: if there is a diffrence in delay
        ialreadyExist++; //TODO: 如果idx+sub里面只有一个文件相同怎么办 ？？~~ 
        break;
      }

      swprintf_s(szTmp, 128, L"%d", ilan);
      szTarget = szBasename + szLangExt + (LPCTSTR)szTmp + szSubTmpDetail[0];
      szTargetBaseName = szBasename + szLangExt + (LPCTSTR)szTmp;
      ilan++;
    }

    if (!CopyFile(szSource.c_str(), szTarget.c_str(), false))
    {
      LPVOID lpMsgBuf;
      DWORD dw = GetLastError(); 
      szDefaultSubPath = szSource;

      FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

      struct _stat sbuf_d, sbuf_s;

      _wstat(szSource.c_str(), &sbuf_s);
      _wstat(szTarget.c_str(), &sbuf_d);

      // Display the error message and exit the process

      Logging(L"fail to copying subtitle file %x %s from %s %f to %s %f",
         dw, lpMsgBuf, szSource.c_str(), (double)sbuf_s.st_size, 
         szTarget.c_str(), (double)sbuf_d.st_size); 

      ULARGE_INTEGER free1_s, free1_d, free2_s, free2_d;
      GetDiskFreeSpaceEx(szSource.c_str(), &free1_s, NULL, &free2_s);
      GetDiskFreeSpaceEx(szTarget.c_str(), &free1_d, NULL, &free2_d);
      Logging(L" %f %f %f %f", (double)free1_s.QuadPart, (double)free1_d.QuadPart,
              (double)free2_s.QuadPart, (double)free2_d.QuadPart);

      LocalFree(lpMsgBuf);
    }
    else if (((bIsIdxSub && szSubTmpDetail[0].compare(L"idx") == 0)
      || !bIsIdxSub) && szDefaultSubPath.empty())
      szDefaultSubPath = szTarget;

    std::vector<std::wstring> szaDesclines;
    if (szaSubDescs.size() > iTmpID)
    {
      Explode(szaSubDescs.at(iTmpID), L"\x0b\x0b", &szaDesclines);
      if(szaDesclines.size() > 0)
      {
        int iDelay = 0;
        swscanf_s(szaDesclines.at(0).c_str(), L"delay=%d", &iDelay);
        if (iDelay)
        {
          wchar_t szBuf[128];
          swprintf_s(szBuf, 128, L"%d", iDelay);
          FilePutContent(szTarget + L".delay", (LPCTSTR)szBuf, false);
        }
        else
          _wremove((szTarget + L".delay").c_str());

      }
    }

    _wremove(szSource.c_str());
  }
  if(ialreadyExist)
    return L"EXIST";
  else
    return szDefaultSubPath;
}

int SubTransFormat::Explode(std::wstring szIn, std::wstring szTok,
                            std::vector<std::wstring>* szaOut)
{
  szaOut->clear();

  std::wstring resToken;
  int curPos= 0;

  wchar_t* str = new wchar_t[szIn.length() + 1];
  lstrcpy(str, szIn.c_str());
  wchar_t* sep = new wchar_t[szTok.length() + 1];
  lstrcpy(sep, szTok.c_str());
  wchar_t* token = NULL;
  wchar_t* next_token = NULL;
  token = wcstok_s(str, sep, &next_token);

  while (token != NULL)
  {
    szaOut->push_back((LPCTSTR)token);
    token = wcstok_s( NULL, sep, &next_token);
  }

  delete[] str;
  delete[] sep;
  return 0;
}


BOOL SubTransFormat::IfDirExist_STL(std::wstring path)
{
  if (path[path.size()-1] != L'\\')
    path.append(L"\\");
  return !_waccess(path.c_str(), 0);
}

BOOL SubTransFormat::IfDirWritable_STL(std::wstring szDir)
{
  HANDLE hFile =
    CreateFile(szDir.c_str(), FILE_ADD_FILE|FILE_WRITE_ATTRIBUTES|FILE_READ_ATTRIBUTES,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

  if (hFile == INVALID_HANDLE_VALUE)
    return false;


  FILETIME ftCreate, ftAccess, ftWrite;
  if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
    return FALSE;

  BOOL ret = false;
  FILE* fp = NULL;
  fp = _wfopen((szDir + L"svpwrtst").c_str(), L"wb");
  if(fp != NULL)
  {
    fclose( fp );
    _wremove((szDir + L"svpwrtst").c_str());
    ret = true;
  }

  SetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite);

  CloseHandle(hFile);

  return ret;
}

std::wstring SubTransFormat::GetPlayerPath_STL(std::wstring progName)
{
  wchar_t path[MAX_PATH];
  GetModuleFileName(/*AfxGetInstanceHandle()*/NULL, path, MAX_PATH);
  if (progName.empty())
    return (LPCTSTR)path;
  else
  {
    std::wstring wpath(path);
    const wchar_t wlastchar = wpath[wpath.size()-1];
    if(wlastchar != L'\\' || wlastchar != L'/')      //backslash
      wpath.append(L"\\");
    wpath.append(progName.c_str());
    return wpath;
  }
}


std::wstring SubTransFormat::DetectSubFileLanguage_STL(std::wstring fn)
{
  std::wstring szRet = L".chn";
  FILE *stream ;
  if (_wfopen_s(&stream, fn.c_str(), L"rb" ) == 0)
  {
    //detect bom?
    int totalWideChar = 0;
    int totalChar     = 0;
    int ch;

    for(int i=0; (feof( stream ) == 0 ); i++)
    {
      ch = 0xff & fgetc(stream);
      if (ch >= 0x80 )
        totalWideChar++;
      totalChar++;
    }

    fclose( stream );

    if(totalWideChar < (totalChar / 10) && totalWideChar < 1700)
      szRet = L".eng";
  }
  return szRet;
}

void SubTransFormat::FilePutContent(std::wstring szFilePath, std::wstring szData, BOOL bAppend)
{
  // CStdioFile::WriteString save text as ansi
  std::ofstream fs(szFilePath.c_str(), std::ios::in| std::ios::out| std::ios::app);//ios::trunc
  if (!fs.bad())
  {
    fs << szData.c_str();
    fs.close();
  }
}

BOOL SubTransFormat::GetAppDataPath(std::wstring& path)
{
  path.clear();
  TCHAR szPath[MAX_PATH];

  HRESULT hr;
  LPITEMIDLIST pidl;
  hr = SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl);
  if (FAILED(hr))
    return false;


  BOOL f = SHGetPathFromIDList(pidl, szPath);
  path = szPath;

  if(path[path.size()-1] != L'\\')
    path.append(L"\\");

  path.append(L"SPlayer");

  if(path.empty())
    return false;
  return true;
}

std::wstring SubTransFormat::GetShortFileNameForSearch2(std::wstring szFn)
{
  std::wstring szFileName = szFn;
  int posDot = szFileName.find_last_of('.');
  szFileName = szFileName.substr(0, posDot);

  std::vector<std::wstring> szaStopWords;
  szaStopWords.push_back(L"blueray");
  szaStopWords.push_back(L"bluray");
  szaStopWords.push_back(L"dvdrip");
  szaStopWords.push_back(L"xvid");
  szaStopWords.push_back(L"cd1");
  szaStopWords.push_back(L"cd2");
  szaStopWords.push_back(L"cd3");
  szaStopWords.push_back(L"cd4");
  szaStopWords.push_back(L"cd5");
  szaStopWords.push_back(L"cd6");
  szaStopWords.push_back(L"vc1");
  szaStopWords.push_back(L"vc-1");
  szaStopWords.push_back(L"hdtv");
  szaStopWords.push_back(L"1080p");
  szaStopWords.push_back(L"720p");
  szaStopWords.push_back(L"1080i");
  szaStopWords.push_back(L"x264");
  szaStopWords.push_back(L"stv");
  szaStopWords.push_back(L"limited");
  szaStopWords.push_back(L"ac3");
  szaStopWords.push_back(L"xxx");
  szaStopWords.push_back(L"hddvd");

  std::transform(szFileName.begin(), szFileName.end(),
    szFileName.begin(), tolower);

  for (size_t i = 0 ; i < szaStopWords.size(); i++)
  {
    int pos = szFileName.find(szaStopWords[i]);
    if(pos != szFileName.npos)
      szFileName = szFileName.substr(0, pos - 1);
  }


  std::wstring szReplace = L"[].-#_=+<>,";
  for (size_t i = 0; i < szReplace.length(); i++)
    for (size_t j = 0; j < szFileName.length(); j++)
      if (szFileName[j] == szReplace[i])
        szFileName[j] = ' ';

  szFileName.erase(szFileName.find_last_not_of(L' ') + 1);
  szFileName.erase(0, szFileName.find_first_not_of(L' '));

  if (szFileName.length() > 1)
    return szFileName;

  return L"";
}

std::wstring SubTransFormat::GetShortFileNameForSearch(std::wstring szFnPath)
{
  std::wstring szFileName = szFnPath.substr(szFnPath.find_last_of(L'\\'));

  std::wstring::size_type pos = szFileName.find_last_of(L'.');
  std::wstring szFileName2 = szFileName.substr(0, pos);

  szFileName = GetShortFileNameForSearch2(szFileName2);
  if(!szFileName.empty())
    return szFileName;

  return szFnPath;
}

std::wstring SubTransFormat::GetHashSignature(const char* szTerm2, const char* szTerm3)
{
  char buffx[4096], uniqueIDHash[UNIQU_HASH_SIZE];
  memset(buffx, 0, 4096);
  memset(uniqueIDHash,0,UNIQU_HASH_SIZE);

#ifdef CLIENTKEY	
  sprintf_s( buffx, 4096, CLIENTKEY , SVP_REV_NUMBER, szTerm2, szTerm3, uniqueIDHash);
#else
  sprintf_s( buffx, 4096, "unauthorized client %d %s %s %s", SVP_REV_NUMBER, szTerm2, szTerm3, uniqueIDHash);
#endif
  int len = strlen(buffx);
  return HashController::GetInstance()->GetMD5Hash(buffx, len);
}

BOOL SubTransFormat::IsSpecFanSub(std::wstring szPath, std::wstring szOEM)
{
  std::wstring szExt = szPath.substr(szPath.length()-4, 4);
  transform(szExt.begin(), szExt.end(), szExt.begin(), tolower);
  if(szExt.c_str() == L".srt" || szExt.c_str() == L".ssa" || szExt.c_str() == L".ass")
  {
    bool ret = false;
    std::string ss;
    std::ifstream fs(szPath.c_str());
    if (fs.is_open())
    {
      fs >> ss;
      std::string::size_type pos = ss.find(Strings::WStringToString(szOEM.c_str())); //check contain string
      if (pos != std::string::npos)
        ret = true;
      fs.close();
    }
    return ret;
  }

  return FALSE;
}
