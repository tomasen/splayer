#ifndef SVPTOOLBOX_H
#define SVPTOOLBOX_H

#include "svplib.h"
#include <atlpath.h>
#include <vector>
#include <string>

class CSVPToolBox
{
public:
	CSVPToolBox(void);
	~CSVPToolBox(void);
	CString getSameTmpName(CString fnin );
	CString getSameTmpExt(CString fnin );
	int packGZfile(CString fnin , CString fnout );
	int unpackGZfile(CString fnin , CString fnout );
	CString extractRarFile(CString rarfn);
	CString PackageSubFiles(CStringArray* szaSubFiles);
  int FindAllSubfile(std::wstring szSubPath, std::vector<std::wstring>* szaSubFiles);
  CString AnsiToCString(UINT codepag, char* szIn, int iLength);
	char* CStringToUTF8(CString szIn, int* iDescLen, UINT codePage = CP_UTF8);
	CString UTF8ToCString(char* szIn, int iLength);
	FILE* getTmpFileSteam();
	WCHAR* getTmpFileName();
	int ClearTmpFiles();
	int Char4ToInt(char* szBuf);
    //************************************
    // Method:    CleanUpOldFiles
    // FullName:  CSVPToolBox::CleanUpOldFiles
    // Access:    public 
    // Returns:   int
    // Qualifier:
    // byNumber not supported yet
    //************************************
    int CleanUpOldFiles(CString szDir, int parm, int ilimit, int byNumber = 0);
	int HandleSubPackage(FILE* fp);
	CStringArray szaSubDescs; //save raw desc data order by id
  CStringArray szaSubTmpFileList; //save tmp file names order by id, format: ".srt;C:\tmp\blahblah;.idx;C:\tmp\blah2;.sub;C:\tmp\blah3"...
  CString getSubFileByTempid(int iTmpID, CString szVidPath);
  std::wstring getSubFileByTempid_STL(int iTmpID, std::wstring szVidPath);
  bool GetAppDataPath(CString& path);
	bool GetAppDataPath(std::wstring& path);
	int ExtractSubFiles(FILE* fp);
	int ExtractEachSubFile(FILE* fp, int iSubPosId);
	char* ReadToPTCharByLength(FILE* fp, size_t length);
  std::wstring getVideoFileBasename(std::wstring szVidPath,
                                    std::vector<std::wstring>* szaPathInfo);
	BOOL FindSystemFile(CString szFn);
  CString GetShortFileNameForSearch(CString szFnPath);
  CString GetShortFileNameForSearch2(CString szFn);
  std::wstring GetShortFileNameForSearch_STL(std::wstring szFnPath);
  std::wstring GetShortFileNameForSearch2_STL(std::wstring szFn);
  int Explode(CString szIn, CString szTok,
    CStringArray* szaOut);
  int Explode(std::wstring szIn, std::wstring szTok,
              std::vector<std::wstring>* szaOut);
  std::wstring Implode(std::wstring szTok, std::vector<std::wstring>* szaOut);
  BOOL ifFileExist(CString szPathname, BOOL evenSlowDriver = true);
  BOOL ifFileExist_STL(std::wstring szPathname, BOOL evenSlowDriver = true);
  BOOL ifDirWritable(CString szDir);
  BOOL ifDirWritable_STL(std::wstring szDir);
	BOOL CanUseCUDAforCoreAVC();
	int GetGPUString(CStringArray * szaGPUString);
	int GetWMIGPURam();
	CString GetTempDir();
	int DetectFileCharset(CString fn);
  CString DetectSubFileLanguage(CString fn);
  std::wstring DetectSubFileLanguage_STL(std::wstring fn);
  CString GetPlayerPath(CString progName = _T(""));
  std::wstring GetPlayerPath_STL(std::wstring progName = L"");
	BOOL CreatDirForFile(CString cPath);
	BOOL CreatDirRecursive(CString cPath);
	BOOL isWriteAble(CString szPath);
	CString getFileVersionHash(CString szPath);
	DWORD             dwMajor   ;
	DWORD             dwMinor   ;
	DWORD             dwRelease ;
	DWORD             dwBuild   ;

	DWORD _httoi(const TCHAR *value);
	

  void filePutContent_STL(std::wstring szFilePath, std::wstring szData, BOOL bAppend = 0);
  void filePutContent(CString szFilePath, CString szData, BOOL bAppend = 0);
	CString fileGetContent(CString szFilePath);
	CString GetDirFromPath(CString path);
  BOOL ifDirExist(CString path);
	BOOL ifDirExist_STL(std::wstring path);
	BOOL bFontExist(CString szFontName, BOOL chkExtFontFile = 1);
	BOOL delDirRecursive(CString path);
	void MergeAltList( CAtlList<CString>& szaRet,  CAtlList<CString>& szaIn  );
	void findMoreFileByFile(CString szFile,CAtlList<CString>& szaRet, CAtlArray<CString>& szaExt);
	void findMoreFileByDir(CString szDir, CAtlList<CString>&  szaRet, CAtlArray<CString>& szaExt, BOOL bSubDir = false);
	BOOL GetDirectoryLeft(CPath* tPath, int rCount );
	static BOOL isAlaphbet(WCHAR wchr);

	HWND m_hWnd;
	UINT GetAdapter(LPVOID lpD3D);
	bool TestD3DCreationAbility(HWND hWnd);

private:
	CStringArray szaTmpFileNames;
};

bool ReadFileToBuffer(CString path, BYTE*& contents, DWORD* pdwsize) ;
bool WriteBufferToFile(CString path, const BYTE* contents, DWORD dwsize);
bool IsFileGziped(CString fnin);

#endif