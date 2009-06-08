#pragma once	

#include <curl\curl.h>
#include "../src/svplib/SVPToolBox.h"
#include "../src/svplib/MD5Checksum.h"

 

#define LFILETOTALPARMS 8
#define LFILESETUPPATH 0 //安装至该路径
#define LFILEHASH 1  //解压后文件md5
#define LFILEID 2  //ID
#define LFILETMPATH 3  //临时存储路径
#define LFILEGZHASH 4  //未解压的GZ文件md5
#define LFILEACTION 5  //文件行为
#define LFILELEN 6  //文件Length
#define LFILEGZLEN 7  //文件Gz Length

//static size_t handleWebQuery( void *ptr, size_t size, size_t nmemb, void *stream);

class cupdatenetlib
{
public:
	BOOL bSVPCU_DONE ;
	int iSVPCU_CURRETN_FILE ;
	size_t iSVPCU_TOTAL_FILEBYTE ;
	size_t iSVPCU_CURRENT_FILEBYTE ;
	CString szBasePath;
	CString szUpdfilesPath;
	CSVPToolBox svpToolBox;	
	cupdatenetlib(void);
	~cupdatenetlib(void);
	void resetCounter();
	void procUpdate();
	BOOL downloadList();
	int downloadFiles();
	int downloadFileByID(CString szID, CString szTmpPath);
	void tryRealUpdate();
	BOOL bWaiting;
	int iSVPCU_TOTAL_FILE  ;
	size_t iSVPCU_TOTAL_FILEBYTE_DONE ;
	size_t iSVPCU_CURRENT_FILEBYTE_DONE ;
	CString szCurFilePath;
private:
	CStringArray szaLists; 
	void SetCURLopt(CURL *curl );
	char errorBuffer[CURL_ERROR_SIZE];
};
