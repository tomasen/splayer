#include "SVPToolBox.h"

CSVPToolBox::CSVPToolBox(void)
{
}

CSVPToolBox::~CSVPToolBox(void)
{
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
