#include "SVPhash.h"
#include "MD5Checksum.h"

CSVPhash::CSVPhash(void)
{
}

CSVPhash::~CSVPhash(void)
{
}
CString CSVPhash::ComputerSubFilesFileHash(CStringArray* szaSubFiles){
	CMD5Checksum cmd5 ;
	BYTE md5buf[16];
	for (int i = 0; i < szaSubFiles->GetCount(); i++)
	{
		if ( !cmd5.GetMD5(szaSubFiles->GetAt(i)).IsEmpty() ){
			if ( i == 0){
				memcpy(md5buf, cmd5.lpszMD5, 16);
			}else{
				for(int j = 0; j < 16; j++){
					md5buf[j] ^= cmd5.lpszMD5[j];
				}
			}
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
