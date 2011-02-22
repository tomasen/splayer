#ifndef SVPHASH_H
#define SVPHASH_H

#include "svplib.h"

class CSVPhash
{
public:
	CSVPhash(void);
	~CSVPhash(void);
	CString ComputerFileHash(CString szFilePath);
  std::wstring ComputerSubFilesFileHash(std::vector<std::wstring>* szaSubFiles);
	CString CSVPhash::HexToString(BYTE* lpszMD5);
};

#endif