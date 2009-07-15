#pragma once
#include "svplib.h"
#include "curl/include/curl/curl.h"
#include "SVPToolBox.h"


class CSVPNet
{
public:
	CSVPNet(void);
	~CSVPNet(void);
	char *mainBuffer ;
	size_t mainBufferSize ;
	CSVPToolBox svpToolBox;	
	BOOL CheckUpdaterExe(CString szFileVerHash, CString szPath);
	
private:
	int SetCURLopt(CURL *curl );
	char errorBuffer[CURL_ERROR_SIZE];;
public:
	int WetherNeedUploadSub(CString fnVideoFilePath, CString szFileHash,CString fnSubHash, int iDelayMS);
	int UploadPinRenderDeadEndReport(CString szPinName, CString szReport);
	int UploadCrashDmp(CString szDmppath, CString szLogPath);
	int UploadSubFileByVideoAndHash(CString fnVideoFilePath, CString szFileHash, CString szSubHash,CStringArray* fnSubPaths, int iDelayMS, CStringArray* szaPostTerms);
	int  QuerySubByVideoPathOrHash(CString szFilePath, CString szFileHash, CString szVHash = _T(""));
	static size_t handleSubQuery( void *ptr, size_t size, size_t nmemb, void *stream);
	int  ExtractDataFromAiSubRecvBuffer(CString szFilePath, FILE* sAiSubRecvBuff);
};

