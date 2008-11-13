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
	//curl_easy_setopt(curl, CURLOPT_POST, 1);
	
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
int CSVPNet::UploadSubFileByVideoAndHash(CString fnVideoFilePath, CString szFileHash,CString fnSubPath){
	CURL *curl;
	CURLcode res;
	CString szPostPerm = _T( "pathinfo=" ) + fnVideoFilePath + _T("&filehash=") + szFileHash ;
	curl = curl_easy_init();
	if(curl) {
		long respcode;

		this->SetCURLopt(curl);

		curl_easy_setopt(curl, CURLOPT_URL, "http://www.svplayer.cn/api/subup.php");

		int iDescLen = 0;
		char* szPostFields = svpToolBox.CStringToUTF8(szPostPerm, &iDescLen) ;
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)szPostFields);

		res = curl_easy_perform(curl);
		if(res == 0){
			curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);

			if(respcode == 200){
				//good to go // continues to upload sub
				
			}else{
				//error
				SVP_LogMsg(_T("Already Have same sub in databases"));
			}
		}else{
			//error
			SVP_LogMsg(_T("HTTP connection error  ")); //TODO handle this
		}
	}
	return 0;
}
int CSVPNet::QuerySubByVideoPathOrHash(CString szFilePath, CString szFileHash, CString szVHash  )
{
	CURL *curl;
	CURLcode res;
	CString szPostPerm = _T( "pathinfo=" ) + szFilePath + _T("&filehash=") + szFileHash + _T("&vhash=") + szVHash;
	
	FILE *stream_http_recv_buffer = svpToolBox.getTmpFileSteam();
	if(!stream_http_recv_buffer){
		SVP_LogMsg(_T("TmpFile Creation for http recv buff fail")); //// TODO: 1. warning!! OR switch to memfile system
		return -1;
	}


	curl = curl_easy_init();
	if(curl) {
		long respcode;
		
		this->SetCURLopt(curl);

		curl_easy_setopt(curl, CURLOPT_URL, "http://www.svplayer.cn/api/subapi.php");
		//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION , &(this->handleSubQuery));
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)stream_http_recv_buffer);
		int iDescLen = 0;
		char* szPostFields = svpToolBox.CStringToUTF8(szPostPerm, &iDescLen) ;
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)szPostFields);
		//curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, iDescLen);
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
		free(szPostFields);

		//if not error, process data
		
		if ( this->ExtractDataFromAiSubRecvBuffer(szFilePath, stream_http_recv_buffer) ){
			SVP_LogMsg(_T("Error On Extract DataFromAiSubRecvBuffer ")); //TODO handle this
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

	return 0;
}

int  CSVPNet::ExtractDataFromAiSubRecvBuffer(CString szFilePath, FILE* sAiSubRecvBuff){

	char szSBuff[2];
	int ret = 0;
	fseek(sAiSubRecvBuff,0,SEEK_SET); // move point yo begining of file
	
	if ( fread(szSBuff , sizeof(char), 1, sAiSubRecvBuff) < 1){
		SVP_LogMsg(_T("Fail to retrive First Stat Code"));
	}
	
	int iStatCode = szSBuff[0];
	if(iStatCode <= 0){
		//TODO error handle
		SVP_LogMsg(_T("First Stat Code 显示有错误发生"));
		ret = -1;
		goto releaseALL;
	}
	
	//handle SubFiles

	for(int j = 0; j < iStatCode; j++){
		int exterr = svpToolBox.HandleSubPackage(sAiSubRecvBuff);
		if(exterr){
			ret = exterr;
			break;
		}
	}
	
	
releaseALL:
	fclose(sAiSubRecvBuff);

	svpToolBox.ClearTmpFiles();
	return ret;
}
