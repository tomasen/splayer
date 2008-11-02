#include "SVPNet.h"


CSVPNet::CSVPNet(void)
{
}

CSVPNet::~CSVPNet(void)
{
	this->mainBuffer = NULL;
	this->mainBufferSize = 0;
}



int CSVPNet::SetCURLopt(CURL *curl )
{
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, _T("SVPlayer 0.1") );
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	
	return 0;
}

size_t CSVPNet::handleSubQuery( void *ptr, size_t size, size_t nmemb, void *stream){
	
	
	size_t realsize = size * nmemb;
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
	}
	return realsize;
}
int CSVPNet::QuerySubByVideoPathOrHash(CString szFilePath, CString szFileHash, CString szVHash = _T("") )
{
	CURL *curl;
	CURLcode res;
	CString szPostPerm = _T( "pathinfo=" ) + szFilePath + _T("&filehash=") + szFileHash + _T("&vhash=") + szVHash;
	
	curl = curl_easy_init();
	if(curl) {
		long respcode;
		this->SetCURLopt(curl);
		
		curl_easy_setopt(curl, CURLOPT_URL, "http://www.svplayer.cn/api/subapi.php");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION , &(this->handleSubQuery));
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);
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

		//if not error, process data
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
		}
	}
	
	this->mainBuffer = NULL;
	this->mainBufferSize = 0;

	return 0;
}


