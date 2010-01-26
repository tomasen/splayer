// Updater.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Updater.h"
#include "UpdaterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

char* szUrl = "http://svplayer.shooter.cn/api/updater.php";

BOOL CFileGetStatus(LPCTSTR lpszFileName, CFileStatus& status)
{
	try
	{
		return CFile::GetStatus(lpszFileName, status);
	}
	catch(CException* e)
	{
		// MFCBUG: E_INVALIDARG / "Parameter is incorrect" is thrown for certain cds (vs2003)
		// http://groups.google.co.uk/groups?hl=en&lr=&ie=UTF-8&threadm=OZuXYRzWDHA.536%40TK2MSFTNGP10.phx.gbl&rnum=1&prev=/groups%3Fhl%3Den%26lr%3D%26ie%3DISO-8859-1
		TRACE(_T("CFile::GetStatus has thrown an exception\n"));
		e->Delete();
		return false;
	}
}

// CUpdaterApp

BEGIN_MESSAGE_MAP(CUpdaterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CUpdaterApp construction

CUpdaterApp::CUpdaterApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CUpdaterApp object

CUpdaterApp theApp;


// CUpdaterApp initialization
void CUpdaterApp::PreProcessCommandLine()
{
	m_cmdln.RemoveAll();
	for(int i = 1; i < __argc; i++)
	{
		CString str = CString(__targv[i]).Trim(_T(" \""));

		if(str[0] != '/' && str[0] != '-' && str.Find(_T(":")) < 0)
		{
			LPTSTR p = NULL;
			CString str2;
			str2.ReleaseBuffer(GetFullPathName(str, MAX_PATH, str2.GetBuffer(MAX_PATH), &p));
			CFileStatus fs;
			if(!str2.IsEmpty() && CFileGetStatus(str2, fs)) str = str2;
		}

		m_cmdln.AddTail(str);
	}
}
BOOL CUpdaterApp::InitInstance()
{
	//禁止多个副本 
	HANDLE hObject = CreateMutex(NULL,FALSE,_T("SPLAYER_REAL_UPDATER"));
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return FALSE;
	}
	//END 禁止多个副本

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	//SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	PreProcessCommandLine();

	CString szDmpFile;
	UINT m_verbose = 1;

	POSITION pos = m_cmdln.GetHeadPosition();
	while(pos)
	{
		CString param = m_cmdln.GetNext(pos);
		if(param.IsEmpty()) continue;

		if((param[0] == '-' || param[0] == '/') && param.GetLength() > 1)
		{
			CString sw = param.Mid(1).MakeLower();
			if(sw == _T("dmp") && pos) {
				szDmpFile = m_cmdln.GetNext(pos);
				
			}else if(sw == _T("verbose") ) {
				m_verbose = 1;
			}else if(sw == _T("hide") ) {
				m_verbose = 0;
			}
		}
		
	}
	if(!szDmpFile.IsEmpty()){
		//Upload DMP
		CString szLogPath;
		CSVPToolBox svpTool;
		svpTool.GetPlayerPath(_T("SVPDebug.log"));
		SVP_UploadCrashDmp(svpTool.GetPlayerPath(szDmpFile), szLogPath);
		return FALSE;
	}

	//Background downloader

	CUpdaterDlg dlg;
	if(m_verbose){
		dlg.bHide  = FALSE;
		dlg.verbose = m_verbose;
	}else{
		dlg.bHide  = TRUE;
		dlg.verbose = m_verbose;
	}
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
