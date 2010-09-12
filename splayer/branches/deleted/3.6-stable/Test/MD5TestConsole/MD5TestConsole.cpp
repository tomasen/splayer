// MD5TestConsole.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "..\..\src\svplib\MD5Checksum.h"
#include <string>
#include <windows.h>

int main()
{
  CMD5Checksum cmd5;
  char buff[100];
  FILE* File;

  wprintf(L"[TEST]\n[THE MD5 CODE OF \"ABCDFAaw2342;WEAS? afewer~~~F\"]\nRESULT SHOULD BE\n98b64c7d91908cd1f77e721dd1ea3b5c\nTHE RESULT OF OVERLOAD-1\n");
  wprintf(cmd5.GetMD5((unsigned char*)"ABCDFAaw2342;WEAS? afewer~~~F", strlen("ABCDFAaw2342;WEAS? afewer~~~F")).c_str());
  wprintf(L"\nTHE RESULT OF OVERLOAD-2\n");
  wprintf(cmd5.GetMD5(L"test1.txt").c_str());
  wprintf(L"\nTHE RESULT OF OVERLOAD-3\n");
  _wfopen_s(&File, L"test1.txt", L"r");
  wprintf(cmd5.GetMD5(File).c_str());
  wprintf(L"\nEND OF TEST\n\n");
  fclose(File);

  scanf( "%s", buff );

	return 0;
}
