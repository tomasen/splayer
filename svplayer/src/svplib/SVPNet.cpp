#include "SVPNet.h"


CSVPNet::CSVPNet(void)
{
}

CSVPNet::~CSVPNet(void)
{
}



int CSVPNet::SetCURLopt(CURL *curl )
{
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, _T("SVPlayer 0.1") );
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	
	return 0;
}

size_t CSVPNet::handleSubQuery( void *ptr, size_t size, size_t nmemb, void *stream){
	
	CString szBuf;
	szBuf.Format(_T("%lu %lu") , size , nmemb);
	SVP_LogMsg(szBuf);

	FILE* f = _wfopen(SVP_DEBUG_LOGFILEPATH, _T("a"));
	if(f){
		fwrite(ptr, size, nmemb, f);
		fclose(f);
	}
	return size * nmemb;
}
int CSVPNet::QuerySubByVideoPathOrHash(CString szFilePath, CString szFileHash)
{
	CURL *curl;
	CURLcode res;
	
	curl = curl_easy_init();
	if(curl) {
		long respcode;
		this->SetCURLopt(curl);
		
		curl_easy_setopt(curl, CURLOPT_URL, "http://shooter.cn/tmp/phpinfo.php");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION , (&this->handleSubQuery));
		
		res = curl_easy_perform(curl);
		if(res == 0){
			curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);

			if(respcode == 200){
				//good to go
				SVP_LogMsg(_T("HTTP return code 200"));
			}else{
				//error
				SVP_LogMsg(_T("HTTP return code is not 200"));
			}
		}else{
			//error
			
			SVP_LogMsg(_T("HTTP connection error  ")); //TODO handle this
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	
	return 0;
}


