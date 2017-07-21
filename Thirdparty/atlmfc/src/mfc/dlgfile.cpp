// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include <dlgs.h>       // for standard control IDs for commdlg


#define new DEBUG_NEW

// Workaround for symbols missing from Windows Server R2 IA64 SDK.
// This enables IA64 applications built with that SDK to link with static MFC libraries.
#include <initguid.h>
extern "C"
{
static CLSID CLSID_FileOpenDialog_MFC       = {0xDC1C5A9C,0xE88A,0x4DDE,{0xA5,0xA1,0x60,0xF8,0x2A,0x20,0xAE,0xF7}};
static CLSID CLSID_FileSaveDialog_MFC       = {0xC0B4E2F3,0xBA21,0x4773,{0x8D,0xBA,0x33,0x5E,0xC9,0x46,0xEB,0x8B}};

static IID IID_IFileDialogEvents_MFC        = {0x973510DB,0x7D7F,0x452B,{0x89,0x75,0x74,0xA8,0x58,0x28,0xD3,0x54}};
static IID IID_IFileDialogControlEvents_MFC = {0x36116642,0xD713,0x4B97,{0x9B,0x83,0x74,0x84,0xA9,0xD0,0x04,0x33}};
}

////////////////////////////////////////////////////////////////////////////
// FileOpen/FileSaveAs common dialog helper

CFileDialog::CFileDialog(BOOL bOpenFileDialog,
	LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags,
	LPCTSTR lpszFilter, CWnd* pParentWnd, DWORD dwSize, BOOL bVistaStyle)
	: CCommonDialog(pParentWnd)
{
	OSVERSIONINFO vi;
	ZeroMemory(&vi, sizeof(OSVERSIONINFO));
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&vi);

	// if running under Vista
	if (vi.dwMajorVersion >= 6)
	{
		m_bVistaStyle = bVistaStyle;
	}
	else
	{
		m_bVistaStyle = FALSE;
	}

	// determine size of OPENFILENAME struct if dwSize is zero
	if (dwSize == 0)
	{
		dwSize = sizeof(OPENFILENAME);
	}

	// size of OPENFILENAME must be at least version 5
	ASSERT(dwSize >= sizeof(OPENFILENAME));
	// allocate memory for OPENFILENAME struct based on size passed in
	m_pOFN = static_cast<LPOPENFILENAME>(malloc(dwSize));
	ASSERT(m_pOFN != NULL);
	if (m_pOFN == NULL)
		AfxThrowMemoryException();

	memset(&m_ofn, 0, dwSize); // initialize structure to 0/NULL
	m_szFileName[0] = '\0';
	m_szFileTitle[0] = '\0';
	m_pofnTemp = NULL;

	m_bOpenFileDialog = bOpenFileDialog;
	m_nIDHelp = bOpenFileDialog ? AFX_IDD_FILEOPEN : AFX_IDD_FILESAVE;

	m_ofn.lStructSize = dwSize;
	m_ofn.lpstrFile = m_szFileName;
	m_ofn.nMaxFile = _countof(m_szFileName);
	m_ofn.lpstrDefExt = lpszDefExt;
	m_ofn.lpstrFileTitle = (LPTSTR)m_szFileTitle;
	m_ofn.nMaxFileTitle = _countof(m_szFileTitle);
	m_ofn.Flags |= dwFlags | OFN_ENABLEHOOK | OFN_EXPLORER;
	if(dwFlags & OFN_ENABLETEMPLATE)
		m_ofn.Flags &= ~OFN_ENABLESIZING;
	m_ofn.hInstance = AfxGetResourceHandle();
	m_ofn.lpfnHook = (COMMDLGPROC)_AfxCommDlgProc;

	// setup initial file name
	if (lpszFileName != NULL)
		Checked::tcsncpy_s(m_szFileName, _countof(m_szFileName), lpszFileName, _TRUNCATE);

	// Translate filter into commdlg format (lots of \0)
	if (lpszFilter != NULL)
	{
		m_strFilter = lpszFilter;
		LPTSTR pch = m_strFilter.GetBuffer(0); // modify the buffer in place
		// MFC delimits with '|' not '\0'
		while ((pch = _tcschr(pch, '|')) != NULL)
			*pch++ = '\0';
		m_ofn.lpstrFilter = m_strFilter;
		// do not call ReleaseBuffer() since the string contains '\0' characters
	}

	if (m_bVistaStyle == TRUE)
	{
		if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
		{ // multi-threaded is not supported
			IFileDialog* pIFileDialog;
			IFileDialogCustomize* pIFileDialogCustomize;

			HRESULT hr;

			USE_INTERFACE_PART_STD(FileDialogEvents);
			USE_INTERFACE_PART_STD(FileDialogControlEvents);

			if (m_bOpenFileDialog)
			{
				hr = CoCreateInstance(CLSID_FileOpenDialog_MFC, NULL, CLSCTX_INPROC_SERVER, 
									  IID_PPV_ARGS(&pIFileDialog));
			}
			else
			{
				hr = CoCreateInstance(CLSID_FileSaveDialog_MFC, NULL, CLSCTX_INPROC_SERVER, 
									  IID_PPV_ARGS(&pIFileDialog));
			}
			if (FAILED(hr))
			{
				m_bVistaStyle = FALSE;
				return;
			}

			hr = pIFileDialog->QueryInterface(IID_PPV_ARGS(&pIFileDialogCustomize));
			ENSURE(SUCCEEDED(hr));

			hr = pIFileDialog->Advise(reinterpret_cast<IFileDialogEvents*>(&m_xFileDialogEvents), &m_dwCookie);
			ENSURE(SUCCEEDED(hr));

			m_pIFileDialog = static_cast<void*>(pIFileDialog);
			m_pIFileDialogCustomize = static_cast<void*>(pIFileDialogCustomize);
		}
		else
		{
			m_bVistaStyle = FALSE;
		}
	}
}

CFileDialog::~CFileDialog()
{
	free(m_pOFN);

	if (m_bVistaStyle == TRUE)
	{
		HRESULT hr;
		hr = (static_cast<IFileDialog*>(m_pIFileDialog))->Unadvise(m_dwCookie);
		ENSURE(SUCCEEDED(hr));

		(static_cast<IFileDialogCustomize*>(m_pIFileDialogCustomize))->Release();

		UINT ref = (static_cast<IFileDialog*>(m_pIFileDialog))->Release();
		ENSURE(ref == 0);
		CoUninitialize();
	}
}

const OPENFILENAME& CFileDialog::GetOFN() const
{
	return *m_pOFN;
}

OPENFILENAME& CFileDialog::GetOFN()
{
	return *m_pOFN;
}

void CFileDialog::UpdateOFNFromShellDialog()
{
	ASSERT(m_bVistaStyle == TRUE);
	if (m_bVistaStyle == TRUE)
	{
		IShellItem *psiResult;
		HRESULT hr = (static_cast<IFileDialog*>(m_pIFileDialog))->GetResult(&psiResult);
		if (SUCCEEDED(hr))
		{
			LPWSTR wcPathName = NULL;
			hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &wcPathName);
			if (SUCCEEDED(hr))
			{
				CStringW strTmp(wcPathName);
				::PathRemoveFileSpecW(strTmp.GetBuffer());
				strTmp.ReleaseBuffer();
				size_t offset = strTmp.GetLength();
				if (wcPathName[offset] == L'\\')
				{
					offset++;
				}
#ifdef UNICODE
				wcsncpy_s(m_ofn.lpstrFile, m_ofn.nMaxFile - 1, wcPathName, _TRUNCATE);
				wcsncpy_s(m_ofn.lpstrFileTitle, m_ofn.nMaxFileTitle, wcPathName + offset, _TRUNCATE);
#else
				::WideCharToMultiByte(CP_ACP, 0, wcPathName + offset,
										-1, m_ofn.lpstrFileTitle, m_ofn.nMaxFileTitle, NULL, NULL);
				m_ofn.lpstrFileTitle[m_ofn.nMaxFileTitle - 1] = _T('\0');
				::WideCharToMultiByte(CP_ACP, 0, wcPathName, -1, m_ofn.lpstrFile,
										m_ofn.nMaxFile - 1, NULL, NULL);
				m_ofn.lpstrFile[m_ofn.nMaxFile - 2] = _T('\0');
#endif
				m_ofn.lpstrFile[_tcslen(m_ofn.lpstrFile) + 1] = _T('\0');
				CoTaskMemFree(wcPathName);
			}
			psiResult->Release();
		}
		else if (m_ofn.Flags & OFN_ALLOWMULTISELECT)
		{
			IFileOpenDialog *pfod = NULL;
			HRESULT hr = (static_cast<IFileDialog*>(m_pIFileDialog))->QueryInterface(IID_PPV_ARGS(&pfod));
			if (SUCCEEDED(hr))
			{
				IShellItemArray *ppenum = NULL;
				HRESULT hr = pfod->GetResults(&ppenum);
				if (SUCCEEDED(hr))
				{
					IEnumShellItems *ppenumShellItems;
					hr = ppenum->EnumItems(&ppenumShellItems);
					if (SUCCEEDED(hr))
					{
						IShellItem *rgelt[1];
						ULONG celtFetched = 0;
						if (ppenumShellItems->Next(1, rgelt, &celtFetched) == S_OK)
						{
							CStringW strTmp;
							LPTSTR pszFileName = m_ofn.lpstrFile;
							LPWSTR wcPathName = NULL;
							hr = rgelt[0]->GetDisplayName(SIGDN_FILESYSPATH, &wcPathName);
							if (SUCCEEDED(hr))
							{
								::PathRemoveFileSpecW(wcPathName);
#ifdef UNICODE
								wcsncpy_s(pszFileName, m_ofn.nMaxFile - 1, wcPathName, _TRUNCATE);
								pszFileName += wcslen(wcPathName) + 1;
#else
								pszFileName += ::WideCharToMultiByte(CP_ACP, 0, wcPathName, -1,
												pszFileName, m_ofn.nMaxFile - 1, NULL, NULL);
#endif
								CoTaskMemFree(wcPathName);
							}
							do
							{
								wcPathName = NULL;
								hr = rgelt[0]->GetDisplayName(SIGDN_FILESYSPATH, &wcPathName);
								if (SUCCEEDED(hr))
								{
									strTmp = wcPathName;
									::PathRemoveFileSpecW(strTmp.GetBuffer());
									strTmp.ReleaseBuffer();
									size_t offset = strTmp.GetLength();
									if (wcPathName[offset] == L'\\')
									{
										offset++;
									}
#ifdef UNICODE
									wcsncpy_s(pszFileName, m_ofn.nMaxFile - (pszFileName - m_ofn.lpstrFile) - 1,
											wcPathName + offset, _TRUNCATE);
									pszFileName += wcslen(wcPathName + offset) + 1;
#else
									pszFileName += ::WideCharToMultiByte(CP_ACP, 0, wcPathName + offset, -1, pszFileName,
													m_ofn.nMaxFile - static_cast<int>(pszFileName - m_ofn.lpstrFile) - 1,
													NULL, NULL);
#endif
									CoTaskMemFree(wcPathName);
								}
								hr = rgelt[0]->Release();
							} while((pszFileName < m_ofn.lpstrFile + m_ofn.nMaxFile - 1)
									&& (ppenumShellItems->Next(1, rgelt, &celtFetched) == S_OK));
							if(pszFileName < m_ofn.lpstrFile + m_ofn.nMaxFile - 1)
							{
								pszFileName[0] = _T('\0');
							}
							else
							{
								m_ofn.lpstrFile[m_ofn.nMaxFile - 2] = _T('\0');
								m_ofn.lpstrFile[m_ofn.nMaxFile - 1] = _T('\0');
							}
						}
						ppenumShellItems->Release();
					}
					ppenum->Release();
				}
				pfod->Release();
			}
		}

		CString strPathName = GetPathName();
		CString strFileName = GetFileName();
		CString strExtension;
		strExtension.Empty();
		LPTSTR pszExtension = ::PathFindExtension(strPathName);
		if (pszExtension != NULL && *pszExtension == _T('.'))
		{
			strExtension = pszExtension + 1;
		}

		m_ofn.nFileOffset = static_cast<WORD>(strPathName.GetLength() -  strFileName.GetLength());
		m_ofn.nFileExtension = static_cast<WORD>(strPathName.GetLength() -  strExtension.GetLength());
	}
}

void CFileDialog::ApplyOFNToShellDialog()
{
	ASSERT(m_bVistaStyle == TRUE);
	if (m_bVistaStyle == TRUE)
	{
		HRESULT hr;
// m_ofn.lpstrTitle
		if(m_ofn.lpstrTitle != NULL)
		{
#ifdef UNICODE
			hr = (static_cast<IFileDialog*>(m_pIFileDialog))->SetTitle(m_ofn.lpstrTitle);
			ENSURE(SUCCEEDED(hr));
#else
			CStringW strTitle(m_ofn.lpstrTitle);
			hr = (static_cast<IFileDialog*>(m_pIFileDialog))->SetTitle(strTitle.GetString());
			ENSURE(SUCCEEDED(hr));
#endif
		}
// m_ofn.lpstrDefExt
		if(m_ofn.lpstrDefExt != NULL)
		{
			CStringW strDefExt(m_ofn.lpstrDefExt);
			hr = (static_cast<IFileDialog*>(m_pIFileDialog))->SetDefaultExtension(strDefExt.GetString());
			ENSURE(SUCCEEDED(hr));
		}
// m_ofn.lpstrFilter
		if(m_ofn.lpstrFilter != NULL)
		{
			UINT nFilterCount = 0;
			LPCTSTR lpstrFilter = m_ofn.lpstrFilter;
			while(lpstrFilter[0])
			{
				lpstrFilter += _tcslen(lpstrFilter)+1;
				lpstrFilter += _tcslen(lpstrFilter)+1;
				nFilterCount ++;
			}
			if (nFilterCount > 0)
			{
				COMDLG_FILTERSPEC* pFilter = NULL;
				pFilter = new COMDLG_FILTERSPEC[nFilterCount];
				ASSERT(pFilter != NULL);
				if (pFilter == NULL)
				{
					AfxThrowMemoryException();
				}
				lpstrFilter = m_ofn.lpstrFilter;
				size_t nFilterIndex = 0;
				size_t filterSize;
				LPWSTR lpwstrFilter;
				while (nFilterIndex < nFilterCount)
				{
					CStringW strTemp;

					filterSize = _tcslen(lpstrFilter)+1;
					lpwstrFilter = static_cast<LPWSTR>(new WCHAR[filterSize]);
					ASSERT(lpwstrFilter != NULL);
					if (lpwstrFilter == NULL)
					{
						AfxThrowMemoryException();
					}
					strTemp = lpstrFilter;
					memcpy_s(lpwstrFilter, (strTemp.GetLength()+1)*sizeof(WCHAR),
						strTemp.GetString(), (strTemp.GetLength()+1)*sizeof(WCHAR));
					pFilter[nFilterIndex].pszName = lpwstrFilter;
					lpstrFilter += filterSize;

					filterSize = _tcslen(lpstrFilter)+1;
					lpwstrFilter = static_cast<LPWSTR>(new WCHAR[filterSize]);
					ASSERT(lpwstrFilter != NULL);
					if (lpwstrFilter == NULL)
					{
						AfxThrowMemoryException();
					}
					strTemp = lpstrFilter;
					memcpy_s(lpwstrFilter, (strTemp.GetLength()+1)*sizeof(WCHAR),
						strTemp.GetString(), (strTemp.GetLength()+1)*sizeof(WCHAR));
					pFilter[nFilterIndex].pszSpec = lpwstrFilter;
					lpstrFilter += filterSize;

					nFilterIndex ++;
				}
				hr = (static_cast<IFileDialog*>(m_pIFileDialog))->SetFileTypes(nFilterCount, pFilter);
				ENSURE(SUCCEEDED(hr));
				for (nFilterIndex = 0; nFilterIndex < nFilterCount; nFilterIndex++)
				{
					delete[] pFilter[nFilterIndex].pszName;
					delete[] pFilter[nFilterIndex].pszSpec;
				}
				delete[] pFilter;

				hr = (static_cast<IFileDialog*>(m_pIFileDialog))->SetFileTypeIndex(m_ofn.nFilterIndex > 1 ? m_ofn.nFilterIndex : 1);
				ENSURE(SUCCEEDED(hr));
			}
		}
// m_ofn.lpstrFile and m_ofn.lpstrInitialDir
		if((m_ofn.lpstrFile != NULL) || (m_ofn.lpstrInitialDir != NULL))
		{
			CStringW strInitialDir;
			if(m_ofn.lpstrFile != NULL)
			{
				CStringW strFile(m_ofn.lpstrFile);
				strInitialDir = strFile;
				::PathRemoveFileSpecW(strInitialDir.GetBuffer());
				strInitialDir.ReleaseBuffer();
				int offset = strInitialDir.GetLength();
				if (strFile[offset] == L'\\')
				{
					offset++;
				}
				hr = (static_cast<IFileDialog*>(m_pIFileDialog))->SetFileName(strFile.GetString() + offset);
				ENSURE(SUCCEEDED(hr));
			}
			if((m_ofn.lpstrInitialDir != NULL) && strInitialDir.IsEmpty())
			{
				strInitialDir = m_ofn.lpstrInitialDir;
			}
			if(!strInitialDir.IsEmpty())
			{
				IShellItem *psiInitialDir = NULL;
				static HMODULE hShellDll = AfxCtxLoadLibrary(_T("Shell32.dll"));
				ENSURE(hShellDll != NULL);
				typedef	HRESULT (__stdcall *PFNSHCREATEITEMFROMPARSINGNAME)(
					PCWSTR,
					IBindCtx*,
					REFIID,
					void**
					);
				static PFNSHCREATEITEMFROMPARSINGNAME pFunc =
					(PFNSHCREATEITEMFROMPARSINGNAME)GetProcAddress(hShellDll, "SHCreateItemFromParsingName");
				ENSURE(pFunc != NULL);
				hr = (*pFunc)(strInitialDir.GetString(), NULL, IID_PPV_ARGS(&psiInitialDir));
				if (SUCCEEDED(hr))
				{
					hr = (static_cast<IFileDialog*>(m_pIFileDialog))->SetFolder(psiInitialDir);
					ENSURE(SUCCEEDED(hr));
					psiInitialDir->Release();
				}
			}
		}
// m_ofn.Flags
		DWORD dwFlags = 0;
		hr = (static_cast<IFileDialog*>(m_pIFileDialog))->GetOptions(&dwFlags);
		ENSURE(SUCCEEDED(hr));

#ifndef VISTA_FILE_DIALOG_FLAG_MAPPING
#define VISTA_FILE_DIALOG_FLAG_MAPPING(OLD,NEW) \
	((m_ofn.Flags & (OLD)) ? (dwFlags |= (NEW)) : (dwFlags &= ~(NEW)))
#ifndef VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING
#define VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(FLAG) \
	VISTA_FILE_DIALOG_FLAG_MAPPING(OFN_##FLAG, FOS_##FLAG)

		VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(ALLOWMULTISELECT);
		VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(CREATEPROMPT);
		VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(DONTADDTORECENT);
		VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(FILEMUSTEXIST);
		VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(FORCESHOWHIDDEN);
		VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(NOCHANGEDIR);
		VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(NODEREFERENCELINKS);
		VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(NOREADONLYRETURN);
		VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(NOTESTFILECREATE);
		VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(NOVALIDATE);
		VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(OVERWRITEPROMPT);
		VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(PATHMUSTEXIST);
		VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING(SHAREAWARE);

		(m_ofn.FlagsEx & OFN_EX_NOPLACESBAR) ?
			(dwFlags |=  FOS_HIDEPINNEDPLACES)
			: (dwFlags &= ~FOS_HIDEPINNEDPLACES);

#undef VISTA_FILE_DIALOG_FLAG_DIRECT_MAPPING
#undef VISTA_FILE_DIALOG_FLAG_MAPPING
#endif
#endif
		hr = (static_cast<IFileDialog*>(m_pIFileDialog))->SetOptions(dwFlags);
		ENSURE(SUCCEEDED(hr));
	}
}

IFileOpenDialog* CFileDialog::GetIFileOpenDialog()
{
	ASSERT(m_bVistaStyle == TRUE);
	IFileOpenDialog* pIFileOpenDialog = NULL;
	if (m_bVistaStyle == TRUE)
	{
		(static_cast<IFileDialog*>(m_pIFileDialog))->QueryInterface(IID_PPV_ARGS(&pIFileOpenDialog));
	}
	return pIFileOpenDialog;
}

IFileSaveDialog* CFileDialog::GetIFileSaveDialog()
{
	ASSERT(m_bVistaStyle == TRUE);
	IFileSaveDialog* pIFileSaveDialog = NULL;
	if (m_bVistaStyle == TRUE)
	{
		(static_cast<IFileDialog*>(m_pIFileDialog))->QueryInterface(IID_PPV_ARGS(&pIFileSaveDialog));
	}
	return pIFileSaveDialog;
}

IFileDialogCustomize* CFileDialog::GetIFileDialogCustomize()
{
	ASSERT(m_bVistaStyle == TRUE);
	IFileDialogCustomize* pIFileDialogCustomize = NULL;
	if (m_bVistaStyle == TRUE)
	{
		(static_cast<IFileDialog*>(m_pIFileDialog))->QueryInterface(IID_PPV_ARGS(&pIFileDialogCustomize));
	}
	return pIFileDialogCustomize;
}

BEGIN_INTERFACE_MAP(CFileDialog, CCmdTarget)
	INTERFACE_PART(CFileDialog, IID_IFileDialogEvents_MFC, FileDialogEvents)
	INTERFACE_PART(CFileDialog, IID_IFileDialogControlEvents_MFC, FileDialogControlEvents)
END_INTERFACE_MAP()

STDMETHODIMP_(ULONG) CFileDialog::XFileDialogEvents::AddRef()
{
	METHOD_PROLOGUE(CFileDialog, FileDialogEvents)
	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CFileDialog::XFileDialogEvents::Release()
{
	METHOD_PROLOGUE(CFileDialog, FileDialogEvents)
	return pThis->ExternalRelease();
}

STDMETHODIMP CFileDialog::XFileDialogEvents::QueryInterface(
	REFIID iid, void FAR* FAR* ppvObj)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogEvents)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CFileDialog::XFileDialogEvents::OnFileOk(IFileDialog *)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogEvents)
	pThis->UpdateOFNFromShellDialog();
	return pThis->OnFileNameOK() ? S_FALSE : S_OK;
}

STDMETHODIMP CFileDialog::XFileDialogEvents::OnFolderChange(IFileDialog *)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogEvents)
	pThis->OnFolderChange();
	return S_OK;
}

STDMETHODIMP CFileDialog::XFileDialogEvents::OnFolderChanging(IFileDialog *, IShellItem *)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogEvents)
	return S_OK;
}

STDMETHODIMP CFileDialog::XFileDialogEvents::OnHelp(IFileDialog *)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogEvents)
	return S_OK;
}

STDMETHODIMP CFileDialog::XFileDialogEvents::OnSelectionChange(IFileDialog *)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogEvents)
	pThis->OnFileNameChange();
	return S_OK;
}

STDMETHODIMP CFileDialog::XFileDialogEvents::OnTypeChange(IFileDialog *)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogEvents)
	UINT uIdx = 0;
	(static_cast<IFileDialog*>(pThis->m_pIFileDialog))->GetFileTypeIndex(&uIdx);
	pThis->m_ofn.nFilterIndex = static_cast<DWORD>(uIdx);
	pThis->OnTypeChange();
	return S_OK;
}

STDMETHODIMP CFileDialog::XFileDialogEvents::OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogEvents)
	return S_OK;
}

STDMETHODIMP CFileDialog::XFileDialogEvents::OnShareViolation(
	IFileDialog *,
	IShellItem *psi,
	FDE_SHAREVIOLATION_RESPONSE *pResponse)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogEvents)
	LPWSTR wcPathName = NULL;
	HRESULT hr;
	ENSURE(psi != NULL);
	hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &wcPathName);
	ENSURE(SUCCEEDED(hr));
	CString strPathName(wcPathName);
	CoTaskMemFree(wcPathName);
	UINT retval = pThis->OnShareViolation(strPathName.GetString());
	ENSURE(pResponse != NULL);
	if (retval == OFN_SHAREWARN)
	{
		*pResponse = FDESVR_DEFAULT;
	}
	else if (retval == OFN_SHARENOWARN)
	{
		*pResponse = FDESVR_ACCEPT;
	}
	else if (retval == OFN_SHAREFALLTHROUGH)
	{
		*pResponse = FDESVR_REFUSE;
	}
	return S_OK;
}

STDMETHODIMP_(ULONG) CFileDialog::XFileDialogControlEvents::AddRef()
{
	METHOD_PROLOGUE(CFileDialog, FileDialogControlEvents)
	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CFileDialog::XFileDialogControlEvents::Release()
{
	METHOD_PROLOGUE(CFileDialog, FileDialogControlEvents)
	return pThis->ExternalRelease();
}

STDMETHODIMP CFileDialog::XFileDialogControlEvents::QueryInterface(
	REFIID iid, void FAR* FAR* ppvObj)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogControlEvents)
	ENSURE(ppvObj != NULL);
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CFileDialog::XFileDialogControlEvents::OnItemSelected(IFileDialogCustomize *, DWORD, DWORD)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogControlEvents)
	return S_OK;
}

STDMETHODIMP CFileDialog::XFileDialogControlEvents::OnButtonClicked(IFileDialogCustomize *, DWORD)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogControlEvents)
	return S_OK;
}

STDMETHODIMP CFileDialog::XFileDialogControlEvents::OnCheckButtonToggled(IFileDialogCustomize *, DWORD, BOOL)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogControlEvents)
	return S_OK;
}

STDMETHODIMP CFileDialog::XFileDialogControlEvents::OnControlActivating(IFileDialogCustomize *, DWORD)
{
	METHOD_PROLOGUE(CFileDialog, FileDialogControlEvents)
	return S_OK;
}

INT_PTR CFileDialog::DoModal()
{
	ASSERT_VALID(this);
	ASSERT(m_ofn.Flags & OFN_ENABLEHOOK);
	ASSERT(m_ofn.lpfnHook != NULL); // can still be a user hook

	// zero out the file buffer for consistent parsing later
	ASSERT(AfxIsValidAddress(m_ofn.lpstrFile, m_ofn.nMaxFile));
	DWORD nOffset = lstrlen(m_ofn.lpstrFile)+1;
	ASSERT(nOffset <= m_ofn.nMaxFile);
	memset(m_ofn.lpstrFile+nOffset, 0, (m_ofn.nMaxFile-nOffset)*sizeof(TCHAR));

	//  This is a special case for the file open/save dialog,
	//  which sometimes pumps while it is coming up but before it has
	//  disabled the main window.
	HWND hWndFocus = ::GetFocus();
	BOOL bEnableParent = FALSE;
	m_ofn.hwndOwner = PreModal();
	AfxUnhookWindowCreate();
	if (m_ofn.hwndOwner != NULL && ::IsWindowEnabled(m_ofn.hwndOwner))
	{
		bEnableParent = TRUE;
		::EnableWindow(m_ofn.hwndOwner, FALSE);
	}

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	ASSERT(pThreadState->m_pAlternateWndInit == NULL);

	if (m_bVistaStyle == TRUE)
	{
		AfxHookWindowCreate(this);
	}
	else if (m_ofn.Flags & OFN_EXPLORER)
		pThreadState->m_pAlternateWndInit = this;
	else
		AfxHookWindowCreate(this);

	INT_PTR nResult = 0;

	if (m_bVistaStyle == TRUE)
	{
		ApplyOFNToShellDialog();
		HRESULT hr = (static_cast<IFileDialog*>(m_pIFileDialog))->Show(m_ofn.hwndOwner);
		nResult = (hr == S_OK) ? IDOK : IDCANCEL;
	}
	else if (m_bOpenFileDialog)
		nResult = ::AfxCtxGetOpenFileName(&m_ofn);
	else
		nResult = ::AfxCtxGetSaveFileName(&m_ofn);

	if (nResult)
		ASSERT(pThreadState->m_pAlternateWndInit == NULL);
	pThreadState->m_pAlternateWndInit = NULL;

	// Second part of special case for file open/save dialog.
	if (bEnableParent)
		::EnableWindow(m_ofn.hwndOwner, TRUE);
	if (::IsWindow(hWndFocus))
		::SetFocus(hWndFocus);

	PostModal();
	return nResult ? nResult : IDCANCEL;
}

CString CFileDialog::GetPathName() const
{
	if (m_bVistaStyle == TRUE)
	{
		if (m_hWnd != NULL)
		{
			CString strResult;
			IShellItem *psiResult;
			HRESULT hr = (static_cast<IFileDialog*>(m_pIFileDialog))->GetCurrentSelection(&psiResult);
			if (SUCCEEDED(hr))
			{
				SFGAOF sfgaoAttribs;
				if ((psiResult->GetAttributes(SFGAO_STREAM, &sfgaoAttribs) == S_FALSE)
					&& (psiResult->GetAttributes(SFGAO_FOLDER, &sfgaoAttribs) == S_OK))
				{
					;
				}
				else
				{
					LPWSTR wcPathName = NULL;
					hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &wcPathName);
					if (SUCCEEDED(hr))
					{
						strResult = wcPathName;
						strResult.ReleaseBuffer();
						CoTaskMemFree(wcPathName);
					}
				}
				psiResult->Release();
			}
			return strResult;
		}
		else
		{
			return m_ofn.lpstrFile;
		}
	}
	else if ((m_ofn.Flags & OFN_EXPLORER) && m_hWnd != NULL)
	{
		ASSERT(::IsWindow(m_hWnd));
		CString strResult;
		if (GetParent()->SendMessage(CDM_GETSPEC, (WPARAM)MAX_PATH,
			(LPARAM)strResult.GetBuffer(MAX_PATH)) < 0)
		{
			strResult.Empty();
		}
		else
		{
			strResult.ReleaseBuffer();
		}

		if (!strResult.IsEmpty())
		{
			if (GetParent()->SendMessage(CDM_GETFILEPATH, (WPARAM)MAX_PATH,
				(LPARAM)strResult.GetBuffer(MAX_PATH)) < 0)
				strResult.Empty();
			else
			{
				strResult.ReleaseBuffer();
				return strResult;
			}
		}
	}
	return m_ofn.lpstrFile;
}

CString CFileDialog::GetFileName() const
{
	if (m_bVistaStyle == TRUE)
	{
		if (m_hWnd != NULL)
		{
			LPWSTR wcFileName;
			HRESULT hr = (static_cast<IFileDialog*>(m_pIFileDialog))->GetFileName(&wcFileName);
			CString strResult(wcFileName);

			if (SUCCEEDED(hr))
			{
				CoTaskMemFree(wcFileName);
			}
			strResult.ReleaseBuffer();
			return strResult;
		}
		else
		{
			return m_ofn.lpstrFileTitle;
		}
	}
	else if ((m_ofn.Flags & OFN_EXPLORER) && m_hWnd != NULL)
	{
		ASSERT(::IsWindow(m_hWnd));
		CString strResult;
		if (GetParent()->SendMessage(CDM_GETSPEC, (WPARAM)MAX_PATH,
			(LPARAM)strResult.GetBuffer(MAX_PATH)) < 0)
		{
			strResult.Empty();
		}
		else
		{
			strResult.ReleaseBuffer();
			return strResult;
		}
	}
	return m_ofn.lpstrFileTitle;
}

CString CFileDialog::GetFileExt() const
{
	if (m_bVistaStyle == TRUE)
	{
		CString strResult;
		if (m_hWnd != NULL)
		{
			strResult = GetFileName();
		}
		else
		{
			strResult = GetPathName();
		}
		strResult.ReleaseBuffer();
		LPTSTR pszExtension = ::PathFindExtension(strResult);
		if (pszExtension != NULL && *pszExtension == _T('.'))
		{
			return pszExtension+1;
		}

		strResult.Empty();
		return strResult;
	}
	else if ((m_ofn.Flags & OFN_EXPLORER) && m_hWnd != NULL)
	{
		ASSERT(::IsWindow(m_hWnd));
		CString strResult;
		LPTSTR pszResult = strResult.GetBuffer(MAX_PATH);
		LRESULT nResult = GetParent()->SendMessage(CDM_GETSPEC, MAX_PATH,
			reinterpret_cast<LPARAM>(pszResult));
		strResult.ReleaseBuffer();
		if (nResult >= 0)
		{
			LPTSTR pszExtension = ::PathFindExtension(strResult);
			if (pszExtension != NULL && *pszExtension == _T('.'))
			{
				return pszExtension+1;
			}
		}

		strResult.Empty();
		return strResult;
	}

	if (m_pofnTemp != NULL)
		if (m_pofnTemp->nFileExtension == 0)
			return _T("");
		else
			return m_pofnTemp->lpstrFile + m_pofnTemp->nFileExtension;

	if (m_ofn.nFileExtension == 0)
		return _T("");
	else
		return m_ofn.lpstrFile + m_ofn.nFileExtension;
}

CString CFileDialog::GetFileTitle() const
{
	CString strResult = GetFileName();
	LPTSTR pszBuffer = strResult.GetBuffer();
	::PathRemoveExtension(pszBuffer);
	strResult.ReleaseBuffer();
	return strResult;
}

CString CFileDialog::GetNextPathName(POSITION& pos) const
{
	BOOL bExplorer = m_ofn.Flags & OFN_EXPLORER;
	TCHAR chDelimiter;
	if (bExplorer)
		chDelimiter = '\0';
	else
		chDelimiter = ' ';

	LPTSTR lpsz = (LPTSTR)pos;
	if (lpsz == m_ofn.lpstrFile) // first time
	{
		if ((m_ofn.Flags & OFN_ALLOWMULTISELECT) == 0)
		{
			pos = NULL;
			return m_ofn.lpstrFile;
		}

		// find char pos after first Delimiter
		while(*lpsz != chDelimiter && *lpsz != '\0')
			lpsz = _tcsinc(lpsz);
		lpsz = _tcsinc(lpsz);

		// if single selection then return only selection
		if (*lpsz == 0)
		{
			pos = NULL;
			return m_ofn.lpstrFile;
		}
	}

	CString strBasePath = m_ofn.lpstrFile;
	if (!bExplorer)
	{
		LPTSTR lpszPath = m_ofn.lpstrFile;
		while(*lpszPath != chDelimiter)
			lpszPath = _tcsinc(lpszPath);
		strBasePath = strBasePath.Left(int(lpszPath - m_ofn.lpstrFile));
	}

	LPTSTR lpszFileName = lpsz;
	CString strFileName = lpsz;

	// find char pos at next Delimiter
	while(*lpsz != chDelimiter && *lpsz != '\0')
		lpsz = _tcsinc(lpsz);

	if (!bExplorer && *lpsz == '\0')
		pos = NULL;
	else
	{
		if (!bExplorer)
			strFileName = strFileName.Left(int(lpsz - lpszFileName));

		lpsz = _tcsinc(lpsz);
		if (*lpsz == '\0') // if double terminated then done
			pos = NULL;
		else
			pos = (POSITION)lpsz;
	}

	TCHAR strDrive[_MAX_DRIVE], strDir[_MAX_DIR], strName[_MAX_FNAME], strExt[_MAX_EXT];
	Checked::tsplitpath_s(strFileName, strDrive, _MAX_DRIVE, strDir, _MAX_DIR, strName, _MAX_FNAME, strExt, _MAX_EXT);
	TCHAR strPath[_MAX_PATH];
	if (*strDrive || *strDir)
	{
		Checked::tcscpy_s(strPath, _countof(strPath), strFileName);
	}
	else
	{
		Checked::tsplitpath_s(strBasePath+_T("\\"), strDrive, _MAX_DRIVE, strDir, _MAX_DIR, NULL, 0, NULL, 0);
		Checked::tmakepath_s(strPath, _MAX_PATH, strDrive, strDir, strName, strExt);
	}
	
	return strPath;
}

void CFileDialog::SetTemplate(LPCTSTR lpWin3ID, LPCTSTR lpWin4ID)
{
	if (m_bVistaStyle == TRUE)
	{
		AfxThrowNotSupportedException();
	}
	if (m_ofn.Flags & OFN_EXPLORER)
		m_ofn.lpTemplateName = lpWin4ID;
	else
		m_ofn.lpTemplateName = lpWin3ID;
	m_ofn.Flags |= OFN_ENABLETEMPLATE;
}

CString CFileDialog::GetFolderPath() const
{
	CString strResult;
	if (m_bVistaStyle == TRUE)
	{
		IShellItem *psiResult;
		HRESULT hr = (static_cast<IFileDialog*>(m_pIFileDialog))->GetFolder(&psiResult);
		if (SUCCEEDED(hr))
		{
			LPWSTR wcFolderPath = NULL;
			hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &wcFolderPath);
			if (SUCCEEDED(hr))
			{
				strResult = wcFolderPath;
				CoTaskMemFree(wcFolderPath);
			}
			psiResult->Release();
		}
	}
	else
	{
		ASSERT(::IsWindow(m_hWnd));
		ASSERT(m_ofn.Flags & OFN_EXPLORER);

		if (GetParent()->SendMessage(CDM_GETFOLDERPATH, (WPARAM)MAX_PATH, (LPARAM)strResult.GetBuffer(MAX_PATH)) < 0)
			strResult.Empty();
		else
			strResult.ReleaseBuffer();
	}
	return strResult;
}

#ifdef UNICODE
void CFileDialog::SetControlText(int nID, const wchar_t  *lpsz)
{
	ASSERT(::IsWindow(m_hWnd));

	if (m_bVistaStyle == TRUE)
	{
		HRESULT hr = (static_cast<IFileDialogCustomize*>(m_pIFileDialogCustomize))->SetControlLabel(nID, lpsz);
		ENSURE(SUCCEEDED(hr));
	}
	else
	{
		ASSERT(m_ofn.Flags & OFN_EXPLORER);
		GetParent()->SendMessage(CDM_SETCONTROLTEXT, (WPARAM)nID, (LPARAM)lpsz);
	}
}
#endif

void CFileDialog::SetControlText(int nID, LPCSTR lpsz)
{
	ASSERT(::IsWindow(m_hWnd));

	if (m_bVistaStyle == TRUE)
	{
		CStringW dest(lpsz);
		HRESULT hr = (static_cast<IFileDialogCustomize*>(m_pIFileDialogCustomize))->SetControlLabel(nID, dest.GetString());
		ENSURE(SUCCEEDED(hr));
	}
	else
	{
		ASSERT(m_ofn.Flags & OFN_EXPLORER);

#ifdef UNICODE
		CStringW dest(lpsz);
		GetParent()->SendMessage(CDM_SETCONTROLTEXT, (WPARAM)nID, (LPARAM)dest.GetString());
#else
		GetParent()->SendMessage(CDM_SETCONTROLTEXT, (WPARAM)nID, (LPARAM)lpsz);
#endif
	}
}

void CFileDialog::HideControl(int nID)
{
	ASSERT(::IsWindow(m_hWnd));
	if (m_bVistaStyle == TRUE)
	{
		HRESULT hr = (static_cast<IFileDialogCustomize*>(m_pIFileDialogCustomize))->SetControlState(nID, CDCS_INACTIVE);
		ENSURE(SUCCEEDED(hr));
	}
	else
	{
		ASSERT(m_ofn.Flags & OFN_EXPLORER);
		GetParent()->SendMessage(CDM_HIDECONTROL, (WPARAM)nID, 0);
	}
}

void CFileDialog::SetDefExt(LPCSTR lpsz)
{
	ASSERT(::IsWindow(m_hWnd));
	if (m_bVistaStyle == TRUE)
	{
		CStringW strExt(lpsz);
		HRESULT hr = (static_cast<IFileDialog*>(m_pIFileDialog))->SetDefaultExtension(strExt.GetString());
		ENSURE(SUCCEEDED(hr));
	}
	else
	{
		ASSERT(m_ofn.Flags & OFN_EXPLORER);
		GetParent()->SendMessage(CDM_SETDEFEXT, 0, (LPARAM)lpsz);
	}
}

UINT CFileDialog::OnShareViolation(LPCTSTR)
{
	ASSERT_VALID(this);

	// Do not call Default() if you override
	return OFN_SHAREWARN; // default
}

BOOL CFileDialog::OnFileNameOK()
{
	ASSERT_VALID(this);

	// Do not call Default() if you override
	return FALSE;
}

void CFileDialog::OnLBSelChangedNotify(UINT, UINT, UINT)
{
	ASSERT_VALID(this);

	// Do not call Default() if you override
	// no default processing needed
}

void CFileDialog::OnInitDone()
{
	ASSERT_VALID(this);
	GetParent()->CenterWindow();

	// Do not call Default() if you override
	// no default processing needed
}

void CFileDialog::OnFileNameChange()
{
	ASSERT_VALID(this);

	// Do not call Default() if you override
	// no default processing needed
}

void CFileDialog::OnFolderChange()
{
	ASSERT_VALID(this);

	// Do not call Default() if you override
	// no default processing needed
}

void CFileDialog::OnTypeChange()
{
	ASSERT_VALID(this);

	// Do not call Default() if you override
	// no default processing needed
}

BOOL CFileDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	ASSERT(pResult != NULL);

	// allow message map to override
	if (CCommonDialog::OnNotify(wParam, lParam, pResult))
		return TRUE;

	OFNOTIFY* pNotify = (OFNOTIFY*)lParam;
	switch(pNotify->hdr.code)
	{
	case CDN_INITDONE:
		OnInitDone();
		return TRUE;
	case CDN_SELCHANGE:
		OnFileNameChange();
		return TRUE;
	case CDN_FOLDERCHANGE:
		OnFolderChange();
		return TRUE;
	case CDN_SHAREVIOLATION:
		*pResult = OnShareViolation(pNotify->pszFile);
		return TRUE;
	case CDN_HELP:
		if (!SendMessage(WM_COMMAND, ID_HELP))
			SendMessage(WM_COMMANDHELP, 0, 0);
		return TRUE;
	case CDN_FILEOK:
		*pResult = OnFileNameOK();
		return TRUE;
	case CDN_TYPECHANGE:
		OnTypeChange();
		return TRUE;
	}

	return FALSE;   // not handled
}

////////////////////////////////////////////////////////////////////////////
// CFileDialog diagnostics

#ifdef _DEBUG
void CFileDialog::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);

	if (m_bOpenFileDialog)
		dc << "File open dialog";
	else
		dc << "File save dialog";
	dc << "\nm_ofn.hwndOwner = " << (void*)m_ofn.hwndOwner;
	dc << "\nm_ofn.nFilterIndex = " << m_ofn.nFilterIndex;
	dc << "\nm_ofn.lpstrFile = " << m_ofn.lpstrFile;
	dc << "\nm_ofn.nMaxFile = " << m_ofn.nMaxFile;
	dc << "\nm_ofn.lpstrFileTitle = " << m_ofn.lpstrFileTitle;
	dc << "\nm_ofn.nMaxFileTitle = " << m_ofn.nMaxFileTitle;
	dc << "\nm_ofn.lpstrTitle = " << m_ofn.lpstrTitle;
	dc << "\nm_ofn.Flags = ";
	dc.DumpAsHex(m_ofn.Flags);
	dc << "\nm_ofn.lpstrDefExt = " << m_ofn.lpstrDefExt;
	dc << "\nm_ofn.nFileOffset = " << m_ofn.nFileOffset;
	dc << "\nm_ofn.nFileExtension = " << m_ofn.nFileExtension;

	dc << "\nm_ofn.lpstrFilter = ";
	LPCTSTR lpstrItem = m_ofn.lpstrFilter;
	LPTSTR lpszBreak = _T("|");

	while (lpstrItem != NULL && *lpstrItem != '\0')
	{
		dc << lpstrItem << lpszBreak;
		lpstrItem += lstrlen(lpstrItem) + 1;
	}
	if (lpstrItem != NULL)
		dc << lpszBreak;

	dc << "\nm_ofn.lpstrCustomFilter = ";
	lpstrItem = m_ofn.lpstrCustomFilter;
	while (lpstrItem != NULL && *lpstrItem != '\0')
	{
		dc << lpstrItem << lpszBreak;
		lpstrItem += lstrlen(lpstrItem) + 1;
	}
	if (lpstrItem != NULL)
		dc << lpszBreak;

	if (m_ofn.lpfnHook == (COMMDLGPROC)_AfxCommDlgProc)
		dc << "\nhook function set to standard MFC hook function";
	else
		dc << "\nhook function set to non-standard hook function";

	dc << "\nVista style" << ((m_bVistaStyle==TRUE)? "enabled" : "disabled");
	dc << "\nm_dwCookie = " << m_dwCookie;
	dc << "\nm_pIFileDialog = " << m_pIFileDialog;
	dc << "\nm_pIFileDialogCustomize = " << m_pIFileDialogCustomize;

	dc << "\n";
}
#endif //_DEBUG


IMPLEMENT_DYNAMIC(CFileDialog, CCommonDialog)

////////////////////////////////////////////////////////////////////////////
