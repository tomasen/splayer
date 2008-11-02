#pragma once
#include "svplib.h"

class CSVPhash
{
public:
	CSVPhash(void);
	~CSVPhash(void);
	CString ComputerFileHash(CString szFilePath);
};
