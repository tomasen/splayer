#include "SVPNet.h"
#include "../apps/mplayerc/revision.h"


#include <streams.h>
#include <afxtempl.h>
#include "..\apps\mplayerc\mplayerc.h"

//#define CURLDEBUG_VERBOSE 

CSVPNet::CSVPNet(void)
: fp_curl_verbose(0)
{
	memset(uniqueIDHash,0,UNIQU_HASH_SIZE);

#ifdef CURLDEBUG_VERBOSE
    _wfopen_s(&fp_curl_verbose, svpToolBox.GetPlayerPath_STL(L"SVPDebugNet.log"), L"a");
#endif
}

CSVPNet::~CSVPNet(void)
{
	this->mainBuffer = NULL;
	this->mainBufferSize = 0;

    if(fp_curl_verbose)
        fclose(fp_curl_verbose);
}


struct LANGANDCODEPAGE {
	WORD wLanguage;
	WORD wCodePage;
} *lpTranslateTX;


int CSVPNet::SetCURLopt(CURL *curl)
{
	//struct curl_slist *headerlist=NULL;
	//static const char buf[] = "Expect:";
	AppSettings& s = AfxGetAppSettings();

	char buff[MAX_PATH];
	if(s.szOEMTitle.IsEmpty()){
		sprintf_s( buff, "SPlayer Build %d", SVP_REV_NUMBER );
	}else{
		CSVPToolBox svpToolBox;
		int iDescLen = 0;
		char *oem = svpToolBox.CStringToUTF8(s.szOEMTitle, &iDescLen);
		sprintf_s( buff, "SPlayer Build %d OEM%s", SVP_REV_NUMBER ,oem );
		free(oem);
	}
	char buff_cookie[UNIQU_HASH_SIZE];
	memset(buff_cookie,0,UNIQU_HASH_SIZE);
	{
		
		CString path;
		GetModuleFileName(NULL, path.GetBuffer(MAX_PATH), MAX_PATH);
		path.ReleaseBuffer();
		int Ret = -1;
		path.MakeLower();
		//SVP_LogMsg5(L"got splayer path %s" ,path);
		if( path.Find(_T("splayer")) >= 0 || path.Find(_T("svplayer")) >= 0 || path.Find(_T("mplayerc")) >= 0  ){
			DWORD             dwHandle;
			UINT              dwLen;
			UINT              uLen;
			UINT              cbTranslate;
			LPVOID            lpBuffer;

			dwLen  = GetFileVersionInfoSize(path, &dwHandle);

			TCHAR * lpData = (TCHAR*) malloc(dwLen);
			if(lpData){
				
			memset((char*)lpData, 0 , dwLen);


				/* GetFileVersionInfo() requires a char *, but the api doesn't
				* indicate that it will modify it */
				if(GetFileVersionInfo(path, dwHandle, dwLen, lpData) != 0)
				{
					
						CString szParm( _T("\\StringFileInfo\\000004b0\\FileDescription"));

						if(VerQueryValue(lpData, szParm, &lpBuffer, &uLen) != 0)
						{

							CString szProductName((TCHAR*)lpBuffer);
							//SVP_LogMsg3("szProductName %s", szProductName);
							szProductName.MakeLower();

							if(szProductName.Find(_T("射手")) >= 0 || szProductName.Find(_T("splayer")) >= 0  ){
								Ret = 125;
								
							}
						}

				
				}
			}
		}
		if(Ret == 125){
			//sprintf_s(buff_cookie , UNIQU_HASH_SIZE, "UQID=%s", uniqueIDHash);
			//curl_easy_setopt(curl, CURLOPT_COOKIE , buff_cookie);
		}else{
			//sprintf_s(buff_cookie , UNIQU_HASH_SIZE, "UQID=%s", uniqueIDHash);
			
			//curl_easy_setopt(curl, CURLOPT_COOKIE , buff_cookie);
		}
	}
	

	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, buff);

	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 40);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);
	curl_easy_setopt(curl,CURLOPT_HTTP_VERSION,CURL_HTTP_VERSION_1_0); //must use 1.0 for proxy

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	//curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_SSLv2);
#if 1
    if(iTryID%2 == 0){
	    DWORD ProxyEnable = 0;
	    CString ProxyServer;
	    DWORD ProxyPort = 0;

	    ULONG len = 256+1;
	    CRegKey key;
	    if( ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), KEY_READ)
		    && ERROR_SUCCESS == key.QueryDWORDValue(_T("ProxyEnable"), ProxyEnable) && ProxyEnable
		    && ERROR_SUCCESS == key.QueryStringValue(_T("ProxyServer"), ProxyServer.GetBufferSetLength(256), &len))
	    {
    		

    	
		    CStringA p_str("http://");
		    p_str.Append(CStringA(ProxyServer));
		    curl_easy_setopt(curl, CURLOPT_PROXY,  p_str.GetBuffer());//
		    p_str.ReleaseBuffer();
		    //curl_easy_setopt(curl, CURLOPT_PROXYPORT, 3128);
		    //p_str.ReleaseBuffer();
		    //curl_easy_setopt(curl,CURLOPT_PROXYTYPE,CURLPROXY_HTTP_1_0);
		    SVP_LogMsg6("Using proxy %s", p_str);
    	
		    //ProxyServer.ReleaseBufferSetLength(len);
	    }else{
		    //curl_easy_setopt(curl,CURLOPT_HTTP_VERSION,CURL_HTTP_VERSION_NONE);
	    }
    }
#endif

    if(fp_curl_verbose){
	    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_STDERR, fp_curl_verbose);
    }

    //SVP_LogMsg5(L"iTryID %d", iTryID);
	//curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip"); not native supported. so dont use this option
	// MUST not have this line curl_easy_setopt(curl, CURLOPT_POST, ....);
	
	//headerlist = curl_slist_append(headerlist, buf); //WTF ??
	//curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
	return 0;
}

size_t CSVPNet::handleSubQuery( void *ptr, size_t size, size_t nmemb, void *stream){
	
	
	size_t realsize = size * nmemb;
	/*
	CSVPNet *svpNet = (CSVPNet *)stream;

	if (svpNet->mainBuffer){
		svpNet->mainBuffer = (char *)realloc( svpNet->mainBuffer, svpNet->mainBufferSize + realsize + 1);
	}else{
		svpNet->mainBuffer = (char *)malloc( svpNet->mainBufferSize + realsize + 1);
	}
	if (svpNet->mainBuffer) {
		memcpy(&(svpNet->mainBuffer[svpNet->mainBufferSize]), ptr, realsize);
		svpNet->mainBufferSize += realsize;
		svpNet->mainBuffer[svpNet->mainBufferSize] = 0;
	}*/
	fwrite(ptr, size ,nmemb,(FILE*)stream);
	return realsize;
}
CStringA GetUrlByType(DWORD req_type , int iTryID){
	
    CStringA apiurl("https://www.shooter.cn/");
    if(iTryID > 1 && iTryID <= 11){
        if(iTryID >= 4){
            int iSvrId = 4 + rand()%7;
            if(iTryID%2){
                apiurl.Format("https://splayer%d.shooter.cn/", iSvrId-1);
            }else{
                apiurl.Format("http://splayer%d.shooter.cn/", iSvrId-1);
            }
        }else{
            apiurl.Format("https://splayer%d.shooter.cn/", iTryID-1);
        }
    }else if(iTryID > 11) {
        apiurl = "http://svplayer.shooter.cn/";
    }

	switch(req_type){
		case 'upda':
            apiurl.Append( "api/updater.php?ver=3.6" );
			break;
		case 'upsb':
            apiurl.Append( "api/subup.php" );
            break;
		case 'sapi':
            apiurl.Append( "api/subapi.php" );
            break;
	}
    SVP_LogMsg6("using api %s", apiurl );
    return apiurl;

}
BOOL CSVPNet::CheckUpdaterExe(CString szFileVerHash, CString szPath){
	FILE* stream_updater_exe;
	CString szTmpFilename = this->svpToolBox.getTmpFileName();
	if ( _wfopen_s( &stream_updater_exe, szTmpFilename, _T("wb") ) != 0){
		return 0; //input file open error
	}
	CURL *curl;
	CURLcode res;
	CString szPostPerm = _T( "current=" ) + szFileVerHash;
	int rret = 0;
	curl = curl_easy_init();
	if(curl) {
		long respcode;

		this->SetCURLopt(curl);

		curl_easy_setopt(curl, CURLOPT_URL, GetUrlByType('upda' , iTryID));

		int iDescLen = 0;
		char* szPostFields = svpToolBox.CStringToUTF8(szPostPerm, &iDescLen) ;
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)szPostFields);
		
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)stream_updater_exe);

		res = curl_easy_perform(curl);
		if(res == 0){
			curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);

			if(respcode == 200){
				//good to go
				rret = 1;
				
			}else{
				//error
				SVP_LogMsg(_T("None Update Required For Updater"));
			}
		}else{
			//error
			SVP_LogMsg5(_T("HTTP connection error %d %s ") , res, CStringW(curl_easy_strerror(res))); //TODO handle this
		}
		curl_easy_cleanup(curl);
	}
	fclose(stream_updater_exe);
	if (rret){
		
		if ( this->svpToolBox.unpackGZfile( szTmpFilename , szPath) == 0 ){	
			SVP_LogMsg(_T("Copy Updater.exe Sucesssed"));
		}else{
			SVP_LogMsg(_T("Copy Updater.exe Failed"));
			rret = 0;
		}
	}
	return rret;
}
int CSVPNet::WetherNeedUploadSub(CString fnVideoFilePath, CString szFileHash,CString fnSubHash, int iDelayMS){
	CURL *curl;
	CURLcode res;
	CString szPostPerm ;
	szPostPerm.Format(_T( "pathinfo=%s&filehash=%s&subhash=%s&subdelay=%d" ) , fnVideoFilePath ,szFileHash , fnSubHash, iDelayMS);
	
	int rret = -1;
	curl = curl_easy_init();
	if(curl) {
		long respcode;

		this->SetCURLopt(curl);

		curl_easy_setopt(curl, CURLOPT_URL,  GetUrlByType('upsb', iTryID));

		int iDescLen = 0;
		char* szPostFields = svpToolBox.CStringToUTF8(szPostPerm, &iDescLen) ;
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)szPostFields);

		res = curl_easy_perform(curl);
		if(res == 0){
			curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);

			if(respcode == 200){
				//good to go // continues to upload sub
				rret = 1;
			}if(respcode == 404){
				//error
				rret = 0;
				SVP_LogMsg(_T("Already Have same sub in databases"));
			}
		}else{
			//error
			SVP_LogMsg5(_T("HTTP connection error %d %s ") , res, CStringW(curl_easy_strerror(res))); //TODO handle this
		}
		curl_easy_cleanup(curl);
	}
	return rret;
}
int CSVPNet::UploadCrashDmp(CString szDmppath, CString szLogPath){
	CURL *curl;
	CURLcode res;
	int ret = 0;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;


	curl_global_init(CURL_GLOBAL_ALL);
	char* szTerm2;
	int iDescLen = 0;
	szTerm2 = svpToolBox.CStringToUTF8(szDmppath, &iDescLen);
	curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "dmpfile", CURLFORM_FILE, szTerm2,CURLFORM_END);
	free(szTerm2);

// 	szTerm2 = svpToolBox.CStringToUTF8(szLogPath, &iDescLen);
// 	curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "logfile", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
// 	free(szTerm2);

	curl = curl_easy_init();
	if(curl) {
		long respcode;

		this->SetCURLopt(curl);

		curl_easy_setopt(curl, CURLOPT_URL, "http://svplayer.shooter.cn/api/dmpreport.php");
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

		res = curl_easy_perform(curl);
		if(res == 0){
			curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);

			if(respcode == 200){
				ret = 1;
			}else{
				//error
			}
		}else{
			//error
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
	}

	//fclose(stream_http_recv_buffer);

	return ret;
}
int CSVPNet::UploadPinRenderDeadEndReport(CString szPinName, CString szReport){
	CURL *curl;
	CURLcode res;
	int ret = 0;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;


	curl_global_init(CURL_GLOBAL_ALL);
	char* szTerm2;
	int iDescLen = 0;
	szTerm2 = svpToolBox.CStringToUTF8(szPinName, &iDescLen);
	curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "piname", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
	free(szTerm2);

	szTerm2 = svpToolBox.CStringToUTF8(szReport, &iDescLen);
	curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "report", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
	free(szTerm2);

	curl = curl_easy_init();
	if(curl) {
		long respcode;

		this->SetCURLopt(curl);

		curl_easy_setopt(curl, CURLOPT_URL, "http://svplayer.shooter.cn/api/pinreport.php");
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

		res = curl_easy_perform(curl);
		if(res == 0){
			curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);

			if(respcode == 200){
				ret = 1;
			}else{
				//error
			}
		}else{
			//error
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
	}

	//fclose(stream_http_recv_buffer);

	return ret;
}
#include "MD5Checksum.h"

	// client key is issued by splayer.org to avoid abuse of  service
	// to acquire a client key require a "proper client"
	// "proper client" means a client must have ability to upload subtitle matching data 
	// to contribute to the match rank system correctly
	// comment out following line if don't have a client key
	// without a client key the service is still available but maybe limited in some way to avoid abuse of service
	#include "shooterclient.key"

CString genVHash(char* szTerm2, char* szTerm3, char* uniqueIDHash){
	CString       szVHash;
	char          buffx[4096];
  std::wstring   wbuffx;
	memset(buffx, 0, 4096);
#ifdef CLIENTKEY	
	sprintf_s( buffx, 4096, CLIENTKEY , SVP_REV_NUMBER, szTerm2, szTerm3, uniqueIDHash);
#else
	sprintf_s( buffx, 4096, "un authiority client %d %s %s %s", SVP_REV_NUMBER, szTerm2, szTerm3, uniqueIDHash);
#endif
	CMD5Checksum cmd5;
	szVHash = cmd5.GetMD5((BYTE*)buffx, strlen(buffx)).c_str();
	return szVHash;
}
int CSVPNet::UploadSubFileByVideoAndHash(std::wstring fnVideoFilePath,
                                         std::wstring szFileHash,
                                         std::wstring szSubHash,
                                         std::vector<std::wstring>* fnSubPaths,
                                         int iDelayMS,
                                         std::vector<std::wstring>* szaPostTerms)
{
  CURL *curl;
  CURLcode res;
  //CString szPostPerm = _T( "pathinfo=" ) + fnVideoFilePath + _T("&filehash=") + szFileHash ;
  int iTotalFiles = fnSubPaths -> size();
  SVP_LogMsg(_T("Upload Begin"));
  struct curl_httppost *formpost = NULL;
  struct curl_httppost *lastptr  = NULL;
  char errorbuf[CURL_ERROR_SIZE];

  curl_global_init(CURL_GLOBAL_ALL);

  char* szTerm2;
  char* szTerm3;
  int iDescLen = 0;

  szTerm2 = svpToolBox.CStringToUTF8(fnVideoFilePath.c_str(), &iDescLen);
  curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "pathinfo",
    CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
  free(szTerm2);

  szTerm2 = svpToolBox.CStringToUTF8(szSubHash.c_str(), &iDescLen);
  curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "subhash",
    CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
  free(szTerm2);

  szTerm3 = svpToolBox.CStringToUTF8(szFileHash.c_str(), &iDescLen);
  curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "filehash",
    CURLFORM_COPYCONTENTS, szTerm3,CURLFORM_END);
  free(szTerm3);

  std::wstring szVHash = (LPCTSTR)genVHash(szTerm2, szTerm3, uniqueIDHash);
  if (!szVHash.empty())
  {
    szTerm2 = svpToolBox.CStringToUTF8(szVHash.c_str(), &iDescLen);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "vhash",
      CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
    free(szTerm2);
  }

  szTerm2 = (char*)malloc(64);
  _itoa_s(iDelayMS , szTerm2, 64, 10);
  curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "subdelay",
    CURLFORM_COPYCONTENTS, szTerm2, CURLFORM_END);
  free(szTerm2);


  for (int i = 0; i < fnSubPaths -> size(); i++)
  {
    char szFname[22];
    /* Fill in the file upload field */
    std::wstring szgzFile = svpToolBox.getSameTmpName(fnSubPaths -> at(i).c_str()) ;
    SVP_LogMsg(_T("Gziping ") +  CString(fnSubPaths -> at(i).c_str()) +
      _T(" to ") + szgzFile.c_str());
    svpToolBox.packGZfile(fnSubPaths -> at(i).c_str(), szgzFile.c_str());

    szTerm2 = svpToolBox.CStringToUTF8(szgzFile.c_str(), &iDescLen, CP_ACP);
    //SVP_LogMsg(fnSubPaths->GetAt(i));
    sprintf_s(szFname, 22, "subfile[%d]", i);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, szFname,
      CURLFORM_FILE, szTerm2,CURLFORM_END);
    free(szTerm2);
  }
  int retx = -1;

  curl = curl_easy_init();
  if (curl)
  {
    long respcode;
    SetCURLopt(curl);
    curl_easy_setopt(curl,  CURLOPT_ERRORBUFFER, errorbuf );
    curl_easy_setopt(curl, CURLOPT_URL,  GetUrlByType('upsb',iTryID));

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    res = curl_easy_perform(curl);
    if (res == 0)
    {
      curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);
      if (respcode == 200)
      {
        //good to go // continues to upload sub
        retx = 0;
        SVP_LogMsg(ResStr(IDS_LOG_MSG_SVPSUB_UPLOAD_FINISHED), 31);
      }
      else if (respcode == 404)
      {
        //error
        retx = 0;
        SVP_LogMsg(_T("Already Have same sub in databases"));
      }
    }
    else
      //error
      SVP_LogMsg5(_T("HTTP connection error %d %s ") , res,
        CStringW(curl_easy_strerror(res))); //TODO handle this
    curl_easy_cleanup(curl);
  }
  /* then cleanup the formpost chain */
  curl_formfree(formpost);
  return retx;
}

int CSVPNet::QuerySubByVideoPathOrHash(CString szFilePath,
                              CString szFileHash,
                              CString szVHash,
                              CString szLang)
{

  return QuerySubByVideoPathOrHash_STL((LPCTSTR)szFilePath,
    (LPCTSTR)szFileHash, (LPCTSTR)szVHash, (LPCTSTR)szLang);
}
int CSVPNet::QuerySubByVideoPathOrHash_STL(std::wstring szFilePath,
                                       std::wstring szFileHash,
                                       std::wstring szVHash,
                                       std::wstring szLang)
{
  CURL *curl;
  CURLcode res;
  int ret = 0;
  std::wstring szPostPerm = L"pathinfo=" + szFilePath + L"&filehash="
    + szFileHash + L"&vhash=" + szVHash + L"&lang=" + szLang + L"&shortname="
    + svpToolBox.GetShortFileNameForSearch_STL(szFilePath);

  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;


  curl_global_init(CURL_GLOBAL_ALL);
  char* szTerm2;
  char* szTerm3;
  int iDescLen = 0;
  szTerm2 = svpToolBox.CStringToUTF8(szFilePath.c_str(), &iDescLen);
  curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "pathinfo", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);

  szTerm3 = svpToolBox.CStringToUTF8(szFileHash.c_str(), &iDescLen);
  curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "filehash", CURLFORM_COPYCONTENTS, szTerm3,CURLFORM_END);

  szVHash = genVHash(szTerm2, szTerm3, uniqueIDHash);


  free(szTerm2);
  free(szTerm3);

  if (!szVHash.empty())
  {
    szTerm2 = svpToolBox.CStringToUTF8(szVHash.c_str(), &iDescLen);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "vhash", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
    free(szTerm2);
  }

  AppSettings& s = AfxGetAppSettings();
  std::wstring szSVPSubPerf = (LPCTSTR)s.szSVPSubPerf;
  if (!szSVPSubPerf.empty())
  {
    szTerm2 = svpToolBox.CStringToUTF8(szSVPSubPerf.c_str(), &iDescLen);
    curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "perf", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
    free(szTerm2);
  }

  if (!szLang.empty())
  {
    szTerm2 = svpToolBox.CStringToUTF8(szLang.c_str(), &iDescLen);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "lang", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
    free(szTerm2);
  }

  szTerm2 = svpToolBox.CStringToUTF8(svpToolBox.GetShortFileNameForSearch_STL(szFilePath).c_str(), &iDescLen);
  curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "shortname", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
  free(szTerm2);

  FILE *stream_http_recv_buffer = svpToolBox.getTmpFileSteam();
  if (!stream_http_recv_buffer)
  {
    SVP_LogMsg(_T("TmpFile Creation for http recv buff fail")); //// TODO: 1. warning!! OR switch to memfile system
    return -1;
  }
  int err = 0;


  curl = curl_easy_init();
  if (curl)
  {
    long respcode;
    wchar_t szFailMsg[1024];

    SetCURLopt(curl);

    curl_easy_setopt(curl, CURLOPT_URL, GetUrlByType('sapi', iTryID));
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION , &(this->handleSubQuery));
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)stream_http_recv_buffer);
    //int iDescLen = 0;
    //char* szPostFields = svpToolBox.CStringToUTF8(szPostPerm, &iDescLen) ;
    //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)szPostFields);
    //curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, iDescLen);
    res = curl_easy_perform(curl);
    if (res == 0)
    {
      curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);
      //double contentlength;
      //curl_easy_getinfo(curl,CURLINFO_SIZE_DOWNLOAD, &contentlength);
      //SVP_LogMsg5(L" contentlength %f", contentlength);
      if (respcode == 200)
      {
        //good to go
        //SVP_LogMsg(_T("字幕已经找到，正在处理..."), 31);
        ret = 1;
      }
      else
      {
        //error
        SVP_LogMsg5(_T("HTTP return code is not 200 but %d") , respcode);
        err = 1;
      }
    }
    else
    {
      //error
      LONG l_oserr = 0;
      curl_easy_getinfo(curl,CURLINFO_OS_ERRNO,&l_oserr);
      err = 2;
      swprintf_s(szFailMsg, 1024, L"%s", CStringW(curl_easy_strerror(res)));
      //szFailMsg.Format(L"%s",CStringW(curl_easy_strerror(res)));
      if(!lstrcmp(szFailMsg, L"Couldn't connect to server"));
        lstrcat(szFailMsg, ResStr(IDS_LOG_MSG_SVPSUB_PLEASE_CHECK_FIREWALL));

      SVP_LogMsg5(_T("HTTP connection error  %s %d"), szFailMsg, l_oserr); //TODO handle this
    }

    /* always cleanup */
    curl_easy_cleanup(curl);

    //free(szPostFields);

    //if not error, process data
    if (ret)
    {
      int extErr  = ExtractDataFromAiSubRecvBuffer_STL(szFilePath, stream_http_recv_buffer);
      if (extErr && extErr != -2)
      { // -2 if there is none match subtile
        SVP_LogMsg(_T("Error On Extract DataFromAiSubRecvBuffer ")); //TODO handle this
        err = 3;
      }
    }
    else
    {
      wchar_t szMsg[1024];
      swprintf_s(szMsg, 1024, ResStr(IDS_LOG_MSG_SVPSUB_NETWORK_FAIL), szFailMsg);
      SVP_LogMsg(szMsg);//,31
      m_lastFailedMsg = szMsg;
      err = 4;
    }
    /*
    if (this->mainBufferSize > 0){
    char statCode = this->mainBuffer[0];
    if(statCode <= 0){
    //error handle
    }else{
    //handSubFiles

    }

    }
    if (this->mainBuffer){
    free(this->mainBuffer);
    }*/
  }

  //this->mainBuffer = NULL;
  //this->mainBufferSize = 0;
  fclose(stream_http_recv_buffer);

  return err;
}
int CSVPNet::ExtractDataFromAiSubRecvBuffer(CString szFilePath,
                                            FILE* sAiSubRecvBuff)
{
  return ExtractDataFromAiSubRecvBuffer_STL((LPCTSTR)szFilePath,
    sAiSubRecvBuff);
}
int CSVPNet::ExtractDataFromAiSubRecvBuffer_STL(std::wstring szFilePath,
                                            FILE* sAiSubRecvBuff)
{
  char szSBuff[2] = {0,0};
  int ret = 0;
  fseek(sAiSubRecvBuff, 0, SEEK_SET); // move point yo begining of file

  if (fread(szSBuff, sizeof(char), 1, sAiSubRecvBuff) < 1)
    SVP_LogMsg(_T("Fail to retrive First Stat Code"));

  int iStatCode = szSBuff[0];
  if (iStatCode <= 0)
  {
    if (iStatCode == -1)
    {
      SVP_LogMsg(ResStr(IDS_LOG_MSG_SVPSUB_NONE_MATCH_SUB), 31);
      ret = -2;
    }
    else
    {
      //TODO error handle
      SVP_LogMsg(ResStr(IDS_LOG_MSG_SVPSUB_DOWNLOAD_FAIL));//, 31
      ret = -1;
      //SVP_LogMsg(_T("First Stat Code TODO: 显示有错误发生"));
    }
    goto releaseALL;
  }
  else
    SVP_LogMsg(ResStr(IDS_LOG_MSG_SVPSUB_GOTMATCHED_AND_DOWNLOADING), 31);

  //handle SubFiles
  svpToolBox.szaSubDescs.RemoveAll();
  svpToolBox.szaSubTmpFileList.RemoveAll();

  for(int j = 0; j < iStatCode; j++)
  {
    int exterr = svpToolBox.HandleSubPackage(sAiSubRecvBuff);
    if(exterr)
    {
      ret = exterr;
      break;
    }
  }


releaseALL:
  fclose(sAiSubRecvBuff);

  svpToolBox.ClearTmpFiles();
  return ret;
}
