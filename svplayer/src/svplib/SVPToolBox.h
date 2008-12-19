#pragma once
#include "svplib.h"

class CSVPToolBox
{
public:
	CSVPToolBox(void);
	~CSVPToolBox(void);
	CString PackageSubFiles(CStringArray* szaSubFiles);
	int FindAllSubfile(CString szSubPath , CStringArray* szaSubFiles);
	char* CStringToUTF8(CString szIn, int* iDescLen, UINT codePage = CP_UTF8);
	CString UTF8ToCString(char* szIn, int iLength);
	FILE* getTmpFileSteam();
	WCHAR* getTmpFileName();
	int ClearTmpFiles();
	int Char4ToInt(char* szBuf);
	int HandleSubPackage(FILE* fp);
	CStringArray szaSubDescs; //save raw desc data order by id
	CStringArray szaSubTmpFileList; //save tmp file names order by id, format: ".srt;C:\tmp\blahblah;.idx;C:\tmp\blah2;.sub;C:\tmp\blah3"...
	CString getSubFileByTempid(int iTmpID, CString szVidPath);
	int ExtractSubFiles(FILE* fp);
	int ExtractEachSubFile(FILE* fp, int iSubPosId);
	char* ReadToPTCharByLength(FILE* fp, size_t length);
	CString getVideoFileBasename(CString szVidPath, CStringArray* szaPathInfo);
	int Explode(CString szIn, CString szTok, CStringArray* szaOut);
	BOOL ifFileExist(CString szPathname);
	BOOL ifDirWritable(CString szDir);
	CString GetTempDir();
	int DetectFileCharset(CString fn);
private:
	CStringArray szaTmpFileNames;
};
