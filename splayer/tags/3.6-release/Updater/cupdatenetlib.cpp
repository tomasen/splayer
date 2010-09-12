//#include "StdAfx.h"
#include <afx.h>
#include "targetver.h"
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#include "cupdatenetlib.h"
#include "conrgette_interface.h"
#include <atlpath.h>

#include "../src/apps/mplayerc/revision.h"

//static size_t handleWebQuery( void *ptr, size_t size, size_t nmemb, void *stream)
//{
//	size_t realsize = size * nmemb;
//
//	
//	fwrite(ptr, size ,nmemb,(FILE*)stream);
//	return realsize;
//}

cupdatenetlib::cupdatenetlib(void)
: m_hD3DX9Dll(NULL)
{
	resetCounter();
	CString path;
	GetModuleFileName(NULL, path.GetBuffer(MAX_PATH), MAX_PATH);
	path.ReleaseBuffer();
	
	CPath cpath(path);
	cpath.RemoveFileSpec();
	cpath.AddBackslash();
	
	szBasePath = CString(cpath);

	cpath.Append(_T("UPD"));
	cpath.AddBackslash();

	szUpdfilesPath = CString(cpath);

	_wmkdir(szUpdfilesPath);

}

cupdatenetlib::~cupdatenetlib(void)
{
}


void cupdatenetlib::SetCURLopt(CURL *curl )
{
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "SPlayer Updater 36");
	return ;
}

void  cupdatenetlib::resetCounter(){
	bSVPCU_DONE = 0;
	iSVPCU_TOTAL_FILE = 0;
	iSVPCU_CURRETN_FILE = 0;
	iSVPCU_TOTAL_FILEBYTE = 0;
	iSVPCU_TOTAL_FILEBYTE_DONE = 0;
	iSVPCU_CURRENT_FILEBYTE = 0;
	iSVPCU_CURRENT_FILEBYTE_DONE = 0;
	bWaiting = FALSE;
}

void cupdatenetlib::procUpdate(){
	
	if (_wmkdir(szUpdfilesPath) < 0 && errno == EEXIST){
        //if(szaLists.GetCount() <= 0){
        //    downloadList() ;
        //}
        if(m_UpdateFileArray.GetCount() <= 0){
            downloadList() ;
        }
		if ( m_UpdateFileArray.GetCount() >0 ){
//			SVP_LogMsg( _T("GOT UPDATE LIST: "));
//			SVP_LogMsg( svpToolBox.Implode(_T("\t") , &szaLists) );
            SVP_LogMsg( _T("DOWNLOAD UPDATE "));
            downloadFiles();
			//int i = 0;
			//while(  downloadFiles() != 0 ){
			//	i++;
			//	SVP_LogMsg( _T("DOWNLOAD UPDATE "));
			//	if(i > 3) break;
			//}
			SVP_LogMsg( _T("REAL UPDATE") );
			tryRealUpdate();
		}
	}else{
		SVP_LogMsg( _T("UPD dir not exist and write able: "));
	}
	bSVPCU_DONE = 1;
}

bool cupdatenetlib::PostUsingCurl(CString strFields, CString strReturnFile, curl_progress_callback pCallback)
{
    FILE* stream_file_list;
    if ( _wfopen_s( &stream_file_list, strReturnFile, _T("wb") ) != 0){
        return false; //input file open error
    }
    CURL *curl;
    CURLcode res;
    CString szPostPerm = strFields;
    bool rret = false;
    SVP_LogMsg(strFields);
    curl = curl_easy_init();
    if(curl) {
        long respcode;
        this->SetCURLopt(curl);
        curl_easy_setopt(curl, CURLOPT_URL, szUrl);
        //curl_easy_setopt(curl, CURLOPT_URL, "http://svplayer.shooter.cn/api/updater.php");
        int iDescLen = 0;
        char* szPostFields = svpToolBox.CStringToUTF8(szPostPerm, &iDescLen) ;
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)szPostFields);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)stream_file_list);

        if (pCallback)
        {
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
            curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, pCallback);
            curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
        }
        res = curl_easy_perform(curl);
        if (szPostFields)
            delete szPostFields;
        if(res == 0){
            curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);
            if(respcode == 200){
                //good to go
                rret = true;

            }else{
                //error
                SVP_LogMsg(_T("None Update Required "));
            }
        }else{
            //error
            SVP_LogMsg5(_T("HTTP connection error %d "), res); //TODO handle this
        }
        curl_easy_cleanup(curl);
    }
    fclose(stream_file_list);
    return rret;
}


HINSTANCE cupdatenetlib::GetD3X9Dll()
{
	
	if (m_hD3DX9Dll == NULL)
	{
		int maxSDK = 46;
		CString m_strD3DX9Version;
		// Try to load latest DX9 available
		for (int i=  maxSDK; i>23; i--)
		{
			if (i != 33)	// Prevent using DXSDK April 2007 (crash sometimes during shader compilation)
			{
				m_strD3DX9Version.Format(_T("d3dx9_%d.dll"), i);
				m_hD3DX9Dll = LoadLibrary (m_strD3DX9Version);
				if (m_hD3DX9Dll) 
				{
					
					break;
				}
			}
		}
	}

	return m_hD3DX9Dll;
}


bool cupdatenetlib::SkipThisFile(CString strName, CString strAction)
{
    bool bRet = false;
    if(strName == _T("msyh.ttf") ){
        if(svpToolBox.bFontExist(_T("微软雅黑")) || svpToolBox.bFontExist(_T("Microsoft YaHei")) ){ 
            bRet = true;
        }
    }
    else if(strName == _T("wmvcore.dll") ){
        if(svpToolBox.FindSystemFile( _T("wmvcore.dll") ))
            bRet = true;
    }
    else if(strName == _T("wmasf.dll") ){
        if(svpToolBox.FindSystemFile( _T("wmasf.dll") ))
            bRet = true;
    }
    else if(strName == _T("wmadmod.dll") ){
        if(svpToolBox.FindSystemFile( _T("wmadmod.dll") ))
            bRet = true;
    }else if(strName.Find(_T("d3dx9_")) >= 0){
        if(this->GetD3X9Dll())
            bRet = true;
    }
    else  if ( strName.Find( L"csfcodec") == 0 ){
        if(!svpToolBox.ifDirExist(szBasePath + L"csfcodec")){
            bRet = true;
        }
    }
    else if ( strAction ==  L"codec" ){
        if(!svpToolBox.ifFileExist(szBasePath + strName)){
            bRet = true;
        }
    }
    return bRet;
}

BOOL cupdatenetlib::downloadList(){
	if ( !(_wmkdir(szUpdfilesPath) < 0 && errno == EEXIST) ){
		SVP_LogMsg( _T("UPD dir not exist and writeable! "));
		return 0;
	}
	resetCounter();

	CString szBranch = svpToolBox.fileGetContent(szUpdfilesPath + _T("branch") );

	if(szBranch.IsEmpty() ){
		/* 如果 mtime 小于 stable 版本的 mtime 就更新 stable ， 大于就更新 beta */
//		struct __stat64  sbuf;
		CString szPlayerPath = svpToolBox.GetPlayerPath(_T("splayer.exe"));
		if(!svpToolBox.ifFileExist(szPlayerPath) ){
			szPlayerPath = svpToolBox.GetPlayerPath(_T("mplayerc.exe"));
			if (!svpToolBox.ifFileExist(szPlayerPath)){
				szPlayerPath = svpToolBox.GetPlayerPath(_T("svplayer.exe"));
			}
		}
		if(svpToolBox.ifFileExist(szPlayerPath) ){
			CMD5Checksum cmd5;
			//szBranch.Format( _T("%I64d") , sbuf.st_mtime );
			szBranch = cmd5.GetMD5((LPCTSTR)szPlayerPath).c_str();
			//AfxMessageBox(szBranch);
		}
		else
			szBranch = _T("stable");
	}

	WCHAR* wsz = this->svpToolBox.getTmpFileName();
    CString szTmpFilename(wsz);
	delete wsz;

    CString szPostPerm;
    szPostPerm.Format(_T("ver=%s&branch=%s"), BRANCHVER, szBranch);

	int rret = 0;
	CString szLog;
	if (PostUsingCurl(szPostPerm, szTmpFilename)){
        rret = 1;
		GetD3X9Dll();
		//iSVPCU_TOTAL_FILE = 0;
		iSVPCU_TOTAL_FILEBYTE  = 0;
		CString szData = svpToolBox.fileGetContent( szTmpFilename ) ;
		CStringArray szaLines;
		svpToolBox.Explode( szData, _T("\n") , &szaLines );
		for(int i = 0; i < szaLines.GetCount(); i++){
			if (szaLines.GetAt(i).IsEmpty()){break;}
			this->iSVPCU_TOTAL_FILE++;
			
			//szLog.Format(_T("Total Files need to download: %d"), iSVPCU_TOTAL_FILE);
			//SVP_LogMsg(szLog);
			CStringArray szaTmp;
			svpToolBox.Explode( szaLines.GetAt(i), _T(";") , &szaTmp );
			if(szaTmp.GetCount() < LFILETOTALPARMS){
				continue;
			}

            if (SkipThisFile(szaTmp.GetAt(LFILESETUPPATH), szaTmp.GetAt(LFILEACTION)))
                continue;

            //检查是否需要下载
			CString szSetupPath = szaTmp.GetAt(LFILESETUPPATH);

			if (szSetupPath.CompareNoCase( _T("splayer.exe")) == 0){
				if(!svpToolBox.ifFileExist(szBasePath + szSetupPath) ){
					if (svpToolBox.ifFileExist(szBasePath + _T("mplayerc.exe")))
						szSetupPath = _T("mplayerc.exe");
					if (svpToolBox.ifFileExist(szBasePath + _T("svplayer.exe")))
						szSetupPath = _T("svplayer.exe");
				}
			}

			bool bDownloadThis = FALSE;

      //check file hash
      CMD5Checksum cmd5;
      CString updTmpHash ;
      CString currentHash ;
      if( svpToolBox.ifFileExist(szUpdfilesPath + szaTmp.GetAt(LFILETMPATH)))
      {
        updTmpHash = cmd5.GetMD5((LPCTSTR)(szUpdfilesPath +
          szaTmp.GetAt(LFILETMPATH))).c_str(); //Get Hash for current Temp File
      }

      if( svpToolBox.ifFileExist(szBasePath + szSetupPath ) ){
        currentHash = cmd5.GetMD5((LPCTSTR)(szBasePath + szSetupPath)).c_str(); //Get Hash for bin file
      }

            if (currentHash.CompareNoCase( szaTmp.GetAt(LFILEHASH) ) == 0 )
                continue;
            if ( updTmpHash.CompareNoCase( szaTmp.GetAt(LFILEHASH) ) != 0 ){
                bDownloadThis = TRUE;
            }

            UpdateInfo* puinfo = new UpdateInfo;
            puinfo->bDownload = bDownloadThis;
            puinfo->dwDowloadedLength = _wtoi(szaTmp.GetAt(LFILEGZLEN));
            puinfo->dwFileLength = _wtoi(szaTmp.GetAt(LFILELEN));
            puinfo->strId = (szaTmp.GetAt(LFILEID));
            puinfo->strAction = szaTmp.GetAt(LFILEACTION);
            puinfo->strDownloadfileMD5 = szaTmp.GetAt(LFILEGZHASH);
            puinfo->strFileMd5 = szaTmp.GetAt(LFILEHASH);
            puinfo->strPath = szaTmp.GetAt(LFILESETUPPATH);
            puinfo->strTempName = szaTmp.GetAt(LFILETMPATH);
            puinfo->strCurrentMD5 = currentHash;
            puinfo->bReadyToCopy = !bDownloadThis;

            m_UpdateFileArray.Add(puinfo);

			if(bDownloadThis){
				iSVPCU_TOTAL_FILEBYTE += _wtoi(szaTmp.GetAt(LFILEGZLEN));
				//szaTmp.SetSize(LFILETOTALPARMS);
				//szaLists.Append( szaTmp );
			}
		}
		szLog.Format(_T("Total Files: %d ; Total Len %d"), iSVPCU_TOTAL_FILE, iSVPCU_TOTAL_FILEBYTE);
		SVP_LogMsg(szLog);
	}
    DeleteFile(szTmpFilename);
	return rret;
}

int cupdatenetlib::GetReadyToCopyCount()
{
    int iret = 0;
    for(int i = 0; i < m_UpdateFileArray.GetCount(); i++)
    {
        UpdateInfo* pInfo = (UpdateInfo*) m_UpdateFileArray.GetAt(i);
        if (pInfo->bReadyToCopy)
            iret++;
    }
    return iret;
}

void cupdatenetlib::tryRealUpdate(BOOL bNoWaiting){

	struct szaMoveFile
	{
		CString szMoveSrcFile;
		CString szMoveDestFile;
	};
	CAtlList<szaMoveFile> szaMoveFiles;

	for(int i = 0; i < m_UpdateFileArray.GetCount(); i++){
		//if(szaLists.GetCount() < (i+LFILETOTALPARMS)){ break; }
        UpdateInfo* pInfo = (UpdateInfo*) m_UpdateFileArray.GetAt(i);
		CString szSetupPath = pInfo->strPath;
        //SVP_LogMsg5(L"Move %s %d",szSetupPath ,pInfo->bReadyToCopy );
		if (szSetupPath.CompareNoCase( _T("splayer.exe")) == 0){
			if(!svpToolBox.ifFileExist(szBasePath + szSetupPath) ){
				if (svpToolBox.ifFileExist(szBasePath + _T("mplayerc.exe")))
					szSetupPath = _T("mplayerc.exe");
				else if (svpToolBox.ifFileExist(szBasePath + _T("svplayer.exe")))
					szSetupPath = _T("svplayer.exe");
                else
                    szSetupPath = _T("splayer.exe");
			}
		}

		BOOL bUpdateThis = pInfo->bReadyToCopy;
	
		////check file hash
		//CMD5Checksum cmd5;
		//CString updTmpHash ;
		//CString currentHash ;
		//if( svpToolBox.ifFileExist(szUpdfilesPath + pInfo->strTempName ) ){
		//	updTmpHash = cmd5.GetMD5(szUpdfilesPath + pInfo->strTempName ); //Get Hash for current Temp File
		//}
		//
		//if( svpToolBox.ifFileExist(szBasePath + szSetupPath ) ){
		//	currentHash = cmd5.GetMD5(szBasePath + szSetupPath); //Get Hash for bin file
		//}


		//if (currentHash.CompareNoCase( pInfo->strFileMd5 ) != 0 && updTmpHash.CompareNoCase( pInfo->strFileMd5 ) == 0 ){
		//	bUpdateThis = TRUE;
		//}

		if(bUpdateThis){
			//if not match download
			szaMoveFile mFiles;
			mFiles.szMoveSrcFile = szUpdfilesPath + pInfo->strTempName ;
			mFiles.szMoveDestFile = szBasePath + szSetupPath;
			szaMoveFiles.AddTail(mFiles);
// 			while( MoveFileEx( szUpdfilesPath + szaLists.GetAt(i+LFILETMPATH)  , szBasePath + szSetupPath , MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH) == 0 ){
// 				Sleep(50000);
// 			}
		}
	}
	BOOL bFirstRound = true;
	while(1){
		
		POSITION pos = szaMoveFiles.GetHeadPosition();
		if(!pos){ break;}
		bWaiting = TRUE;
		while(pos){
			szaMoveFile mFiles;
			POSITION orgPos = pos;
			mFiles = szaMoveFiles.GetNext(pos);
			if(!svpToolBox.ifFileExist(mFiles.szMoveSrcFile)){
				szaMoveFiles.RemoveAt(orgPos);
				continue;
			}
			svpToolBox.CreatDirForFile(mFiles.szMoveDestFile);
			SetFileAttributes(mFiles.szMoveDestFile , FILE_ATTRIBUTE_NORMAL);
            //SVP_LogMsg5(L"Move %s %s", mFiles.szMoveSrcFile , mFiles.szMoveDestFile );
			if( MoveFileEx( mFiles.szMoveSrcFile , mFiles.szMoveDestFile , MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH) == 0 && bFirstRound){
				// only use MOVEFILE_DELAY_UNTIL_REBOOT on FirstRound
				MoveFileEx( mFiles.szMoveSrcFile , mFiles.szMoveDestFile , /*MOVEFILE_COPY_ALLOWED|*/MOVEFILE_REPLACE_EXISTING|MOVEFILE_DELAY_UNTIL_REBOOT) ;
			}
			/*
			if(mFiles.szMoveDestFile.Right(11).CompareNoCase(_T("Updater.exe")) == 0){
							szaMoveFiles.RemoveAt(orgPos);
							continue;
						}
						SVP_LogMsg5(mFiles.szMoveDestFile.Right(11));*/
			
			
		}

		bFirstRound = FALSE;
		
		Sleep(1500);

		if(bNoWaiting)
			break;
	}

	bWaiting = FALSE;
}

int my_progress_func(/*cupdatenetlib*/void *cup,
					 double t, /* dltotal */
					 double d, /* dlnow */
					 double ultotal,
					 double ulnow)
{
	((cupdatenetlib*)cup)->iSVPCU_CURRENT_FILEBYTE_DONE = (int)d;
	//cup->iSVPCU_CURRENT_FILEBYTE = t;

// 	CString szLog;
// 	szLog.Format(_T("Progress %d / %d "), (int)d, (int)cup->iSVPCU_CURRENT_FILEBYTE);
// 	SVP_LogMsg(szLog);
	return 0;
}
double cupdatenetlib::getProgressBytes(){
	if(iSVPCU_TOTAL_FILEBYTE < iSVPCU_TOTAL_FILEBYTE_DONE + iSVPCU_CURRENT_FILEBYTE_DONE){
		iSVPCU_TOTAL_FILEBYTE = iSVPCU_TOTAL_FILEBYTE_DONE + iSVPCU_CURRENT_FILEBYTE_DONE;
	}
	if (iSVPCU_TOTAL_FILEBYTE  <= 0){
		iSVPCU_TOTAL_FILEBYTE = 1;
	}
	double progress = 0;
	if(iSVPCU_TOTAL_FILEBYTE){
		progress = (double)( iSVPCU_TOTAL_FILEBYTE_DONE + iSVPCU_CURRENT_FILEBYTE_DONE ) * 100/ (iSVPCU_TOTAL_FILEBYTE);
	}
	return progress;
}

bool cupdatenetlib::IsMd5Match(CString strFileName, CString strMd5)
{
    CMD5Checksum cmd5;
    CString strMd5Cal;
    if(svpToolBox.ifFileExist(strFileName))
    {
        strMd5Cal = CString(cmd5.GetMD5((LPCTSTR)strFileName).c_str());
        return (strMd5Cal.CompareNoCase(strMd5) == 0 );
    }
    else
        return false;
}

//, pInfo->strId, pInfo->strPath, pInfo->strCurrentMD5,szUpdfilesPath + pInfo->strTempName

//int cupdatenetlib::downloadFileByIDAndMD5(UpdateInfo* p, CString szID, CString strOrgName, CString strMD5, CString szTmpPath)
//{
//    CString szTmpFilename = this->svpToolBox.getTmpFileName();
//    CString szPostPerm;
//    szPostPerm.Format(_T("setupfileid=%s&MD5=%s"), szID, strMD5);
//    int rret = 0;
//
//    if (PostUsingCurl(szPostPerm, szTmpFilename, my_progress_func)){
//        rret = 1;
//        if (IsFileGziped(szTmpFilename))
//            svpToolBox.unpackGZfile( szTmpFilename, szTmpPath );
//        else
//        {
//            bool b = ApplyEnsemblePatch(szBasePath + strOrgName, szTmpFilename, szTmpPath);
//        }
//    }
//    return rret;
//}

//, pInfo->strId, pInfo->strPath, szUpdfilesPath + pInfo->strTempName
int cupdatenetlib::downloadFileByID(UpdateInfo* pInfo, bool UsingMd5){
	CString szTmpFilename = this->svpToolBox.getTmpFileName();
	CString szPostPerm;
    CString szTmpPath = szUpdfilesPath + pInfo->strTempName;
    if (UsingMd5)
        szPostPerm.Format(_T("setupfileid=%s&MD5=%s"), pInfo->strId, pInfo->strCurrentMD5);
    else
        szPostPerm.Format(_T("setupfileid=%s"), pInfo->strId);

	int rret = 0;

	if (PostUsingCurl(szPostPerm, szTmpFilename, my_progress_func)){
        ////check MD5 of the downloaded file...
        //we should do the check here, but now fro one given file we can have 2 kind of downloads, 
        //thus the MD5 is not determined. SO we just check the result file.
        //if (IsMd5Match(szTmpFilename, pInfo->strDownloadfileMD5))
        //{
            if (IsFileGziped(szTmpFilename))
                svpToolBox.unpackGZfile( szTmpFilename, szTmpPath );
            else
                bool b = ApplyEnsemblePatch(szBasePath + pInfo->strPath, szTmpFilename, szTmpPath);

            //check the MD5 of the result file...
            rret = IsMd5Match(szTmpPath, pInfo->strFileMd5);
            SVP_LogMsg5(L"Md5 %d %s %s", rret, szTmpPath, pInfo->strFileMd5);
            pInfo->bReadyToCopy = rret;

        //}
	}
    DeleteFile(szTmpFilename);
	return rret;
}

int cupdatenetlib::downloadFiles(){
    int iTotalDown = 0;

    for(int i = 0; i < m_UpdateFileArray.GetCount(); i++) {
        UpdateInfo* pInfo = (UpdateInfo*) m_UpdateFileArray.GetAt(i);

        if (!pInfo->bDownload)
            continue;

        CString szSetupPath = pInfo->strPath;
        szCurFilePath = szSetupPath;

        if (szSetupPath.CompareNoCase( _T("splayer.exe")) == 0){
            if(!svpToolBox.ifFileExist(szBasePath + szSetupPath) ){
                if (svpToolBox.ifFileExist(szBasePath + _T("mplayerc.exe")))
                    szSetupPath = _T("mplayerc.exe");
                if (svpToolBox.ifFileExist(szBasePath + _T("svplayer.exe")))
                    szSetupPath = _T("svplayer.exe");
            }
        }

        iSVPCU_CURRENT_FILEBYTE = pInfo->dwDowloadedLength;//_wtoi(szGZlen);
        iSVPCU_CURRENT_FILEBYTE_DONE = 0;
#ifdef NEW_UPDATE
        if( svpToolBox.ifFileExist(szBasePath + szSetupPath ) )
        {
            downloadFileByID/*AndMD5*/( pInfo, true);
        }
        else
            downloadFileByID( pInfo);
#else
        downloadFileByID( pInfo);
#endif
        iTotalDown++;
        iSVPCU_CURRETN_FILE++;
        iSVPCU_CURRENT_FILEBYTE_DONE = iSVPCU_CURRENT_FILEBYTE;
        iSVPCU_TOTAL_FILEBYTE_DONE += iSVPCU_CURRENT_FILEBYTE;
    }
    return iTotalDown;
}

