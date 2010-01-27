#pragma once	

#include "curl/include/curl/curl.h"
#include "SVPToolBox.h"
#include "MD5Checksum.h"



#define LFILETOTALPARMS 8
#define LFILESETUPPATH 0 //安装至该路径
#define LFILEHASH 1  //解压后文件md5
#define LFILEID 2  //ID
#define LFILETMPATH 3  //临时存储路径
#define LFILEGZHASH 4  //未解压的GZ文件md5
#define LFILEACTION 5  //文件行为
#define LFILELEN 6  //文件Length
#define LFILEGZLEN 7  //文件Gz Length

extern char* szUrl;

//static size_t handleWebQuery( void *ptr, size_t size, size_t nmemb, void *stream);

typedef struct _UpdateInfo 
{
    CString strPath;
    CString strFileMd5;
    CString strId;
    CString strTempName; //MD5 of strpath
    CString strDownloadfileMD5;
    CString strAction;
    DWORD dwFileLength;
    DWORD dwDowloadedLength;
    BOOL bDownload;
    CString strCurrentMD5;
    BOOL bReadyToCopy;
} UpdateInfo;

class cupdatenetlib
{
public:
    cupdatenetlib(void);
    ~cupdatenetlib(void);
public:
    void resetCounter();
    void procUpdate();
    HINSTANCE m_hD3DX9Dll;
    BOOL downloadList();
    HINSTANCE GetD3X9Dll();
    int downloadFiles();
    int downloadFileByID(UpdateInfo* p, bool bUsingMd5 = false);
    //int downloadFileByIDAndMD5(UpdateInfo* p, CString szID, CString strOrgName, CString strMD5, CString szTmpPath);
    void tryRealUpdate(BOOL bNoWaiting = FALSE);
    double getProgressBytes();
public:
    //for test only
    int GetReadyToCopyCount();
private:
    void SetCURLopt(CURL *curl );
    bool SkipThisFile(CString strName, CString strAction);
    bool PostUsingCurl(CString strFields, CString strReturnFile, curl_progress_callback pCallback = NULL);
    bool IsMd5Match(CString strFileName, CString strMd5);
public:
    BOOL bSVPCU_DONE ;
    int iSVPCU_CURRETN_FILE ;
    size_t iSVPCU_TOTAL_FILEBYTE ;
    size_t iSVPCU_CURRENT_FILEBYTE ;
    size_t iSVPCU_CURRENT_GZFILEBYTE ;
    CString szBasePath;
    CString szUpdfilesPath;
    CSVPToolBox svpToolBox;	
    BOOL bWaiting;
    int iSVPCU_TOTAL_FILE  ;
    size_t iSVPCU_TOTAL_FILEBYTE_DONE ;
    size_t iSVPCU_CURRENT_FILEBYTE_DONE ;
    CString szCurFilePath;
    //CStringArray szaLists; 
protected:
    char errorBuffer[CURL_ERROR_SIZE];
    CPtrArray m_UpdateFileArray;
};

