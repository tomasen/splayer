#pragma once
#include "svplib.h"

class CSVPToolBox
{
public:
	CSVPToolBox(void);
	~CSVPToolBox(void);
	int CStringToUTF8(CString szIn, char* szOut);
	int UTF8ToCString(char* szIn, WCHAR * szOut);
	FILE* getTmpFileSteam();
	int ClearTmpFiles();
	int Char4ToInt(char* szBuf);
private:
	CStringArray szaTmpFileNames;
};
