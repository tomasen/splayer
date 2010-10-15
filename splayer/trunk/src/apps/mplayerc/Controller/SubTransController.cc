#include "stdafx.h"
#include "SubTransController.h"
#include "../revision.h"
#include "../../../svplib/MD5Checksum.h"
#include "../mplayerc.h"
#include "HashController.h"
#include "shooterclient.key"
#include "../Utils/Strings.h"
#include <sinet.h>
#include <sys/stat.h>

#include "PlayerPreference.h"
#include "SPlayerDefs.h"


#undef __MACTYPES__
#include "../../../zlib/zlib.h"
#include "../../../include/libunrar/dll.hpp"

using namespace sinet;

#define UNIQU_HASH_SIZE 512

void SubSinetInit(std::wstring &agent, std::wstring &proxy, int sid)
{
  AppSettings& s = AfxGetAppSettings();
  wchar_t agentbuff[MAX_PATH];
  if (s.szOEMTitle.IsEmpty())
    wsprintf(agentbuff, L"SPlayer Build %d", SVP_REV_NUMBER);
  else
  {
    std::wstring oem(s.szOEMTitle);
    wsprintf(agentbuff, L"SPlayer Build %d OEM%s", SVP_REV_NUMBER ,oem.c_str());
  }
  agent.assign(agentbuff);
  std::wstring tmpproxy;

  if (sid%2 == 0)
  {
    DWORD ProxyEnable = 0;
    wchar_t ProxyServer[256];
    DWORD ProxyPort = 0;

    ULONG len;
    CRegKey key;
    if( ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), KEY_READ)
      && ERROR_SUCCESS == key.QueryDWORDValue(_T("ProxyEnable"), ProxyEnable) && ProxyEnable
      && ERROR_SUCCESS == key.QueryStringValue(_T("ProxyServer"), ProxyServer, &len))
    {
      tmpproxy += L"http://";
      std::wstring proxystr(ProxyServer);
      tmpproxy += proxystr.c_str();
    }
  }
  proxy = tmpproxy.c_str();
}

void Make_TextForm(refptr<postdata>* data ,std::map<std::wstring, std::wstring> postform)
{
  for (std::map<std::wstring, std::wstring>::iterator it = postform.begin();
        it != postform.end(); it++)
  {
    refptr<postdataelem> elem = postdataelem::create_instance();
    elem->set_name((it->first).c_str());
    elem->setto_text((it->second).c_str());
    (*data)->add_elem(elem);
  }
}

std::wstring GetTempDir()
{
  wchar_t lpPathBuffer[MAX_PATH];
  GetTempPath(MAX_PATH,  lpPathBuffer); 
  std::wstring szTmpPath(lpPathBuffer);

  return szTmpPath;
}

wchar_t* getTmpFileName()
{
  wchar_t* tmpnamex ;
  int i;

  for (i = 0; i < 5; i++) //try 5 times for tmpfile creation
  {
    tmpnamex = _wtempnam( GetTempDir().c_str(), L"svp");
    if (tmpnamex)
      break;
  }
  if (!tmpnamex)
  {
    return 0;
  }
  else
  {
    return tmpnamex;
  }
}

std::wstring get_url(DWORD req_type , int iTryID)
{

  std::wstring apiurl;
  wchar_t str[100] = L"https://www.shooter.cn/";

  if (iTryID > 1 && iTryID <= 11)
  {
    if (iTryID >= 4)
    {
      int iSvrId = 4 + rand()%7;    
      if (iTryID%2)
        wsprintf(str, L"https://splayer%d.shooter.cn/", iSvrId-1);
      else
        wsprintf(str, L"http://splayer%d.shooter.cn/", iSvrId-1);
    }
    else
      wsprintf(str, L"https://splayer%d.shooter.cn/", iTryID-1);
  }
  else if (iTryID > 11)
    wsprintf(str, L"http://svplayer.shooter.cn/");

  apiurl.assign(str);
  switch(req_type)
  {
    case 'upda':
      apiurl += L"api/updater.php";
      break;
    case 'upsb':
      apiurl += L"api/subup.php";
      break;
    case 'sapi':
      apiurl += L"api/subapi.php";
      break;
  }
  return apiurl;
}

std::wstring GetShortFileNameForSearch2(std::wstring szFn)
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

std::wstring GetShortFileNameForSearch(std::wstring szFnPath)
{
  CPath szPath(szFnPath.c_str());
  szPath.StripPath();

  std::wstring szFileName = (LPCTSTR)szPath;

  szFileName = GetShortFileNameForSearch2(szFileName);

  if (szFileName.empty())
  {
    CPath szPath2(szFnPath.c_str());
    szPath2.RemoveFileSpec();
    std::wstring szFileName2 = (LPCTSTR)szPath2;
    szFileName = GetShortFileNameForSearch2(szFileName2);

    if (szFileName.empty())
    {
      szPath2.RemoveFileSpec();
      std::wstring szFileName3 = (LPCTSTR)szPath2;
      szFileName = GetShortFileNameForSearch2(szFileName3);
      if (szFileName.empty())
        return szFnPath;
    }
  }
  return szFileName;
}

std::wstring get_genhash(const char* szTerm2, const char* szTerm3, char* uniqueIDHash)
{
  char          buffx[4096];
  memset(buffx, 0, 4096);
#ifdef CLIENTKEY	
  sprintf_s( buffx, 4096, CLIENTKEY , SVP_REV_NUMBER, szTerm2, szTerm3, uniqueIDHash);
#else
  sprintf_s( buffx, 4096, "un authiority client %d %s %s %s", SVP_REV_NUMBER, szTerm2, szTerm3, uniqueIDHash);
#endif
  CMD5Checksum cmd5;
  return cmd5.GetMD5((BYTE*)buffx, strlen(buffx)).c_str();
}

int Char4ToInt(char* szBuf)
{

  int iData =   ( ((int)szBuf[0] & 0xff) << 24) |  ( ((int)szBuf[1] & 0xff) << 16) | ( ((int)szBuf[2] & 0xff) << 8) |  szBuf[3] & 0xff;;

  return iData;
}

char* ReadToPTCharByLength(FILE* fp, size_t length)
{
  char * szBuffData =(char *)calloc( length + 1, sizeof(char));

  if(!szBuffData)
    return 0;
  
  if ( fread(szBuffData , sizeof(char), length, fp) < length)
    return 0;
  
  return szBuffData;
}

int unpackGZfile(std::wstring fnin, std::wstring fnout)
{

  FILE* fout;
  int ret = 0;
  if ( _wfopen_s( &fout, fnout.c_str(), _T("wb") ) != 0){
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

#define SVP_MIN(a, b)  (((a) < (b)) ? (a) : (b))
int ExtractEachSubFile(FILE* fp, std::vector<std::wstring> &tmpfiles)
{
  // get file ext name
  char szSBuff[4096];
  if ( fread(szSBuff , sizeof(char), 4, fp) < 4)
    return -1;

  if ( fread(szSBuff , sizeof(char), 4, fp) < 4)
    return -1;

  size_t iExtLength = Char4ToInt(szSBuff);
  char* szExtName = ReadToPTCharByLength(fp, iExtLength);
  if(!szExtName)
    return -2;

  //get filedata length
  if ( fread(szSBuff , sizeof(char), 4, fp) < 4)
    return -1;

  size_t iFileLength = Char4ToInt(szSBuff);

  // gen tmp name and tmp file point
  wchar_t* otmpfilename = getTmpFileName();
  if(!otmpfilename)
    return -5;

  FILE* fpt;
  errno_t err = _wfopen_s(&fpt, otmpfilename, _T("wb"));
  if(err){
    free(otmpfilename);
    return -4;
  }

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

  wchar_t* otmpfilenameraw = getTmpFileName();
  std::wstring szLogmsg, of1, of2;
  of1.assign(otmpfilename);
  of2.assign(otmpfilenameraw);
  int gzret = unpackGZfile(of1 , of2);
  
  // add filename and tmp name to szaTmpFileNames
  std::wstring str;
  std::string extname(szExtName);
  str = Strings::Utf8StringToWString(extname);
  str += L"|";
  
  std::wstring str2(otmpfilenameraw);
  str += str2.c_str();
  str += L";";

  tmpfiles.push_back(str);

  free(szExtName);
  free(otmpfilename);

  return 0;
}

int ExtractSubFiles(FILE* fp, std::vector<std::wstring> &tmpfiles)
{
  char szSBuff[8];
  if ( fread(szSBuff , sizeof(char), 4, fp) < 4)
    return -1;

  size_t iFileDataLength = Char4ToInt(szSBuff); //确认是否确实有文件下载
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

int HandleSubPackage(FILE* fp, std::vector<std::wstring> &szaSubDescs,
                     std::vector<std::wstring> &tmpfiles)
{
  //Extract Package

  char szSBuff[8];
  int err;
  if ( fread(szSBuff , sizeof(char), 4, fp) < 4)
    return -1;

  int iPackageLength = Char4ToInt(szSBuff);

  if ( fread(szSBuff , sizeof(char), 4, fp) < 4)
    return -2;

  size_t iDescLength = Char4ToInt(szSBuff);

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

int ExtractDataFromAiSubRecvBuffer_STL(CAtlList<CString>* m_handlemsgs, std::wstring szFilePath,
                                       FILE* sAiSubRecvBuff,wchar_t* tmpoutfile,
                                       std::vector<std::wstring> &szaSubDescs,
                                       std::vector<std::wstring> &tmpfiles)
{
  char szSBuff[2] = {0,0};
  int ret = 0;

  fseek(sAiSubRecvBuff, 0, SEEK_SET); // move point yo begining of file

  fread(szSBuff, sizeof(char), 1, sAiSubRecvBuff);

  int iStatCode = szSBuff[0];
  if (iStatCode <= 0)
  {
    if (iStatCode == -1)
      ret = -2;
    else
      ret = -1;
    goto releaseALL;
  }
  else
    m_handlemsgs->AddTail((LPCTSTR)ResStr(IDS_LOG_MSG_SVPSUB_GOTMATCHED_AND_DOWNLOADING));

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
  _wremove(tmpoutfile);
  free(tmpoutfile);
  return ret;
}

BOOL SplitPath_STL(std::wstring fnSVPRarPath, std::wstring &fnrar, std::wstring &fninrar)
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

std::wstring getVideoFileBasename(std::wstring szVidPath, std::vector<std::wstring>* szaPathInfo = NULL)
{
  BOOL bIsRar = false;
  std::wstring fninrar, fnrar;
  if(SplitPath_STL(szVidPath.c_str(), fnrar, fninrar))
  {
    bIsRar = true;
    szVidPath = fnrar.c_str();
  }
  CPath szTPath(szVidPath.c_str());
  int posDot    = szVidPath.find_last_of(_T('.'));
  int posSlash  = szVidPath.find_last_of(_T('\\'));
  int posSlash2 = szVidPath.find_last_of(_T('/'));
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
        szExtName = (LPCTSTR)CPath(fninrar.c_str()).GetExtension();

      szaPathInfo -> push_back(szExtName ); //ExtName
      szaPathInfo -> push_back(szDirName); //Dir Name ()
      szaPathInfo -> push_back(szFileName); // file name only
    }
    return szVidPath.substr(posDot);
  }
  return szVidPath;
}

BOOL ifDirExist_STL(std::wstring path)
{
  CPath cpath(path.c_str());
  cpath.RemoveBackslash();
  return !_waccess(cpath, 0);

}

BOOL ifDirWritable_STL(std::wstring szDir)
{
  CPath szPath(szDir.c_str());
  szPath.RemoveBackslash();
  szPath.AddBackslash();
  szDir = (LPCTSTR)szPath;

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

std::wstring GetPlayerPath_STL(std::wstring progName)
{
  wchar_t path[MAX_PATH];
  GetModuleFileName(/*AfxGetInstanceHandle()*/NULL, path, MAX_PATH);
  if (progName.empty())
    return (LPCTSTR)path;
  else
  {
    CPath cpath(path);
    cpath.RemoveFileSpec();
    cpath.AddBackslash();
    cpath.Append(progName.c_str());
    return (LPCTSTR)cpath;
  }
}

int Explode(std::wstring szIn,
             std::wstring szTok,
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

BOOL ifFileExist_STL(std::wstring szPathname, BOOL evenSlowDriver = TRUE)
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
    CPath Driver(szPathname.c_str());
    Driver.StripToRoot();
    switch(GetDriveType(Driver))
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

std::wstring DetectSubFileLanguage_STL(std::wstring fn)
{
  std::wstring szRet = L".chn";
  FILE *stream ;
  if (_wfopen_s(&stream, fn.c_str(), _T("rb") ) == 0)
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

void filePutContent(std::wstring szFilePath, std::wstring szData, BOOL bAppend)
{
  // CStdioFile::WriteString save text as ansi
  CStdioFile f;

  if(f.Open(szFilePath.c_str(), CFile::modeCreate | CFile::modeWrite | CFile::typeText))
  {
    f.WriteString(szData.c_str());
    f.Close();
  }
}

bool GetAppDataPath(std::wstring& path)
{
  path.clear();
  TCHAR szPath[MAX_PATH];

  HRESULT hr;
  LPITEMIDLIST pidl;
  hr = SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl);
  if (hr)
  {
    //Old method
    CRegKey key;
    if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), KEY_READ))
    {
      ULONG len = MAX_PATH;
      if(ERROR_SUCCESS == key.QueryStringValue(_T("AppData"), szPath, &len))
        path.resize(len);
    }
  }


  BOOL f = SHGetPathFromIDList(pidl, szPath);
  PathRemoveBackslash(szPath);
  PathAddBackslash(szPath); 
  PathAppend(szPath, _T("SPlayer"));
  path = szPath;


  if(path.empty())
    return false;
	return true;
}

std::wstring extractRarFile(std::wstring rarfn)
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
	szRet = getTmpFileName() ;
	szRet +=L".sub";
	CFile m_sub (szRet.c_str(), CFile::modeCreate|CFile::modeReadWrite|CFile::typeBinary);

	while(RARReadHeaderEx(hrar, &HeaderDataEx) == 0)
	{
		std::wstring subfn(HeaderDataEx.FileNameW);

		if(subfn.substr(subfn.size()-4, 4) == L".sub")
		{
			CAutoVectorPtr<char> buff;
			if(!buff.Allocate(HeaderDataEx.UnpSize))
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

			m_sub.SetLength(HeaderDataEx.UnpSize);
			m_sub.SeekToBegin();
			m_sub.Write(buff, HeaderDataEx.UnpSize);
			m_sub.Flush();
			m_sub.SeekToBegin();
			m_sub.Close();

			RARExtractChunkClose(hrar);
			break;
		}

		RARProcessFile(hrar, RAR_SKIP, NULL, NULL);
	}

	RARCloseArchive(hrar);
	return szRet;
}

int FindAllSubfile(std::wstring szSubPath , std::vector<std::wstring>* szaSubFiles)
{
  szaSubFiles -> clear();
  szaSubFiles -> push_back(szSubPath);
  std::vector<std::wstring> szaPathInfo;
  std::wstring szBaseName = getVideoFileBasename(szSubPath.c_str(), &szaPathInfo);
  std::wstring szExt = szaPathInfo.at(1);

  if(szExt == _T(".idx"))
  {
    szSubPath = szBaseName + _T(".sub");
    if(ifFileExist_STL(szSubPath.c_str()))
      szaSubFiles -> push_back(szSubPath);
    else
    {
      szSubPath = szBaseName + _T(".rar");
      if(ifFileExist_STL(szSubPath.c_str()))
      {
        std::wstring szSubFilepath = extractRarFile(szSubPath);
        if(!szSubFilepath.empty())
          szaSubFiles -> push_back(szSubFilepath);
      }
    }
  }
  //TODO: finding other subfile
  return 0;
}

std::wstring getSameTmpName(std::wstring fnin)
{
  std::vector<std::wstring> szaPathinfo;
  getVideoFileBasename(fnin, &szaPathinfo);
  std::wstring fntdir = GetTempDir();
  std::wstring fnout(fntdir.c_str());
  fnout += szaPathinfo.at(3).c_str();
  fnout += szaPathinfo.at(1).c_str();
  int i = 0;
  while(ifFileExist_STL(fnout))
  {
    i++;
    wchar_t str[100];
    wsprintf(str, L".svr%d", i);
    fnout = fntdir.c_str();
    fnout += szaPathinfo.at(3).c_str();
    fnout += str;
    fnout += szaPathinfo.at(1).c_str();
  }
  return fnout;
}

int packGZfile(std::wstring fnin, std::wstring fnout)
{

  FILE* fp;
  int ret = 0;
  if ( _wfopen_s( &fp, fnin.c_str(), _T("rb") ) != 0)
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

int WetherNeedUploadSub(HANDLE m_stopevent,
                        std::wstring fnVideoFilePath, std::wstring szFileHash, 
                        std::wstring fnSubHash, int iDelayMS, int sid)
{
  std::map<std::wstring, std::wstring> postform;
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  refptr<request> req = request::create_instance();

  postform[L"pathinfo"] = fnVideoFilePath;
  postform[L"filehash"] = szFileHash;
  postform[L"subhash"]  = fnSubHash;
 
  wchar_t delay[32];
  _itow_s(iDelayMS, delay, 32, 10);
  postform[L"subdelay"] = delay;

  refptr<postdata> data = postdata::create_instance();
  Make_TextForm(&data, postform);

  int rret = -1;
  std::wstring agent;
  std::wstring proxy;

  SubSinetInit(agent, proxy, sid);
  cfg->set_strvar(CFG_STR_AGENT, agent);
  if (!proxy.empty())
    cfg->set_strvar(CFG_STR_PROXY, proxy);

  std::wstring url = get_url('upsb', sid);
  req->set_request_method(REQ_POST);
  req->set_postdata(data);

  task->use_config(cfg);
  task->append_request(req);
  pool->execute(task);
  while (pool->is_running_or_queued(task))
  {
    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return 0;
  }

  if (req->get_response_errcode() == 0)
    return 1;
  else
    return 0;
}

int UploadSubFileByVideoAndHash(CAtlList<CString>* m_handlemsgs,
                                HANDLE m_stopevent,
                                std::wstring fnVideoFilePath,
                                std::wstring szFileHash,
                                std::wstring szSubHash,
                                std::vector<std::wstring>* fnSubPaths,
                                int iDelayMS, int sid)
{
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  refptr<request> req = request::create_instance();
  std::map<std::wstring, std::wstring> postform;
  
  postform[L"pathinfo"] = fnVideoFilePath;
  postform[L"filehash"] = szFileHash;
  postform[L"subhash"]  = szSubHash;
  
  wchar_t delay[32];
  _itow_s(iDelayMS, delay, 32, 10);
  postform[L"subdelay"] = delay;

  char uniqueIDHash[UNIQU_HASH_SIZE];
  memset(uniqueIDHash,0,UNIQU_HASH_SIZE);
  std::wstring vhash = get_genhash(Strings::WStringToUtf8String(fnVideoFilePath).c_str(),
    Strings::WStringToUtf8String(szFileHash).c_str(),
    uniqueIDHash);
  if (!vhash.empty())
    postform[L"vhash"]  = vhash;

  refptr<postdata> data = postdata::create_instance();
  Make_TextForm(&data, postform);

  size_t iTotalFiles = fnSubPaths->size();
  for (size_t i = 0; i < iTotalFiles; i++)
  {
    wchar_t szFname[22];
    /* Fill in the file upload field */
    std::wstring szgzFile = getSameTmpName(fnSubPaths->at(i));
    packGZfile(fnSubPaths->at(i), szgzFile);

    wsprintf(szFname, L"subfile[%d]", i);
    refptr<postdataelem> elem = postdataelem::create_instance();
    elem->set_name(szFname);
    elem->setto_file(szgzFile.c_str());
    data->add_elem(elem);
  }
  
  std::wstring agent;
  std::wstring proxy;

  SubSinetInit(agent, proxy, sid);
  cfg->set_strvar(CFG_STR_AGENT, agent);
  if (!proxy.empty())
    cfg->set_strvar(CFG_STR_PROXY, proxy);

  std::wstring url = get_url('upsb', sid);
  req->set_request_url(url.c_str());
  req->set_request_method(REQ_POST);
  req->set_postdata(data);

  task->use_config(cfg);
  task->append_request(req);
  pool->execute(task);
  while (pool->is_running_or_queued(task))
  {
    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return -1;
  }

  if (req->get_response_errcode() == 0)
    m_handlemsgs->AddTail((LPCTSTR)ResStr(IDS_LOG_MSG_SVPSUB_UPLOAD_FINISHED));
  
  return 0;
}

void Upload_Subfile(CAtlList<CString>* m_handlemsgs,
                    HANDLE m_stopevent,
                    std::wstring fnVideoFilePath,
                    std::wstring szSubPath,
                    int iDelayMS)
{
  HashController::GetInstance()->SetFileName(fnVideoFilePath.c_str());
  std::wstring szFileHash = HashController::GetInstance()->GetHash();

  std::vector<std::wstring> szaSubFiles;
  FindAllSubfile(szSubPath.c_str(), &szaSubFiles);

  std::wstring subfilestr;
  for (std::vector<std::wstring>::iterator it = szaSubFiles.begin();
       it != szaSubFiles.end(); it++)
  {
    if (it != szaSubFiles.begin())
      subfilestr += L"\0";
    subfilestr += (*it).c_str();
  }

  HashController::GetInstance()->SetFileName(subfilestr.c_str());
  std::wstring szSubHash = HashController::GetInstance()->GetHash();
  if (szSubHash.empty())
    return;

  for (int i = 1; i <= 7; i++)
  {
    int chk = WetherNeedUploadSub(m_stopevent, fnVideoFilePath, szFileHash, szSubHash, iDelayMS, i);
    if (chk > 0)
      if (0 == UploadSubFileByVideoAndHash(m_handlemsgs, m_stopevent, fnVideoFilePath,szFileHash,
        szSubHash, &szaSubFiles, iDelayMS, i))
        return ;
      else if(0 == chk)
        break;
    //Fail
    if (::WaitForSingleObject(m_stopevent, 2000) == WAIT_OBJECT_0)
      return;
  }
}

#define SVPATH_BASENAME 0  //Without Dot
#define SVPATH_EXTNAME 1  //With Dot
#define SVPATH_DIRNAME 2 //With Slash
#define SVPATH_FILENAME 3  //Without Dot
std::wstring getSubFileByTempid_STL(size_t iTmpID, std::wstring szVidPath,
                                    std::vector<std::wstring> szaSubDescs,
                                    std::vector<std::wstring> tmpfiles)
{
  //get base path name
  std::vector<std::wstring> szVidPathInfo;
  std::wstring szTargetBaseName = L"";
  std::wstring szDefaultSubPath = L"";

  AppSettings& s = AfxGetAppSettings();
  std::wstring StoreDir = (LPCTSTR)s.SVPSubStoreDir;
  getVideoFileBasename(szVidPath, &szVidPathInfo); 

  if(s.bSaveSVPSubWithVideo && 
    ifDirWritable_STL(szVidPathInfo.at(SVPATH_DIRNAME)))
  {
    StoreDir = szVidPathInfo.at(SVPATH_DIRNAME).c_str();
  }
  else
  {
    if(StoreDir.empty() || !ifDirExist_STL(StoreDir) || 
      !ifDirWritable_STL(StoreDir))
    {
      GetAppDataPath(StoreDir);
      CPath tmPath(StoreDir.c_str());
      tmPath.RemoveBackslash();
      tmPath.AddBackslash();
      tmPath.Append(L"SVPSub");
      StoreDir = (LPCTSTR)tmPath;
      _wmkdir(StoreDir.c_str());
      if(StoreDir.empty() || !ifDirExist_STL(StoreDir) || 
        !ifDirWritable_STL(StoreDir))
      {
        StoreDir = GetPlayerPath_STL(L"SVPSub");
        _wmkdir(StoreDir.c_str());
        if(StoreDir.empty() || !ifDirExist_STL(StoreDir) || 
          !ifDirWritable_STL(StoreDir))
        {   
          //WTF cant create fordler ?
        }
        else
          s.SVPSubStoreDir = StoreDir.c_str();
      }
      else
        s.SVPSubStoreDir = StoreDir.c_str();
    }
  }

  CPath tmBasenamePath(StoreDir.c_str());
  tmBasenamePath.RemoveBackslash();
  tmBasenamePath.AddBackslash();
  StoreDir =  (LPCTSTR)tmBasenamePath;
  getVideoFileBasename(szVidPath, &szVidPathInfo);
  tmBasenamePath.Append(szVidPathInfo.at(SVPATH_FILENAME).c_str());

  std::wstring szBasename = (LPCTSTR)tmBasenamePath;

  //set new file name
  std::vector<std::wstring> szSubfiles;
  std::wstring szXTmpdata = tmpfiles.at(iTmpID);
  Explode(szXTmpdata, L";", &szSubfiles);
  bool bIsIdxSub = FALSE;
  int ialreadyExist = 0;
  if (szXTmpdata.find(L"idx|") != szXTmpdata.npos && szXTmpdata.find(L"sub|") != szXTmpdata.npos)
    if (!ifFileExist_STL(szBasename + L".idx") && !ifFileExist_STL(szBasename + L".sub"))
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

    CMD5Checksum cm5source;
    std::wstring szSourceMD5 = cm5source.GetMD5(szSource);
    std::wstring szTargetMD5;
    //check if target exist
    wchar_t szTmp[128];
    lstrcpy(szTmp, L"");
    int ilan = 1;
    while(ifFileExist_STL(szTarget))
    {
      //TODO: compare if its the same file
      cm5source.Clean();
      szTargetMD5 = cm5source.GetMD5(szTarget);
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
      szDefaultSubPath = szSource;

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
        swscanf_s(szaDesclines.at(0).c_str(), _T("delay=%d"), &iDelay);
        if (iDelay)
        {
          wchar_t szBuf[128];
          swprintf_s(szBuf, 128, L"%d", iDelay);
          filePutContent(szTarget + L".delay", (LPCTSTR)szBuf, false);
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

int down_subfile(CAtlList<CString>* m_handlemsgs,
                 HANDLE m_stopevent, std::wstring szFilePath,
                 std::wstring szFileHash, std::wstring szVHash,
                 std::wstring szLang, int sid,
                 std::vector<std::wstring> &szaSubDescs,
                 std::vector<std::wstring> &tmpfiles)
{
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  refptr<request> req = request::create_instance();
  std::map<std::wstring, std::wstring> postform;

  postform[L"pathinfo"] = szFilePath;
  postform[L"filehash"] = szFileHash;


  char uniqueIDHash[UNIQU_HASH_SIZE];
  memset(uniqueIDHash,0,UNIQU_HASH_SIZE);
  std::wstring vhash = get_genhash(Strings::WStringToUtf8String(szFilePath).c_str(),
                                  Strings::WStringToUtf8String(szFileHash).c_str(),
                                  uniqueIDHash);
  if (!vhash.empty())
      postform[L"vhash"]  = vhash;

  AppSettings& s = AfxGetAppSettings();
  std::wstring szSVPSubPerf = (LPCTSTR)s.szSVPSubPerf;
  if (!szSVPSubPerf.empty())
    postform[L"perf"] = szSVPSubPerf;

  if (!szLang.empty())
    postform[L"lang"] = szLang;

  std::wstring shortname = GetShortFileNameForSearch(szFilePath);
  postform[L"shortname"] = shortname;
  
  std::wstring url = get_url('sapi', sid);
  req->set_request_url(url.c_str());

  refptr<postdata> data = postdata::create_instance();
  Make_TextForm(&data, postform);

  wchar_t* tmpoutfile = getTmpFileName();
  if (!tmpoutfile)
    return 1;

  req->set_request_method(REQ_POST);
  req->set_request_outmode(REQ_OUTFILE);
  req->set_outfile(tmpoutfile);
  req->set_postdata(data);


  std::wstring agent;
  std::wstring proxy;
  SubSinetInit(agent, proxy, sid);

  cfg->set_strvar(CFG_STR_AGENT, agent);
  if (!proxy.empty())
    cfg->set_strvar(CFG_STR_PROXY, proxy);

  task->use_config(cfg);
  task->append_request(req);
  pool->execute(task);
  while (pool->is_running_or_queued(task))
  {
    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return 1;
  }

  if (req->get_response_errcode() != 0)
  {
    m_handlemsgs->AddTail((LPCTSTR)ResStr(IDS_LOG_MSG_SVPSUB_NONE_MATCH_SUB));
    return 1;
  }

  FILE *stream;
  _wfopen_s(&stream, tmpoutfile, _T("rb"));
  if (0 != ExtractDataFromAiSubRecvBuffer_STL(m_handlemsgs, szFilePath, stream, tmpoutfile,
                                              szaSubDescs,
                                              tmpfiles))
    return 1;

  return 0;
}

#define fansub_search_buf 30000
bool isSpecFanSub(std::wstring szPath, std::wstring szOEM){

  std::wstring szExt = szPath.substr(szPath.length()-4, 4);
  transform(szExt.begin(), szExt.end(), szExt.begin(), tolower);
  if(szExt.c_str() == L".srt" || szExt.c_str() == L".ssa" || szExt.c_str() == L".ass")
  {
    CFile subFile;
    if(!subFile.Open( szPath.c_str(), CFile::modeRead|CFile::typeText))
      return FALSE;
    char qBuff[fansub_search_buf];
    memset(qBuff, 0, fansub_search_buf);

    subFile.Read( qBuff, fansub_search_buf );

    bool ret = false;
    if(strstr(qBuff, Strings::WStringToString(szOEM).c_str()))
      ret = true;

    subFile.Close();
    return ret;
  }

  return FALSE;
}

void get_subfiles(CAtlList<CString>* m_handlemsgs, HANDLE m_stopevent, std::wstring fnVideoFilePath,
                  std::vector<std::wstring>* szSubArray,
                  std::wstring szLang = L"")
{
  AppSettings& s = AfxGetAppSettings();

  if (szLang.empty() && !(s.iLanguage == 0 || s.iLanguage == 2))
    szLang = L"eng";

  HashController::GetInstance()->SetFileName(fnVideoFilePath.c_str());
  std::wstring szFileHash = HashController::GetInstance()->GetHash();

  std::vector<std::wstring> szaSubDescs, tmpfiles;
  for (int i = 1; i <= 7; i++)
  {
    if (0 == down_subfile(m_handlemsgs, m_stopevent, fnVideoFilePath, szFileHash,
                          L"", szLang, i, szaSubDescs, tmpfiles))
      break;

    if (::WaitForSingleObject(m_stopevent, 2300) == WAIT_OBJECT_0)
      return;
  }

  //load sub file to sublist
  std::wstring szSubFilePath;
  int iSubTotal = 0;
  for (size_t i = 0; i < tmpfiles.size(); i++)
  {
    szSubFilePath = getSubFileByTempid_STL(i,
      fnVideoFilePath, szaSubDescs, tmpfiles);
    
    if (!szSubFilePath.empty())
    {
      szSubArray -> push_back(szSubFilePath);
      CPath fnPath(szSubFilePath.c_str());
      fnPath.StripPath();
      iSubTotal++;
    }
  }

  if (iSubTotal > 1)
  {
    std::wstring szSVPSubPerf = (LPCTSTR)s.szSVPSubPerf;
    if (!szSVPSubPerf.empty())
    {
      for (std::vector<std::wstring>::iterator iter = szSubArray -> begin();
        iter != szSubArray -> end(); iter++)
        if (isSpecFanSub((*iter), szSVPSubPerf))
        {
          std::wstring szFirst = szSubArray -> at(0);
          (*szSubArray)[0] = szSubFilePath;
          *iter = szFirst;
          break;
        }
    }
  }
  return;
}

SubTransController::SubTransController(void):
  m_stopevent(::CreateEvent(NULL, TRUE, FALSE, NULL)),
  m_thread(NULL),
  m_operation(Unknown)
{
}

SubTransController::~SubTransController(void)
{
  Stop();
}

void SubTransController::SetFrame(HWND hwnd)
{
  m_frame = hwnd;
}

void SubTransController::Start(const wchar_t* video_filename, 
                               SubTransOperation operation, 
                               StringList files_upload /*= StringList()*/)
{
  // we should stop running tasks first
  Stop();
  // record parameters
  m_operation = operation;
  m_videofile.assign(video_filename);
  // create thread
  m_thread = (HANDLE)::_beginthread(_thread_dispatch, 0, (void*)this);
}

void SubTransController::Stop()
{
  unsigned long thread_exitcode;
  if (m_thread && m_thread != INVALID_HANDLE_VALUE &&
    GetExitCodeThread(m_thread, &thread_exitcode) &&
    thread_exitcode == STILL_ACTIVE)
  {
    ::SetEvent(m_stopevent);
    ::WaitForSingleObject(m_thread, INFINITE);
  }
  m_thread = NULL;
  ::ResetEvent(m_stopevent);
}

void SubTransController::_thread_dispatch(void* param)
{
  static_cast<SubTransController*>(param)->_thread();
}

void SubTransController::_thread()
{
  switch (m_operation)
  {
  case DownloadSubtitle:
    _thread_download();
    break;
  case UploadSubtitle:
    _thread_upload();
    break;
  }
}

void SubTransController::_thread_download()
{
  std::vector<std::wstring> subtitles;
  get_subfiles(m_handlemsgs, m_stopevent, m_videofile, &subtitles);
  
  BOOL bSubSelected = false;
  for (size_t i = 0 ;i < subtitles.size(); i++)
  {
    if (!bSubSelected)
    {
      bSubSelected = true;
      PlayerPreference::GetInstance()->SetStringVar(STRVAR_QUERYSUBTITLE, subtitles.at(i));
      ::SendMessage(m_frame, WM_COMMAND, ID_COMPLETE_QUERY_SUBTITLE, NULL);
    }
  }
}

void SubTransController::_thread_upload()
{
  Upload_Subfile(m_handlemsgs, m_stopevent, m_videofile, m_subfile, m_delayms);
}

void SubTransController::SetMsgs(CAtlList<CString>* msgs)
{
  m_handlemsgs = msgs;
}

void SubTransController::SetSubfile(std::wstring subfile)
{
  m_subfile = subfile;
}

void SubTransController::SetDelayMs(int ms)
{
  m_delayms = ms;
}