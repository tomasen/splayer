#pragma once
#include "svplib.h"

class CSVPToolBox
{
public:
	CSVPToolBox(void);
	~CSVPToolBox(void);
	int CStringToUTF8(CString szIn, char* szOut);
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
	CString getVideoFileBasename(CString szVidPath);
	int Explode(CString szIn, CString szTok, CStringArray* szaOut);
	BOOL ifFileExist(CString szPathname);
private:
	CStringArray szaTmpFileNames;
};
