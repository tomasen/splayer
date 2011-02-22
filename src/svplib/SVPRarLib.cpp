#include "SVPRarLib.h"
#include "..\..\include\libunrar\dll.hpp"
#include <algorithm>

#define SVP_LogMsg5 __noop
#define SVP_LogMsg3 __noop

CSVPRarLib::CSVPRarLib(void) :
m_hRar(0)
, m_bIsRAR(0)
{
}

CSVPRarLib::~CSVPRarLib(void)
{
}

BOOL CSVPRarLib::SplitPath(CString fnSVPRarPath)
{
  return SplitPath_STL((LPCTSTR)fnSVPRarPath);
}

BOOL CSVPRarLib::SplitPath_STL(std::wstring fnSVPRarPath)
{
  std::wstring fnSVPExt = fnSVPRarPath.substr(0, 6);
  std::transform(fnSVPExt.begin(), fnSVPExt.end(), fnSVPExt.begin(), tolower);
  m_bIsRAR = (fnSVPExt == L"rar://");
  //SVP_LogMsg5(L"CAsyncFileReader File %s" , fn);
  BOOL ret = false;
  if(m_bIsRAR)
    //SVP_LogMsg5(L"This is RAR File %s" , fn);
  {
    //		SVP_LogMsg5(_T("rar library loaded"));
    int iPos = fnSVPRarPath.find('?');
    if(iPos != fnSVPRarPath.npos)
    {
      m_fnRAR = fnSVPRarPath.substr(6, iPos - 6).c_str();
      m_fnInsideRar = fnSVPRarPath.substr(iPos + 1,
        fnSVPRarPath.size() - iPos - 1).c_str();
      ret = true;
    }
  }

  return ret;
}

int CSVPRarLib::ListRar(CString fnRarPath , CStringArray* szaFiles)
{
		szaFiles->RemoveAll();
		struct RAROpenArchiveDataEx ArchiveDataEx;
		memset(&ArchiveDataEx, 0, sizeof(ArchiveDataEx));

		ArchiveDataEx.ArcNameW = (LPTSTR)(LPCTSTR)fnRarPath;
		char fnA[MAX_PATH];
		if(wcstombs(fnA, fnRarPath, fnRarPath.GetLength()+1) == -1) fnA[0] = 0;
		ArchiveDataEx.ArcName = fnA;

		ArchiveDataEx.OpenMode = RAR_OM_EXTRACT;
		ArchiveDataEx.CmtBuf = 0;
	try{
		HANDLE hrar = RAROpenArchiveEx(&ArchiveDataEx);
		if(hrar) 
		{
		
			struct RARHeaderDataEx HeaderDataEx;
			HeaderDataEx.CmtBuf = NULL;
			
			while(RARReadHeaderEx(hrar, &HeaderDataEx) == 0)
			{
        if (HeaderDataEx.Method == 0x30)
        {
				  CString subfn(HeaderDataEx.FileNameW);

				  BOOL bAlreadyHaveTheSame = false;
				  for(int i = 0; i < szaFiles->GetCount(); i++){
					  if(szaFiles->GetAt(i) == subfn){
						  bAlreadyHaveTheSame = true;
						  break;
					  }
				  }
				  if(!bAlreadyHaveTheSame)
					  szaFiles->Add(subfn);
        }

				RARProcessFile(hrar, RAR_SKIP, NULL, NULL);
			}

			RARCloseArchive(hrar);
		}

	}catch (...) {
		return 0;
	}
	return szaFiles->GetCount();
}