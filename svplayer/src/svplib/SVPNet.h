#pragma once
#include "svplib.h"
#include <curl\curl.h>
#include "SVPToolBox.h"


class CSVPNet
{
public:
	CSVPNet(void);
	~CSVPNet(void);
	char *mainBuffer ;
	size_t mainBufferSize ;
private:
	CSVPToolBox svpToolBox;	
	int SetCURLopt(CURL *curl );
	char errorBuffer[CURL_ERROR_SIZE];;
public:
	int  QuerySubByVideoPathOrHash(CString szFilePath, CString szFileHash, CString szVHash);
	static size_t handleSubQuery( void *ptr, size_t size, size_t nmemb, void *stream);

};

