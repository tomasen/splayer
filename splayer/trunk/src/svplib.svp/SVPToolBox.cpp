#include "SVPToolBox.h"
#include "..\zlib\zlib.h"


#include <Streams.h>
#include <afxtempl.h>
#include "../apps/mplayerc/mplayerc.h"

#include <io.h> 
#include <wchar.h> 
#include "MD5Checksum.h"

#include <Shlobj.h>
#include "SVPRarLib.h"
#include "..\..\include\libunrar\dll.hpp"

#include <d3d9.h>
#include <d3dx9.h>

BOOL CALLBACK EnumFamCallBack(LPLOGFONT lplf, LPNEWTEXTMETRIC lpntm, DWORD FontType, LPVOID aFontCount) 
{ 
	int * aiFontCount = (int *)aFontCount;
	*aiFontCount = 1;
	
	return TRUE;  
} 

CSVPToolBox::CSVPToolBox(void)
{
}

CSVPToolBox::~CSVPToolBox(void)
{
}

CString CSVPToolBox::GetShortFileNameForSearch2(CString szFn){
	CString szFileName(szFn);
	int posDot = szFileName.ReverseFind('.');
	szFileName = szFileName.Left(posDot);

	CStringArray szaStopWords;
	szaStopWords.Add(_T("blueray"));
	szaStopWords.Add(_T("bluray"));
	szaStopWords.Add(_T("dvdrip"));
	szaStopWords.Add(_T("xvid"));
	szaStopWords.Add(_T("cd1"));
	szaStopWords.Add(_T("cd2"));
	szaStopWords.Add(_T("cd3"));
	szaStopWords.Add(_T("cd4"));
	szaStopWords.Add(_T("cd5"));
	szaStopWords.Add(_T("cd6"));
	szaStopWords.Add(_T("vc1"));
	szaStopWords.Add(_T("vc-1"));
	szaStopWords.Add(_T("hdtv"));
	szaStopWords.Add(_T("1080p"));
	szaStopWords.Add(_T("720p"));
	szaStopWords.Add(_T("1080i"));
	szaStopWords.Add(_T("x264"));
	szaStopWords.Add(_T("stv"));
	szaStopWords.Add(_T("limited"));
	szaStopWords.Add(_T("ac3"));
	szaStopWords.Add(_T("xxx"));
	szaStopWords.Add(_T("hddvd"));

	szFileName.MakeLower();

	for(int i = 0 ; i < szaStopWords.GetCount(); i++){
		int pos = szFileName.Find(szaStopWords[i]);

		if( pos >= 0){
			szFileName = szFileName.Left( pos - 1 );
		}
	}

	
	CString szReplace(_T("[].-#_=+<>,"));
	for(int i = 0; i < szReplace.GetLength(); i++){
		szFileName.Replace(szReplace[i], ' ');
	}
	
	szFileName.Trim();

	if(szFileName.GetLength() > 1){
		return szFileName;
	}

	return _T("");
}
CString CSVPToolBox::GetShortFileNameForSearch(CString szFnPath){
	CPath szPath(szFnPath);
	szPath.StripPath();
	
	CString szFileName(szPath);

	szFileName = GetShortFileNameForSearch2(szFileName);
	
	if(szFileName.IsEmpty()){
		CPath szPath2(szFnPath);
		szPath2.RemoveFileSpec();
		CString szFileName2(szPath2);
		szFileName = GetShortFileNameForSearch2(szFileName2);

		if(szFileName.IsEmpty()){
			
			szPath2.RemoveFileSpec();
			CString szFileName3(szPath2);
			szFileName = GetShortFileNameForSearch2(szFileName3);

			if(szFileName.IsEmpty()){
				return szFnPath;
			}
		}
	}
	return szFileName;
}
BOOL CSVPToolBox::FindSystemFile(CString szFn){
	TCHAR szBuf[MAX_PATH];
	int len = GetSystemDirectory(szBuf, MAX_PATH);
	if(len > 0){
		//AfxMessageBox(szBuf);
		CString szTmp;
		szTmp.SetString(szBuf , len);
		CPath szPath(szTmp);
		szPath.RemoveBackslash();
		szPath.AddBackslash();
		szPath.Append(szFn);
		return ifFileExist(szPath);
	}
	return false;
}
#define SVPATH_BASENAME 0  //Without Dot
#define SVPATH_EXTNAME 1  //With Dot
#define SVPATH_DIRNAME 2 //With Slash
#define SVPATH_FILENAME 3  //Without Dot
CString CSVPToolBox::getVideoFileBasename(CString szVidPath, CStringArray* szaPathInfo = NULL){


	CSVPRarLib svpRar;
	BOOL bIsRar = false;
	if(svpRar.SplitPath( szVidPath )){

		bIsRar = true;
		szVidPath = svpRar.m_fnRAR;

	}
	CPath szTPath(szVidPath);

	int posDot = szVidPath.ReverseFind(_T('.'));

	int posSlash = szVidPath.ReverseFind(_T('\\'));
	int posSlash2 = szVidPath.ReverseFind(_T('/'));
	if(posSlash2 > posSlash){posSlash = posSlash2;}

	if(posDot > posSlash ){
		if (szaPathInfo != NULL){
			CString szBaseName = szVidPath.Left(posDot);
			CString szExtName = szVidPath.Right(szVidPath.GetLength() - posDot).MakeLower();
			CString szFileName = szVidPath.Mid(posSlash+1, (posDot - posSlash - 1));
			CString szDirName = szVidPath.Left(posSlash + 1) ;
			szaPathInfo->RemoveAll();
			szaPathInfo->Add(szBaseName); // Base Name
			if(bIsRar){
				szExtName = CPath(svpRar.m_fnInsideRar).GetExtension();
			}
			
			szaPathInfo->Add(szExtName ); //ExtName

			szaPathInfo->Add(szDirName); //Dir Name ()
			szaPathInfo->Add(szFileName); // file name only
			SVP_LogMsg(szBaseName);
			SVP_LogMsg(szaPathInfo->GetAt(0) + _T(" | ") + szaPathInfo->GetAt(1) + _T(" | ") + szaPathInfo->GetAt(2) + _T(" | ") + szaPathInfo->GetAt(3) );
		}
		return szVidPath.Left(posDot);
	}

	return szVidPath;
}

void CSVPToolBox::MergeAltList( CAtlList<CString>& szaRet,  CAtlList<CString>& szaIn  ){
	POSITION pos = szaIn.GetHeadPosition();
	while(pos){
		CString szBuf = szaIn.GetNext(pos);
		if ( szaRet.Find( szBuf) == NULL){
			szaRet.AddTail(szBuf);
		}
	}
}

BOOL CSVPToolBox::isAlaphbet(WCHAR wchr)
{
  //need covert to \xXXXX
  CString szAlaphbet = _T("1234567890-=qwertyuiop[]asdfghjkl;'zxcvbnm,./`~!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:\"\\ZXCVBNM<>?	 ∫♩♫♬€¶♯$¥∮“”‘’；：，。、《》？！·—");
  return !!( szAlaphbet.Find(wchr) >= 0);
}

void CSVPToolBox::findMoreFileByFile( CString szFile, CAtlList<CString>& szaRet,  CAtlArray<CString>& szaExt  ){
	CString szFindPatten;
	CStringArray szFilePathinfo;
	getVideoFileBasename(szFile, &szFilePathinfo);
	if( szFilePathinfo.GetCount() >= 4 ){
		szFindPatten = szFilePathinfo.GetAt(SVPATH_FILENAME);
		CString szFilterString = _T("1234567890!@#$%^&*()_+[]{}\\/|?.,<>`~ -=;:\'\"");
		CString szRealPattern = _T("");
		BOOL lastStar = false;
		BOOL haveMatchString = false;
		for(int i = 0 ; i < szFindPatten.GetLength(); i++){
			
			TCHAR xbuf = szFindPatten.GetAt(i);
			if(szFilterString.Find(xbuf) >= 0 ){
				if(!lastStar){
					szRealPattern += _T("*");
					lastStar = true;
				}
				if(i > 4 && haveMatchString){
					break;
				}
				continue;
			}else{
				lastStar = false;
				haveMatchString = true;
				szRealPattern += xbuf;
			}
			
		}
		if(!lastStar){
			szRealPattern += _T("*");
		}
		szFindPatten = szFilePathinfo.GetAt(SVPATH_DIRNAME) + szRealPattern + _T(".*");
		findMoreFileByDir( szFindPatten, szaRet, szaExt);
	}
}
BOOL CSVPToolBox::GetDirectoryLeft(CPath* tPath, int rCount ){
	if(!tPath->IsDirectory() && !tPath->IsRoot() && rCount > 0){
		tPath->RemoveBackslash();
		tPath->RemoveFileSpec();
		tPath->AddBackslash();
		return this->GetDirectoryLeft(tPath, rCount-1);
	}else{
		return false;
	}

}
void CSVPToolBox::findMoreFileByDir(  CString szDir, CAtlList<CString>& szaRet,  CAtlArray<CString>& szaExt , BOOL bSubDir ){
	
		__time64_t ts = _time64(NULL);
		CFileFind finder;
		CString szExtMatch ;
		for(UINT i = 0; i < szaExt.GetCount(); i++){
			szExtMatch +=  szaExt.GetAt(i) + _T(";");
		}

		BOOL bWorking = finder.FindFile(szDir);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			if (finder.IsDots())
				continue;

			if (finder.IsDirectory()){
				if(bSubDir){
					CPath tPath(finder.GetFilePath());
					tPath.RemoveBackslash();
					tPath.AddBackslash();
					tPath.Append(_T("*"));
					findMoreFileByDir( tPath , szaRet,szaExt, true);
				}
				continue;
			}
			CString szTmp = finder.GetFilePath();
			szTmp.MakeLower();
			if( szTmp.Find(_T("sample")) >= 0){
				continue;
			}
			BOOL matchExt = false;
			CPath szTPath(szTmp);
			CString szThisExt  = szTPath.GetExtension();
			if(szExtMatch.Find( szThisExt + _T(";") ) >= 0){
                //SVP_LogMsg5(L"find %s", finder.GetFilePath());
				szaRet.AddTail(finder.GetFilePath());
			}
			if ( ( _time64(NULL) - ts ) > 12){
				//搜索文件时间超过12秒，取消搜索
				break;
			}
						
		}

		finder.Close();

}
BOOL CSVPToolBox::bFontExist(CString szFontName, BOOL chkExtFontFile){
	int aFontCount = 0;
	BOOL ret;
	HDC hdc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	ret =  EnumFontFamilies(hdc,szFontName, (FONTENUMPROC) EnumFamCallBack, (LPARAM) &aFontCount) ;

	CancelDC(hdc);
	DeleteDC(hdc);
	
	return !!aFontCount;
}

bool IsFileGziped(CString fnin)
{
    BYTE bBuffer[2]; 
    UINT nActual = 0; 
    CFile myFile;

    if ( myFile.Open( fnin, CFile::modeRead  |  CFile::typeBinary, NULL ) )
    {
        myFile.Seek( 0, CFile::begin );
        nActual = myFile.Read( bBuffer, sizeof( bBuffer ) ); 
        if (nActual == 2)
        {
            //http://www.ietf.org/rfc/rfc1952.txt
            //ID1 (IDentification 1)
            //ID2 (IDentification 2)
            //    These have the fixed values ID1 = 31 (0x1f, \037), ID2 = 139
            //    (0x8b, \213), to identify the file as being in gzip format

            if ((0x1f == bBuffer[0])&&(0x8b == bBuffer[1]))
                return true;
        }
    }
    return false;
}

bool WriteBufferToFile(CString path, const BYTE* contents, DWORD dwsize)
{
    DWORD dwResult;
    HANDLE hFile = CreateFile(path,                // name of the write
        GENERIC_WRITE,          // open for writing
        0,                      // do not share
        NULL,                   // default security
        CREATE_ALWAYS,          // overwrite existing
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);                  // no attr. template

    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        return false;
    }

    if(!WriteFile (hFile, contents, dwsize, &dwResult, NULL))
    {
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);
    return true;
}

bool ReadFileToBuffer(CString path, BYTE*& contents, DWORD* pdwsize) 
{
    HANDLE hFile = CreateFile(path,               // file to open
        GENERIC_READ,          // open for reading
        FILE_SHARE_READ,       // share for reading
        NULL,                  // default security
        OPEN_EXISTING,         // existing file only
        FILE_ATTRIBUTE_NORMAL, // normal file
        NULL);                 // no attr. template

    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        return false; 
    }

    DWORD dwSize = GetFileSize(hFile, NULL);

    if (INVALID_FILE_SIZE == dwSize)
        return false;
    contents = new BYTE[dwSize];

    if(!ReadFile(hFile, contents, dwSize, pdwsize, NULL))
    {
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);
    return true;
}

CString CSVPToolBox::fileGetContent(CString szFilePath){
	CStdioFile f;

	CString szBuf;
	CString szRet;
	if(f.Open(szFilePath, CFile::modeRead | CFile::typeText))
	{
		while( f.ReadString(szBuf) ){
			szRet.Append(szBuf + _T("\n"));
		}
		f.Close();
	}
	return szRet.TrimRight();
}
void CSVPToolBox::filePutContent(CString szFilePath, CString szData, BOOL bAppend){
	CStdioFile f;
	
	if(f.Open(szFilePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
	{
		f.WriteString(szData);
		
		f.Close();
	}
}
DWORD CSVPToolBox::_httoi(const TCHAR *value)
{
	struct CHexMap
	{
		TCHAR chr;
		int value;
	};

	CHexMap HexMap[16] =
	{
		{'0', 0}, {'1', 1},
		{'2', 2}, {'3', 3},
		{'4', 4}, {'5', 5},
		{'6', 6}, {'7', 7},
		{'8', 8}, {'9', 9},
		{'A', 10}, {'B', 11},
		{'C', 12}, {'D', 13},
		{'E', 14}, {'F', 15}
	};
	TCHAR *mstr = _tcsupr(_tcsdup(value));
	TCHAR *s = mstr;
	DWORD result = 0;
	if (*s == '0' && *(s + 1) == 'X') s += 2;
	bool firsttime = true;
	while (*s != '\0')
	{
		bool found = false;
		for (int i = 0; i < 16; i++)
		{
			if (*s == HexMap[i].chr)
			{
				if (!firsttime) result <<= 4;
				result |= HexMap[i].value;
				found = true;
				break;
			}
		}
		if (!found) break;
		s++;
		firsttime = false;
	}
	free(mstr);
	return (result&0xff000000 | ((result&0x000000ff) << 16) | ((result&0x0000ff00)) | ((result&0x00ff0000) >> 16)) ;

}
CString CSVPToolBox::getFileVersionHash(CString szPath){
	DWORD             dwHandle;
	UINT              dwLen;
	UINT              uLen;
	LPVOID            lpBuffer;
	VS_FIXEDFILEINFO  *lpBuffer2;
	
	dwBuild = 0;

	dwLen  = GetFileVersionInfoSize(szPath, &dwHandle);

	TCHAR * lpData = (TCHAR*) malloc(dwLen);
	if(!lpData)
		return _T("");
	memset((char*)lpData, 0 , dwLen);

	/* GetFileVersionInfo() requires a char *, but the api doesn't
	* indicate that it will modify it */
	if(GetFileVersionInfo(szPath, dwHandle, dwLen, lpData) != 0)
	{
		if(VerQueryValue(lpData, _T("\\"), &lpBuffer, &uLen) != 0)
		{
			lpBuffer2 = (VS_FIXEDFILEINFO *)lpBuffer;
			dwMajor   = HIWORD(lpBuffer2->dwFileVersionMS);
			dwMinor   = LOWORD(lpBuffer2->dwFileVersionMS);
			dwRelease = HIWORD(lpBuffer2->dwFileVersionLS);
			dwBuild   = LOWORD(lpBuffer2->dwFileVersionLS);
		}
	}
	long iFileLen;
	int fp;
	if( _wsopen_s ( &fp, szPath, _O_RDONLY, _SH_DENYNO,
		_S_IREAD ) == 0 )
	{

		iFileLen = filelength(fp);
		_close( fp);

	}
	
	CString szRet;
	szRet.Format(_T("%d.%d.%d.%d.%d"), dwMajor, dwMinor, dwRelease, dwBuild,iFileLen);
	return szRet;
}
BOOL CSVPToolBox::isWriteAble(CString szPath){
	FILE* fp;
	if ( _wfopen_s( &fp, szPath, _T("ab") ) != 0){
		return 0; //input file open error
	}else{
		fclose(fp);
	}
	return 1;
}
CString CSVPToolBox::getSameTmpName(CString fnin )
{
	CStringArray szaPathinfo;
	this->getVideoFileBasename(fnin, &szaPathinfo);
	CString fntdir = this->GetTempDir();
	CString fnout = fntdir + szaPathinfo.GetAt(3) + szaPathinfo.GetAt(1) ;
	int i = 0;
	while(this->ifFileExist(fnout)){
		i++;
		CString szBuf;
		szBuf.Format(_T(".svr%d"), i);
		fnout = fntdir + szaPathinfo.GetAt(3) + szBuf + szaPathinfo.GetAt(1) ;
	}
	return fnout;
}
CString CSVPToolBox::getSameTmpExt(CString fnin )
{
	CStringArray szaPathinfo;
	this->getVideoFileBasename(fnin, &szaPathinfo);
	CString fnout = this->getTmpFileName();
	fnout += szaPathinfo.GetAt(1) ;
	return fnout;
}
int CSVPToolBox::packGZfile(CString fnin , CString fnout)
{
	
	FILE* fp;
	int ret = 0;
	if ( _wfopen_s( &fp, fnin, _T("rb") ) != 0){
		return -1; //input file open error
	}
	int iDescLen;
	char * szFnout = this->CStringToUTF8(fnout, &iDescLen, CP_ACP);

	gzFile gzfOut = gzopen( szFnout , "wb9");	
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
	free(szFnout);

	if (ret != 0 ){
		CString szLog ; 
		szLog.Format(_T("Gz pack file fail: %s to %s ret %d"), fnin , fnout, ret);
		SVP_LogMsg(szLog);
	}
	return ret;
}

int CSVPToolBox::unpackGZfile(CString fnin , CString fnout)
{
	
	FILE* fout;
	int ret = 0;
	if ( _wfopen_s( &fout, fnout, _T("wb") ) != 0){
		return -1; //output file open error
	}
	int iDescLen;
	char * szFnin = this->CStringToUTF8(fnin, &iDescLen, CP_ACP);
	
	gzFile gzfIn = gzopen( szFnin , "rb");	
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
	free(szFnin);
	if (ret != 0 ){
		CString szLog ; 
		szLog.Format(_T("Gz unpack file fail: %s to %s ret %d"), fnin , fnout, ret);
		SVP_LogMsg(szLog);
	}
	return ret;
}
/*

static unsigned char* RARbuff = NULL;
static unsigned int RARpos = 0;

static int PASCAL MyProcessDataProc(unsigned char* Addr, int Size)
{
	ASSERT(RARbuff);

	memcpy(&RARbuff[RARpos], Addr, Size);
	RARpos += Size;

	return(1);
}*/

CString CSVPToolBox::extractRarFile(CString rarfn){
	CString szRet = _T("");
	/*
	HMODULE h = LoadLibrary(_T("unrar.dll"));
		if(!h) return szRet;
	
		RAROpenArchiveEx OpenArchiveEx = (RAROpenArchiveEx)GetProcAddress(h, "RAROpenArchiveEx");
		RARCloseArchive CloseArchive = (RARCloseArchive)GetProcAddress(h, "RARCloseArchive");
		RARReadHeaderEx ReadHeaderEx = (RARReadHeaderEx)GetProcAddress(h, "RARReadHeaderEx");
		RARProcessFile ProcessFile = (RARProcessFile)GetProcAddress(h, "RARProcessFile");
		RARSetChangeVolProc SetChangeVolProc = (RARSetChangeVolProc)GetProcAddress(h, "RARSetChangeVolProc");
		RARSetProcessDataProc SetProcessDataProc = (RARSetProcessDataProc)GetProcAddress(h, "RARSetProcessDataProc");
		RARSetPassword SetPassword = (RARSetPassword)GetProcAddress(h, "RARSetPassword");
	
		if(!(OpenArchiveEx && CloseArchive && ReadHeaderEx && ProcessFile 
			&& SetChangeVolProc && SetProcessDataProc && SetPassword))
		{
			FreeLibrary(h);
			return szRet;
		}*/
	

	struct RAROpenArchiveDataEx ArchiveDataEx;
	memset(&ArchiveDataEx, 0, sizeof(ArchiveDataEx));

	ArchiveDataEx.ArcNameW = (LPTSTR)(LPCTSTR)rarfn;
	char fnA[MAX_PATH];
	if(wcstombs(fnA, rarfn, rarfn.GetLength()+1) == -1) fnA[0] = 0;
	ArchiveDataEx.ArcName = fnA;

	ArchiveDataEx.OpenMode = RAR_OM_EXTRACT;
	ArchiveDataEx.CmtBuf = 0;
	HANDLE hrar = RAROpenArchiveEx(&ArchiveDataEx);
	if(!hrar) 
	{
		//FreeLibrary(h);
		return szRet;
	}

	//SetProcessDataProc(hrar, MyProcessDataProc);

	struct RARHeaderDataEx HeaderDataEx;
	HeaderDataEx.CmtBuf = NULL;
	szRet = this->getTmpFileName() ;
	szRet += _T(".sub");
	CFile m_sub ( szRet, CFile::modeCreate|CFile::modeReadWrite|CFile::typeBinary );


	while(RARReadHeaderEx(hrar, &HeaderDataEx) == 0)
	{

		CString subfn(HeaderDataEx.FileNameW);


		if(!subfn.Right(4).CompareNoCase(_T(".sub")))
		{
			CAutoVectorPtr<char> buff;
			if(!buff.Allocate(HeaderDataEx.UnpSize))
			{
				RARCloseArchive(hrar);
				//FreeLibrary(h);
				return szRet;
			}

			/*
			RARbuff = buff;
						RARpos = 0;
			
						if(ProcessFile(hrar, RAR_TEST, NULL, NULL))
						{
							CloseArchive(hrar);
							FreeLibrary(h);
			
							return szRet;
						}*/
			
			int errRar = RARExtractChunkInit(hrar, HeaderDataEx.FileName);
			if (errRar != 0) {
				RARCloseArchive(hrar);
				//SVP_LogMsg5(L"RARExtractChunkInit Failed");
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

			//free(buff);
			//RARbuff = NULL;
			//RARpos = 0;

			break;
		}

		RARProcessFile(hrar, RAR_SKIP, NULL, NULL);
	}

	RARCloseArchive(hrar);
	//FreeLibrary(h);
	return szRet;
}
CString CSVPToolBox::DetectSubFileLanguage(CString fn){
	CString szRet = _T(".chn");
	FILE *stream ;
	if ( _wfopen_s( &stream, fn, _T("rb") ) == 0 ){
		//detect bom?

		int totalWideChar = 0;
		int totalChar = 0;
		int ch;
		
		for( int i=0;  ( feof( stream ) == 0 ); i++ )
		{
			ch = 0xff & fgetc( stream );
			if (ch >= 0x80 ){
				totalWideChar++;
			}
			totalChar++;
		}

		fclose( stream );
		
		if(totalWideChar < (totalChar / 10) && totalWideChar < 1700){
			szRet = _T(".eng");
		}
	}

	return szRet;
}
int CSVPToolBox::DetectFileCharset(CString fn){
// 	;
// 	;
	//
	FILE *stream ;
	if ( _wfopen_s( &stream, fn, _T("rb") ) == 0 ){
		//detect bom?

		int totalWideChar = 0;
		int totalGBKChar = 0;
		int totalBig5Char = 0;
		
		int ch, ch2;
		int xch;
        int i=0;
		for(  i=0;  ( feof( stream ) == 0 ); i++ )
		{
			ch = 0xff & fgetc( stream );
			if (ch >= 0x80 ){
				totalWideChar++;
				ch2 = 0xff & fgetc( stream );

				if ( ch >= 0xA1 && ch < 0xA9 && ch2 >= 0xA1 && ch2 <= 0xFE ){
					totalGBKChar++;
				}else if ( ch >= 0xB0 && ch < 0xF7 && ch2 >= 0xA1 && ch2 <= 0xFE){
					totalGBKChar++;
				}
				
				xch = ch << 8 | ch2;
				if ( xch >= 0xa440 && xch <= 0xc67e){
					totalBig5Char++;
				}

			}
			
		}

		fclose( stream );
        //TODO: detect Cyrillic
        /*
        * http://en.wikipedia.org/wiki/Windows-1251
        */
        SVP_LogMsg5(L"Charset detect %d %d %d %d %f",i, totalGBKChar , totalBig5Char, totalWideChar, double(totalGBKChar/totalWideChar));
		if ( totalGBKChar > totalBig5Char && totalGBKChar > totalWideChar*6/7 && totalWideChar > 500){
            return GB2312_CHARSET;
		}else if ( totalGBKChar < totalBig5Char && totalBig5Char > totalWideChar*6/7 && totalWideChar > 500 ){
            return CHINESEBIG5_CHARSET;
		}
        
	}
	
	
   // AfxMessageBox(L"Default");

	return DEFAULT_CHARSET;
}
int CSVPToolBox::ClearTmpFiles(){
	int i;
	for( i = 0; i < this->szaTmpFileNames.GetCount(); i++){
		_wremove(szaTmpFileNames.GetAt(i));
	}
	return i;
}
WCHAR* CSVPToolBox::getTmpFileName(){
	WCHAR* tmpnamex ;
	int i;

	for (i = 0; i < 5; i++) //try 5 times for tmpfile creation
	{
		tmpnamex = _wtempnam( this->GetTempDir(), _T("svp") );
		if (tmpnamex)
			break;

	}
	if (!tmpnamex){
		SVP_LogMsg(_T("TMP FILE name genarater error")); 
		return 0;
	}else{
		//SVP_LogMsg(tmpnamex);
		return tmpnamex;
	}
}
FILE* CSVPToolBox::getTmpFileSteam(){
	FILE *stream;
	WCHAR* tmpnamex = this->getTmpFileName();
	errno_t err;
	//SVP_LogMsg5(L"tmpnamex %s", tmpnamex);
	if (!tmpnamex){
		SVP_LogMsg(_T("TMP FILE stream name genarater error")); 
		return 0;
	}else{
		
		err = _wfopen_s(&stream, tmpnamex, _T("wb+"));
		if(err){
			SVP_LogMsg(_T("TMP FILE stream Open for Write error")); 
			stream = 0;
		}else{
			this->szaTmpFileNames.Add(tmpnamex);
			free(tmpnamex);
		}
	}
	return stream;
}
BOOL CSVPToolBox::CreatDirForFile(CString cPath){
	return CreatDirRecursive( GetDirFromPath( cPath ) );
}
BOOL CSVPToolBox::CreatDirRecursive(CString cPath){
	if(_wmkdir(cPath) < 0 && errno == ENOENT ){
		CPath parentPath(cPath);
		parentPath.Canonicalize();
		parentPath.RemoveBackslash();
		if( parentPath.RemoveFileSpec() ){
			CreatDirRecursive(parentPath);
			return _wmkdir(cPath);
		}
	}
	return TRUE;
}
CString CSVPToolBox::GetPlayerPath(CString progName){
	CString path;
	GetModuleFileName(/*AfxGetInstanceHandle()*/NULL, path.GetBuffer(MAX_PATH), MAX_PATH);
	path.ReleaseBuffer();
	if (progName.IsEmpty()){
		return path;
	}else{
		CPath cpath(path);
		cpath.RemoveFileSpec();
		cpath.AddBackslash();
		cpath.Append(progName);
		return cpath;
	}
}
BOOL CSVPToolBox::delDirRecursive(CString path){
	SHFILEOPSTRUCT sh;
	CPath cpath(path);
	cpath.RemoveBackslash();
	cpath.Append(_T("\0\0"));

	sh.hwnd   = NULL;
	sh.wFunc  = FO_DELETE;
	sh.pFrom  = cpath;
	sh.pTo    = NULL;
	sh.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;
	sh.hNameMappings = 0;
	sh.lpszProgressTitle = NULL;

	return SHFileOperation (&sh);


}
CString CSVPToolBox::GetDirFromPath(CString path){
	CPath cpath(path);
	cpath.RemoveFileSpec();
	cpath.AddBackslash();
	return cpath;
}
int CSVPToolBox::CleanUpOldFiles(CString szDir, int parm, int ilimit, int byNumber)
{

    CFileFind finder;

    // build a string with wildcards
    CString strWildcard(szDir + _T("\\*.*"));
    int iTotalDeleted = 0;
    // start working for files
    BOOL bWorking = finder.FindFile(strWildcard);
    CTime ctNow = CTime::GetCurrentTime();
    while (bWorking)
    {
        bWorking = finder.FindNextFile();

        // skip . and .. files; otherwise, we'd
        // recur infinitely!

        if (finder.IsDots())
            continue;

        // if it's a directory, recursively search it

        if (finder.IsDirectory())
            continue;
        
        CTime lAccessTime;
        finder.GetLastAccessTime(lAccessTime);
        if(CTimeSpan (ctNow - lAccessTime) > CTimeSpan(parm *24*3600))
        {
            _wunlink(  finder.GetFilePath() );
            //break;
            //SVP_LogMsg5(L"Clean file %s for havn't been used for %d days %s",  finder.GetFilePath(), 
            //    parm, lAccessTime.Format(L"%Y%m%d") );

            if(iTotalDeleted++ > ilimit){
                break;
            }
        }
        

        
    }

    finder.Close();


    return 0;
}
int CSVPToolBox::HandleSubPackage(FILE* fp){
	//Extract Package
	SVP_LogMsg( _T("Extracting Package") );


	char szSBuff[8];
	int err;
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		
		SVP_LogMsg( _T("Fail to retrive Package Data Length ") );
		return -1;
	}
	int iPackageLength = this->Char4ToInt(szSBuff);
	
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive Desc Data Length"));
		return -2;
	}

	
	size_t iDescLength = this->Char4ToInt(szSBuff);

	if(iDescLength > 0){
		SVP_LogMsg(_T("retriving Desc Data"));
		char * szDescData = this->ReadToPTCharByLength(fp, iDescLength);
		if(!szDescData){
			SVP_LogMsg(_T("Fail to retrive Desc Data"));
			return -4;
		}
		// convert szDescData to Unicode and save to CString
		szaSubDescs.Add(this->UTF8ToCString( szDescData , iDescLength));
		free(szDescData);
	}else{
		szaSubDescs.Add(_T(""));
	}
	err = this->ExtractSubFiles(fp);
	
// 	fpos_t pos;
// 	fgetpos( fp, &pos ) ;
// 	CString szErr;
// 	szErr.Format(_T("Reading At %d got next %ld") , pos,iDescLength);
// 	SVP_LogMsg(szErr);

	

	return 0;

}
int CSVPToolBox::ExtractSubFiles(FILE* fp){
	char szSBuff[8];
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive File Data Length"));
		return -1;
	}

	size_t iFileDataLength = this->Char4ToInt(szSBuff); //确认是否确实有文件下载
	if(!iFileDataLength){
		SVP_LogMsg(_T("No real file released"));
		return 0;
	}

	if ( fread(szSBuff , sizeof(char), 1, fp) < 1){
		SVP_LogMsg(_T("Fail to retrive how many Files are there"));
		return -2;
	}
	int iFileCount = szSBuff[0];
	if( iFileCount <= 0 ){
		SVP_LogMsg(_T("No file in File Data"));
		return -3;
	}

	int iCurrentId = this->szaSubTmpFileList.GetCount();
	this->szaSubTmpFileList.Add(_T(""));
	for(int k = 0; k < iFileCount; k++){
		if(this->ExtractEachSubFile(fp, iCurrentId) ){
			SVP_LogMsg(_T("File Extract Error ") );
			return -4;
		}
	}

	return 0;
}
char* CSVPToolBox::ReadToPTCharByLength(FILE* fp, size_t length){
	char * szBuffData =(char *)calloc( length + 1, sizeof(char));

	if(!szBuffData){
		SVP_LogMsg(_T("Fail to calloc Data "));
		return 0;
	}

	if ( fread(szBuffData , sizeof(char), length, fp) < length){
		SVP_LogMsg(_T("Fail to retrive  Data "));
		return 0;
	}

	return szBuffData;
}
int CSVPToolBox::ExtractEachSubFile(FILE* fp, int iSubPosId){
	// get file ext name
	char szSBuff[4096];
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive Single File Pack Length"));
		return -1;
	}

	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive File Ext Name Length"));
		return -1;
	}

	size_t iExtLength = this->Char4ToInt(szSBuff);
	char* szExtName = this->ReadToPTCharByLength(fp, iExtLength);
	if(!szExtName){
		SVP_LogMsg(_T("Fail to retrive File Ext Name"));
		return -2;
	}
	
	
	//get filedata length
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive File Data Length"));
		return -1;
	}

	size_t iFileLength = this->Char4ToInt(szSBuff);
	
	// gen tmp name and tmp file point
	WCHAR* otmpfilename = this->getTmpFileName();
	if(!otmpfilename){
		SVP_LogMsg(_T("TMP FILE name for sub file error")); 
		return -5;
	}
	FILE* fpt;
	errno_t err = _wfopen_s(&fpt, otmpfilename, _T("wb"));
	if(err){
		SVP_LogMsg(_T("TMP FILE stream for sub file  Write error")); 
		free(otmpfilename);
		return -4;
	}

	// copy date to tmp file
	size_t leftoread = iFileLength;
	do{
		size_t needtoread = SVP_MIN( 4096, leftoread );
		size_t accturead = fread(szSBuff , sizeof(char), needtoread, fp);
		if(accturead == 0){
			//wtf
			break;
		}
		leftoread -= accturead;
		err = fwrite(szSBuff,  sizeof(char), accturead , fpt);

		
	}while(leftoread > 0);
	fclose( fpt );

	WCHAR* otmpfilenameraw = this->getTmpFileName();
	CString szLogmsg;
	int gzret = this->unpackGZfile( otmpfilename , otmpfilenameraw );
	szLogmsg.Format(_T(" Gzip decompress %s -> %s : %d "),  otmpfilename , otmpfilenameraw , gzret );

	SVP_LogMsg(szLogmsg);
	// add filename and tmp name to szaTmpFileNames
	this->szaSubTmpFileList[iSubPosId].Append( this->UTF8ToCString(szExtName, iExtLength)); //why cant use + ???
	this->szaSubTmpFileList[iSubPosId].Append(_T("|") );
	this->szaSubTmpFileList[iSubPosId].Append(otmpfilenameraw);
	this->szaSubTmpFileList[iSubPosId].Append( _T(";"));
	//SVP_LogMsg(this->szaSubTmpFileList[iSubPosId] + _T(" is the szaTmpFileName"));
	//this->szaSubTmpFileList[iSubPosId].SetString( otmpfilename);
	
	free(szExtName);
	free(otmpfilename);

	return 0;
}

CString CSVPToolBox::Implode(CString szTok, CStringArray* szaOut){
	CString szRet;
	for(int i = 0; i < szaOut->GetCount(); i++){
		if(i > 0) { szRet.Append(szTok); }
		szRet.Append(szaOut->GetAt(i));
	}
	return szRet;
}
int CSVPToolBox::Explode(CString szIn, CString szTok, CStringArray* szaOut){
	szaOut->RemoveAll();

	CString resToken;
	int curPos= 0;

	resToken= szIn.Tokenize(szTok, curPos);
	while (resToken != _T(""))
	{
		szaOut->Add(resToken);
		resToken= szIn.Tokenize(szTok,curPos);
	};

	return 0;
}
BOOL CSVPToolBox::ifDirExist(CString path){
	CPath cpath(path);
	cpath.RemoveBackslash();
	return !_waccess(cpath, 0);

}
BOOL CSVPToolBox::ifFileExist(CString szPathname, BOOL evenSlowDriver ){
	if( szPathname.Left(6).MakeLower() == _T("rar://") ){
		//RARTODO: 检测rar内的文件是否存在 //Done
		CSVPRarLib svpRar;
		if( svpRar.SplitPath(szPathname) )
			szPathname = svpRar.m_fnRAR;
		else
			return false;
	}

	if(!evenSlowDriver){
		CPath Driver(szPathname);
		Driver.StripToRoot();
		switch(GetDriveType(Driver)){
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

	return ( !_wstat(szPathname, &sbuf) && _S_IFREG & sbuf.st_mode);
}
BOOL CSVPToolBox::CanUseCUDAforCoreAVC(){
	//if( !ifFileExist( this->GetPlayerPath(L"codecs\\cavc.ax")) &&   !ifFileExist( this->GetPlayerPath(L"codecs\\CoreAVCDecoder.ax")) )
	{
	//	return false;
	}
	HMODULE hDll = ::LoadLibrary(_T("nvcuda.dll"));
	bool canCUDA = false;
	if (hDll)
	{
		CString dllpath;
		GetModuleFileName(hDll, dllpath.GetBuffer(MAX_PATH), MAX_PATH);
		dllpath.ReleaseBuffer();

		//get build
		this->getFileVersionHash(dllpath);
		struct _stat  fbuf;
		_wstat ( dllpath, &fbuf );

		if(fbuf.st_ctime > 1238112000 ||  fbuf.st_mtime > 1238112000 ){
			canCUDA = true;
		}
		
		{
			if ( this->dwBuild ){
				if(this->dwBuild > 8205 || this->dwBuild < 1105){ // not sure when will build version over 200.xx
					//ok
					canCUDA = true;
				}
			}
		}
		//get version
		/*
		NvAPI_Status nvapiStatus;
		NV_DISPLAY_DRIVER_VERSION version = {0};
		version.version = NV_DISPLAY_DRIVER_VERSION_VER;
		nvapiStatus = NvAPI_Initialize();
		nvapiStatus = NvAPI_GetDisplayDriverVersion (NVAPI_DEFAULT_HANDLE, &version);
		if(nvapiStatus != NVAPI_OK) {... inform user nvidia card not found ...}
		if(version.drvVersion < DESIRED_NVIDIA_DRIVER_VERSION) {... inform user nvidia driver version not the one you wanted ...}
		*/

	}
	return canCUDA;
}
#include <wbemidl.h>

int CSVPToolBox::GetWMIGPURam()
{
	IWbemLocator* locator = NULL;

	//CoInitialize(NULL);
	SVP_LogMsg6("GetWMIGPURam Start");

	HRESULT hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**)&locator);
	if( FAILED(hr) )
	{
		SVP_LogMsg6("WMI Couldn't create locator.\n");
		return 0;
	}

	IWbemServices* services = NULL;
	hr = locator->ConnectServer(_T("root\\cimv2"), NULL, NULL, NULL, 0, NULL, NULL, &services);
	locator->Release();
	if( FAILED(hr) )
	{
		SVP_LogMsg6("WMI Couldn't connect.\n");
		return -1;
	}

	hr = CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if( FAILED(hr) )
	{
		services->Release();
		SVP_LogMsg6("WMI Could not set security.\n");
		return -2;
	}

	IEnumWbemClassObject* instanceEnum = NULL;
	hr = services->CreateInstanceEnum(_T("Win32_VideoController"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &instanceEnum);
	services->Release();
	if( FAILED(hr) )
	{
		SVP_LogMsg6("WMI Couldn't create enumerator.\n");
		return -3;
	}

	IWbemClassObject* instance;
	ULONG objectsReturned = 0;
	int iGPUMAXRAM = 0;
	while( 1 )
	{
		hr = instanceEnum->Next(WBEM_INFINITE, 1, &instance, &objectsReturned);
		if( FAILED(hr) )
		{
			instanceEnum->Release();
			SVP_LogMsg6("WMI Enum->Next() failed.\n");
			return 4;
		}

		if( objectsReturned != 1 )
			break;

		VARIANT v;
		VariantInit(&v);
		hr = instance->Get(_T("AdapterRAM"), 0, &v, NULL, NULL);
		if( FAILED(hr) )
		{
			SVP_LogMsg6("Could not retrieve \"AdapterRAM\" property.\n");
		}
		else
		{
			int iGPURAM = V_UI4(&v);
			SVP_LogMsg6("AdapterRAM = %d\n", iGPURAM);
			iGPUMAXRAM = max(iGPUMAXRAM, iGPURAM );
		}

		VariantClear(&v);
		instance->Release();
		instance = NULL;
	}

	instanceEnum->Release();

	//printf("Completed Successfully.\n");
	//CoUninitialize();
	return iGPUMAXRAM;
}
int CSVPToolBox::GetGPUString(CStringArray * szaGPUString){
	int ret = 0; //no gpu
	IDirect3D9* pD3D;
	int m_nPCIVendor = 0;
	int m_nPCIDevice = 0;
	LARGE_INTEGER							m_VideoDriverVersion;
	m_VideoDriverVersion.HighPart = 0;
	m_VideoDriverVersion.LowPart = 0;
	CString GPUString;
	szaGPUString->RemoveAll();

	HMODULE m_hD3D9 =  LoadLibrary(L"d3d9.dll");
	if (m_hD3D9)
	{
		if(! GetProcAddress(m_hD3D9, "Direct3DCreate9") ){
			return 0 ;
		}
	}else{	return 0 ;}

	
	if (pD3D = Direct3DCreate9(D3D_SDK_VERSION)) 
	{
		
		//HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
		//if(hMonitor == NULL) return D3DADAPTER_DEFAULT;

		for(UINT adp = 0, num_adp = pD3D->GetAdapterCount(); adp < num_adp; ++adp)
		{
			
			//HMONITOR hAdpMon = pD3D->GetAdapterMonitor(adp);
			//if(hAdpMon == hMonitor) return adp;
			CString m_strDeviceDescription;
			D3DADAPTER_IDENTIFIER9 adapterIdentifier;
			if (pD3D->GetAdapterIdentifier( adp, 0, &adapterIdentifier) == S_OK)
			{
				m_nPCIVendor = adapterIdentifier.VendorId;
				m_nPCIDevice = adapterIdentifier.DeviceId;
				m_VideoDriverVersion = adapterIdentifier.DriverVersion;
				m_strDeviceDescription = adapterIdentifier.Description;
				m_strDeviceDescription.AppendFormat (_T(" (0x%x::0x%x) 驱动：%d.%d.%d.%d \n"),
						m_nPCIVendor , m_nPCIDevice,
						HIWORD(adapterIdentifier.DriverVersion.HighPart),
						LOWORD(adapterIdentifier.DriverVersion.HighPart),
						HIWORD(adapterIdentifier.DriverVersion.LowPart),
						LOWORD(adapterIdentifier.DriverVersion.LowPart)
						//GET_WHQL_YEAR(adapterIdentifier.WHQLLevel),
						//GET_WHQL_MONTH(adapterIdentifier.WHQLLevel),
						//GET_WHQL_DAY(adapterIdentifier.WHQLLevel)

				);
				szaGPUString->Add( m_strDeviceDescription );

				CString szDetect  = CString(adapterIdentifier.Description);

				szDetect.MakeUpper();
				if(szDetect.Find(_T("ATI")) >= 0 || 0x1002 == m_nPCIVendor){
					int imodel = _wtoi(szDetect);
					if(m_nPCIDevice >= 0x9000){
						ret = 1;
					}else if(m_nPCIDevice >= 0x6800 && m_nPCIDevice <= 0x68ff){
						//HD57xx
						ret = 1;
					}else if(m_nPCIDevice >= 0x5a00 && m_nPCIDevice <= 0x5aff){
						ret = 0;
					}else if(m_nPCIDevice == 0x94c3 ){ //ATI Radeon HD 2400 PRO (0x1002::0x94c3) 驱动：8.14.10.678
						ret = 0;
					}else if(imodel > 2000 && imodel < 7000){
						ret = 1;
					}
					
				}else if(szDetect.Find(_T("NVIDIA")) >= 0  || 0x10DE == m_nPCIVendor){
					int imodel = _wtoi(szDetect);
					if (m_VideoDriverVersion.HighPart > 170) {
						ret = 1;
					}else if(imodel > 7000 && imodel < 10000){
						ret = 1;
					}
				}else if(szDetect.Find(_T("Intel")) >= 0  || 0x8086 == m_nPCIVendor){
					if(m_nPCIDevice >= 0x2e00 && m_nPCIDevice <= 0x2eff){
						ret = 1; // G45
					}else if(m_nPCIDevice >= 0x2a40 && m_nPCIDevice <= 0x2a4f){
						ret = 1; // G45 移动版
					}
				}

				if(ret)
					break;
				
			}
		}

		
		pD3D->Release();
	}

	int iGPURam = GetWMIGPURam();
	if(iGPURam){
		CString szBuf;
		iGPURam = iGPURam /1024/1024;
		szBuf.Format(L"GPU RAM %dMB" ,iGPURam);
		szaGPUString->Add(szBuf);

		if(ret > 0){
			if(iGPURam < 256)
				ret = 0;
		}else if(iGPURam > 256)
		{
			ret = 1;
		}
	}
	return ret;
}
BOOL CSVPToolBox::ifDirWritable(CString szDir){
	
	CPath szPath(szDir);
	szPath.RemoveBackslash();
	szPath.AddBackslash();
	szDir = CString(szPath);

	HANDLE hFile =
		CreateFile(szDir, FILE_ADD_FILE|FILE_WRITE_ATTRIBUTES|FILE_READ_ATTRIBUTES,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

	if (hFile == INVALID_HANDLE_VALUE){
		
		return false;
	}


	FILETIME ftCreate, ftAccess, ftWrite;
	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
		return FALSE;

	BOOL ret = false;
	FILE* fp = NULL;
	fp = _wfopen( szDir + _T("svpwrtst"), _T("wb") );
	if( fp != NULL )
	{
		fclose( fp );
		_wremove(szDir + _T("svpwrtst"));
		ret = true;
	}

	SetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite);

	CloseHandle(hFile);
	
	return ret;
	
	
	
	
	
	
}
bool CSVPToolBox::GetAppDataPath(CString& path)
{
	path.Empty();
	TCHAR szPath[MAX_PATH];

	HRESULT hr;
	LPITEMIDLIST pidl;
	hr = SHGetSpecialFolderLocation( NULL, CSIDL_APPDATA, &pidl);
	if (hr){
		//Old method
		CRegKey key;
		if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), KEY_READ))
		{
			ULONG len = MAX_PATH;
			if(ERROR_SUCCESS == key.QueryStringValue(_T("AppData"), szPath, &len))
				path.ReleaseBufferSetLength(len);
		}
	}


	BOOL f = SHGetPathFromIDList(pidl, szPath);
	PathRemoveBackslash(szPath);
	PathAddBackslash(szPath); 
	PathAppend(szPath,  _T("SPlayer"));
	path = szPath;


	if(path.IsEmpty())
		return(false);
	/*

	PathCombine(  szPath ,)
	CPath p;
	p.Combine(szPath, _T("SPlayer"));
	path = (LPCTSTR)p;
	*/

	return(true);
}

CString CSVPToolBox::getSubFileByTempid(int iTmpID, CString szVidPath)
{
  //get base path name
  CStringArray szVidPathInfo ;
  CString szTargetBaseName = _T("");
  CString szDefaultSubPath = _T("");

  AppSettings& s = AfxGetAppSettings();
  CString StoreDir = s.SVPSubStoreDir;
  this->getVideoFileBasename(szVidPath, &szVidPathInfo); 

  if(s.bSaveSVPSubWithVideo && 
    this->ifDirWritable(szVidPathInfo.GetAt(SVPATH_DIRNAME)))
  {
    StoreDir = szVidPathInfo.GetAt(SVPATH_DIRNAME);
  }
  else
  {
    if(StoreDir.IsEmpty() || !ifDirExist(StoreDir) || 
      !ifDirWritable(StoreDir))
    {
      GetAppDataPath(StoreDir);
      CPath tmPath(StoreDir);
      tmPath.RemoveBackslash();
      tmPath.AddBackslash();
      tmPath.Append( _T("SVPSub"));
      StoreDir = (CString)tmPath;
      _wmkdir(StoreDir);
      if(StoreDir.IsEmpty() || !ifDirExist(StoreDir) || 
        !ifDirWritable(StoreDir))
      {
        StoreDir = GetPlayerPath(_T("SVPSub"));
        _wmkdir(StoreDir);
        if(StoreDir.IsEmpty() || !ifDirExist(StoreDir) || 
          !ifDirWritable(StoreDir))
        {
          //WTF cant create fordler ?
        }
        else
          s.SVPSubStoreDir = StoreDir;
      }
      else
        s.SVPSubStoreDir = StoreDir;
    }
  }

  CPath tmBasenamePath(StoreDir);
  tmBasenamePath.RemoveBackslash();
  tmBasenamePath.AddBackslash();
  StoreDir =  (CString) tmBasenamePath;
  this->getVideoFileBasename(szVidPath, &szVidPathInfo);
  tmBasenamePath.Append(szVidPathInfo.GetAt(SVPATH_FILENAME));

  CString szBasename =tmBasenamePath;

  {
    //StoreDir = 

  }
  //SVP_LogMsg5(L"Save Sub Use %s" , szBasename);
  //if(!this->ifDirWritable(szBasename) ){
  //	szBasename = this->GetTempDir() + szVidPathInfo.GetAt(SVPATH_FILENAME);
  //}

  //set new file name
  CStringArray szSubfiles;
  CString szXTmpdata = this->szaSubTmpFileList.GetAt(iTmpID);
  this->Explode(szXTmpdata, _T(";"), &szSubfiles);
  bool bIsIdxSub = FALSE;
  int ialreadyExist = 0;
  if ( szXTmpdata.Find(_T("idx|")) >= 0 && szXTmpdata.Find(_T("sub|")) >= 0){
    if ( !this->ifFileExist( szBasename + _T(".idx") ) && !this->ifFileExist( szBasename + _T(".sub") ) ){
      bIsIdxSub = TRUE;
    }
  }

  if( szSubfiles.GetCount() < 1){
    SVP_LogMsg( _T("Not enough files in tmp array"));

  }
  for(int i = 0; i < szSubfiles.GetCount(); i++){
    CStringArray szSubTmpDetail;
    this->Explode(szSubfiles[i], _T("|"), &szSubTmpDetail);
    if (szSubTmpDetail.GetCount() < 2){
      SVP_LogMsg( _T("Not enough detail in sub tmp string") + szSubfiles[i]);
      continue;
    }
    CString szSource = szSubTmpDetail[1];
    CString szLangExt  = _T(".chn"); //TODO: use correct language perm 
    if(bIsIdxSub){
      szLangExt  = _T("");
    }else{
      szLangExt  = DetectSubFileLanguage(szSource);
    }
    if (szSubTmpDetail[0].GetAt(0) != _T('.')){
      szSubTmpDetail[0] = CString(_T(".")) + szSubTmpDetail[0];
    }
    CString szTarget = szBasename + szLangExt + szSubTmpDetail[0];
    szTargetBaseName = szBasename + szLangExt ;

    CMD5Checksum cm5source;
    CString szSourceMD5 = cm5source.GetMD5((LPCTSTR)szSource).c_str();
    CString szTargetMD5 ;
    //check if target exist
    CString szTmp = _T("") ;
    int ilan = 1;
    while( this->ifFileExist(szTarget) ){ 
      //TODO: compare if its the same file
      cm5source.Clean();

      szTargetMD5 = cm5source.GetMD5((LPCTSTR)szTarget).c_str();
      if(szTargetMD5 == szSourceMD5){
        SVP_LogMsg5(_T("同样的字幕文件已经存在了 %s %s") , szSource , szTarget);
        ialreadyExist++; //TODO: 如果idx+sub里面只有一个文件相同怎么办 ？？~~ 
        break;
      }

      szTmp.Format(_T("%d"), ilan);
      szTarget = szBasename + szLangExt + szTmp + szSubTmpDetail[0]; 
      szTargetBaseName = szBasename + szLangExt + szTmp;
      ilan++;
    }

    //SVP_LogMsg5(_T("Copy 字幕文件 %s %s") , szSource , szTarget);
    if( !CopyFile( szSource, szTarget, false) ){
      SVP_LogMsg( szSource + _T(" to ") + szTarget + _T(" 复制字幕文件失败"));
      SVP_LogMsg( CString( _T("字幕文件无法写入 ") ) + szTarget, 31);
      szDefaultSubPath = szSource;
    }else if ( ( (bIsIdxSub && szSubTmpDetail[0].CompareNoCase(_T(".idx")) == 0) || !bIsIdxSub )
      && szDefaultSubPath.IsEmpty()){
        szDefaultSubPath = szTarget;
    }
    CStringArray szaDesclines;
    if(szaSubDescs.GetCount() > iTmpID){
      this->Explode( szaSubDescs.GetAt(iTmpID) , _T("\x0b\x0b") , &szaDesclines);
      if(szaDesclines.GetCount() > 0){
        int iDelay = 0;
        swscanf_s(szaDesclines.GetAt(0), _T("delay=%d"), &iDelay);
        if(iDelay){
          CString szBuf;
          szBuf.Format(_T("%d"), iDelay);
          filePutContent( szTarget + _T(".delay"), szBuf );
          SVP_LogMsg(szTarget + _T(" has delay ") + szBuf);
        }else{
          _wremove(szTarget + _T(".delay"));
        }

      }
    }else{
      SVP_LogMsg(_T("Count of szaSubDescs not match with count of subs "));
    }

    _wremove(szSource);
  }
  if(ialreadyExist){
    return _T("EXIST");
  }else
    return szDefaultSubPath;

}

CString CSVPToolBox::PackageSubFiles(CStringArray* szaSubFiles){
	return _T("");
}
CString CSVPToolBox::GetTempDir(){
	TCHAR lpPathBuffer[MAX_PATH];
	GetTempPath(MAX_PATH,  lpPathBuffer); 
	CString szTmpPath (lpPathBuffer);
	if (szTmpPath.Right(1) != _T("\\") && szTmpPath.Right(1) != _T("/")){
		szTmpPath.Append(_T("\\"));
	}
		
	return szTmpPath;
}
int CSVPToolBox::FindAllSubfile(CString szSubPath , CStringArray* szaSubFiles){
	szaSubFiles->RemoveAll();
	szaSubFiles->Add(szSubPath);
	CStringArray szaPathInfo ;
	CString szBaseName = this->getVideoFileBasename( szSubPath, &szaPathInfo );
	CString szExt = szaPathInfo.GetAt(1);
	
	if(szExt == _T(".idx")){
		szSubPath = szBaseName + _T(".sub");
		if(this->ifFileExist(szSubPath)){
			szaSubFiles->Add(szSubPath);
		}else{
			szSubPath = szBaseName + _T(".rar");
			if(this->ifFileExist(szSubPath)){
				CString szSubFilepath = this->extractRarFile(szSubPath);
				if(!szSubFilepath.IsEmpty()){
					SVP_LogMsg(szSubFilepath + _T(" unrar ed"));
					szaSubFiles->Add(szSubFilepath);
				}else{
					SVP_LogMsg(_T("Unrar error"));
				}
			}
		}
	}
	
	//TODO: finding other subfile
	return 0;
}
int CSVPToolBox::Char4ToInt(char* szBuf){

	int iData =   ( ((int)szBuf[0] & 0xff) << 24) |  ( ((int)szBuf[1] & 0xff) << 16) | ( ((int)szBuf[2] & 0xff) << 8) |  szBuf[3] & 0xff;;
	
	return iData;
}
char* CSVPToolBox::CStringToUTF8(CString szIn, int* iDescLen, UINT codePage)
{
	char* szOut = 0;
	int   targetLen = ::WideCharToMultiByte(codePage,0,szIn,-1,szOut,0,NULL,NULL);
	szOut   =   new   char[targetLen+1];          
	memset(szOut,0,targetLen+1);                
	targetLen = ::WideCharToMultiByte(codePage,0,szIn,-1,szOut,targetLen,NULL,NULL);                
	*iDescLen = targetLen;
	return szOut;
}
CString CSVPToolBox::AnsiToCString(UINT codepag, char* szIn, int iLength)
{
    int   targetLen = ::MultiByteToWideChar(codepag,0,szIn,iLength+1,0,0);
    WCHAR* szOut   =  (WCHAR *)calloc( targetLen + 1, sizeof(WCHAR)); 

    ::MultiByteToWideChar(codepag,0,szIn,iLength+1,szOut,targetLen);
    CString szBuf(szOut,targetLen);
    free(szOut);
    return szBuf;
}

CString CSVPToolBox::UTF8ToCString(char* szIn, int iLength)
{
	int   targetLen = ::MultiByteToWideChar(CP_UTF8,0,szIn,iLength+1,0,0);
	WCHAR* szOut   =  (WCHAR *)calloc( targetLen + 1, sizeof(WCHAR)); 

	::MultiByteToWideChar(CP_UTF8,0,szIn,iLength+1,szOut,targetLen);
	CString szBuf(szOut,targetLen);
	free(szOut);
	return szBuf;
}


UINT CSVPToolBox::GetAdapter(LPVOID lpD3D)
{
	IDirect3D9* pD3D = (IDirect3D9*)lpD3D;
	if(m_hWnd == NULL || pD3D == NULL) return D3DADAPTER_DEFAULT;

	HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	if(hMonitor == NULL) return D3DADAPTER_DEFAULT;

	for(UINT adp = 0, num_adp = pD3D->GetAdapterCount(); adp < num_adp; ++adp)
	{
		HMONITOR hAdpMon = pD3D->GetAdapterMonitor(adp);
		if(hAdpMon == hMonitor) return adp;
	}
	return D3DADAPTER_DEFAULT;
}
bool CSVPToolBox::TestD3DCreationAbility(HWND hWnd)
{
	//SVP_LogMsg5(L"TestD3DCreationAbility start");

	m_hWnd = hWnd;
	AppSettings& s = AfxGetAppSettings();
	bool ret = true;
	do{
		CComPtr<IDirect3D9Ex> m_pD3DEx;
		CComPtr<IDirect3D9> m_pD3D;
		CComPtr<IDirect3DDevice9Ex> m_pD3DDevEx;
		CComPtr<IDirect3DDevice9> m_pD3DDev;

		if (AfxGetMyApp()->m_pDirect3DCreate9Ex)
		{
			AfxGetMyApp()->m_pDirect3DCreate9Ex(D3D_SDK_VERSION, (LPVOID**)&m_pD3DEx);
			if(!m_pD3DEx) 
			{
				AfxGetMyApp()->m_pDirect3DCreate9Ex(D3D9b_SDK_VERSION, (LPVOID**)&m_pD3DEx);
				

			}
		}
		if(!m_pD3DEx) 
		{
			m_pD3D.Attach(Direct3DCreate9(D3D_SDK_VERSION));
			if(!m_pD3D) 
			{
				m_pD3D.Attach(Direct3DCreate9(D3D9b_SDK_VERSION));
			}
			if(!m_pD3D) 
			{
				//_Error += L"Failed to create D3D9\n";
				ret = FALSE;
				break;
			}
		}
		else{
			m_pD3D = m_pD3DEx;
			//SVP_LogMsg5(L"HAVE m_pDirect3DCreate9Ex");
		}

		UINT ulMonitor = GetAdapter(m_pD3D);

		D3DDISPLAYMODE d3ddm;
		HRESULT hr;
		ZeroMemory(&d3ddm, sizeof(d3ddm));
		if(FAILED(m_pD3D->GetAdapterDisplayMode(ulMonitor, &d3ddm)))
		{
			//_Error += L"GetAdapterDisplayMode failed\n";
			ret = FALSE;
			break;
		}


		CSize m_ScreenSize(32,24);


		D3DPRESENT_PARAMETERS pp;
		ZeroMemory(&pp, sizeof(pp));

		BOOL bCompositionEnabled = s.bAeroGlassAvalibility;


		{
			pp.Windowed = TRUE;
			pp.hDeviceWindow = m_hWnd;
			pp.SwapEffect = D3DSWAPEFFECT_COPY;
			pp.Flags = D3DPRESENTFLAG_VIDEO;
			pp.BackBufferCount = 1; 
			pp.BackBufferWidth = m_ScreenSize.cx;
			pp.BackBufferHeight = m_ScreenSize.cy;

			if (bCompositionEnabled)
			{
				// Desktop composition takes care of the VSYNC
				pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

			}
			else
			{
				pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

			}
			if (m_pD3DEx)
			{
				hr = m_pD3DEx->CreateDeviceEx(
					ulMonitor, D3DDEVTYPE_HAL, m_hWnd,
					D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED,
					&pp, NULL, &m_pD3DDevEx);
				if (m_pD3DDevEx)
					m_pD3DDev = m_pD3DDevEx;
			}
			else
			{
				hr = m_pD3D->CreateDevice(
					ulMonitor, D3DDEVTYPE_HAL, m_hWnd,
					D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED,
					&pp, &m_pD3DDev);
			}
		}

		if(FAILED(hr))
		{
			//_Error += L"CreateDevice failed\n";
			ret = FALSE;
			break;
		}
		break;
	}while(0);

//	SVP_LogMsg5(L"TestD3DCreationAbility %d", ret);
	return ret;
}