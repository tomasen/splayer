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

#include "stdafx.h"
#include "mplayerc.h"
#include "FGManager.h"
#include "..\..\DSUtil\DSUtil.h"
#include "..\..\Filters\Filters.h"
#include "DX7AllocatorPresenter.h"
#include "DX9AllocatorPresenter.h"
#include "EVRAllocatorPresenter.h"
#include "DeinterlacerFilter.h"
#include "internal_filter_config.h"
#include <initguid.h>
#include "..\..\..\include\moreuuids.h"
#include <dmodshow.h>
#include <D3d9.h>
#include <Vmr9.h>
#include "../../svplib/SVPToolBox.h"
#include <evr.h>
#include <evr9.h>

#include "..\..\filters\transform\svpfilter\SVPSubFilter.h"

//
// CFGManager
//
//#define  SVP_LogMsg5 

CFGManager::CFGManager(LPCTSTR pName, LPUNKNOWN pUnk)
	: CUnknown(pName, pUnk)
	, m_dwRegister(0)
{
	if( S_OK != m_pUnkInner.CoCreateInstance(CLSID_FilterGraph, GetOwner()) ){
		m_pUnkInner = NULL;
	}
	if( S_OK != m_pFM.CoCreateInstance(CLSID_FilterMapper2) ){
		m_pFM = NULL;
	}
}

CFGManager::~CFGManager()
{
	CAutoLock cAutoLock(this);
	while(!m_source.IsEmpty()) delete m_source.RemoveHead();
	while(!m_transform.IsEmpty()) delete m_transform.RemoveHead();
	while(!m_override.IsEmpty()) delete m_override.RemoveHead();
	m_pUnks.RemoveAll();
	m_pUnkInner.Release();
}

STDMETHODIMP CFGManager::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

	return
		QI(IFilterGraph)
		QI(IGraphBuilder)
		QI(IFilterGraph2)
		QI(IGraphBuilder2)
		QI(IGraphBuilderDeadEnd)
		m_pUnkInner && (riid != IID_IUnknown && SUCCEEDED(m_pUnkInner->QueryInterface(riid, ppv))) ? S_OK :
		__super::NonDelegatingQueryInterface(riid, ppv);
}

//

void CFGManager::CStreamPath::Append(IBaseFilter* pBF, IPin* pPin)
{
	path_t p;
	p.clsid = GetCLSID(pBF);
	p.filter = GetFilterName(pBF);
	p.pin = GetPinName(pPin);
	AddTail(p);
}

bool CFGManager::CStreamPath::Compare(const CStreamPath& path)
{
	POSITION pos1 = GetHeadPosition();
	POSITION pos2 = path.GetHeadPosition();

	while(pos1 && pos2)
	{
		const path_t& p1 = GetNext(pos1);
		const path_t& p2 = path.GetNext(pos2);

		if(p1.filter != p2.filter) return true;
		else if(p1.pin != p2.pin) return false;
	}

	return true;
}

//

bool CFGManager::CheckBytes(HANDLE hFile, CString chkbytes)
{
	CAtlList<CString> sl;
	Explode(chkbytes, sl, ',');

	if(sl.GetCount() < 4)
		return false;

	ASSERT(!(sl.GetCount()&3));

	LARGE_INTEGER size = {0, 0};
	size.LowPart = GetFileSize(hFile, (DWORD*)&size.HighPart);

	POSITION pos = sl.GetHeadPosition();
	while(sl.GetCount() >= 4)
	{
		CString offsetstr = sl.RemoveHead();
		CString cbstr = sl.RemoveHead();
		CString maskstr = sl.RemoveHead();
		CString valstr = sl.RemoveHead();

		long cb = _ttol(cbstr);

		if(offsetstr.IsEmpty() || cbstr.IsEmpty() 
		|| valstr.IsEmpty() || (valstr.GetLength() & 1)
		|| cb*2 != valstr.GetLength())
			return false;

		LARGE_INTEGER offset;
		offset.QuadPart = _ttoi64(offsetstr);
		if(offset.QuadPart < 0) offset.QuadPart = size.QuadPart - offset.QuadPart;
		SetFilePointer(hFile, offset.LowPart, &offset.HighPart, FILE_BEGIN);

		// LAME
		while(maskstr.GetLength() < valstr.GetLength())
			maskstr += 'F';

		CAtlArray<BYTE> mask, val;
		CStringToBin(maskstr, mask);
		CStringToBin(valstr, val);

		for(size_t i = 0; i < val.GetCount(); i++)
		{
			BYTE b;
			DWORD r;
			if(!ReadFile(hFile, &b, 1, &r, NULL) || (b & mask[i]) != val[i])
				return false;
		}
	}
	if(sl.IsEmpty()){
		SVP_LogMsg( chkbytes + _T(" CheckBytes Success ") );
	}

	return sl.IsEmpty();
}

HRESULT CFGManager::EnumSourceFilters(LPCWSTR lpcwstrFileName, CFGFilterList& fl)
{
	// TODO: use overrides

	CheckPointer(lpcwstrFileName, E_POINTER);

	fl.RemoveAll();

	CStringW fn = CStringW(lpcwstrFileName).TrimLeft();
	CStringW protocol = fn.Left(fn.Find(':')+1).TrimRight(':').MakeLower();
	CStringW ext = CPathW(fn).GetExtension().MakeLower();

	HANDLE hFile = INVALID_HANDLE_VALUE;
	//SVP_LogMsg5(_T("FGM: EnumSourceFilters 1 %s"), fn);
	BOOL bIsRAR = (fn.Left(6).MakeLower() == _T("rar://"));
	if( ( protocol.GetLength() <= 1 || protocol == L"file" ) && !bIsRAR )
	{
		hFile = CreateFile(CString(fn), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

		if(hFile == INVALID_HANDLE_VALUE)
		{
			//SVP_LogMsg5(_T("FGM: EnumSourceFilters VFW_E_NOT_FOUND %s"), fn);
			return VFW_E_NOT_FOUND;
		}
	}

    //AfxMessageBox(fn);
	// exceptions first

	if(ext == _T(".dvr-ms")) // doh, this is stupid 
	{
		fl.Insert(new CFGFilterRegistry(CLSID_StreamBufferSource, MERIT64_PREFERRED), 0);
	}

	TCHAR buff[256], buff2[256];
	ULONG len, len2;

	if(hFile == INVALID_HANDLE_VALUE && !bIsRAR)
	{
		// internal / protocol

		POSITION pos = m_source.GetHeadPosition();
		while(pos)
		{
			CFGFilter* pFGF = m_source.GetNext(pos);
			if(pFGF->m_protocols.Find(CString(protocol)))
				fl.Insert(pFGF, 0, false, false);
		}
	}
	else
	{
		// internal / check bytes

		POSITION pos = m_source.GetHeadPosition();
		while(pos)
		{
			CFGFilter* pFGF = m_source.GetNext(pos);

			POSITION pos2 = pFGF->m_chkbytes.GetHeadPosition();
			while(pos2)
			{
				//RARTODO : 支持 Rar内文件的  CheckBytes
				if(CheckBytes(hFile, pFGF->m_chkbytes.GetNext(pos2)) || bIsRAR)
				{
					SVP_LogMsg5(_T("FGM: AddSourceFilter1 '%s' %s\n"), CStringFromGUID(pFGF->GetCLSID()) , pFGF->GetName());
					fl.Insert(pFGF, 1, false, false);
					break;
				}
			}
		}
	}

	if(!ext.IsEmpty())
	{
		// internal / file extension

		POSITION pos = m_source.GetHeadPosition();
		while(pos)
		{
			CFGFilter* pFGF = m_source.GetNext(pos);
			if(pFGF->m_extensions.Find(CString(ext))){
				SVP_LogMsg5(_T("FGM: AddSourceFilter2 '%s' %s\n"), CStringFromGUID(pFGF->GetCLSID()) , pFGF->GetName());
				fl.Insert(pFGF, 2, false, false);
			}
		}
	}

	{
		// internal / the rest

		POSITION pos = m_source.GetHeadPosition();
		while(pos)
		{
			CFGFilter* pFGF = m_source.GetNext(pos);
			if(pFGF->m_protocols.IsEmpty() && pFGF->m_chkbytes.IsEmpty() && pFGF->m_extensions.IsEmpty()){
				SVP_LogMsg5(_T("FGM: AddSourceFilter3 '%s' %s\n"), CStringFromGUID(pFGF->GetCLSID()) , pFGF->GetName());
				fl.Insert(pFGF, 3, false, false);
			}
		}
	}
	AppSettings& s = AfxGetAppSettings();
	if(1){
		if(hFile == INVALID_HANDLE_VALUE)
		{
			// protocol
		
			CRegKey key;
			if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, CString(protocol), KEY_READ))
			{
				CRegKey exts;
				if(ERROR_SUCCESS == exts.Open(key, _T("Extensions"), KEY_READ))
				{
					len = countof(buff);
					if(ERROR_SUCCESS == exts.QueryStringValue(CString(ext), buff, &len))
						fl.Insert(new CFGFilterRegistry(GUIDFromCString(buff)), 4);
				}

				len = countof(buff);
				if(ERROR_SUCCESS == key.QueryStringValue(_T("Source Filter"), buff, &len))
					fl.Insert(new CFGFilterRegistry(GUIDFromCString(buff)), 5);
			}

			fl.Insert(new CFGFilterRegistry(CLSID_URLReader), 6);
		}
		else if(!s.fBUltraFastMode) //急速模式
		{
			// check bytes

			CRegKey key;
			if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("Media Type"), KEY_READ))
			{
				FILETIME ft;
				len = countof(buff);
				for(DWORD i = 0; ERROR_SUCCESS == key.EnumKey(i, buff, &len, &ft); i++, len = countof(buff))
				{
					GUID majortype;
					if(FAILED(GUIDFromCString(buff, majortype)))
						continue;

					CRegKey majorkey;
					if(ERROR_SUCCESS == majorkey.Open(key, buff, KEY_READ))
					{
						len = countof(buff);
						for(DWORD j = 0; ERROR_SUCCESS == majorkey.EnumKey(j, buff, &len, &ft); j++, len = countof(buff))
						{
							GUID subtype;
							if(FAILED(GUIDFromCString(buff, subtype)))
								continue;

							CRegKey subkey;
							if(ERROR_SUCCESS == subkey.Open(majorkey, buff, KEY_READ))
							{
								len = countof(buff);
								if(ERROR_SUCCESS != subkey.QueryStringValue(_T("Source Filter"), buff, &len))
									continue;

								GUID clsid = GUIDFromCString(buff);

								len = countof(buff);
								len2 = sizeof(buff2);
								for(DWORD k = 0, type; 
									clsid != GUID_NULL && ERROR_SUCCESS == RegEnumValue(subkey, k, buff2, &len2, 0, &type, (BYTE*)buff, &len); 
									k++, len = countof(buff), len2 = sizeof(buff2))
								{
									if(CheckBytes(hFile, CString(buff)))
									{
										CFGFilter* pFGF = new CFGFilterRegistry(clsid);
										pFGF->AddType(majortype, subtype);
										fl.Insert(pFGF, 8);
										break;
									}
								}
							}
						}
					}
				}
			}
		}

		if(!ext.IsEmpty() && !s.fBUltraFastMode)  //急速模式
		{
			// file extension

			CRegKey key;
			if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("Media Type\\Extensions\\") + CString(ext), KEY_READ))
			{
				ULONG len = countof(buff);
				memset(buff, 0, sizeof(buff));
				LONG ret = key.QueryStringValue(_T("Source Filter"), buff, &len); // QueryStringValue can return ERROR_INVALID_DATA on bogus strings (radlight mpc v1003, fixed in v1004)
				if(ERROR_SUCCESS == ret || ERROR_INVALID_DATA == ret && GUIDFromCString(buff) != GUID_NULL)
				{
					GUID clsid = GUIDFromCString(buff);
					GUID majortype = GUID_NULL;
					GUID subtype = GUID_NULL;

					len = countof(buff);
					if(ERROR_SUCCESS == key.QueryStringValue(_T("Media Type"), buff, &len))
						majortype = GUIDFromCString(buff);

					len = countof(buff);
					if(ERROR_SUCCESS == key.QueryStringValue(_T("Subtype"), buff, &len))
						subtype = GUIDFromCString(buff);

					CFGFilter* pFGF = new CFGFilterRegistry(clsid);
					pFGF->AddType(majortype, subtype);
					fl.Insert(pFGF, 7);
				}
			}
		}
	}
	if(hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
	}

	CFGFilter* pFGF = new CFGFilterRegistry(CLSID_AsyncReader);
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_NULL);
	fl.Insert(pFGF, 9);

	return S_OK;
}

HRESULT CFGManager::AddSourceFilter(CFGFilter* pFGF, LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppBF)
{
	
	CString szFName = pFGF->GetName();
	//if(szFName.Find(_T("GPAC")) >= 0 ) return E_NOINTERFACE;

	//CLSID pFID = pFGF->GetCLSID();
	//if(pFID == GUIDFromCString(_T("{E436EBB5-524F-11CE-9F53-0020AF0BA770}") ) ) return E_NOINTERFACE;

	SVP_LogMsg5(_T("FGM: AddSourceFilter trying '%s' %s\n"), CStringFromGUID(pFGF->GetCLSID()) , pFGF->GetName());
	AfxGetAppSettings().szFGMLog.AppendFormat(_T("\r\nFGM: AddSourceFilter trying '%s' %s\n"), CStringFromGUID(pFGF->GetCLSID()) , pFGF->GetName());

	CheckPointer(lpcwstrFileName, E_POINTER);
	CheckPointer(ppBF, E_POINTER);

	ASSERT(*ppBF == NULL);

	HRESULT hr;

	CComPtr<IBaseFilter> pBF;
	CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
	if(FAILED(hr = pFGF->Create(&pBF, pUnks)))
		return hr;

	CComQIPtr<IFileSourceFilter> pFSF = pBF;
	if(!pFSF) return E_NOINTERFACE;

	if(FAILED(hr = AddFilter(pBF, lpcwstrFilterName)))
		return hr;

	const AM_MEDIA_TYPE* pmt = NULL;

	CMediaType mt;
	const CAtlList<GUID>& types = pFGF->GetTypes();
	if(types.GetCount() == 2 && (types.GetHead() != GUID_NULL || types.GetTail() != GUID_NULL))
	{
		mt.majortype = types.GetHead();
		mt.subtype = types.GetTail();
		pmt = &mt;
	}

	if(FAILED(hr = pFSF->Load(lpcwstrFileName, pmt)))
	{
		RemoveFilter(pBF);
		return hr;
	}

	// doh :P
	BeginEnumMediaTypes(GetFirstPin(pBF, PINDIR_OUTPUT), pEMT, pmt)
	{
		if(pmt->subtype == GUIDFromCString(_T("{640999A0-A946-11D0-A520-000000000000}"))
		|| pmt->subtype == GUIDFromCString(_T("{640999A1-A946-11D0-A520-000000000000}"))
		|| pmt->subtype == GUIDFromCString(_T("{D51BD5AE-7548-11CF-A520-0080C77EF58A}")))
		{
			RemoveFilter(pBF);
			pFGF = new CFGFilterRegistry(CLSID_NetShowSource);
			hr = AddSourceFilter(pFGF, lpcwstrFileName, lpcwstrFilterName, ppBF);
			delete pFGF;
			return hr;
		}
	}
	EndEnumMediaTypes(pmt)

	*ppBF = pBF.Detach();

	m_pUnks.AddTailList(&pUnks);

	return S_OK;
}

// IFilterGraph

STDMETHODIMP CFGManager::AddFilter(IBaseFilter* pFilter, LPCWSTR pName)
{
	if(!m_pUnkInner) return E_UNEXPECTED;

	CAutoLock cAutoLock(this);

	HRESULT hr;

	if(FAILED(hr = CComQIPtr<IFilterGraph2>(m_pUnkInner)->AddFilter(pFilter, pName)))
		return hr;

	// TODO
	hr = pFilter->JoinFilterGraph(NULL, NULL);
	hr = pFilter->JoinFilterGraph(this, pName);

	return hr;
}

STDMETHODIMP CFGManager::RemoveFilter(IBaseFilter* pFilter)
{
	if(!m_pUnkInner) return E_UNEXPECTED;

	CAutoLock cAutoLock(this);

	return CComQIPtr<IFilterGraph2>(m_pUnkInner)->RemoveFilter(pFilter);
}

STDMETHODIMP CFGManager::EnumFilters(IEnumFilters** ppEnum)
{
	if(!m_pUnkInner) return E_UNEXPECTED;

	CAutoLock cAutoLock(this);

	return CComQIPtr<IFilterGraph2>(m_pUnkInner)->EnumFilters(ppEnum);
}

STDMETHODIMP CFGManager::FindFilterByName(LPCWSTR pName, IBaseFilter** ppFilter)
{
	if(!m_pUnkInner) return E_UNEXPECTED;

	CAutoLock cAutoLock(this);

	return CComQIPtr<IFilterGraph2>(m_pUnkInner)->FindFilterByName(pName, ppFilter);
}

STDMETHODIMP CFGManager::ConnectDirect(IPin* pPinOut, IPin* pPinIn, const AM_MEDIA_TYPE* pmt)
{
	if(!m_pUnkInner) return E_UNEXPECTED;

	CAutoLock cAutoLock(this);
	HRESULT ret;
	try {
		CComPtr<IBaseFilter> pBF = GetFilterFromPin(pPinIn);
		CLSID clsid = GetCLSID(pBF);
		//FILTER_INFO fInfo;
		//pBF->QueryFilterInfo(&fInfo);
		//SVP_LogMsg5(_T("ConnectDirect %s "), CStringFromGUID(clsid));
		//AfxGetAppSettings().szFGMLog.AppendFormat(_T("\r\nFConnectDirect %s "),  CStringFromGUID(clsid) );

		// TODO: GetUpStreamFilter goes up on the first input pin only
		for(CComPtr<IBaseFilter> pBFUS = GetFilterFromPin(pPinOut); pBFUS; pBFUS = GetUpStreamFilter(pBFUS))
		{
			if(pBFUS == pBF) return VFW_E_CIRCULAR_GRAPH;
			if(GetCLSID(pBFUS) == clsid) return VFW_E_CANNOT_CONNECT;

		//	FILTER_INFO fInfo2;
		//	pBFUS->QueryFilterInfo(&fInfo2);
			//SVP_LogMsg5(_T("ConnectDirect2 %s  "),  CStringFromGUID(GetCLSID(pBFUS)));
			//AfxGetAppSettings().szFGMLog.AppendFormat(_T("\r\nFConnectDirect2 %s"),  CStringFromGUID(GetCLSID(pBFUS)) );

		}
		
		ret = CComQIPtr<IFilterGraph2>(m_pUnkInner)->ConnectDirect(pPinOut, pPinIn, pmt);
		
	}
	catch (CException* e){
		ret = E_UNEXPECTED;
	}
	return ret;
}

STDMETHODIMP CFGManager::Reconnect(IPin* ppin)
{
	if(!m_pUnkInner) return E_UNEXPECTED;

	CAutoLock cAutoLock(this);

	return CComQIPtr<IFilterGraph2>(m_pUnkInner)->Reconnect(ppin);
}

STDMETHODIMP CFGManager::Disconnect(IPin* ppin)
{
	if(!m_pUnkInner) return E_UNEXPECTED;

	CAutoLock cAutoLock(this);

	return CComQIPtr<IFilterGraph2>(m_pUnkInner)->Disconnect(ppin);
}

STDMETHODIMP CFGManager::SetDefaultSyncSource()
{
	if(!m_pUnkInner) return E_UNEXPECTED;

	CAutoLock cAutoLock(this);

	return CComQIPtr<IFilterGraph2>(m_pUnkInner)->SetDefaultSyncSource();
}

// IGraphBuilder

STDMETHODIMP CFGManager::Connect(IPin* pPinOut, IPin* pPinIn)
{
	CAutoLock cAutoLock(this);

	CheckPointer(pPinOut, E_POINTER);
	AppSettings& s = AfxGetAppSettings();

	HRESULT hr;

	if(S_OK != IsPinDirection(pPinOut, PINDIR_OUTPUT) 
	|| pPinIn && S_OK != IsPinDirection(pPinIn, PINDIR_INPUT))
		return VFW_E_INVALID_DIRECTION;

	if(S_OK == IsPinConnected(pPinOut)
	|| pPinIn && S_OK == IsPinConnected(pPinIn))
		return VFW_E_ALREADY_CONNECTED;

	bool fDeadEnd = true;

	if(pPinIn)
	{
		// 1. Try a direct connection between the filters, with no intermediate filters

		if(SUCCEEDED(hr = ConnectDirect(pPinOut, pPinIn, NULL)))
			return hr;
	}
	else
	{
		// 1. Use IStreamBuilder

		if(CComQIPtr<IStreamBuilder> pSB = pPinOut)
		{
			if(SUCCEEDED(hr = pSB->Render(pPinOut, this)))
				return hr;

			pSB->Backout(pPinOut, this);
		}
	}

	// 2. Try cached filters

	if(CComQIPtr<IGraphConfig> pGC = (IGraphBuilder2*)this)
	{
		BeginEnumCachedFilters(pGC, pEF, pBF)
		{
			if(pPinIn && GetFilterFromPin(pPinIn) == pBF)
				continue;

			hr = pGC->RemoveFilterFromCache(pBF);

			// does RemoveFilterFromCache call AddFilter like AddFilterToCache calls RemoveFilter ?

			if(SUCCEEDED(hr = ConnectFilterDirect(pPinOut, pBF, NULL)))
			{
				if(!IsStreamEnd(pBF)) fDeadEnd = false;

				if(SUCCEEDED(hr = ConnectFilter(pBF, pPinIn)))
					return hr;
			}

			hr = pGC->AddFilterToCache(pBF);
		}
		EndEnumCachedFilters
	}

	// 3. Try filters in the graph

	{
		CInterfaceList<IBaseFilter> pBFs;

		BeginEnumFilters(this, pEF, pBF)
		{
			if(pPinIn && GetFilterFromPin(pPinIn) == pBF 
			|| GetFilterFromPin(pPinOut) == pBF)
				continue;

			// HACK: ffdshow - audio capture filter
			if(GetCLSID(pPinOut) == GUIDFromCString(_T("{04FE9017-F873-410E-871E-AB91661A4EF7}"))
			&& GetCLSID(pBF) == GUIDFromCString(_T("{E30629D2-27E5-11CE-875D-00608CB78066}")))
				continue;
			//if(GetCLSID(pPinOut) == GUIDFromCString(_T("{95F57653-71ED-42BA-9131-986CA0C6514F}")) || GetCLSID(pBF) == GUIDFromCString(_T("{95F57653-71ED-42BA-9131-986CA0C6514F}")) ) continue;

			//SVP_LogMsg5(_T(" Try filters in the graph %s %s") , CStringFromGUID(GetCLSID(pPinOut)) , CStringFromGUID(GetCLSID(pBF)));
			pBFs.AddTail(pBF);
		}
		EndEnumFilters

		POSITION pos = pBFs.GetHeadPosition();
		while(pos)
		{
			IBaseFilter* pBF = pBFs.GetNext(pos);

			if(SUCCEEDED(hr = ConnectFilterDirect(pPinOut, pBF, NULL)))
			{
				if(!IsStreamEnd(pBF)) fDeadEnd = false;

				if(SUCCEEDED(hr = ConnectFilter(pBF, pPinIn)))
					return hr;
			}

			EXECUTE_ASSERT(Disconnect(pPinOut));
		}
	}

	// 4. Look up filters in the registry
	
	{
		CFGFilterList fl;

		CAtlArray<GUID> types;
		ExtractMediaTypes(pPinOut, types);

		POSITION pos = m_transform.GetHeadPosition();
		while(pos)
		{
			CFGFilter* pFGF = m_transform.GetNext(pos);
			if(pFGF->GetMerit() < MERIT64_DO_USE || pFGF->CheckTypes(types, false)) 
				fl.Insert(pFGF, 0, pFGF->CheckTypes(types, true), false);
		}

		pos = m_override.GetHeadPosition();
		while(pos)
		{
			CFGFilter* pFGF = m_override.GetNext(pos);
			if(pFGF->GetMerit() < MERIT64_DO_USE || pFGF->CheckTypes(types, false)) 
				fl.Insert(pFGF, 0, pFGF->CheckTypes(types, true), false);
		}

	{
		CComPtr<IEnumMoniker> pEM;
		if(types.GetCount() > 0 
		&& SUCCEEDED(m_pFM->EnumMatchingFilters(
			&pEM, 0, FALSE, MERIT_DO_NOT_USE+1, 
			TRUE, types.GetCount()/2, types.GetData(), NULL, NULL, FALSE,
			!!pPinIn, 0, NULL, NULL, NULL)))
		{
			for(CComPtr<IMoniker> pMoniker; S_OK == pEM->Next(1, &pMoniker, NULL); pMoniker = NULL)
			{
				CFGFilterRegistry* pFGF = new CFGFilterRegistry(pMoniker);
				BOOL alreadyOverrided = false; //not adding it if its overrided
				pos = m_override.GetHeadPosition();
				while(pos)
				{
					CFGFilter* pFGFo = m_override.GetNext(pos);
					if(pFGF->GetCLSID() == pFGFo->GetCLSID()){
						alreadyOverrided = true;
					}
				}
				if(!alreadyOverrided)
					fl.Insert(pFGF, 0, pFGF->CheckTypes(types, true));
			}
		}
	}
		pos = fl.GetHeadPosition();
		while(pos)
		{
			CFGFilter* pFGF = fl.GetNext(pos);

			CString szFName = pFGF->GetName();
			if (szFName.Find(_T("SMV")) >= 0 && (szFName.Find(_T("DSP")) >= 0 || szFName.Find(_T("Switcher")) >= 0)) continue; //disable SMV filter that cause flip
			if (szFName.Find(_T("DivXG400")) >= 0 ) continue; //disable DivxG400 filter that may cause flip
			if (szFName.Find(_T("ArcSoft")) >= 0 ) continue; //disable ArcSoft filter that may cause flip
			if (szFName.Find(_T("Nero")) >= 0 ) continue; //disable Nero filter that may cause flip
			if (szFName.Find(_T("Adobe")) >= 0 ) continue; //disable Adobe filter that may cause flip
			if (szFName.Find(_T("TTL2 Decompressor")) >= 0 ) continue;
			if (szFName.Find(_T("RDP DShow Redirection Filter")) >= 0 ) continue;
			if (szFName.Find(_T("Sonic MP4 Demultiplexer")) >= 0 ) continue;
			if (szFName.Find(_T("MainConcept")) >= 0 ) continue;
			if (szFName.Find(_T("UUSEE DeMultiplexer")) >= 0 ) continue;
			if (szFName.Find(_T("VideoTune")) >= 0 ) continue;
			if (szFName.Find(_T("Sonic HD Demuxer")) >= 0 ) continue;
            if (szFName.Find(_T("Thunder RM Video Decoder")) >= 0 ) continue;
            if (szFName.Find(_T("Roxio Mp3 Encoder")) >= 0 ) continue;
			
			//if (szFName.Find(_T("SHN to Wave Filter")) >= 0 ) continue;
			//if (szFName.Find(_T("AVI Decompressor (YV12)")) >= 0 ) continue;
			//if (szFName.Find(_T("AVI Decompressor (I420)")) >= 0 ) continue;
			//if (szFName.Find(_T("AVI Decompressor (YUY2)")) >= 0 ) continue;
			/*
			if (szFName.Find(_T("AVI Decompressor")) >= 0 ){
						
							continue;
						}*/
			
			//if (szFName.Find(_T("Color Space Converter")) >= 0 ) continue;
			
			CLSID FGID = pFGF->GetCLSID() ;

				if (szFName.Find(_T("DirectVobSub")) >= 0 ) continue;
				
				if (FGID == GUIDFromCString(_T("{48CF8992-4161-49D6-9A9B-F1FDB3BAE74D}"))  ) continue; //"UUSEE DeMultiplexer"
              
				//if (FGID == GUIDFromCString(_T("{E8D381DD-8C7D-4a6f-96ED-92BBB64064CF}"))  ) continue; SVPSubFilter
			
			
			if (s.TraFilters & TRA_AC3 && szFName.Find(_T("AC3Filter")) >= 0 ) continue; //disable AC3 filter if internal AC3 is enabled
			
			
				
			if ( FGID == GUIDFromCString(_T("{AA59CBFA-F731-49E9-BE78-08665F339EFC}")) ) continue;  //disable  Bicubic Video Resizer  that may cause flip
			if ( FGID == GUIDFromCString(_T("{1643E180-90F5-11CE-97D5-00AA0055595A}")) ) continue;  //Color Space Converter
			//if ( FGID == GUIDFromCString(_T("{CF49D4E0-1115-11CE-B03A-0020AF0BA770}")) ) continue;  //AVI Decompressor
			
			AfxGetAppSettings().szFGMLog.AppendFormat(_T("\r\nFGM: Connecting '%s' %s "), szFName, CStringFromGUID(pFGF->GetCLSID()) );
			SVP_LogMsg5(_T("FGM: Connecting '%s' %s "), szFName, CStringFromGUID(pFGF->GetCLSID()) );
			if(s.bNoMoreDXVA){
				//SVP_LogMsg5(_T("FindFilterByName(MPC Video Decoder DXVA "));
				CComPtr<IBaseFilter> pBFX;
				if( SUCCEEDED( FindFilterByName(L"MPC Video Decoder DXVA", &pBFX) )){
					RemoveFilter(pBFX);
					s.bNoMoreDXVA = false;
					//SVP_LogMsg5(_T("FindFilterByName(MPC Video Decoder DXVA remove") );
					//if( SUCCEEDED( FindFilterByName(L"MPC Video Decoder DXVA", &pBFX) )){
						//SVP_LogMsg5(_T("FindFilterByName(MPC Video Decoder DXVA still here") );
					//}
					continue;
				}

			}
			

			CComPtr<IBaseFilter> pBF;
			CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
			if(FAILED(pFGF->Create(&pBF, pUnks)))
            {
                SVP_LogMsg5(L"Create Filter Failed");
				continue;
            }
            if(FAILED(hr = AddFilter(pBF, pFGF->GetName()))){
			     SVP_LogMsg5(L"Add Filter Failed");
                 continue;
            }

			hr = E_FAIL;

			if(FAILED(hr))
			{
				hr = ConnectFilterDirect(pPinOut, pBF, NULL);
			}

			
/*
			if(FAILED(hr))
			{
				if(types.GetCount() >= 2 && types[0] == MEDIATYPE_Stream && types[1] != GUID_NULL)
				{
					CMediaType mt;
					
					mt.majortype = types[0];
					mt.subtype = types[1];
					mt.formattype = FORMAT_None;
					if(FAILED(hr)) hr = ConnectFilterDirect(pPinOut, pBF, &mt);

					mt.formattype = GUID_NULL;
					if(FAILED(hr)) hr = ConnectFilterDirect(pPinOut, pBF, &mt);
				}
			}
*/
			if(SUCCEEDED(hr))
			{
				if(!IsStreamEnd(pBF)) fDeadEnd = false;

				hr = ConnectFilter(pBF, pPinIn);

				if(SUCCEEDED(hr))
				{
					m_pUnks.AddTailList(&pUnks);

					// maybe the application should do this...
					
					POSITION pos = pUnks.GetHeadPosition();
					while(pos)
					{
						if(CComQIPtr<IMixerPinConfig, &IID_IMixerPinConfig> pMPC = pUnks.GetNext(pos))
							pMPC->SetAspectRatioMode(AM_ARMODE_STRETCHED);
					}

					if(CComQIPtr<IVMRAspectRatioControl> pARC = pBF)
						pARC->SetAspectRatioMode(VMR_ARMODE_NONE);
					
					if(CComQIPtr<IVMRAspectRatioControl9> pARC = pBF)
						pARC->SetAspectRatioMode(VMR_ARMODE_NONE);

					if(CComQIPtr<IVMRMixerControl9> pMC = pBF)
						m_pUnks.AddTail (pMC);

					if(CComQIPtr<IVMRMixerBitmap9> pMB = pBF)
						m_pUnks.AddTail (pMB);

					if(CComQIPtr<IMFGetService, &__uuidof(IMFGetService)> pMFGS = pBF)
					{
						CComPtr<IMFVideoDisplayControl>		pMFVDC;
						CComPtr<IMFVideoMixerBitmap>		pMFMB;
						if (SUCCEEDED (pMFGS->GetService (MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, (void**)&pMFVDC)))
							m_pUnks.AddTail (pMFVDC);

						if (SUCCEEDED (pMFGS->GetService (MR_VIDEO_MIXER_SERVICE, IID_IMFVideoMixerBitmap, (void**)&pMFMB)))
							m_pUnks.AddTail (pMFMB);

					}
					return hr;
				}
			}

			EXECUTE_ASSERT(SUCCEEDED(RemoveFilter(pBF)));

			TRACE(_T("FGM: Connecting '%s' FAILED!\n"), pFGF->GetName());
		}
	}

	if(fDeadEnd)
	{
		CAutoPtr<CStreamDeadEnd> psde(new CStreamDeadEnd());
		psde->AddTailList(&m_streampath);
		int skip = 0;
		BeginEnumMediaTypes(pPinOut, pEM, pmt)
		{
			if(pmt->majortype == MEDIATYPE_Stream && pmt->subtype == MEDIASUBTYPE_NULL) skip++;
			psde->mts.AddTail(CMediaType(*pmt));
		}
		EndEnumMediaTypes(pmt)
		if(skip < psde->mts.GetCount())
			m_deadends.Add(psde);
	}

	return pPinIn ? VFW_E_CANNOT_CONNECT : VFW_E_CANNOT_RENDER;
}

STDMETHODIMP CFGManager::Render(IPin* pPinOut)
{
	CAutoLock cAutoLock(this);

	return RenderEx(pPinOut, 0, NULL);
}

STDMETHODIMP CFGManager::RenderFile(LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrPlayList)
{
	CAutoLock cAutoLock(this);

	m_streampath.RemoveAll();
	m_deadends.RemoveAll();

	HRESULT hr;

/*
	CComPtr<IBaseFilter> pBF;
	if(FAILED(hr = AddSourceFilter(lpcwstrFile, lpcwstrFile, &pBF)))
		return hr;

	return ConnectFilter(pBF, NULL);
*/

	CFGFilterList fl;
	if(FAILED(hr = EnumSourceFilters(lpcwstrFileName, fl)))
		return hr;

	CAutoPtrArray<CStreamDeadEnd> deadends;

	hr = VFW_E_CANNOT_RENDER;

	POSITION pos = fl.GetHeadPosition();
	while(pos)
	{
		CComPtr<IBaseFilter> pBF;
		
		if(SUCCEEDED(hr = AddSourceFilter(fl.GetNext(pos), lpcwstrFileName, lpcwstrFileName, &pBF)))
		{
			m_streampath.RemoveAll();
			m_deadends.RemoveAll();

			if(SUCCEEDED(hr = ConnectFilter(pBF, NULL)))
				return hr;

			NukeDownstream(pBF);
			RemoveFilter(pBF);

			deadends.Append(m_deadends);
		}
	}

	m_deadends.Copy(deadends);

	return hr;
}

STDMETHODIMP CFGManager::AddSourceFilter(LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppFilter)
{
	CAutoLock cAutoLock(this);

	HRESULT hr;

	CFGFilterList fl;
	if(FAILED(hr = EnumSourceFilters(lpcwstrFileName, fl)))
		return hr;

	POSITION pos = fl.GetHeadPosition();
	while(pos)
	{
		if(SUCCEEDED(hr = AddSourceFilter(fl.GetNext(pos), lpcwstrFileName, lpcwstrFilterName, ppFilter)))
			return hr;
	}

	return VFW_E_CANNOT_LOAD_SOURCE_FILTER;
}

STDMETHODIMP CFGManager::SetLogFile(DWORD_PTR hFile)
{
	if(!m_pUnkInner) return E_UNEXPECTED;

	CAutoLock cAutoLock(this);

	return CComQIPtr<IFilterGraph2>(m_pUnkInner)->SetLogFile(hFile);
}

STDMETHODIMP CFGManager::Abort()
{
	if(!m_pUnkInner) return E_UNEXPECTED;

	CAutoLock cAutoLock(this);

	return CComQIPtr<IFilterGraph2>(m_pUnkInner)->Abort();
}

STDMETHODIMP CFGManager::ShouldOperationContinue()
{
	if(!m_pUnkInner) return E_UNEXPECTED;

	CAutoLock cAutoLock(this);

	return CComQIPtr<IFilterGraph2>(m_pUnkInner)->ShouldOperationContinue();
}

// IFilterGraph2

STDMETHODIMP CFGManager::AddSourceFilterForMoniker(IMoniker* pMoniker, IBindCtx* pCtx, LPCWSTR lpcwstrFilterName, IBaseFilter** ppFilter)
{
	if(!m_pUnkInner) return E_UNEXPECTED;

	CAutoLock cAutoLock(this);

	return CComQIPtr<IFilterGraph2>(m_pUnkInner)->AddSourceFilterForMoniker(pMoniker, pCtx, lpcwstrFilterName, ppFilter);
}

STDMETHODIMP CFGManager::ReconnectEx(IPin* ppin, const AM_MEDIA_TYPE* pmt)
{
	if(!m_pUnkInner) return E_UNEXPECTED;

	CAutoLock cAutoLock(this);

	return CComQIPtr<IFilterGraph2>(m_pUnkInner)->ReconnectEx(ppin, pmt);
}

STDMETHODIMP CFGManager::RenderEx(IPin* pPinOut, DWORD dwFlags, DWORD* pvContext)
{
	CAutoLock cAutoLock(this);

	m_streampath.RemoveAll();
	m_deadends.RemoveAll();

	if(!pPinOut || dwFlags > AM_RENDEREX_RENDERTOEXISTINGRENDERERS || pvContext)
		return E_INVALIDARG;

	HRESULT hr;

	if(dwFlags & AM_RENDEREX_RENDERTOEXISTINGRENDERERS)
	{
		CInterfaceList<IBaseFilter> pBFs;

		BeginEnumFilters(this, pEF, pBF)
		{
			if(CComQIPtr<IAMFilterMiscFlags> pAMMF = pBF)
			{
				if(pAMMF->GetMiscFlags() & AM_FILTER_MISC_FLAGS_IS_RENDERER)
				{
					pBFs.AddTail(pBF);
				}
			}
			else
			{
				BeginEnumPins(pBF, pEP, pPin)
				{
					CComPtr<IPin> pPinIn;
					DWORD size = 1;
					if(SUCCEEDED(pPin->QueryInternalConnections(&pPinIn, &size)) && size == 0)
					{
						pBFs.AddTail(pBF);
						break;
					}
				}
				EndEnumPins
			}
		}
		EndEnumFilters

		while(!pBFs.IsEmpty())
		{
			if(SUCCEEDED(hr = ConnectFilter(pPinOut, pBFs.RemoveHead())))
				return hr;
		}

		return VFW_E_CANNOT_RENDER;
	}

	return Connect(pPinOut, (IPin*)NULL);
}

// IGraphBuilder2

STDMETHODIMP CFGManager::IsPinDirection(IPin* pPin, PIN_DIRECTION dir1)
{
	CAutoLock cAutoLock(this);

	CheckPointer(pPin, E_POINTER);

	PIN_DIRECTION dir2;
	if(FAILED(pPin->QueryDirection(&dir2)))
		return E_FAIL;

	return dir1 == dir2 ? S_OK : S_FALSE;
}

STDMETHODIMP CFGManager::IsPinConnected(IPin* pPin)
{
	CAutoLock cAutoLock(this);

	CheckPointer(pPin, E_POINTER);

	CComPtr<IPin> pPinTo;
	return SUCCEEDED(pPin->ConnectedTo(&pPinTo)) && pPinTo ? S_OK : S_FALSE;
}

STDMETHODIMP CFGManager::ConnectFilter(IBaseFilter* pBF, IPin* pPinIn)
{
	CAutoLock cAutoLock(this);

	CheckPointer(pBF, E_POINTER);

	if(pPinIn && S_OK != IsPinDirection(pPinIn, PINDIR_INPUT))
		return VFW_E_INVALID_DIRECTION;

	int nTotal = 0, nRendered = 0;

	BeginEnumPins(pBF, pEP, pPin)
	{
		

		if( GetPinName(pPin)[0] != '~'
		&& S_OK == IsPinDirection(pPin, PINDIR_OUTPUT)
		&& S_OK != IsPinConnected(pPin))
		{

			//CLSID pClassID ;
			//pBF->GetClassID(&pClassID);
			m_streampath.Append(pBF, pPin);
			//if(CStringFromGUID(pClassID).CompareNoCase( _T("{95F57653-71ED-42BA-9131-986CA0C6514F}") ) != 0){

				HRESULT hr = Connect(pPin, pPinIn);

				if(SUCCEEDED(hr))
				{
					for(int i = m_deadends.GetCount()-1; i >= 0; i--)
						if(m_deadends[i]->Compare(m_streampath))
							m_deadends.RemoveAt(i);

					nRendered++;

					
					//SVP_LogMsg5(_T("ConnectFilter %s %s"), CStringFromGUID(pClassID) , GetPinName(pPin));

				}

				nTotal++;

				m_streampath.RemoveTail();

				if(SUCCEEDED(hr) && pPinIn) 
					return S_OK;
		//	}

			
		}
	}
	EndEnumPins

	return 
		nRendered == nTotal ? (nRendered > 0 ? S_OK : S_FALSE) :
		nRendered > 0 ? VFW_S_PARTIAL_RENDER :
		VFW_E_CANNOT_RENDER;
}

STDMETHODIMP CFGManager::ConnectFilter(IPin* pPinOut, IBaseFilter* pBF)
{
	CAutoLock cAutoLock(this);

	CheckPointer(pPinOut, E_POINTER);
	CheckPointer(pBF, E_POINTER);

	if(S_OK != IsPinDirection(pPinOut, PINDIR_OUTPUT))
		return VFW_E_INVALID_DIRECTION;

	HRESULT hr;

	BeginEnumPins(pBF, pEP, pPin)
	{
		if(GetPinName(pPin)[0] != '~'
		&& S_OK == IsPinDirection(pPin, PINDIR_INPUT)
		&& S_OK != IsPinConnected(pPin)
		&& SUCCEEDED(hr = Connect(pPinOut, pPin)))
			return hr;
	}
	EndEnumPins

	return VFW_E_CANNOT_CONNECT;
}

STDMETHODIMP CFGManager::ConnectFilterDirect(IPin* pPinOut, IBaseFilter* pBF, const AM_MEDIA_TYPE* pmt)
{
	CAutoLock cAutoLock(this);

	CheckPointer(pPinOut, E_POINTER);
	CheckPointer(pBF, E_POINTER);

	if(S_OK != IsPinDirection(pPinOut, PINDIR_OUTPUT))
		return VFW_E_INVALID_DIRECTION;

	HRESULT hr;

	BeginEnumPins(pBF, pEP, pPin)
	{
		if(GetPinName(pPin)[0] != '~'
		&& S_OK == IsPinDirection(pPin, PINDIR_INPUT)
		&& S_OK != IsPinConnected(pPin)
		&& SUCCEEDED(hr = ConnectDirect(pPinOut, pPin, pmt)))
			return hr;
	}
	EndEnumPins

	return VFW_E_CANNOT_CONNECT;
}

STDMETHODIMP CFGManager::NukeDownstream(IUnknown* pUnk)
{
	CAutoLock cAutoLock(this);

	if(CComQIPtr<IBaseFilter> pBF = pUnk)
	{
		BeginEnumPins(pBF, pEP, pPin)
		{
			NukeDownstream(pPin);
		}
		EndEnumPins
	}
	else if(CComQIPtr<IPin> pPin = pUnk)
	{
		CComPtr<IPin> pPinTo;
		if(S_OK == IsPinDirection(pPin, PINDIR_OUTPUT)
		&& SUCCEEDED(pPin->ConnectedTo(&pPinTo)) && pPinTo)
		{
			if(CComPtr<IBaseFilter> pBF = GetFilterFromPin(pPinTo))
			{
				NukeDownstream(pBF);
				Disconnect(pPinTo);
				Disconnect(pPin);
				RemoveFilter(pBF);
			}
		}
	}
	else
	{
		return E_INVALIDARG;
	}

	return S_OK;
}

STDMETHODIMP CFGManager::FindInterface(REFIID iid, void** ppv, BOOL bRemove)
{
	CAutoLock cAutoLock(this);

	CheckPointer(ppv, E_POINTER);

	for(POSITION pos = m_pUnks.GetHeadPosition(); pos; m_pUnks.GetNext(pos))
	{
		if(SUCCEEDED(m_pUnks.GetAt(pos)->QueryInterface(iid, ppv)))
		{
			if(bRemove) m_pUnks.RemoveAt(pos);
			return S_OK;
		}
	}

	return E_NOINTERFACE;
}

STDMETHODIMP CFGManager::AddToROT()
{
	CAutoLock cAutoLock(this);

    HRESULT hr;

	if(m_dwRegister) return S_FALSE;

    CComPtr<IRunningObjectTable> pROT;
	CComPtr<IMoniker> pMoniker;
	WCHAR wsz[256];
    swprintf(wsz, L"FilterGraph %08p pid %08x (MPC)", (DWORD_PTR)this, GetCurrentProcessId());
    if(SUCCEEDED(hr = GetRunningObjectTable(0, &pROT))
	&& SUCCEEDED(hr = CreateItemMoniker(L"!", wsz, &pMoniker)))
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, (IGraphBuilder2*)this, pMoniker, &m_dwRegister);

	return hr;
}

STDMETHODIMP CFGManager::RemoveFromROT()
{
	CAutoLock cAutoLock(this);

	HRESULT hr;

	if(!m_dwRegister) return S_FALSE;

	CComPtr<IRunningObjectTable> pROT;
    if(SUCCEEDED(hr = GetRunningObjectTable(0, &pROT))
	&& SUCCEEDED(hr = pROT->Revoke(m_dwRegister)))
		m_dwRegister = 0;

	return hr;
}

// IGraphBuilderDeadEnd

STDMETHODIMP_(size_t) CFGManager::GetCount()
{
	CAutoLock cAutoLock(this);

	return m_deadends.GetCount();
}

STDMETHODIMP CFGManager::GetDeadEnd(int iIndex, CAtlList<CStringW>& path, CAtlList<CMediaType>& mts)
{
	CAutoLock cAutoLock(this);

	if(iIndex < 0 || iIndex >= m_deadends.GetCount()) return E_FAIL;

	path.RemoveAll();
	mts.RemoveAll();

	POSITION pos = m_deadends[iIndex]->GetHeadPosition();
	while(pos)
	{
		const path_t& p = m_deadends[iIndex]->GetNext(pos);

		CStringW str;
		str.Format(L"%s::%s", p.filter, p.pin);
		path.AddTail(str);
	}

	mts.AddTailList(&m_deadends[iIndex]->mts);

	return S_OK;
}

//
// 	CFGManagerCustom
//

CFGManagerCustom::CFGManagerCustom(LPCTSTR pName, LPUNKNOWN pUnk, UINT src, UINT tra)
	: CFGManager(pName, pUnk)
{
	AppSettings& s = AfxGetAppSettings();
	s.bNoMoreDXVA = false;
	if(!s.bShouldUseGPUAcel() ){// || s.iDSVideoRendererType == VIDRNDT_DS_VMR7RENDERLESS
		s.DXVAFilters = 0;
	}else{
		s.DXVAFilters = ~0;
	}

	if(s.iSVPRenderType ){
		s.iDSVideoRendererType = VIDRNDT_DS_VMR9RENDERLESS;
		s.iRMVideoRendererType = VIDRNDT_RM_DX9;
		s.iQTVideoRendererType = VIDRNDT_QT_DX9;
		s.iAPSurfaceUsage = VIDRNDT_AP_TEXTURE3D;
	}else{// if(m_sgs_videorender == _T("DX7"))
		s.iSVPRenderType = 0; 
		//if(AfxGetMyApp()->IsVista())
			s.iDSVideoRendererType = VIDRNDT_DS_OLDRENDERER;
		//else
		//	s.iDSVideoRendererType = VIDRNDT_DS_OVERLAYMIXER;

		s.iRMVideoRendererType = VIDRNDT_RM_DEFAULT;
		s.iQTVideoRendererType = VIDRNDT_QT_DEFAULT;
	}
	
	CFGFilter* pFGF;

	// Source filters

	//if(src & SRC_SHOUTCAST)
	{
		pFGF = new CFGFilterInternal<CShoutcastSource>(_T("CShoutcastSource"), MERIT64_ABOVE_DSHOW);
		pFGF->m_protocols.AddTail(_T("http"));
		m_source.AddTail(pFGF);
	}

	{

		//pFGF = new CFGFilterInternal<CRarSource>(_T("CRarSource"), MERIT64_ABOVE_DSHOW);
		//pFGF->m_protocols.AddTail(_T("rar"));
		//m_source.AddTail(pFGF);
	}
	// if(src & SRC_UDP)
	{
		pFGF = new CFGFilterInternal<CUDPReader>(_T("CUDPReader"), MERIT64_ABOVE_DSHOW);
		pFGF->m_protocols.AddTail(_T("udp"));
		m_source.AddTail(pFGF);
	}

	//if(src & SRC_AVI)
	{
		pFGF = new CFGFilterInternal<CAviSourceFilter>(_T("CAviSourceFilter"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,4,,52494646,8,4,,41564920"));
		pFGF->m_chkbytes.AddTail(_T("0,4,,52494646,8,4,,41564958"));
		//pFGF->m_extensions.AddTail(_T(".vp6")); //not work in this way?
		m_source.AddTail(pFGF);
	}

	//if(src & SRC_MP4)
	{
		pFGF = new CFGFilterInternal<CMP4SourceFilter>(_T("CMP4SourceFilter"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("4,4,,66747970")); // ftyp
		pFGF->m_chkbytes.AddTail(_T("4,4,,6d6f6f76")); // moov
		pFGF->m_chkbytes.AddTail(_T("4,4,,6d646174")); // mdat
		pFGF->m_chkbytes.AddTail(_T("4,4,,736b6970")); // skip
		pFGF->m_chkbytes.AddTail(_T("4,12,ffffffff00000000ffffffff,77696465027fe3706d646174")); // wide ? mdat
		pFGF->m_chkbytes.AddTail(_T("3,3,,000001")); // raw mpeg4 video
		pFGF->m_extensions.AddTail(_T(".mov"));
		//pFGF->m_extensions.AddTail(_T(".vp6")); 
		m_source.AddTail(pFGF);
	}

	//if(src & SRC_FLV)
	{
		pFGF = new CFGFilterInternal<CFLVSourceFilter>(_T("CFLVSourceFilter"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,4,,464C5601")); // FLV (v1)
		m_source.AddTail(pFGF);
	}

	//if(src & SRC_MATROSKA)
	{
		pFGF = new CFGFilterInternal<CMatroskaSourceFilter>(_T("CMatroskaSourceFilter"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,4,,1A45DFA3"));
		m_source.AddTail(pFGF);
	}

	if(src & SRC_REALMEDIA)
	{
		pFGF = new CFGFilterInternal<CRealMediaSourceFilter>(_T("CRealMediaSourceFilter"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,4,,2E524D46"));
		m_source.AddTail(pFGF);
	}

	if(src & SRC_DSM)
	{
		pFGF = new CFGFilterInternal<CDSMSourceFilter>(_T("CDSMSourceFilter"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,4,,44534D53"));
		m_source.AddTail(pFGF);
	}

	if(src & SRC_FLIC)
	{
		pFGF = new CFGFilterInternal<CFLICSource>(_T("CFLICSource"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("4,2,,11AF"));
		pFGF->m_chkbytes.AddTail(_T("4,2,,12AF"));
		pFGF->m_extensions.AddTail(_T(".fli"));
		pFGF->m_extensions.AddTail(_T(".flc"));
		m_source.AddTail(pFGF);
	}
	//if(src & SRC_FLAC)
	{
		pFGF = new CFGFilterInternal<CFlacSource>(_T("CFlacSource"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,4,,664C6143"));
		pFGF->m_extensions.AddTail(_T(".flac"));
		m_source.AddTail(pFGF);
	}
	if(src & SRC_CDDA)
	{
		pFGF = new CFGFilterInternal<CCDDAReader>(_T("CCDDAReader"), MERIT64_ABOVE_DSHOW);
		pFGF->m_extensions.AddTail(_T(".cda"));
		m_source.AddTail(pFGF);
	}

	if(src & SRC_CDXA)
	{
		pFGF = new CFGFilterInternal<CCDXAReader>(_T("CCDXAReader"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,4,,52494646,8,4,,43445841"));
		m_source.AddTail(pFGF);
	}

	if(src & SRC_VTS)
	{
		pFGF = new CFGFilterInternal<CVTSReader>(_T("CVTSReader"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,12,,445644564944454F2D565453"));
		m_source.AddTail(pFGF);
	}

	__if_exists(CD2VSource)
	{
	if(src & SRC_D2V)
	{
		pFGF = new CFGFilterInternal<CD2VSource>(_T("CD2VSource"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,18,,4456443241564950726F6A65637446696C65"));
		pFGF->m_extensions.AddTail(_T(".d2v"));
		m_source.AddTail(pFGF);
	}
	}

	__if_exists(CRadGtSourceFilter)
	{
	if(src & SRC_RADGT)
	{
		pFGF = new CFGFilterInternal<CRadGtSourceFilter>(_T("CRadGtSourceFilter"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,3,,534D4B"));
		pFGF->m_chkbytes.AddTail(_T("0,3,,42494B"));
		pFGF->m_extensions.AddTail(_T(".smk"));
		pFGF->m_extensions.AddTail(_T(".bik"));
		m_source.AddTail(pFGF);
	}
	}

	if(src & SRC_ROQ)
	{
		pFGF = new CFGFilterInternal<CRoQSourceFilter>(_T("CRoQSourceFilter"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,8,,8410FFFFFFFF1E00"));
		m_source.AddTail(pFGF);
	}

	if(src & SRC_OGG)
	{
		pFGF = new CFGFilterInternal<COggSourceFilter>(_T("COggSourceFilter"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,4,,4F676753"));
		m_source.AddTail(pFGF);
	}

	__if_exists(CEASourceFilter)
	{
		pFGF = new CFGFilterInternal<CEASourceFilter>(_T("CEASourceFilter"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,4,,4D566864"));
		//.vp6 file
		//TODO there are more chkbytes in this kind of file
		pFGF->m_extensions.AddTail(_T(".vp6"));
		m_source.AddTail(pFGF);

	}
	
	//__if_exists(CNutSourceFilter)
	{
	if(src & SRC_NUT)
	{
		pFGF = new CFGFilterInternal<CNutSourceFilter>(_T("CNutSourceFilter"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,8,,F9526A624E55544D"));
		m_source.AddTail(pFGF);
	}
	}

	//__if_exists(CDiracSourceFilter)
	{
	if(src & SRC_DIRAC)
	{
		pFGF = new CFGFilterInternal<CDiracSourceFilter>(_T("CDiracSourceFilter"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,8,,4B572D4449524143"));
		m_source.AddTail(pFGF);
	}
	}

	//if(src & SRC_MPEG )
	{
		pFGF = new CFGFilterInternal<CMpegSourceFilter>(_T("CMpegSourceFilter"), MERIT64_ABOVE_DSHOW );
		pFGF->m_chkbytes.AddTail(_T("0,16,FFFFFFFFF100010001800001FFFFFFFF,000001BA2100010001800001000001BB"));
		pFGF->m_chkbytes.AddTail(_T("0,5,FFFFFFFFC0,000001BA40"));
		pFGF->m_chkbytes.AddTail(_T("0,1,,47,188,1,,47,376,1,,47")); //some file cant play for this
		pFGF->m_chkbytes.AddTail(_T("4,1,,47,196,1,,47,388,1,,47"));
		pFGF->m_chkbytes.AddTail(_T("0,4,,54467263,1660,1,,47"));
		pFGF->m_chkbytes.AddTail(_T("0,8,fffffc00ffe00000,4156000055000000"));
		pFGF->m_chkbytes.AddTail(_T("0,8,,4D504C5330323030"));	// MPLS0200
		pFGF->m_chkbytes.AddTail(_T("0,8,,4D504C5330313030"));	// MPLS0100
		//pFGF->m_extensions.AddTail(_T(".ts"));
		//pFGF->m_extensions.AddTail(_T(".m2ts"));
		m_source.AddTail(pFGF );
	}



	if(src & SRC_DTSAC3)
	{
		pFGF = new CFGFilterInternal<CDTSAC3Source>(_T("CDTSAC3Source"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,4,,7FFE8001"));
		pFGF->m_chkbytes.AddTail(_T("0,2,,0B77"));
		pFGF->m_chkbytes.AddTail(_T("0,2,,770B"));
		pFGF->m_extensions.AddTail(_T(".ac3"));
		pFGF->m_extensions.AddTail(_T(".dts"));
		m_source.AddTail(pFGF);
	}

	if(src & SRC_MPA)
	{
		pFGF = new CFGFilterInternal<CMpaSourceFilter>(_T("CMpaSourceFilter"), MERIT64_ABOVE_DSHOW);
		pFGF->m_chkbytes.AddTail(_T("0,2,FFE0,FFE0"));
		pFGF->m_chkbytes.AddTail(_T("0,10,FFFFFF00000080808080,49443300000000000000"));
		m_source.AddTail(pFGF);
	}


#if GDCLWMVFILTER
    /*
    pFGF = new CFGFilterInternal<WMFDemuxFilter>(_T("CWMVSourceFilter"), MERIT64_ABOVE_DSHOW+100);
    //pFGF->m_chkbytes.AddTail(_T("0,4,,3026B275"));
    //pFGF->m_chkbytes.AddTail(_T("0,4,,D129E2D6"));		
    //m_source.AddTail(pFGF);
    pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_ASF);
    pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_NULL);
    pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_NULL);
    m_transform.AddTail(pFGF);


    if(1){
        pFGF = new CFGFilterRegistry(CLSID_AsyncReader, MERIT64_ABOVE_DSHOW+1);//MERIT64_ABOVE_DSHOW
        pFGF->m_chkbytes.AddTail(_T("0,4,,3026B275"));
        pFGF->m_chkbytes.AddTail(_T("0,4,,D129E2D6"));		
        m_source.AddTail(pFGF);
    }
    */
#else
   
   if(0) {
        pFGF = new CFGFilterInternal<CWMVSourceFilter>(_T("CWMVSourceFilter"), MERIT64_ABOVE_DSHOW+100);
        pFGF->m_chkbytes.AddTail(_T("0,4,,3026B275"));
        pFGF->m_chkbytes.AddTail(_T("0,4,,D129E2D6"));		
        m_source.AddTail(pFGF);
    }
    
#endif

 
	//if(AfxGetAppSettings().fUseWMASFReader)
	{
		pFGF = new CFGFilterRegistry(CLSID_WMAsfReader, MERIT64_ABOVE_DSHOW);//MERIT64_ABOVE_DSHOW
		pFGF->m_chkbytes.AddTail(_T("0,4,,3026B275"));
		pFGF->m_chkbytes.AddTail(_T("0,4,,D129E2D6"));		
		m_source.AddTail(pFGF);
	}

	// Transform filters

	/* this is useless
	pFGF = new CFGFilterInternal<CAVI2AC3Filter>(L"AVI<->AC3/DTS", MERIT64(0x00680000)+1);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_WAVE_DOLBY_AC3);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_WAVE_DTS);
	m_transform.AddTail(pFGF);
*/

	if(src & SRC_MATROSKA) {
		pFGF = new CFGFilterInternal<CMatroskaSplitterFilter>(L"Matroska Splitter", MERIT64_ABOVE_DSHOW);
	} else {
		pFGF = new CFGFilterInternal<CMatroskaSplitterFilter>(L"Matroska Splitter (low merit)", MERIT64_UNLIKELY);
	}
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_Matroska);
	pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
	m_transform.AddTail(pFGF);

	if(src & SRC_REALMEDIA)	{
		pFGF = new CFGFilterInternal<CRealMediaSplitterFilter>(L"MPC RealMedia Splitter", MERIT64_ABOVE_DSHOW);
	} else {
		pFGF = new CFGFilterInternal<CRealMediaSplitterFilter>(L"MPC RealMedia Splitter (low merit)", MERIT64_UNLIKELY);
	}
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_RealMedia);
	pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
	m_transform.AddTail(pFGF);
	
	if(src & SRC_AVI)	{
		pFGF = new CFGFilterInternal<CAviSplitterFilter>(L"Avi Splitter", MERIT64_ABOVE_DSHOW);
	} else {
		pFGF = new CFGFilterInternal<CAviSplitterFilter>(L"Avi Splitter (low merit)", MERIT64_UNLIKELY);
	}
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_Avi);
	pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
	m_transform.AddTail(pFGF);

	__if_exists(CRadGtSplitterFilter)
	{
	if(src & SRC_RADGT)	{
		pFGF = new CFGFilterInternal<CRadGtSplitterFilter>(L"RadGt Splitter", MERIT64_ABOVE_DSHOW);
	} else {
		pFGF = new CFGFilterInternal<CRadGtSplitterFilter>(L"RadGt Splitter (low merit)", MERIT64_UNLIKELY);
	}
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_Bink);
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_Smacker);
	pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
	m_transform.AddTail(pFGF);
	}

	if(src & SRC_ROQ)	{
		pFGF = new CFGFilterInternal<CRoQSplitterFilter>(L"RoQ Splitter", MERIT64_ABOVE_DSHOW);
	} else {
		pFGF = new CFGFilterInternal<CRoQSplitterFilter>(L"RoQ Splitter (low merit)", MERIT64_UNLIKELY);
	}
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_RoQ);
	pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
	m_transform.AddTail(pFGF);

	if(src & SRC_OGG)	{
		pFGF = new CFGFilterInternal<COggSplitterFilter>(L"Ogg Splitter", MERIT64_ABOVE_DSHOW);
	} else {
		pFGF = new CFGFilterInternal<COggSplitterFilter>(L"Ogg Splitter (low merit)", MERIT64_UNLIKELY);
	}
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_Ogg);
	pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
	m_transform.AddTail(pFGF);

	__if_exists(CNutSplitterFilter)
	{
	if(src & SRC_NUT)	{
		pFGF = new CFGFilterInternal<CNutSplitterFilter>(L"Nut Splitter", MERIT64_ABOVE_DSHOW);
	} else {
		pFGF = new CFGFilterInternal<CNutSplitterFilter>(L"Nut Splitter (low merit)", MERIT64_UNLIKELY);
	}
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_Nut);
	pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
	m_transform.AddTail(pFGF);
	}

 	
 		//if(src & SRC_MPEG) {
 	 		pFGF = new CFGFilterInternal<CMpegSplitterFilter>(L"Mpeg 分离器", MERIT64_ABOVE_DSHOW + 1000);
 	 	//} else {
 	 	//	pFGF = new CFGFilterInternal<CMpegSplitterFilter>(L"Mpeg 分离器 (备用方案)", MERIT64_UNLIKELY);
 	 	//}
 	 	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_MPEG1System);
 	 	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_MPEG2_PROGRAM);
 	 	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_MPEG2_PVA);
 		pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_MPEG2_TRANSPORT);
 		pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
 	 	m_transform.AddTail(pFGF);
/*
 	

	pFGF = new CFGFilterInternal<CMpegSplitterFilter>(L"Mpeg Splitter (Plan-B)", MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_MPEG1System);
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_MPEG2_PROGRAM);
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_MPEG2_PVA);
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_MPEG2_TRANSPORT);
	pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
	m_transform.AddTail(pFGF);
*/

	__if_exists(CDiracSplitterFilter)
	{
	if(src & SRC_DIRAC)	{
		pFGF = new CFGFilterInternal<CDiracSplitterFilter>(L"Dirac Splitter", MERIT64_ABOVE_DSHOW);
	} else {
		pFGF = new CFGFilterInternal<CDiracSplitterFilter>(L"Dirac Splitter (low merit)", MERIT64_UNLIKELY);
	}
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_Dirac);
	pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
	m_transform.AddTail(pFGF);
	}

	if(src & SRC_MPA) {
		pFGF = new CFGFilterInternal<CMpaSplitterFilter>(L"Mpa Splitter", MERIT64_ABOVE_DSHOW);
	} else {
		pFGF = new CFGFilterInternal<CMpaSplitterFilter>(L"Mpa Splitter (low merit)", MERIT64_UNLIKELY);
	}
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_MPEG1Audio);
	pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
	m_transform.AddTail(pFGF);

	
	if(src & SRC_DSM) {
		pFGF = new CFGFilterInternal<CDSMSplitterFilter>(L"DSM Splitter", MERIT64_ABOVE_DSHOW);
	} else {
		pFGF = new CFGFilterInternal<CDSMSplitterFilter>(L"DSM Splitter (low merit)", MERIT64_UNLIKELY);
	}
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_DirectShowMedia);
	pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
	m_transform.AddTail(pFGF);

	if(src & SRC_MP4)	{
		pFGF = new CFGFilterInternal<CMP4SplitterFilter>(L"MP4 Splitter", MERIT64_ABOVE_DSHOW);
	} else {
		pFGF = new CFGFilterInternal<CMP4SplitterFilter>(L"MP4 Splitter (low merit)", MERIT64_UNLIKELY);
	}
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_MP4);
	pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
	m_transform.AddTail(pFGF);

	if(src & SRC_FLV) {
		pFGF = new CFGFilterInternal<CFLVSplitterFilter>(L"FLV Splitter", MERIT64_ABOVE_DSHOW);
	} else {
		pFGF = new CFGFilterInternal<CFLVSplitterFilter>(L"FLV Splitter (low merit)", MERIT64_UNLIKELY);
	}
	pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_FLV);
	pFGF->AddType(MEDIATYPE_Stream, GUID_NULL);
	m_transform.AddTail(pFGF);

/*

    pFGF = new CFGFilterInternal<CMpeg2DecFilter>(
        (tra & TRA_MPEG2) ? L"MPEG-2 Video Decoder 422" : L"MPEG-2 Video Decoder (low merit)", 
        (tra & TRA_MPEG2) ? MERIT64_ABOVE_DSHOW + 10 : MERIT64_UNLIKELY);
    pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_MPEG2_VIDEO);
    pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_MPEG2_VIDEO);
    pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_MPEG2_VIDEO);
    pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPEG2_VIDEO);
    //pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MMES);
    m_transform.AddTail(pFGF);



    //Microsoft MPEG-2 Video Decoder support DXVA but not 422 :(
    pFGF = new CFGFilterRegistry(CLSID_CMPEG2VidDecoderDS, MERIT64_ABOVE_DSHOW+5);
    pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_MPEG2_VIDEO);
    pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_MPEG2_VIDEO);
    pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_MPEG2_VIDEO);
    pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPEG2_VIDEO);
    m_transform.AddTail(pFGF);	
*/

	pFGF = new CFGFilterInternal<CMpeg2DecFilter>(
		(s.fVMDetected) ? L"MPEG-1 Video Decoder" : L"MPEG-1 Video Decoder (low merit)", 
		(s.fVMDetected) ? MERIT64_ABOVE_DSHOW : MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPEG1Packet);
	//pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPEG1Video);
	//pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPEG1VideoCD);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPEG1Payload);
	m_transform.AddTail(pFGF);

	pFGF = new CFGFilterInternal<CMpeg2DecFilter>(
		(tra & TRA_MPEG2) ? L"MPEG-2 Video Decoder" : L"MPEG-2 Video Decoder (low merit)", 
		(tra & TRA_MPEG2) ? MERIT64_ABOVE_DSHOW : MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_MPEG2_VIDEO);
	pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_MPEG2_VIDEO);
	pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_MPEG2_VIDEO);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPEG2_VIDEO);
	//pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MMES);
	m_transform.AddTail(pFGF);

	
	/*
pFGF = new CFGFilterInternal<CMpaDecFilter>( L"MPC WMA Audio Decoder", MERIT64_ABOVE_DSHOW);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_WMA1);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_WMA2);
	m_transform.AddTail(pFGF);
*/

		
	pFGF = new CFGFilterInternal<CMpaDecFilter>( L"PCM RAW Audio Decoder", MERIT64_ABOVE_DSHOW);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_PCM_RAW);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_PCM_SOWT);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_PCM_TWOS);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_PCM_ULAW);
	m_transform.AddTail(pFGF);

	

	pFGF = new CFGFilterInternal<CMpaDecFilter>(
		(tra & TRA_MPA) ? L"MPEG-1 Audio Decoder" : L"MPEG-1 Audio Decoder (low merit)",
		(tra & TRA_MPA) ? MERIT64_ABOVE_DSHOW : MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_MPEG1AudioPayload);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_MPEG1Payload);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_MPEG1Packet);
	m_transform.AddTail(pFGF);

	pFGF = new CFGFilterInternal<CMpaDecFilter>(L"IMA Decoder", MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_IMA4);
	m_transform.AddTail(pFGF);

	pFGF = new CFGFilterInternal<CMpaDecFilter>(
		L"AMR Audio Decoder" ,MERIT64_ABOVE_DSHOW );
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_SAMR);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_AMR);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_SAWB);
	m_transform.AddTail(pFGF);

	pFGF = new CFGFilterInternal<CMpaDecFilter>( L"MP3 Audio Decoder"  , MERIT64_ABOVE_DSHOW); //MERIT64_UNLIKELY   MERIT64_ABOVE_DSHOW
		pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_MP3); // Some MP3 just cant handler by MPA, maybe because of sample rate
	m_transform.AddTail(pFGF);

	pFGF = new CFGFilterInternal<CMpaDecFilter>(
		(tra & TRA_MPA) ? L"MPEG-2 Audio Decoder" : L"MPEG-2 Audio Decoder (low merit)",
		(tra & TRA_MPA) ? MERIT64_ABOVE_DSHOW : MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_MPEG2_AUDIO);
	pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_MPEG2_AUDIO);
	pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_MPEG2_AUDIO);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_MPEG2_AUDIO);
	m_transform.AddTail(pFGF);

	pFGF = new CFGFilterInternal<CMpaDecFilter>(
		(tra & TRA_LPCM) ? L"LPCM Audio Decoder" : L"LPCM Audio Decoder (low merit)",
		(tra & TRA_LPCM) ? MERIT64_ABOVE_DSHOW : MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_DVD_LPCM_AUDIO);
	pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_DVD_LPCM_AUDIO);
	pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_DVD_LPCM_AUDIO);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_DVD_LPCM_AUDIO);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_HDMV_LPCM_AUDIO);
	m_transform.AddTail(pFGF);

	if(s.fbUseSPDIF){
		pFGF = new CFGFilterInternal<CMpaDecFilter>(
			(tra & TRA_AC3) ? L"AC3 Audio Decoder SPDIF" : L"AC3 Audio Decoder SPDIF (low merit)",
			(tra & TRA_AC3) ? (MERIT64_ABOVE_DSHOW+1) : MERIT64_UNLIKELY);
		pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_DOLBY_AC3);
		pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_DOLBY_AC3);
		pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_DOLBY_AC3);
		pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_DOLBY_AC3);
		pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_WAVE_DOLBY_AC3);
		pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_DOLBY_EAC3);
		m_transform.AddTail(pFGF);

		pFGF = new CFGFilterInternal<CMpaDecFilter>(
			(tra & TRA_DTS) ? L"DTS Decoder SPDIF" : L"DTS Decoder  SPDIF(low merit)",
			(tra & TRA_DTS) ? ( MERIT64_ABOVE_DSHOW + 1) : MERIT64_UNLIKELY);
		pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_DTS);
		pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_DTS);
		pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_DTS);
		pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_DTS);
		pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_WAVE_DTS);
		m_transform.AddTail(pFGF);
	}
	pFGF = new CFGFilterInternal<CMpaDecFilter>(
		(tra & TRA_AC3) ? L"AC3 Audio Decoder" : L"AC3 Audio Decoder (low merit)",
		(tra & TRA_AC3) ? MERIT64_ABOVE_DSHOW : MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_DOLBY_AC3);
	pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_DOLBY_AC3);
	pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_DOLBY_AC3);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_DOLBY_AC3);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_WAVE_DOLBY_AC3);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_DOLBY_EAC3);
	m_transform.AddTail(pFGF);

	pFGF = new CFGFilterInternal<CMpaDecFilter>(
		(tra & TRA_DTS) ? L"DTS Decoder" : L"DTS Decoder (low merit)",
		(tra & TRA_DTS) ? MERIT64_ABOVE_DSHOW : MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_DTS);
	pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_DTS);
	pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_DTS);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_DTS);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_WAVE_DTS);
	m_transform.AddTail(pFGF);

	pFGF = new CFGFilterInternal<CMpaDecFilter>(
		(tra & TRA_AAC) ? L"AAC Decoder" : L"AAC Decoder (low merit)",
		(tra & TRA_AAC) ? MERIT64_ABOVE_DSHOW+1 : MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_AAC);
	pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_AAC);
	pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_AAC);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_AAC);
	pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_MP4A);
	pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_MP4A);
	pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_MP4A);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_MP4A);
	pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_mp4a);
	pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_mp4a);
	pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_mp4a);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_mp4a);
	m_transform.AddTail(pFGF);

	pFGF = new CFGFilterInternal<CMpaDecFilter>(
		(tra & TRA_PS2AUD) ? L"PS2 Audio Decoder" : L"PS2 Audio Decoder (low merit)",
		(tra & TRA_PS2AUD) ? MERIT64_ABOVE_DSHOW : MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_PS2_PCM);
	pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_PS2_PCM);
	pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_PS2_PCM);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_PS2_PCM);
	m_transform.AddTail(pFGF);

#if 0
	pFGF = new CFGFilterInternal<CMpaDecFilter>(
		  L"SVP Real Audio Decoder 2.0" ,
		 MERIT64_ABOVE_DSHOW+10);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_COOK);
    pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_SIPR);
	m_transform.AddTail(pFGF);
#endif

	pFGF = new CFGFilterInternal<CMpaDecFilter>(
		(tra & TRA_FLAC) ? L"Flac Audio Decoder" : L"Flac Audio Decoder (low merit)",		// TODO : put in resource !
		(tra & TRA_FLAC) ? MERIT64_ABOVE_DSHOW : MERIT64_DO_USE);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_FLAC_FRAMED);
    pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_F1AC_FLAC);
	m_transform.AddTail(pFGF);

	pFGF = new CFGFilterInternal<CMpaDecFilter>(
		(tra & TRA_VORBIS) ? L"Vorbis Audio Decoder" : L"Vorbis Audio Decoder (low merit)",
		(tra & TRA_VORBIS) ? MERIT64_ABOVE_DSHOW : MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_Vorbis2);
	m_transform.AddTail(pFGF);

	pFGF = new CFGFilterInternal<CMpaDecFilter>(
		(tra & TRA_NELLY) ? L"Nellymoser Audio Decoder" : L"Nellymoser Audio Decoder (low merit)",		// TODO : put in resource !
		(tra & TRA_NELLY) ? MERIT64_ABOVE_DSHOW : MERIT64_DO_USE);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_NELLYMOSER);
	m_transform.AddTail(pFGF);

	pFGF = new CFGFilterInternal<CRoQVideoDecoder>(
		(tra & TRA_RV) ? L"RoQ Video Decoder" : L"RoQ Video Decoder (low merit)",
		(tra & TRA_RV) ? MERIT64_ABOVE_DSHOW : MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RoQV);
	m_transform.AddTail(pFGF);

	pFGF = new CFGFilterInternal<CRoQAudioDecoder>(
		(tra & TRA_RA) ? L"RoQ Audio Decoder" : L"RoQ Audio Decoder (low merit)",
		(tra & TRA_RA) ? MERIT64_ABOVE_DSHOW : MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_RoQA);
	m_transform.AddTail(pFGF);

	__if_exists(CDiracVideoDecoder)
	{
	pFGF = new CFGFilterInternal<CDiracVideoDecoder>(
		(tra & TRA_DIRAC) ? L"Dirac Video Decoder" : L"Dirac Video Decoder (low merit)",
		(tra & TRA_DIRAC) ? MERIT64_ABOVE_DSHOW : MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DiracVideo);
	m_transform.AddTail(pFGF);
	}
    __if_exists(VP8DecoderLib::Filter)
    {
        pFGF = new CFGFilterInternal<VP8DecoderLib::Filter>(
            L"VP8 Video Decoder" , MERIT64_ABOVE_DSHOW );
        pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP80);
        m_transform.AddTail(pFGF);
    }
    
	pFGF = new CFGFilterInternal<CNullTextRenderer>(L"NullTextRenderer", MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_Text, MEDIASUBTYPE_NULL);
	pFGF->AddType(MEDIATYPE_ScriptCommand, MEDIASUBTYPE_NULL);
	pFGF->AddType(MEDIATYPE_Subtitle, MEDIASUBTYPE_NULL);
	pFGF->AddType(MEDIATYPE_Text, MEDIASUBTYPE_NULL);
	pFGF->AddType(MEDIATYPE_NULL, MEDIASUBTYPE_DVD_SUBPICTURE);
	pFGF->AddType(MEDIATYPE_NULL, MEDIASUBTYPE_CVD_SUBPICTURE);
	pFGF->AddType(MEDIATYPE_NULL, MEDIASUBTYPE_SVCD_SUBPICTURE);
	m_transform.AddTail(pFGF);

	__if_exists(CFLVVideoDecoder)
	{
	pFGF = new CFGFilterInternal<CFLVVideoDecoder>(L"FLV4 Video Decoder (low merit)",MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_FLV4);
	m_transform.AddTail(pFGF);
	}	


#if 1
    pFGF = new CFGFilterInternal<CRealAudioDecoder>(
#ifndef RA_FFMPEG
        L"MPC RealAudio Decoder" ,
#else
        L"SVP Real Audio Decoder 2.0",
#endif		 
        MERIT64_ABOVE_DSHOW );
    pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_ATRC);
    pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_14_4);
    pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_28_8);
    pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_COOK);
    pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_DNET);
    pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_SIPR);
    pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_RAAC);
    m_transform.AddTail(pFGF);
#endif	

#if 1
	pFGF = new CFGFilterInternal<CMPCVideoDecFilter>(
		L"SVP RealVideo Decoder 2.0",
		MERIT64_NORMAL -1 );//MERIT64_ABOVE_DSHOW
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RV10);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RV20);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RV30);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RV40);
	m_transform.AddTail(pFGF);

#endif

#if 0
    pFGF = new CFGFilterInternal<CRealVideoDecoder>(
        L"MPC RealVideo Decoder",
        MERIT64_ABOVE_DSHOW + 5);
    pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RV10);
    pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RV20);
    pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RV30);
    pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RV40);
    m_transform.AddTail(pFGF);
#endif

#ifdef INTERNALVP8
    pFGF = new CFGFilterInternal<CMPCVideoDecFilter>(_T("SVP VP8 Video Decoder"), MERIT64_UNLIKELY);
    pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP80);
    m_transform.AddTail(pFGF);
#endif

	pFGF = new CFGFilterInternal<CMPCVideoDecFilter>(_T("MJPEG Video Decoder"), MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_QTJpeg);
	m_transform.AddTail(pFGF);

    pFGF = new CFGFilterInternal<CMPCVideoDecFilter>(_T("SVP RLE Video Decoder"), MERIT64_UNLIKELY);
    pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_QTRle);
	m_transform.AddTail(pFGF);
	pFGF = new CFGFilterInternal<CMPCVideoDecFilter>(_T("MMES Video Decoder"), MERIT64_UNLIKELY);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MMES);
	m_transform.AddTail(pFGF);

	// High merit MPC Video Decoder
	UINT dxva_filters = s.DXVAFilters;
	UINT ffmpeg_filters = s.FFmpegFilters;

	//pFGF = new CFGFilterInternal<CMPCVideoDecFilter>(L"MPEG-1 FF Video Decoder" , MERIT64_ABOVE_DSHOW);
	//pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPEG1Packet);
	//pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPEG1Payload);
	//m_transform.AddTail(pFGF);

	if ( s.bShouldUseGPUAcel() && !s.bIsIVM)
	{
		UINT64 gMerit =  MERIT64_ABOVE_DSHOW+100;
		if(s.useGPUCUDA){gMerit =  MERIT64_ABOVE_DSHOW+5;}
		pFGF = new CFGFilterInternal<CMPCVideoDecFilter>(_T("MPC Video Decoder DXVA"), gMerit);
		if(dxva_filters & MPCDXVA_H264){
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_H264);
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_h264);
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_X264);
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_x264);
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VSSH);
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vssh);
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DAVC);
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_davc);
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_PAVC);
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_pavc);
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_AVC1);
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_avc1);
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_H264_bis);
/*
            pFGF->AddType(MEDIATYPE_DVD_ENCRYPTED_PACK, MEDIASUBTYPE_MPEG2_VIDEO);
            pFGF->AddType(MEDIATYPE_MPEG2_PACK, MEDIASUBTYPE_MPEG2_VIDEO);
            pFGF->AddType(MEDIATYPE_MPEG2_PES, MEDIASUBTYPE_MPEG2_VIDEO);
            pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPEG2_VIDEO);
            */
		}
		if(dxva_filters & MPCDXVA_VC1){
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_WVC1);
			pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_wvc1);

           
		}
		m_transform.AddTail(pFGF);
	}
#if INCLUDE_MPC_VIDEO_DECODER | INCLUDE_MPC_DXVA_VIDEO_DECODER

#if INTERNAL_DECODER_WMV
	//if (ffmpeg_filters & FFM_WMV )
	{
		UINT64 gMerit = MERIT64_NORMAL;
		if(s.useFFMPEGWMV)
			 gMerit = MERIT64_ABOVE_DSHOW+100;

		pFGF = new CFGFilterInternal<CMPCVideoDecFilter>(_T("WMV SVP Video Decoder"), gMerit);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_WMV1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_wmv1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_WMV2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_wmv2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_WMV3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_wmv3);
		m_transform.AddTail(pFGF);
	}
#endif

	UINT64 merit = MERIT64_ABOVE_DSHOW;
	if(s.optionDecoder == _T("internaldec") && !s.bShouldUseGPUAcel()){
		merit += 100;
	}
	pFGF = new CFGFilterInternal<CMPCVideoDecFilter>(_T("MPC Video Decoder"),  merit);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_tscc);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_HUFFYUV);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MJPG);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_CVID);
	//pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_QTSmc);
	
#if INTERNAL_DECODER_FLV
	if (ffmpeg_filters & FFM_FLV4)
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_FLV1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_flv1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_FLV4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_flv4);
	}
#endif
#if INTERNAL_DECODER_VP6
	if (ffmpeg_filters & FFM_VP62 && 0)
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP50);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vp50);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP60);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vp60);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP61);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vp61);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP62);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vp62);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP6F);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vp6f);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP6A);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vp6a);
	}
#endif
#if INTERNAL_DECODER_H264 | INTERNAL_DECODER_H264_DXVA
	if ((ffmpeg_filters & FFM_H264) || (dxva_filters & MPCDXVA_H264))
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_H264);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_h264);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_X264);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_x264);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VSSH);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vssh);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DAVC);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_davc);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_PAVC);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_pavc);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_AVC1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_avc1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_H264_bis);
	}
#endif
#if INTERNAL_DECODER_VC1 | INTERNAL_DECODER_VC1_DXVA
	if ((ffmpeg_filters & FFM_VC1) || (dxva_filters & MPCDXVA_VC1))
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_WVC1); //不支持隔行VC1 :(
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_wvc1);
	}
#endif
#if INTERNAL_DECODER_XVID
	if (ffmpeg_filters & FFM_XVID)
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_XVID);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_xvid);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_XVIX);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_xvix);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MP4V);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mp4v);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_M4S2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_m4s2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MP4S);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mp4s);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_3IV1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_3iv1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_3IV2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_3iv2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_3IVX);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_3ivx);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_BLZ0);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_blz0);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DM4V);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_dm4v);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DXGM);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_dxgm);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_FMP4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_fmp4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_HDX4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_hdx4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_LMP4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_lmp4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_NDIG);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_ndig);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RMP4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_rmp4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_SMP4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_smp4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_SEDG);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_sedg);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_UMP4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_ump4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_WV1F);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_wv1f);
	}
#endif
#if 1 //INTERNAL_DECODER_DIVX
	//if (ffmpeg_filters & FFM_DIVX)
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIVX);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_divx);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DX50);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_dx50);
	}
#endif

#if INTERNAL_DECODER_MSMPEG4
	if (ffmpeg_filters & FFM_MSMPEG4)
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIV3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_div3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DVX3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_dvx3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MP43);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mp43);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_COL1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_col1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIV4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_div4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIV5);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_div5);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIV6);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_div6);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_AP41);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_ap41);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPG3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mpg3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIV2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_div2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MP42);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mp42);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPG4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mpg4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIV1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_div1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MP41);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mp41);
	}
#endif
#if INTERNAL_DECODER_SVQ
	if (ffmpeg_filters & FFM_SVQ3)
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_SVQ3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_SVQ1);
	}
#endif
#if INTERNAL_DECODER_H263
	if (ffmpeg_filters & FFM_H263)
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_H263);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_h263);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_S263);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_s263);
	}
#endif
	
#if INTERNAL_DECODER_THEORA
	if (ffmpeg_filters & FFM_THEORA)
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_THEORA);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_theora);
	}
#endif
#if INTERNAL_DECODER_AMVV
	if (ffmpeg_filters & FFM_AMVV)
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_AMVV);
	}
#endif
	m_transform.AddTail(pFGF);

	// Low merit MPC Video Decoder
	pFGF = new CFGFilterInternal<CMPCVideoDecFilter>(_T("MPC Video Decoder (low merit)"),  MERIT64_UNLIKELY);
#if INTERNAL_DECODER_FLV
	if (!(ffmpeg_filters & FFM_FLV4))
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_FLV1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_flv1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_FLV4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_flv4);
	}
#endif

#if INTERNAL_DECODER_VP6
	if (!(ffmpeg_filters & FFM_VP62) || 1)
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP50);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vp50);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP60);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vp60);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP61);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vp61);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP62);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vp62);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP6F);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vp6f);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VP6A);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vp6a);
	}
#endif
#if INTERNAL_DECODER_H264 | INTERNAL_DECODER_H264_DXVA
	if (!(ffmpeg_filters & FFM_H264) && !(dxva_filters & MPCDXVA_H264))
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_H264);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_h264);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_X264);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_x264);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_VSSH);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_vssh);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DAVC);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_davc);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_PAVC);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_pavc);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_AVC1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_avc1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_H264_bis);
	}
	
#endif
	
#if INTERNAL_DECODER_VC1 | INTERNAL_DECODER_VC1_DXVA
	if (!(ffmpeg_filters & FFM_VC1) && !(dxva_filters & MPCDXVA_VC1))
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_WVC1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_wvc1);
	}
#endif
#if INTERNAL_DECODER_XVID
	if (!(ffmpeg_filters & FFM_XVID))
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_XVID);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_xvid);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_XVIX);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_xvix);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MP4V);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mp4v);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_M4S2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_m4s2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MP4S);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mp4s);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_3IV1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_3iv1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_3IV2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_3iv2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_3IVX);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_3ivx);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_BLZ0);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_blz0);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DM4V);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_dm4v);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DXGM);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_dxgm);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_FMP4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_fmp4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_HDX4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_hdx4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_LMP4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_lmp4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_NDIG);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_ndig);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RMP4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_rmp4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_SMP4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_smp4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_SEDG);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_sedg);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_UMP4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_ump4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_WV1F);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_wv1f);
	}
#endif
#if INTERNAL_DECODER_DIVX
	if (!(ffmpeg_filters & FFM_DIVX))
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIVX);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_divx);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DX50);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_dx50);
	}
#endif
#if INTERNAL_DECODER_WMV
	if (!(ffmpeg_filters & FFM_WMV))
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_WMV1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_wmv1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_WMV2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_wmv2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_WMV3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_wmv3);
	}
#endif
#if INTERNAL_DECODER_MSMPEG4
	if (!(ffmpeg_filters & FFM_MSMPEG4))
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIV3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_div3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DVX3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_dvx3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MP43);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mp43);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_COL1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_col1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIV4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_div4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIV5);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_div5);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIV6);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_div6);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_AP41);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_ap41);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPG3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mpg3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIV2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_div2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MP42);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mp42);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MPG4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mpg4);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_DIV1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_div1);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_MP41);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_mp41);

		
	}
#endif
#if INTERNAL_DECODER_SVQ
	if (!(ffmpeg_filters & FFM_SVQ3))
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_SVQ3);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_SVQ1);
	}
#endif
#if INTERNAL_DECODER_H263
	if (!(ffmpeg_filters & FFM_H263))
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_H263);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_h263);
	}
#endif
#if INTERNAL_DECODER_THEORA
	if (!(ffmpeg_filters & FFM_THEORA))
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_THEORA);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_theora);
	}
#endif
#if INTERNAL_DECODER_AMVV
	if (!(ffmpeg_filters & FFM_AMVV))
	{
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_AMVV);
	}
#endif
	m_transform.AddTail(pFGF);
#endif

	CMPCVideoDecFilter::FFmpegFilters = s.FFmpegFilters;
	CMPCVideoDecFilter::DXVAFilters = s.DXVAFilters;
	
	CMPCVideoDecFilter::m_ref_frame_count_check_skip = s.bDVXACompat ;
	
	CMPCVideoDecFilter::m_bUSERGB =  s.bRGBOnly;
//	CBaseVideoFilter::m_bUSERGB =  s.bRGBOnly;
	
	// Blocked filters

	// "Subtitle Mixer" makes an access violation around the 
	// 11-12th media type when enumerating them on its output.
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{00A95963-3BE5-48C0-AD9F-3356D67EA09D}")), MERIT64_DO_NOT_USE));

	// ISCR suxx
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{48025243-2D39-11CE-875D-00608CB78066}")), MERIT64_DO_NOT_USE));

	// Samsung's "mpeg-4 demultiplexor" can even open matroska files, amazing...
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{99EC0C72-4D1B-411B-AB1F-D561EE049D94}")), MERIT64_DO_NOT_USE));

	// LG Video Renderer (lgvid.ax) just crashes when trying to connect it
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{9F711C60-0668-11D0-94D4-0000C02BA972}")), MERIT64_DO_NOT_USE));

	// palm demuxer crashes (even crashes graphedit when dropping an .ac3 onto it)
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{BE2CF8A7-08CE-4A2C-9A25-FD726A999196}")), MERIT64_DO_NOT_USE));

	// MainConcept (Adobe2) MPEG Splitter crash on pmp if splitter not exist
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{25AD5720-4DE0-4CF8-952A-2AEF53AC4321}")), MERIT64_DO_NOT_USE));
	// DCDSPFilter (early versions crash mpc)
	{
		CRegKey key;

		TCHAR buff[256];
		ULONG len = sizeof(buff);
		memset(buff, 0, len);

		CString clsid = _T("{B38C58A0-1809-11D6-A458-EDAE78F1DF12}");

		if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("CLSID\\") + clsid + _T("\\InprocServer32"), KEY_READ)
		&& ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len)
		&& GetFileVersion(buff) < 0x0001000000030000ui64)
		{
			m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(clsid), MERIT64_DO_NOT_USE));
		}
	}
/*
	// NVIDIA Transport Demux crashed for someone, I could not reproduce it
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{735823C1-ACC4-11D3-85AC-006008376FB8}")), MERIT64_DO_NOT_USE));	
*/

	if(s.iDSVideoRendererType != VIDRNDT_DS_OVERLAYMIXER ){
		// {CD8743A1-3736-11d0-9E69-00C04FD7C15B}
		m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{CD8743A1-3736-11d0-9E69-00C04FD7C15B}")), MERIT64_DO_NOT_USE));
		//m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{95F57653-71ED-42BA-9131-986CA0C6514F}")), MERIT64_DO_NOT_USE)); //disable overlay
		//m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("F683B0C4-AF99-4B62-87B1-947C1075EF4F")) , MERIT64_DO_NOT_USE)); //SMV
	}

	// mainconcept color space converter
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{272D77A0-A852-4851-ADA4-9091FEAD4C86}")), MERIT64_DO_NOT_USE));
	
	
	// Block VSFilter when internal subtitle renderer will get used
//	if(s.fAutoloadSubtitles && s.fBlockVSFilter) {
		//if(s.iDSVideoRendererType == VIDRNDT_DS_VMR7RENDERLESS || s.iDSVideoRendererType == VIDRNDT_DS_VMR9RENDERLESS || s.iDSVideoRendererType == VIDRNDT_DS_DXR) {
			m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{9852A670-F845-491B-9BE6-EBD841B8A613}")), MERIT64_DO_NOT_USE));			
		//}
//	}
			
	
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{3D446B6F-71DE-4437-BE15-8CE47174340F}")), MERIT64_DO_NOT_USE)); //AC3Filter
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{04FE9017-F873-410E-871E-AB91661A4EF7}")), MERIT64_DO_NOT_USE)); //ffdshow video
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{0F40E1E5-4F79-4988-B1A9-CC98794E6B55}")), MERIT64_DO_NOT_USE)); //ffdshow audio
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{AA59CBFA-F731-49E9-BE78-08665F339EFC}")), MERIT64_DO_NOT_USE)); //Bicubic Video Resizer
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{9D2935C7-3D8B-4EF6-B0D1-C14064698794}")), MERIT64_DO_NOT_USE)); //DivXG400 ??

	

	CSVPToolBox svptoolbox;
	CStringArray szaExtFilterPaths;
	
	szaExtFilterPaths.RemoveAll();
	
    HKEY fKey;
    if(RegOpenKey(HKEY_LOCAL_MACHINE , _T("SOFTWARE\\CoreCodec\\CoreAVC Pro 2.x") , &fKey ) == ERROR_SUCCESS && s.bShouldUseGPUAcel()){
        //SVP_SetCoreAvcCUDA(true);
        //m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{09571A4B-F1FE-4C60-9760-DE6D310C7C31}")), MERIT64_ABOVE_DSHOW+651));
    }
   
    if(s.bUsePowerDVD)
        m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{C16541FF-49ED-4DEA-9126-862F57722E31}")), MERIT64_ABOVE_DSHOW+651));

	//if(!s.onlyUseInternalDec){
		/*
		if ( s.bUsePowerDVD && ( s.optionDecoder == _T("PDVDGPUdec") || s.optionDecoder.IsEmpty() ) && s.useGPUAcel && !s.useGPUCUDA && !IsVista() ) {
					//szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("codecs\\powerdvd\\CL264dec.ax")) );
					
					
					//m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{09571A4B-F1FE-4C60-9760-DE6D310C7C31}")), MERIT64_ABOVE_DSHOW+1)); //not use CoreAVC
							//use powerdvd
							//disable FLV MP4 for powerdvd because of bugs https://bbs.shooter.cn/viewthread.php?tid=264
							//pFGFR->RemoveType( MEDIATYPE_Video, MEDIASUBTYPE_avc1 ); 
							//pFGFR->RemoveType( MEDIATYPE_Video, MEDIASUBTYPE_AVC1 ); 
					m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{C16541FF-49ED-4DEA-9126-862F57722E31}")), MERIT64_ABOVE_DSHOW+2)); //PDVD8
							
					
					m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{3EAE534B-A98F-4452-8F2A-4BCA1CD4F319}")), MERIT64_ABOVE_DSHOW+4)); //PDVD9
					
				}	else*/
		
	
	bool cavcexist = 0;
	CString czvcPath = svptoolbox.GetPlayerPath(_T("codecs\\CoreAVCDecoder.ax"));
	if(svptoolbox.ifFileExist(czvcPath))
		cavcexist = 1;
	else{
		czvcPath = svptoolbox.GetPlayerPath(_T("codecs\\cavc.ax"));
		if(svptoolbox.ifFileExist(czvcPath)){
			cavcexist = 1;
		}
	}

	    if( ! s.fVMR9MixerMode  ) 
		{
			if( (s.bShouldUseGPUAcel() && s.bHasCUDAforCoreAVC) || (!s.bShouldUseGPUAcel() && !s.bDisableSoftCAVC && !s.bDisableSoftCAVCForce)){
				//if((s.useGPUAcel)){
					//SVP_ForbidenCoreAVCTrayIcon();
					//SVP_SetCoreAvcCUDA(true); 
				//}
				//cavc
				//szaExtFilterPaths.Add( czvcPath  );  //will crash without why
			}
			////VMR9 seems not work with coreplayer
	  		
  			//CFGFilter* pFGFR = new CFGFilterRegistry(GUIDFromCString(_T("{09571A4B-F1FE-4C60-9760-DE6D310C7C31}")), MERIT64_ABOVE_DSHOW+20); //use CoreAVC
			//szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("PMPSplitter.ax")) );
  		  	//m_transform.AddTail(pFGFR); 
  		  	
			m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{C16541FF-49ED-4DEA-9126-862F57722E31}")), MERIT64_UNLIKELY) ); //not use POWERDVD
	  		
	  		
		}
	//}
		//szaExtFilterPaths.Add(  svptoolbox.GetPlayerPath(_T("codecs\\DivXDecH264.ax")) );

		if (!s.bShouldUseGPUAcel() && !s.bDisableSoftCAVC ){// && ( s.bDisableSoftCAVCForce )|| !cavcexist
			CString cclPath = svptoolbox.GetPlayerPath(_T("dh264.ax"));
			if(svptoolbox.ifFileExist(cclPath))
				szaExtFilterPaths.Add( cclPath  );  //will crash without why
			
		}

    szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("codecs\\cavc2.ax")) );

	szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("PMPSplitter.ax")) );

	szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("ir41_32.ax")) );

    //if(s.szCurrentExtension == _T("webm")){
        szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("vp8decoder.dll")) );
    //}
	//szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("wmadmod.dll")) );
	
	//szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("rms.ax")) );
	
	//szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("NeSplitter.ax")) ); 

	if(s.bIsIVM){
		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("IVMSource.ax")) );
	}
	if(s.szCurrentExtension == _T(".csf")){
// 		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_mdssockc.dll")) );
// 		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_mxaudio.dll")) );
// 		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_mxvideo.dll")) );
// 		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_mxscreen.dll")) );
// 		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_mxshbasu.dll")) );
// 		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_mxshmaiu.dll")) );
// 		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_mxshsour.dll")) );
// 		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_mcucltu.dll")) );
// 		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_mcufilecu.dll")) );
// 		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_mtcontrol.dll")) );
 		//szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_mtcontain.dll")) );
 		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_mxsource.dll")) );
 		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_mxrender.dll")) );
// 		szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("csfcodec\\mpc_wtlvcl.dll")) );
	}
	

	szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("mmamrdmx.ax")) );

    //szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("CSMX.dll")) );

    szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("scmpack.dll")) );
	
    //szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("WMFDemux.dll")) );
    

	szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("rlapedec.ax")) ); 

	szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("RadGtSplitter.ax")) ); 

	m_source.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{564FD788-86C9-4444-971E-CC4A243DA150}")), MERIT64_ABOVE_DSHOW + 1200) );

	szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("haalis.ax")) ); 
	szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("ts.dll")) ); 
    szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("mp4.dll")) ); 
	szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("ogm.dll")) ); 

	szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("vp6dec.ax")) ); 

    //szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("rtsp.ax")) ); 

	//szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("svplayer.bin\\real\\rmoc3260.dll")) );
	//szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("svplayer.bin\\real\\Codecs\\rv40.dll")) );
	//szaExtFilterPaths.Add( svptoolbox.GetPlayerPath(_T("svplayer.bin\\real\\Codecs\\drvc.dll")) );
	
	for(int l = 0; l < szaExtFilterPaths.GetCount(); l++){
		CString szFPath = szaExtFilterPaths.GetAt(l); //以文件模式调入解码器
		CString szLog ; 
		if(svptoolbox.ifFileExist(szFPath)){
			SVP_LogMsg5(_T("Loading %s"), szFPath);
			CFilterMapper2 fm2(false);
			fm2.Register(szFPath);
			POSITION pos = fm2.m_filters.GetHeadPosition();
			while(pos){
				FilterOverride* fo = fm2.m_filters.GetNext(pos);
				if(fo->name == _T("RealAudio Decoder")){
					continue;
				}
				CFGFilter* pFGF = new CFGFilterFile(fo->clsid, fo->path, CStringW(fo->name), MERIT64_ABOVE_DSHOW + 10);
				szLog.Format(_T("Loading Filter %s %s %s "), CStringFromGUID(fo->clsid) ,fo->path, CStringW(fo->name) );
				SVP_LogMsg(szLog);
				if(pFGF){
                    
                    if(szFPath.Find(_T("WMFDemux.dll")) > 0){
                        pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_ASF);
                        pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_NULL);
                        pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_NULL);
                        m_transform.AddTail(pFGF);
                        /*pFGF->m_chkbytes.AddTail(_T("0,4,,3026B275"));
                        pFGF->m_chkbytes.AddTail(_T("0,4,,D129E2D6"));		
                        m_source.AddTail(pFGF);
                        */
                    }else if(szFPath.Find(_T("rtsp.ax")) > 0){
                        pFGF->m_protocols.AddTail(_T("rtsp"));
                        m_source.AddTail(pFGF);
                    }else if(szFPath.Find(_T("mpc_mxsource.dll")) > 0){
						pFGF->m_extensions.AddTail(_T(".csf"));
						m_source.AddTail(pFGF);
					}else if( szFPath.Find(_T("haalis.ax")) > 0 || szFPath.Find(_T("ts.dll")) > 0 || szFPath.Find(_T("ogm.dll")) > 0|| szFPath.Find(_T("mp4.dll")) > 0){ //useless
						pFGF->m_extensions.AddTail(_T(".ts"));
						pFGF->m_extensions.AddTail(_T(".m2ts"));
						pFGF->m_extensions.AddTail(_T(".tp"));
						pFGF->m_extensions.AddTail(_T(".ogg"));
                        pFGF->m_extensions.AddTail(_T(".ogv"));
						pFGF->m_extensions.AddTail(_T(".ogm"));
						pFGF->m_extensions.AddTail(_T(".mkv"));
						pFGF->m_extensions.AddTail(_T(".vob"));
                        pFGF->m_extensions.AddTail(_T(".mp4"));
						pFGF->SetTypes(fo->guids);
						CString szName(fo->name);
                        
						if(szName.Find(_T("Media Splitter")) >= 0)
							m_source.AddTail(pFGF);
						//else
						//	m_transform.AddTail(pFGF);
					}else	if(szFPath.Find(_T("IVMSource.ax")) > 0){
						pFGF->m_extensions.AddTail(_T(".ivm"));
						m_source.AddTail(pFGF);
					}else	if(szFPath.Find(_T("NeSplitter.ax")) > 0){
						pFGF->m_extensions.AddTail(_T(".ts"));
						pFGF->m_extensions.AddTail(_T(".m2ts"));
						m_source.AddTail(pFGF);
					}else if(szFPath.Find(_T("rlapedec.ax")) > 0){
						pFGF->m_extensions.AddTail(_T(".ape"));
						m_source.AddTail(pFGF);
/*
					}else if(szFPath.Find(_T("ir41_32.ax")) > 0){
						pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_IV41);
						pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_iv41);
						m_transform.AddTail(pFGF);
					}
/*
					}else if(szFPath.Find(_T("wmadmod.dll")) > 0){
						pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_WMA1);
						pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_WMA2);
						pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_WMAPRO);
						pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_WMALOSSLESS);
						m_transform.AddTail(pFGF);
*/

					}else if(szFPath.Find(_T("RadGtSplitter.ax")) > 0){
						pFGF->m_chkbytes.AddTail(_T("0,3,,534D4B"));
						pFGF->m_chkbytes.AddTail(_T("0,3,,42494B"));
						pFGF->m_extensions.AddTail(_T(".smk"));
						pFGF->m_extensions.AddTail(_T(".bik"));
						m_source.AddTail(pFGF);
					}else{
						pFGF->SetTypes(fo->guids);
                         if(szFPath.Find(_T("dh264.ax")) > 0){
                             pFGF->SetMerit( MERIT64_ABOVE_DSHOW+ 3);
                         }

                        m_transform.AddTail(pFGF);
					}
				}
			}
		}else{
			SVP_LogMsg5(_T("File Not Exist %s"), szFPath);
		}
	}
	// Overrides

	WORD merit_low = 1;

	POSITION pos = s.filters.GetTailPosition();
	while(pos)
	{
		FilterOverride* fo = s.filters.GetPrev(pos);

		if(fo->fDisabled || fo->type == FilterOverride::EXTERNAL && !CPath(MakeFullPath(fo->path)).FileExists()) 
			continue;

		ULONGLONG merit = 
			fo->iLoadType == FilterOverride::PREFERRED ? MERIT64_ABOVE_DSHOW : 
			fo->iLoadType == FilterOverride::MERIT ? MERIT64(fo->dwMerit) : 
			MERIT64_DO_NOT_USE; // fo->iLoadType == FilterOverride::BLOCKED

		merit += merit_low++;

		CFGFilter* pFGF = NULL;

		if(fo->type == FilterOverride::REGISTERED)
		{
			pFGF = new CFGFilterRegistry(fo->dispname, merit);
		}
		else if(fo->type == FilterOverride::EXTERNAL)
		{
			pFGF = new CFGFilterFile(fo->clsid, fo->path, CStringW(fo->name), merit);
		}

		if(pFGF)
		{
			pFGF->SetTypes(fo->guids);
			m_override.AddTail(pFGF);
		}
	}
}

STDMETHODIMP CFGManagerCustom::AddFilter(IBaseFilter* pBF, LPCWSTR pName)
{
	CAutoLock cAutoLock(this);

	HRESULT hr;

	if(FAILED(hr = __super::AddFilter(pBF, pName)))
		return hr;

	AppSettings& s = AfxGetAppSettings();

	if(GetCLSID(pBF) == CLSID_DMOWrapperFilter)
	{
		if(CComQIPtr<IPropertyBag> pPB = pBF)
		{
			CComVariant var(true);
			pPB->Write(CComBSTR(L"_HIRESOUTPUT"), &var);
		}
	}

	if(CComQIPtr<IAudioSwitcherFilter> pASF = pBF)
	{
		//SVP_LogMsg5(L"Init Audioswitch");
		pASF->EnableDownSamplingTo441(s.fDownSampleTo441);
		//pASF->SetSpeakerConfig(s.fCustomChannelMapping, s.pSpeakerToChannelMap);
		pASF->SetSpeakerChannelConfig(AfxGetMyApp()->GetNumberOfSpeakers(), s.pSpeakerToChannelMap2 , s.pSpeakerToChannelMapOffset, 0,s.iSS);
		pASF->SetAudioTimeShift(s.fAudioTimeShift ? 10000i64*s.tAudioTimeShift : 0);
		pASF->SetNormalizeBoost(s.fAudioNormalize, s.fAudioNormalizeRecover, s.AudioBoost);
		pASF->SetEQControl(s.pEQBandControlPerset, s.pEQBandControlCustom);
	}

	return hr;
}

//
// 	CFGManagerPlayer
//

CFGManagerPlayer::CFGManagerPlayer(LPCTSTR pName, LPUNKNOWN pUnk, UINT src, UINT tra, HWND hWnd, BOOL isCapture)
	: CFGManagerCustom(pName, pUnk, src, tra)
	, m_hWnd(hWnd)
	, m_vrmerit(MERIT64(MERIT_PREFERRED))
	, m_armerit(MERIT64(MERIT_PREFERRED))
{
	CFGFilter* pFGF;

	AppSettings& s = AfxGetAppSettings();


	if(m_pFM)
	{
		CComPtr<IEnumMoniker> pEM;

		GUID guids[] = {MEDIATYPE_Video, MEDIASUBTYPE_NULL};

		if(SUCCEEDED(m_pFM->EnumMatchingFilters(&pEM, 0, FALSE, MERIT_DO_NOT_USE+1,
			TRUE, 1, guids, NULL, NULL, TRUE, FALSE, 0, NULL, NULL, NULL)))
		{
			for(CComPtr<IMoniker> pMoniker; S_OK == pEM->Next(1, &pMoniker, NULL); pMoniker = NULL)
			{
				CFGFilterRegistry f(pMoniker);
				m_vrmerit = max(m_vrmerit, f.GetMerit());
			}
		}

		m_vrmerit += 0x100;
	}

	if(m_pFM)
	{
		CComPtr<IEnumMoniker> pEM;

		GUID guids[] = {MEDIATYPE_Audio, MEDIASUBTYPE_NULL};

		if(SUCCEEDED(m_pFM->EnumMatchingFilters(&pEM, 0, FALSE, MERIT_DO_NOT_USE+1,
			TRUE, 1, guids, NULL, NULL, TRUE, FALSE, 0, NULL, NULL, NULL)))
		{
			for(CComPtr<IMoniker> pMoniker; S_OK == pEM->Next(1, &pMoniker, NULL); pMoniker = NULL)
			{
				CFGFilterRegistry f(pMoniker);
				m_armerit = max(m_armerit, f.GetMerit());
			}
		}

		BeginEnumSysDev(CLSID_AudioRendererCategory, pMoniker)
		{
			CFGFilterRegistry f(pMoniker);
			m_armerit = max(m_armerit, f.GetMerit());
		}
		EndEnumSysDev

		m_armerit += 0x100;
	}
/*


	CComPtr<IBaseFilter> pBF;
	CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
	HRESULT d3dhr;
	CFGFilterVideoRenderer* pFGRVMR9 = new CFGFilterVideoRenderer(m_hWnd, CLSID_VMR9AllocatorPresenter, L"DX9(VMR)渲染器", m_vrmerit);
	CFGFilterVideoRenderer* pFGREVR;
	if ( (CMPlayerCApp::IsVista()  ) && !s.bDisableEVR ) {
		pFGREVR = new CFGFilterVideoRenderer(m_hWnd, CLSID_EVRAllocatorPresenter, L"EVR渲染器", m_vrmerit+1);
		d3dhr = pFGREVR->Create(&pBF, pUnks);
	}else{
		d3dhr = pFGRVMR9->Create(&pBF, pUnks);


	}
	
	FAILED(d3dhr)*/

	

	CSVPToolBox svptoolbox;
	if( !isCapture && ( s.iSVPRenderType == 0 ) ){ //|| !svptoolbox.TestD3DCreationAbility(m_hWnd) ( s.iDSVideoRendererType == VIDRNDT_DS_OVERLAYMIXER || VIDRNDT_DS_OLDRENDERER == s.iDSVideoRendererType)
		
		pFGF = new CFGFilterInternal<CSVPSubFilter>(
			ResStr(IDS_SVP_FILTER_NAME) ,
			MERIT64_ABOVE_DSHOW );
		
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_YV12);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_I420);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_IYUV);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_YUY2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RGB32);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RGB24);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RGB565);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_RGB555);

		m_transform.AddTail(pFGF);
	}


	// Switchers

	if(s.fEnableAudioSwitcher)
	{
		pFGF = new CFGFilterInternal<CAudioSwitcherFilter>(L"Audio Switcher", m_armerit + 0x100);
		pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_NULL);
		m_transform.AddTail(pFGF);

		// morgan stream switcher
		m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{D3CD7858-971A-4838-ACEC-40CA5D529DC8}")), MERIT64_DO_NOT_USE));
	}

	// Renderers
	if(s.iDSVideoRendererType != VIDRNDT_DS_OVERLAYMIXER ){
		m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, CLSID_OverlayMixer, L"Overlay Mixer", MERIT64_DO_NOT_USE));
	}
	//m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, GUIDFromCString(_T("{95F57653-71ED-42BA-9131-986CA0C6514F}")), L"Unknown Overlay Mixer", MERIT64_DO_NOT_USE));

	if(s.bIsIVM){

		m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, CLSID_VideoMixingRenderer, L"IVM渲染器(DX7)", m_vrmerit+10));
		m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, CLSID_VideoMixingRenderer9, L"IVM渲染器(DX9)", m_vrmerit + ( (s.iDSVideoRendererType == VIDRNDT_DS_VMR9RENDERLESS) ? 12 : 8 ) ));
	}

	if(s.iDSVideoRendererType == VIDRNDT_DS_OLDRENDERER)
		m_transform.AddTail(new CFGFilterRegistry(CLSID_VideoRenderer, m_vrmerit));
	else if(s.iDSVideoRendererType == VIDRNDT_DS_OVERLAYMIXER){
		m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, CLSID_OverlayMixer, L"Overlay Mixer", m_vrmerit));
	/*
		else if(s.iDSVideoRendererType == VIDRNDT_DS_VMR7WINDOWED)
				m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, CLSID_VideoMixingRenderer, L"Video Mixing Render 7 (Windowed)", m_vrmerit));
			else if(s.iDSVideoRendererType == VIDRNDT_DS_VMR9WINDOWED)
				m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, CLSID_VideoMixingRenderer9, L"Video Mixing Render 9 (Windowed)", m_vrmerit));
			else if(s.iDSVideoRendererType == VIDRNDT_DS_VMR7RENDERLESS)
				m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, CLSID_VMR7AllocatorPresenter, L"DX7(VMR)渲染器", m_vrmerit));*/
	}
	else if(s.iDSVideoRendererType == VIDRNDT_DS_DXR)
		m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, CLSID_DXRAllocatorPresenter, L"Haali's Video Renderer", m_vrmerit));
	else //
	{
		if(s.iDSVideoRendererType == VIDRNDT_DS_VMR7RENDERLESS)
			m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, CLSID_VMR7AllocatorPresenter, L"DX7(VMR)渲染器", m_vrmerit+5));


		if ( s.bShouldUseEVR() ) //s.fVMRGothSyncFix )//|| (!CMPlayerCApp::IsVista() && s.useGPUAcel) // No EVR for XP!
			m_transform.AddTail( new CFGFilterVideoRenderer(m_hWnd, CLSID_EVRAllocatorPresenter, L"EVR渲染器", m_vrmerit+1));

		m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, CLSID_VMR9AllocatorPresenter, L"DX9(VMR)渲染器", m_vrmerit));
		m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, CLSID_VMR7AllocatorPresenter, L"DX7(VMR)渲染器", m_vrmerit-1));
	}

/*
	else if(s.iDSVideoRendererType == VIDRNDT_DS_NULL_COMP)
	{
		pFGF = new CFGFilterInternal<CNullVideoRenderer>(L"Null Video Renderer (Any)", MERIT64_ABOVE_DSHOW+2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_NULL);
		m_transform.AddTail(pFGF);
	}
	else if(s.iDSVideoRendererType == VIDRNDT_DS_NULL_UNCOMP)
	{
		pFGF = new CFGFilterInternal<CNullUVideoRenderer>(L"Null Video Renderer (Uncompressed)", MERIT64_ABOVE_DSHOW+2);
		pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_NULL);
		m_transform.AddTail(pFGF);
	}*/

	//pFGF = new CFGFilterInternal<CMpcAudioRenderer>(_T("WSAS Audio Device"), MERIT64_ABOVE_DSHOW+4);
	//pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_NULL);
	//m_transform.AddTail(pFGF);
	if(s.AudioRendererDisplayName == AUDRNDT_NULL_COMP)
	{
		pFGF = new CFGFilterInternal<CNullAudioRenderer>(AUDRNDT_NULL_COMP, MERIT64_ABOVE_DSHOW+2);
		pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_NULL);
		m_transform.AddTail(pFGF);
	}
	else if(s.AudioRendererDisplayName == AUDRNDT_NULL_UNCOMP)
	{
		pFGF = new CFGFilterInternal<CNullUAudioRenderer>(AUDRNDT_NULL_UNCOMP, MERIT64_ABOVE_DSHOW+2);
		pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_NULL);
		m_transform.AddTail(pFGF);
	}
	else if(!s.AudioRendererDisplayName.IsEmpty())
	{
		pFGF = new CFGFilterRegistry(s.AudioRendererDisplayName, m_armerit);
		pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_NULL);
		m_transform.AddTail(pFGF);
	}else if(s.bUseWaveOutDeviceByDefault){

		//E30629D1-27E5-11CE-875D-00608CB78066
		pFGF = new CFGFilterRegistry(CLSID_AudioRender, m_armerit);
		pFGF->AddType(MEDIATYPE_Audio, MEDIASUBTYPE_NULL);
		m_transform.AddTail(pFGF);
	}
}

STDMETHODIMP CFGManagerPlayer::ConnectDirect(IPin* pPinOut, IPin* pPinIn, const AM_MEDIA_TYPE* pmt)
{
	CAutoLock cAutoLock(this);

	if(GetCLSID(pPinOut) == CLSID_MPEG2Demultiplexer)
	{
		CComQIPtr<IMediaSeeking> pMS = pPinOut;
		REFERENCE_TIME rtDur = 0;
		if(!pMS || FAILED(pMS->GetDuration(&rtDur)) || rtDur <= 0)
			 return E_FAIL;
	}

	return __super::ConnectDirect(pPinOut, pPinIn, pmt);
}

//
// CFGManagerDVD
//

CFGManagerDVD::CFGManagerDVD(LPCTSTR pName, LPUNKNOWN pUnk, UINT src, UINT tra, HWND hWnd)
	: CFGManagerPlayer(pName, pUnk, src, tra, hWnd)
{
	AppSettings& s = AfxGetAppSettings();

	// have to avoid the old video renderer
	if(!s.fXpOrBetter && s.iDSVideoRendererType != VIDRNDT_DS_OVERLAYMIXER || s.iDSVideoRendererType == VIDRNDT_DS_OLDRENDERER)
		m_transform.AddTail(new CFGFilterVideoRenderer(m_hWnd, CLSID_OverlayMixer, L"Overlay Mixer", MERIT64_DO_NOT_USE));

	// elecard's decoder isn't suited for dvd playback (atm)
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{F50B3F13-19C4-11CF-AA9A-02608C9BABA2}")), MERIT64_DO_NOT_USE));
}

#include "..\..\decss\VobFile.h"

class CResetDVD : public CDVDSession
{
public:
	CResetDVD(LPCTSTR path)
	{
		if(Open(path))
		{
			if(BeginSession()) {Authenticate(); /*GetDiscKey();*/ EndSession();}
			Close();
		}
	}
};

STDMETHODIMP CFGManagerDVD::RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList)
{
	CAutoLock cAutoLock(this);

	HRESULT hr;

	CComPtr<IBaseFilter> pBF;
	if(FAILED(hr = AddSourceFilter(lpcwstrFile, lpcwstrFile, &pBF)))
		return hr;

	return ConnectFilter(pBF, NULL);
}

STDMETHODIMP CFGManagerDVD::AddSourceFilter(LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppFilter)
{
	CAutoLock cAutoLock(this);

	CheckPointer(lpcwstrFileName, E_POINTER);
	CheckPointer(ppFilter, E_POINTER);

	HRESULT hr;

	CStringW fn = CStringW(lpcwstrFileName).TrimLeft();
	CStringW protocol = fn.Left(fn.Find(':')+1).TrimRight(':').MakeLower();
	CStringW ext = CPathW(fn).GetExtension().MakeLower();

	GUID clsid = ext == L".ratdvd" ? GUIDFromCString(_T("{482d10b6-376e-4411-8a17-833800A065DB}")) : CLSID_DVDNavigator;

	CComPtr<IBaseFilter> pBF;
	if(FAILED(hr = pBF.CoCreateInstance(clsid))
	|| FAILED(hr = AddFilter(pBF, L"DVD Navigator")))
		return VFW_E_CANNOT_LOAD_SOURCE_FILTER;

	CComQIPtr<IDvdControl2> pDVDC;
	CComQIPtr<IDvdInfo2> pDVDI;

	if(!((pDVDC = pBF) && (pDVDI = pBF)))
		return E_NOINTERFACE;

	WCHAR buff[MAX_PATH];
	ULONG len;
	if((!fn.IsEmpty()
	&& FAILED(hr = pDVDC->SetDVDDirectory(fn))
	&& FAILED(hr = pDVDC->SetDVDDirectory(fn + L"VIDEO_TS"))
	&& FAILED(hr = pDVDC->SetDVDDirectory(fn + L"\\VIDEO_TS")))
	|| FAILED(hr = pDVDI->GetDVDDirectory(buff, countof(buff), &len)) || len == 0)
		return E_INVALIDARG;

	pDVDC->SetOption(DVD_ResetOnStop, FALSE);
	pDVDC->SetOption(DVD_HMSF_TimeCodeEvents, TRUE);

	if(clsid == CLSID_DVDNavigator)
		CResetDVD(CString(buff));

	*ppFilter = pBF.Detach();

	return S_OK;
}

//
// CFGManagerCapture
//

CFGManagerCapture::CFGManagerCapture(LPCTSTR pName, LPUNKNOWN pUnk, UINT src, UINT tra, HWND hWnd)
	: CFGManagerPlayer(pName, pUnk, src, tra, hWnd, true)
{
	AppSettings& s = AfxGetAppSettings();

	CFGFilter* pFGF = new CFGFilterInternal<CDeinterlacerFilter>(L"Deinterlacer", m_vrmerit + 0x100);
	pFGF->AddType(MEDIATYPE_Video, MEDIASUBTYPE_NULL);
	m_transform.AddTail(pFGF);

	// morgan stream switcher
	m_transform.AddTail(new CFGFilterRegistry(GUIDFromCString(_T("{D3CD7858-971A-4838-ACEC-40CA5D529DC8}")), MERIT64_DO_NOT_USE));
}

//
// CFGManagerMuxer
//

CFGManagerMuxer::CFGManagerMuxer(LPCTSTR pName, LPUNKNOWN pUnk)
	: CFGManagerCustom(pName, pUnk, ~0, ~0)
{
	m_source.AddTail(new CFGFilterInternal<CSubtitleSourceASS>());
	m_source.AddTail(new CFGFilterInternal<CSSFSourceFilter>());
}

//
// CFGAggregator
//

CFGAggregator::CFGAggregator(const CLSID& clsid, LPCTSTR pName, LPUNKNOWN pUnk, HRESULT& hr)
	: CUnknown(pName, pUnk)
{
	hr = m_pUnkInner.CoCreateInstance(clsid, GetOwner());
}

CFGAggregator::~CFGAggregator()
{
	m_pUnkInner.Release();
}

STDMETHODIMP CFGAggregator::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

	return
		m_pUnkInner && (riid != IID_IUnknown && SUCCEEDED(m_pUnkInner->QueryInterface(riid, ppv))) ? S_OK :
		__super::NonDelegatingQueryInterface(riid, ppv);
}
