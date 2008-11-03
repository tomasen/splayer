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

FILE* CSVPToolBox::getTmpFileSteam(){
	FILE *stream;
	WCHAR tmpnamex[L_tmpnam_s];
	errno_t err;
	int i;

	for (i = 0; i < 5; i++) //try 5 times for tmpfile creation
	{
		err = _wtmpnam_s( tmpnamex, L_tmpnam_s );
		if (!err)
			break;

	}
	if (err){
		SVP_LogMsg(_T("TMP FILE name genarater error")); 
		return 0;
	}else{
		
		err = _wfopen_s(&stream, tmpnamex, _T("w+"));
		if(err){
			SVP_LogMsg(_T("TMP FILE Open for Write error")); 
			return 0;
		}
		this->szaTmpFileNames.Add(tmpnamex);
	}
	return stream;
}
int CSVPToolBox::Char4ToInt(char* szBuf){

	int iData;

	iData = szBuf[0];
	iData = iData << 8;
	iData |= szBuf[1];
	iData = iData << 8;
	iData |= szBuf[2];
	iData = iData << 8;
	iData |= szBuf[3];
	
	return iData;
}
int CSVPToolBox::CStringToUTF8(CString szIn, char* szOut)
{
	int   targetLen = ::WideCharToMultiByte(CP_UTF8,0,szIn,-1,szOut,0,NULL,NULL);
	szOut   =   new   char[targetLen+1];          
	memset(szOut,0,targetLen+1);                
	::WideCharToMultiByte(CP_UTF8,0,szIn,-1,szOut,targetLen,NULL,NULL);                
	
	return 0;
}

int CSVPToolBox::UTF8ToCString(char* szIn, WCHAR * szOut)
{
	int   targetLen = ::MultiByteToWideChar(CP_UTF8,0,szIn,-1,szOut,0);
	szOut   =   new   WCHAR[targetLen+1];     
	memset(szOut,0, (targetLen+1) * sizeof(WCHAR) ); 
	::MultiByteToWideChar(CP_UTF8,0,szIn,strlen(szIn)+1,szOut,targetLen);
	return 0;
}
