/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */


#include "StdAfx.h"
#include "AsyncReader.h"
#include <afxsock.h>
#include <afxinet.h>
#include "..\..\..\DSUtil\DSUtil.h"
#include "..\..\..\svplib\svplib.h"

#include "..\..\..\..\include\libunrar\dll.hpp"

#define SVP_LogMsg5 __noop
#define SVP_LogMsg3 __noop
//
// CAsyncFileReader
//

CAsyncFileReader::CAsyncFileReader(CString fn, HRESULT& hr) 
	: CUnknown(NAME("CAsyncFileReader"), NULL, &hr)
	, m_len(-1)
	, m_hBreakEvent(NULL)
	, m_lOsError(0)
	, m_bIsRAR(0)
	, m_hRar(0)
{
	
	m_bIsRAR =  ( fn.Left(6).MakeLower() == _T("rar://") ) ;
	SVP_LogMsg5(L"CAsyncFileReader File %s %d" , fn, m_bIsRAR);
    
	if(m_bIsRAR ){
		SVP_LogMsg5(L"This is RAR File %s" , fn);
		
		{
			

			
			{

				SVP_LogMsg5(_T("rar library loaded"));
				int iPos = fn.Find('?');
				if(iPos >= 0){
					m_fnRAR = fn.Mid(6, iPos - 6);
					m_fnInsideRar = fn.Right( fn.GetLength() - iPos - 1 );


					SVP_LogMsg5(L"RAR = %s \r\n file = %s" , m_fnRAR , m_fnInsideRar);

					struct RAROpenArchiveDataEx ArchiveDataEx;
					memset(&ArchiveDataEx, 0, sizeof(ArchiveDataEx));
#ifdef UNICODE
					ArchiveDataEx.ArcNameW = (LPTSTR)(LPCTSTR)m_fnRAR;
					char fnA[MAX_PATH];
					if(wcstombs(fnA, m_fnRAR, m_fnRAR.GetLength()+1) == -1) fnA[0] = 0;
					ArchiveDataEx.ArcName = fnA;
#else
					ArchiveDataEx.ArcName = (LPTSTR)(LPCTSTR)m_fnRAR;
#endif
					ArchiveDataEx.OpenMode = RAR_OM_EXTRACT;
					ArchiveDataEx.CmtBuf = 0;
					m_hRar = RAROpenArchiveEx(&ArchiveDataEx);
					if(!m_hRar) 
					{
						
						hr = E_FAIL;
					}else{


						//RARSetCallback(m_hRar, MyRARCallBackFunc, (LPARAM)this); useless


						struct RARHeaderDataEx HeaderDataEx;
						HeaderDataEx.CmtBuf = NULL;

						while(RARReadHeaderEx(m_hRar, &HeaderDataEx) == 0)
						{
#ifdef UNICODE
							CString subfn(HeaderDataEx.FileNameW);
#else
							CString subfn(HeaderDataEx.FileName);
#endif

							if(subfn.CompareNoCase(m_fnInsideRar) == 0)
							{
								m_len = HeaderDataEx.UnpSize;

								//RARbuff = buff;
								//RARpos = 0;
								SVP_LogMsg5(L"Got RAR File");

								
								//RARbuff = NULL;
								//RARpos = 0;

								int errRar = RARExtractChunkInit(m_hRar, HeaderDataEx.FileName);
								if (errRar != 0) {
									RARCloseArchive(m_hRar);
									hr = E_FAIL;
									SVP_LogMsg5(L"RARExtractChunkInit Failed");
									
								}else{
									SVP_LogMsg5(L"RARExtractChunkInit Done %s " , CStringW(HeaderDataEx.FileName));
									SVP_LogMsg3("RARExtractChunkInit Done %x %s " , m_hRar, (HeaderDataEx.FileName));
									hr = S_OK;
									
								}


								return;
								break;
							}

							RARProcessFile(m_hRar, RAR_SKIP, NULL, NULL);
						}
						SVP_LogMsg5(L" RAR inside File Not Found");

						RARCloseArchive(m_hRar);
						hr = E_FAIL;
						
					}

				}else
					hr = E_FAIL;
			}

			
		}
		

	}else{
		hr = Open(fn, modeRead|shareDenyNone|typeBinary|osSequentialScan) ? S_OK : E_FAIL;
		if(SUCCEEDED(hr)) m_len = GetLength();
	}
	
}
CAsyncFileReader::CAsyncFileReader(CAtlList<CHdmvClipInfo::PlaylistItem>& Items, HRESULT& hr) 
: CUnknown(NAME("CAsyncFileReader"), NULL, &hr)
, m_len(-1)
, m_hBreakEvent(NULL)
, m_lOsError(0)
, m_bIsRAR(0)
, m_hRar(0)
{
	hr = OpenFiles(Items, modeRead|shareDenyNone|typeBinary|osSequentialScan) ? S_OK : E_FAIL;
	if(SUCCEEDED(hr)) m_len = GetLength();
}
STDMETHODIMP CAsyncFileReader::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	return 
		QI(IAsyncReader)
		QI(ISyncReader)
		QI(IFileHandle)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

UINT CAsyncFileReader::Read( void* lpBuf,	UINT nCount ){
	//SVP_LogMsg5(L" CAsyncFileReader::Read %d ", nCount);
	if(m_bIsRAR)
	{
		
			try
			{
				
				int iExtractRet = RARExtractChunk(m_hRar, (char*)lpBuf, nCount);
				SVP_LogMsg3(" CAsyncFileReader::Rea RARExtractChunk %0.1f  ", double(nCount) );
				if ( iExtractRet <= 0 ) {
					SVP_LogMsg3(" CAsyncFileReader::Read  RARExtractChunk %d ", iExtractRet);
					return 0; 
				}
				//
				//if(llPosition != Seek(llPosition, begin)) return E_FAIL;
				//if((UINT)lLength < Read(pBuffer, lLength)) return E_FAIL;

				return nCount;
			}
			catch(...)
			{
				return 0;
			}
		
	}else{
		return __super::Read(lpBuf, nCount);
	}
}

// IAsyncReader

STDMETHODIMP CAsyncFileReader::SyncRead(LONGLONG llPosition, LONG lLength, BYTE* pBuffer)
{
	//SVP_LogMsg5(L" CAsyncFileReader::SyncRead %d at %f", lLength, double(llPosition));
	if(m_bIsRAR)
	{
	//	SVP_LogMsg5(L" CAsyncFileReader::SyncRead is RAR");
		do
		{
			try
			{
				if((llPosition+lLength) > m_len) {
					SVP_LogMsg5(L" CAsyncFileReader::SyncRead  RAR Length > %d" ,GetLength());
					return E_FAIL; 
				}

				int iSeekRet = RARExtractChunkSeek(m_hRar, llPosition, SEEK_SET) ;
                SVP_LogMsg3("CAsyncFileReader::SyncRead RARExtractChunkSeek %0.1f  ", double(llPosition*10) );
				if ( iSeekRet != 0 ) {
					SVP_LogMsg5(L"CAsyncFileReader::SyncRead FAILSeek RARExtractChunkSeek %d ", iSeekRet);
					return E_FAIL; 
				}

				int iExtractRet = RARExtractChunk(m_hRar, (char*)pBuffer, lLength);
				SVP_LogMsg3("CAsyncFileReader::SyncRead RARExtractChunk %0.1f %d %d ", double(llPosition), iExtractRet , lLength);
				//SVP_LogMsg3("CAsyncFileReader::SyncRead RARExtractChunk %d %d %d %d %d %d ", pBuffer[0] ,  pBuffer[1], pBuffer[2] ,  pBuffer[3], pBuffer[4] ,  pBuffer[5]);
				if ( iExtractRet < 0 ) {
					SVP_LogMsg5(L"CAsyncFileReader::SyncRead FAIL RARExtractChunk %0.1f %d %d ", double(llPosition), iExtractRet , lLength);
					return E_FAIL; 
				}
				if ( iExtractRet == 0 ) {
					SVP_LogMsg3("CAsyncFileReader::SyncRead FAIL0 RARExtractChunk Error %0.1f %d %d ", double(llPosition), iExtractRet , lLength);
					RARExtractChunkSeek(m_hRar, llPosition-lLength, SEEK_SET) ;
					iExtractRet = RARExtractChunk(m_hRar, (char*)pBuffer, lLength);
					SVP_LogMsg3("CAsyncFileReader::SyncRead FAIL1 RARExtractChunk Error2 %0.1f %d %d ", double(m_len), iExtractRet , lLength);
					RARExtractChunkSeek(m_hRar, llPosition, SEEK_SET) ;
					iExtractRet = RARExtractChunk(m_hRar, (char*)pBuffer, lLength);
					SVP_LogMsg3("CAsyncFileReader::SyncRead FAIL2 RARExtractChunk Error3 %0.1f %d %d ", double(m_len), iExtractRet , lLength);
					SVP_LogMsg3("CAsyncFileReader::SyncRead FAIL3 RARExtractChunk %d %d %d %d %d %d ", pBuffer[0] ,  pBuffer[1], pBuffer[2] ,  pBuffer[3], pBuffer[4] ,  pBuffer[5]);
					return S_OK; 
				}

		
	//			SVP_LogMsg5(L"%d %d %d %d %d %d",pBuffer[1],pBuffer[2],pBuffer[3],pBuffer[4],pBuffer[5],pBuffer[6]);
				//
				//if(llPosition != Seek(llPosition, begin)) return E_FAIL;
				//if((UINT)lLength < Read(pBuffer, lLength)) return E_FAIL;

				return S_OK;
			}
			catch(CException* e)
			{
				SVP_LogMsg5(L"CAsyncFileReader::SyncRead CException  ");
			}
		}while(m_hBreakEvent && WaitForSingleObject(m_hBreakEvent, 0) == WAIT_TIMEOUT);
	}else{
		do
		{
			try
			{
				if(llPosition+lLength > GetLength()) return E_FAIL; // strange, but the Seek below can return llPosition even if the file is not that big (?)
				if(llPosition != Seek(llPosition, begin)) return E_FAIL;
				if((UINT)lLength < Read(pBuffer, lLength)) return E_FAIL;

#if 0 // def DEBUG
				static __int64 s_total = 0, s_laststoppos = 0;
				s_total += lLength;
				if(s_laststoppos > llPosition)
					TRACE(_T("[%I64d - %I64d] %d (%I64d)\n"), llPosition, llPosition + lLength, lLength, s_total);
				s_laststoppos = llPosition + lLength;
#endif

				return S_OK;
			}
			catch(CFileException* e)
			{
				m_lOsError = e->m_lOsError;
				e->Delete();
				Sleep(1);
				CString fn = m_strFileName;
				try {Close();} catch(CFileException* e) {e->Delete();}
				try {Open(fn, modeRead|shareDenyNone|typeBinary|osSequentialScan);} catch(CFileException* e) {e->Delete();}
				m_strFileName = fn;
			}
		}
		while(m_hBreakEvent && WaitForSingleObject(m_hBreakEvent, 0) == WAIT_TIMEOUT);
	}
	

	return E_FAIL;
}

STDMETHODIMP CAsyncFileReader::Length(LONGLONG* pTotal, LONGLONG* pAvailable)
{
	
	LONGLONG len = m_len >= 0 ? m_len : GetLength();
	if(pTotal) *pTotal = len;
	if(pAvailable) *pAvailable = len;
	
	return S_OK;
}

// IFileHandle

STDMETHODIMP_(HANDLE) CAsyncFileReader::GetFileHandle()
{

	if(m_bIsRAR)
	{
		SVP_LogMsg5(L" CAsyncFileReader::GetFileHandle()");
		return m_hRar;
	}else{
		return m_hFile;
	}
}
CAsyncFileReader::~CAsyncFileReader(){
	if(m_bIsRAR && m_hRar){
		try{
			RARExtractChunkClose(m_hRar);
			RARCloseArchive(m_hRar);
		}catch(...){  }
	}
}
//
// CAsyncUrlReader
//

CAsyncUrlReader::CAsyncUrlReader(CString url, HRESULT& hr) 
	: CAsyncFileReader(url, hr)
{
	if(SUCCEEDED(hr)) return;

	m_url = url;

	if(CAMThread::Create())
		CallWorker(CMD_INIT);

	hr = Open(m_fn, modeRead|shareDenyRead|typeBinary|osSequentialScan) ? S_OK : E_FAIL;
	m_len = -1; // force GetLength() return actual length always
}

CAsyncUrlReader::~CAsyncUrlReader()
{
	if(ThreadExists())
		CallWorker(CMD_EXIT);

	if(!m_fn.IsEmpty())
	{
		CMultiFiles::Close();
		DeleteFile(m_fn);
	}
}

// IAsyncReader

STDMETHODIMP CAsyncUrlReader::Length(LONGLONG* pTotal, LONGLONG* pAvailable)
{
	if(pTotal) *pTotal = 0;
	return __super::Length(NULL, pAvailable);
}

// CAMThread

DWORD CAsyncUrlReader::ThreadProc()
{
	AfxSocketInit(NULL);

	DWORD cmd = GetRequest();
	if(cmd != CMD_INIT) {Reply(E_FAIL); return E_FAIL;}

	try
	{
		CInternetSession is;
		CAutoPtr<CStdioFile> fin(is.OpenURL(m_url, 1, INTERNET_FLAG_TRANSFER_BINARY|INTERNET_FLAG_EXISTING_CONNECT|INTERNET_FLAG_NO_CACHE_WRITE));

		TCHAR path[MAX_PATH], fn[MAX_PATH];
		CFile fout;
		if(GetTempPath(MAX_PATH, path) && GetTempFileName(path, _T("mpc_http"), 0, fn)
		&& fout.Open(fn, modeCreate|modeWrite|shareDenyWrite|typeBinary))
		{
			m_fn = fn;

			char buff[1024];
			int len = fin->Read(buff, sizeof(buff));
			if(len > 0) fout.Write(buff, len);

			Reply(S_OK);

			while(!CheckRequest(&cmd))
			{
				int len = fin->Read(buff, sizeof(buff));
				if(len > 0) fout.Write(buff, len);
			}
		}
		else
		{
			Reply(E_FAIL);
		}

		fin->Close(); // must close it because the destructor doesn't seem to do it and we will get an exception when "is" is destroying
	}
	catch(CInternetException* ie)
	{
		ie->Delete();
		Reply(E_FAIL);
	}

	//

	cmd = GetRequest();
	ASSERT(cmd == CMD_EXIT);
	Reply(S_OK);

	//

	m_hThread = NULL;

	return S_OK;
}
