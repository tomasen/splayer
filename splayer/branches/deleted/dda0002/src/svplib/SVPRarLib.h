#pragma once
#include "svplib.h"

class CSVPRarLib
{

public:
	HANDLE m_hRar;

	BOOL m_bIsRAR;
	CString m_fnRAR;
	CString m_fnInsideRar;

	CSVPRarLib(void);
	~CSVPRarLib(void);

	BOOL SplitPath(	CString fnSVPRarPath );

	int ListRar(CString fnRarPath , CStringArray* szaFiles);

};
