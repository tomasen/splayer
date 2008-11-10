#pragma once
#include "svplib.h"

class CSVPhash
{
public:
	CSVPhash(void);
	~CSVPhash(void);
	CString ComputerFileHash(CString szFilePath);
	CString ComputerSubFilesFileHash(CStringArray* szaSubFiles);
	CString CSVPhash::HexToString(BYTE* lpszMD5);
};
