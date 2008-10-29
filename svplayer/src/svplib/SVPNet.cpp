#include "SVPNet.h"


CSVPNet::CSVPNet(void)
{
}

CSVPNet::~CSVPNet(void)
{
}



int CSVPNet::Post(CString szURL, CString szPostParm = _T(""), CString szFilePath  = _T(""))
{
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if(curl) {
		long respcode;
		curl_easy_setopt(curl, CURLOPT_URL, szURL);
		res = curl_easy_perform(curl);
		if(res == 0){
			curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);
			if(respcode == 200){
				//good to go
				
			}else{
				//error
				SVP_LogMsg(_T("HTTP return code is not 200"));
			}
		}else{
			//error
			SVP_LogMsg(_T("HTTP connection error")); //TODO handle this
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	return 0;
}

int CSVPNet::QuerySubByVideoPathOrHash(CString szFilePath, CString szFileHash)
{
	return 0;
}
