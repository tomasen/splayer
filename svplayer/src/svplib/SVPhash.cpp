#include "SVPhash.h"
#include "MD5Checksum.h"

CSVPhash::CSVPhash(void)
{
}

CSVPhash::~CSVPhash(void)
{
}
CString CSVPhash::ComputerSubFilesFileHash(CStringArray* szaSubFiles){
	
	BYTE md5buf[16];
	for (int i = 0; i < szaSubFiles->GetCount(); i++)
	{
		CMD5Checksum cmd5 ;
		CString szBuf = cmd5.GetMD5(szaSubFiles->GetAt(i));
		if ( !szBuf.IsEmpty() ){
			SVP_LogMsg(szBuf);
			if ( i == 0){
				memcpy_s(md5buf, 16, cmd5.lpszMD5, 16);
			}else{
				for(int j = 0; j < 16; j++){
					md5buf[j] ^= cmd5.lpszMD5[j];
				}
			}
		}else{
			SVP_LogMsg(_T("md5 error"));
		}
	}
	return this->HexToString(md5buf);
}
CString CSVPhash::HexToString(BYTE* lpszMD5){
	//Convert the hexadecimal checksum to a CString
	CString strMD5;
	for ( int i=0; i < 16; i++) 
	{
		CString Str;
		if (lpszMD5[i] == 0) {
			Str = CString(_T("00"));
		}
		else if (lpszMD5[i] <= 15) 	{
			Str.Format(_T("0%x"),lpszMD5[i]);
		}
		else {
			Str.Format(_T("%x"),lpszMD5[i]);
		}

		ASSERT( Str.GetLength() == 2 );
		strMD5 += Str;
	}
	ASSERT( strMD5.GetLength() == 32 );
	return strMD5;
}
CString CSVPhash::ComputerFileHash(CString szFilePath)
{
	return 0;
}
