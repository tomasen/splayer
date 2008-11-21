#include "SVPToolBox.h"

CSVPToolBox::CSVPToolBox(void)
{
}

CSVPToolBox::~CSVPToolBox(void)
{
}
int CSVPToolBox::ClearTmpFiles(){
	int i;
	for( i = 0; i < this->szaTmpFileNames.GetCount(); i++){
		_wremove(szaTmpFileNames.GetAt(i));
	}
	return i;
}
WCHAR* CSVPToolBox::getTmpFileName(){
	WCHAR* tmpnamex ;
	int i;

	for (i = 0; i < 5; i++) //try 5 times for tmpfile creation
	{
		tmpnamex = _wtempnam( this->GetTempDir(), _T("svp") );
		if (tmpnamex)
			break;

	}
	if (!tmpnamex){
		SVP_LogMsg(_T("TMP FILE name genarater error")); 
		return 0;
	}else{
		//SVP_LogMsg(tmpnamex);
		return tmpnamex;
	}
}
FILE* CSVPToolBox::getTmpFileSteam(){
	FILE *stream;
	WCHAR* tmpnamex = this->getTmpFileName();
	errno_t err;

	if (!tmpnamex){
		SVP_LogMsg(_T("TMP FILE stream name genarater error")); 
		return 0;
	}else{
		
		err = _wfopen_s(&stream, tmpnamex, _T("wb+"));
		if(err){
			SVP_LogMsg(_T("TMP FILE stream Open for Write error")); 
			stream = 0;
		}else{
			this->szaTmpFileNames.Add(tmpnamex);
			free(tmpnamex);
		}
	}
	return stream;
}
int CSVPToolBox::HandleSubPackage(FILE* fp){
	//Extract Package
	SVP_LogMsg( _T("Extracting Package") );


	char szSBuff[8];
	int err;
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		
		SVP_LogMsg( _T("Fail to retrive Package Data Length ") );
		return -1;
	}
	int iPackageLength = this->Char4ToInt(szSBuff);
	
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive Desc Data Length"));
		return -2;
	}

	szaSubDescs.RemoveAll();
	szaSubTmpFileList.RemoveAll();
	
	size_t iDescLength = this->Char4ToInt(szSBuff);

	if(iDescLength > 0){
		SVP_LogMsg(_T("retriving Desc Data"));
		char * szDescData = this->ReadToPTCharByLength(fp, iDescLength);
		if(!szDescData){
			SVP_LogMsg(_T("Fail to retrive Desc Data"));
			return -4;
		}
		// convert szDescData to Unicode and save to CString
		szaSubDescs.Add(this->UTF8ToCString( szDescData , iDescLength));
		free(szDescData);
	}
	err = this->ExtractSubFiles(fp);
	
// 	fpos_t pos;
// 	fgetpos( fp, &pos ) ;
// 	CString szErr;
// 	szErr.Format(_T("Reading At %d got next %ld") , pos,iDescLength);
// 	SVP_LogMsg(szErr);

	

	return 0;

}
int CSVPToolBox::ExtractSubFiles(FILE* fp){
	char szSBuff[8];
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive File Data Length"));
		return -1;
	}

	size_t iFileDataLength = this->Char4ToInt(szSBuff); //确认是否确实有文件下载
	if(!iFileDataLength){
		SVP_LogMsg(_T("No real file released"));
		return 0;
	}

	if ( fread(szSBuff , sizeof(char), 1, fp) < 1){
		SVP_LogMsg(_T("Fail to retrive how many Files are there"));
		return -2;
	}
	int iFileCount = szSBuff[0];
	if( iFileCount <= 0 ){
		SVP_LogMsg(_T("No file in File Data"));
		return -3;
	}

	int iCurrentId = this->szaSubTmpFileList.GetCount();
	this->szaSubTmpFileList.Add(_T(""));
	for(int k = 0; k < iFileCount; k++){
		if(this->ExtractEachSubFile(fp, iCurrentId) ){
			SVP_LogMsg(_T("File Extract Error ") );
			return -4;
		}
	}

	return 0;
}
char* CSVPToolBox::ReadToPTCharByLength(FILE* fp, size_t length){
	char * szBuffData =(char *)calloc( length + 1, sizeof(char));

	if(!szBuffData){
		SVP_LogMsg(_T("Fail to calloc Data "));
		return 0;
	}

	if ( fread(szBuffData , sizeof(char), length, fp) < length){
		SVP_LogMsg(_T("Fail to retrive  Data "));
		return 0;
	}

	return szBuffData;
}
int CSVPToolBox::ExtractEachSubFile(FILE* fp, int iSubPosId){
	// get file ext name
	char szSBuff[4096];
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive Single File Pack Length"));
		return -1;
	}

	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive File Ext Name Length"));
		return -1;
	}

	size_t iExtLength = this->Char4ToInt(szSBuff);
	char* szExtName = this->ReadToPTCharByLength(fp, iExtLength);
	if(!szExtName){
		SVP_LogMsg(_T("Fail to retrive File Ext Name"));
		return -2;
	}
	
	
	//get filedata length
	if ( fread(szSBuff , sizeof(char), 4, fp) < 4){
		SVP_LogMsg(_T("Fail to retrive File Data Length"));
		return -1;
	}

	size_t iFileLength = this->Char4ToInt(szSBuff);
	
	// gen tmp name and tmp file point
	WCHAR* otmpfilename = this->getTmpFileName();
	if(!otmpfilename){
		SVP_LogMsg(_T("TMP FILE name for sub file error")); 
		return -5;
	}
	FILE* fpt;
	errno_t err = _wfopen_s(&fpt, otmpfilename, _T("wb"));
	if(err){
		SVP_LogMsg(_T("TMP FILE stream for sub file  Write error")); 
		free(otmpfilename);
		return -4;
	}

	// copy date to tmp file
	size_t leftoread = iFileLength;
	do{
		size_t needtoread = SVP_MIN( 4096, leftoread );
		size_t accturead = fread(szSBuff , sizeof(char), needtoread, fp);
		if(accturead == 0){
			//wtf
			break;
		}
		leftoread -= accturead;
		err = fwrite(szSBuff,  sizeof(char), accturead , fpt);

		
	}while(leftoread > 0);
	fclose( fpt );

	// add filename and tmp name to szaTmpFileNames
	this->szaSubTmpFileList[iSubPosId].Append( this->UTF8ToCString(szExtName, iExtLength)); //why cant use + ???
	this->szaSubTmpFileList[iSubPosId].Append(_T("|") );
	this->szaSubTmpFileList[iSubPosId].Append(otmpfilename);
	this->szaSubTmpFileList[iSubPosId].Append( _T(";"));
	//SVP_LogMsg(this->szaSubTmpFileList[iSubPosId] + _T(" is the szaTmpFileName"));
	//this->szaSubTmpFileList[iSubPosId].SetString( otmpfilename);
	
	free(szExtName);
	free(otmpfilename);

	return 0;
}
#define SVPATH_BASENAME 0  //Without Dot
#define SVPATH_EXTNAME 1  //With Dot
#define SVPATH_DIRNAME 2 //With Slash
#define SVPATH_FILENAME 3  //Without Dot
CString CSVPToolBox::getVideoFileBasename(CString szVidPath, CStringArray* szaPathInfo = NULL){

	int posDot = szVidPath.ReverseFind(_T('.'));
	
	int posSlash = szVidPath.ReverseFind(_T('\\'));
	int posSlash2 = szVidPath.ReverseFind(_T('/'));
	if(posSlash2 > posSlash){posSlash = posSlash2;}

	if(posDot > posSlash ){
		if (szaPathInfo != NULL){
			CString szBaseName = szVidPath.Left(posDot);
			CString szExtName = szVidPath.Right(szVidPath.GetLength() - posDot).MakeLower();
			CString szFileName = szVidPath.Mid(posSlash+1, (posDot - posSlash - 1));
			CString szDirName = szVidPath.Left(posSlash + 1) ;
			szaPathInfo->RemoveAll();
			szaPathInfo->Add(szBaseName); // Base Name
			szaPathInfo->Add(szExtName ); //ExtName
			szaPathInfo->Add(szDirName); //Dir Name ()
			szaPathInfo->Add(szFileName); // file name only
			SVP_LogMsg(szBaseName);
			SVP_LogMsg(szaPathInfo->GetAt(0) + _T(" | ") + szaPathInfo->GetAt(1) + _T(" | ") + szaPathInfo->GetAt(2) + _T(" | ") + szaPathInfo->GetAt(3) );
		}
		return szVidPath.Left(posDot);
	}

	return szVidPath;
}
int CSVPToolBox::Explode(CString szIn, CString szTok, CStringArray* szaOut){
	szaOut->RemoveAll();

	CString resToken;
	int curPos= 0;

	resToken= szIn.Tokenize(szTok, curPos);
	while (resToken != _T(""))
	{
		szaOut->Add(resToken);
		resToken= szIn.Tokenize(szTok,curPos);
	};

	return 0;
}
BOOL CSVPToolBox::ifFileExist(CString szPathname){
	FILE* fp = NULL;

	
	fp = _wfopen( szPathname, _T("rb") );
	if( fp != NULL )
	{
		fclose( fp );
		return true;
	}
	return false;
}
BOOL CSVPToolBox::ifDirWritable(CString szDir){
	FILE* fp = NULL;
	fp = _wfopen( szDir + _T("svpwrtst"), _T("wb") );
	if( fp != NULL )
	{
		fclose( fp );
		_wremove(szDir + _T("svpwrtst"));
		return true;
	}
	return false;
}
CString CSVPToolBox::getSubFileByTempid(int iTmpID, CString szVidPath){
	//get base path name
	CStringArray szVidPathInfo ;
	CString szBasename = this->getVideoFileBasename(szVidPath, &szVidPathInfo);
	if(!this->ifDirWritable(szVidPathInfo.GetAt(SVPATH_DIRNAME)) ){
		szBasename = this->GetTempDir() + szVidPathInfo.GetAt(SVPATH_FILENAME);
	}
	//set new file name
	CStringArray szSubfiles;
	this->Explode(this->szaSubTmpFileList.GetAt(iTmpID), _T(";"), &szSubfiles);

	if( szSubfiles.GetCount() < 1){
		SVP_LogMsg( _T("Not enough files in tmp array"));
		
	}
	CString szTargetBaseName = _T("");
	CString szDefaultSubPath = _T("");
	for(int i = 0; i < szSubfiles.GetCount(); i++){
		CStringArray szSubTmpDetail;
		this->Explode(szSubfiles[i], _T("|"), &szSubTmpDetail);
		if (szSubTmpDetail.GetCount() < 2){
			SVP_LogMsg( _T("Not enough detail in sub tmp string") + szSubfiles[i]);
			continue;
		}
		CString szSource = szSubTmpDetail[1];
		CString szLangExt  = _T(".chn"); //TODO: use correct language perm 
		if (szSubTmpDetail[0].GetAt(0) != _T('.')){
			szSubTmpDetail[0] = CString(_T(".")) + szSubTmpDetail[0];
		}
		CString szTarget = szBasename + szLangExt + szSubTmpDetail[0];
		if(szTargetBaseName.IsEmpty()){
			szTargetBaseName = szBasename + szLangExt ;
			//check if target exist
			CString szTmp = _T("") ;
			int ilan = 1;
			while( this->ifFileExist(szTarget) ){ 
				//TODO: compare if its the same file
				
				szTmp.Format(_T("%d"), ilan);
				szTarget = szBasename + szLangExt + szTmp + szSubTmpDetail[0]; 
				szTargetBaseName = szBasename + szLangExt + szTmp;
				ilan++;
			}
			
		}else{
			szTarget = szTargetBaseName + szSubTmpDetail[0];
		}
		if(szDefaultSubPath.IsEmpty()){
			szDefaultSubPath = szTarget;
		}
		if( !CopyFile( szSource, szTarget, true) ){
			SVP_LogMsg( szSource + _T(" to ") + szTarget + _T(" Fail to copy subtitle file to position"));
		}
		_wremove(szSource);
	}
	
	return szDefaultSubPath;
	
}
CString CSVPToolBox::PackageSubFiles(CStringArray* szaSubFiles){
	return _T("");
}
CString CSVPToolBox::GetTempDir(){
	TCHAR lpPathBuffer[MAX_PATH];
	GetTempPath(MAX_PATH,  lpPathBuffer); 
	CString szTmpPath (lpPathBuffer);
	if (szTmpPath.Right(1) != _T("\\") && szTmpPath.Right(1) != _T("/")){
		szTmpPath.Append(_T("\\"));
	}
		
	return szTmpPath;
}
int CSVPToolBox::FindAllSubfile(CString szSubPath , CStringArray* szaSubFiles){
	szaSubFiles->RemoveAll();
	szaSubFiles->Add(szSubPath);
	CStringArray szaPathInfo ;
	CString szBaseName = this->getVideoFileBasename( szSubPath, &szaPathInfo );
	CString szExt = szaPathInfo.GetAt(1);

	if(szExt == _T(".idx")){
		szSubPath = szBaseName + _T(".sub");
		if(this->ifFileExist(szSubPath)){
			szaSubFiles->Add(szSubPath);
		}else{
			szSubPath = szBaseName + _T(".rar");
			if(this->ifFileExist(szSubPath)){
				szaSubFiles->Add(szSubPath);
			}
		}
	}
	
	//TODO: finding other subfile
	return 0;
}
int CSVPToolBox::Char4ToInt(char* szBuf){

	int iData =   ( ((int)szBuf[0] & 0xff) << 24) |  ( ((int)szBuf[1] & 0xff) << 16) | ( ((int)szBuf[2] & 0xff) << 8) |  szBuf[3] & 0xff;;
	
	return iData;
}
char* CSVPToolBox::CStringToUTF8(CString szIn, int* iDescLen, UINT codePage)
{
	char* szOut = 0;
	int   targetLen = ::WideCharToMultiByte(codePage,0,szIn,-1,szOut,0,NULL,NULL);
	szOut   =   new   char[targetLen+1];          
	memset(szOut,0,targetLen+1);                
	targetLen = ::WideCharToMultiByte(codePage,0,szIn,-1,szOut,targetLen,NULL,NULL);                
	*iDescLen = targetLen;
	return szOut;
}

CString CSVPToolBox::UTF8ToCString(char* szIn, int iLength)
{
	int   targetLen = ::MultiByteToWideChar(CP_UTF8,0,szIn,iLength+1,0,0);
	WCHAR* szOut   =  (WCHAR *)calloc( targetLen + 1, sizeof(WCHAR)); 

	::MultiByteToWideChar(CP_UTF8,0,szIn,iLength+1,szOut,targetLen);
	CString szBuf(szOut,targetLen);
	free(szOut);
	return szBuf;
}
