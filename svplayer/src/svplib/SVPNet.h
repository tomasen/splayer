#pragma once
#include "svplib.h"
#include "curl\curl.h"
#include "SVPToolBox.h"

class CSVPNet
{
public:
	CSVPNet(void);
	~CSVPNet(void);
private:
	CSVPToolBox svpToolBox;	
	int Post(CString szURL, CString szPostParm, CString szFilePath );
public:
	int QuerySubByVideoPathOrHash(CString szFilePath, CString szFileHash);
};
