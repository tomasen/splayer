#include "SVPNet.h"
#include "../apps/mplayerc/revision.h"


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
	//struct curl_slist *headerlist=NULL;
	//static const char buf[] = "Expect:";

	char buff[MAX_PATH];
	sprintf_s( buff, "SVPlayer Build %d", SVP_REV_NUMBER);

	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, buff);
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

		curl_easy_setopt(curl, CURLOPT_URL, "http://svplayer.shooter.cn/api/updater.php");

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
			SVP_LogMsg(_T("HTTP connection error  ")); //TODO handle this
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
	
	int rret = 0;
	curl = curl_easy_init();
	if(curl) {
		long respcode;

		this->SetCURLopt(curl);

		curl_easy_setopt(curl, CURLOPT_URL, "http://svplayer.shooter.cn/api/subup.php");

		int iDescLen = 0;
		char* szPostFields = svpToolBox.CStringToUTF8(szPostPerm, &iDescLen) ;
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)szPostFields);

		res = curl_easy_perform(curl);
		if(res == 0){
			curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);

			if(respcode == 200){
				//good to go // continues to upload sub
				rret = 1;
			}else{
				//error
				SVP_LogMsg(_T("Already Have same sub in databases"));
			}
		}else{
			//error
			SVP_LogMsg(_T("HTTP connection error  ")); //TODO handle this
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
int CSVPNet::UploadSubFileByVideoAndHash(CString fnVideoFilePath, CString szFileHash, CString szSubHash,CStringArray* fnSubPaths, int iDelayMS, CStringArray* szaPostTerms){
	CURL *curl;
	CURLcode res;
	//CString szPostPerm = _T( "pathinfo=" ) + fnVideoFilePath + _T("&filehash=") + szFileHash ;
	int iTotalFiles = fnSubPaths->GetCount();
	SVP_LogMsg(_T("Upload Begin"));
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	char errorbuf[CURL_ERROR_SIZE];

	curl_global_init(CURL_GLOBAL_ALL);
	
	char* szTerm2;
	int iDescLen = 0;

	szTerm2 = svpToolBox.CStringToUTF8(fnVideoFilePath, &iDescLen);
	curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "pathinfo", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
	free(szTerm2);

	szTerm2 = svpToolBox.CStringToUTF8(szSubHash, &iDescLen);
	curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "subhash", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
	free(szTerm2);

	szTerm2 = svpToolBox.CStringToUTF8(szFileHash, &iDescLen);
	curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "filehash", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
	free(szTerm2);

	szTerm2 = (char*)malloc(64);
	_itoa_s( iDelayMS , szTerm2, 64, 10);
	curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "subdelay", CURLFORM_COPYCONTENTS,szTerm2 ,CURLFORM_END);
	free(szTerm2);

	for(int i = 0; i < fnSubPaths->GetCount(); i++){
		char szFname[22];
		/* Fill in the file upload field */
		CString szgzFile = svpToolBox.getSameTmpName(fnSubPaths->GetAt(i)) ;
		SVP_LogMsg( CString(_T("Gziping ")) +  fnSubPaths->GetAt(i) + _T(" to ") + szgzFile );
		svpToolBox.packGZfile( fnSubPaths->GetAt(i) , szgzFile);
		
		szTerm2 = svpToolBox.CStringToUTF8(szgzFile, &iDescLen, CP_ACP);
		//SVP_LogMsg(fnSubPaths->GetAt(i));
		sprintf_s(szFname, 22, "subfile[%d]", i);
		curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, szFname, CURLFORM_FILE, szTerm2,CURLFORM_END);
		free(szTerm2);
		
	}
	

	curl = curl_easy_init();
	if(curl) {
		long respcode;

		this->SetCURLopt(curl);
		curl_easy_setopt(curl,  CURLOPT_ERRORBUFFER, errorbuf );
		curl_easy_setopt(curl, CURLOPT_URL, "http://svplayer.shooter.cn/api/subup.php");

		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		
		res = curl_easy_perform(curl);
		if(res == 0){
			curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);

			if(respcode == 200){
				//good to go // continues to upload sub
				SVP_LogMsg(_T("字幕上传完毕"), 31);
			}else{
				//error
				SVP_LogMsg(_T("Already Have same sub in databases"));
			}
		}else{
			//error
			SVP_LogMsg(_T("HTTP connection error ") + this->svpToolBox.UTF8ToCString( errorbuf , strlen(errorbuf))); //TODO handle this
		}
		curl_easy_cleanup(curl);

	}
	/* then cleanup the formpost chain */
	curl_formfree(formpost);

	return 0;
}
int CSVPNet::QuerySubByVideoPathOrHash(CString szFilePath, CString szFileHash, CString szVHash  )
{
	CURL *curl;
	CURLcode res;
	int ret = 0;
	CString szPostPerm = _T( "pathinfo=" ) + szFilePath + _T("&filehash=") + szFileHash + _T("&vhash=") + szVHash;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;


	curl_global_init(CURL_GLOBAL_ALL);
	char* szTerm2;
	int iDescLen = 0;
	szTerm2 = svpToolBox.CStringToUTF8(szFilePath, &iDescLen);
	curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "pathinfo", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
	free(szTerm2);

	szTerm2 = svpToolBox.CStringToUTF8(szFileHash, &iDescLen);
	curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "filehash", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
	free(szTerm2);

	if(!szVHash.IsEmpty()){
		szTerm2 = svpToolBox.CStringToUTF8(szVHash, &iDescLen);
		curl_formadd(&formpost,	&lastptr, CURLFORM_COPYNAME, "vhash", CURLFORM_COPYCONTENTS, szTerm2,CURLFORM_END);
		free(szTerm2);
	}

	FILE *stream_http_recv_buffer = svpToolBox.getTmpFileSteam();
	if(!stream_http_recv_buffer){
		SVP_LogMsg(_T("TmpFile Creation for http recv buff fail")); //// TODO: 1. warning!! OR switch to memfile system
		return -1;
	}


	curl = curl_easy_init();
	if(curl) {
		long respcode;
		
		this->SetCURLopt(curl);

		curl_easy_setopt(curl, CURLOPT_URL, "http://svplayer.shooter.cn/api/subapi.php");
		//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION , &(this->handleSubQuery));
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)stream_http_recv_buffer);
		//int iDescLen = 0;
		//char* szPostFields = svpToolBox.CStringToUTF8(szPostPerm, &iDescLen) ;
		//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)szPostFields);
		//curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, iDescLen);
		res = curl_easy_perform(curl);
		if(res == 0){
			curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);

			if(respcode == 200){
				//good to go
				SVP_LogMsg(_T("HTTP return code 200"));
				ret = 1;
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

		//free(szPostFields);

		//if not error, process data
		if (ret){
			if ( this->ExtractDataFromAiSubRecvBuffer(szFilePath, stream_http_recv_buffer) ){
				SVP_LogMsg(_T("Error On Extract DataFromAiSubRecvBuffer ")); //TODO handle this
			}
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
		if (iStatCode == -1){
			SVP_LogMsg(_T("没有找到字幕"), 31);
		}else{
			//TODO error handle
			SVP_LogMsg(_T("First Stat Code TODO: 显示有错误发生"));
		}
		ret = -1;
		goto releaseALL;
	}
	
	//handle SubFiles
	svpToolBox.szaSubDescs.RemoveAll();
	svpToolBox.szaSubTmpFileList.RemoveAll();

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
