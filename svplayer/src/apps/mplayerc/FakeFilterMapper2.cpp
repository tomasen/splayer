/*
* $Id: FakeFilterMapper2.cpp 917 2008-12-07 13:56:44Z casimir666 $
*
* (C) 2003-2006 Gabest
* (C) 2006-2007 see AUTHORS
*
* This file is part of mplayerc.
*
* Mplayerc is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* Mplayerc is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "stdafx.h"
#include "fakefiltermapper2.h"
#include "MacrovisionKicker.h"
#include "..\..\DSUtil\DSUtil.h"

#include <initguid.h>
#include <qedit.h>


#include <detours\detours.h>
#include "..\..\svplib\svplib.h"

#define TRACE_SVP   __noop
//SVP_LogMsg6
//
#define TRACE_SVP5 __noop
// SVP_LogMsg5

HRESULT (__stdcall * Real_CoCreateInstance)(CONST IID& a0,
											LPUNKNOWN a1,
											DWORD a2,
											CONST IID& a3,
											LPVOID* a4)
											= CoCreateInstance;

LONG (WINAPI * Real_RegCreateKeyExA)(HKEY a0,
									 LPCSTR a1,
									 DWORD a2,
									 LPSTR a3,
									 DWORD a4,
									 REGSAM a5,
									 LPSECURITY_ATTRIBUTES a6,
									 PHKEY a7,
									 LPDWORD a8)
									 = RegCreateKeyExA;

LONG (WINAPI * Real_RegCreateKeyExW)(HKEY a0,
									 LPCWSTR a1,
									 DWORD a2,
									 LPWSTR a3,
									 DWORD a4,
									 REGSAM a5,
									 LPSECURITY_ATTRIBUTES a6,
									 PHKEY a7,
									 LPDWORD a8)
									 = RegCreateKeyExW;

LONG (WINAPI * Real_RegDeleteKeyA)(HKEY a0,
								   LPCSTR a1)
								   = RegDeleteKeyA;

LONG (WINAPI * Real_RegDeleteKeyW)(HKEY a0,
								   LPCWSTR a1)
								   = RegDeleteKeyW;

LONG (WINAPI * Real_RegDeleteValueA)(HKEY a0,
									 LPCSTR a1)
									 = RegDeleteValueA;


LONG (WINAPI * Real_RegDeleteValueW)(HKEY a0,
									 LPCWSTR a1)
									 = RegDeleteValueW;

LONG (WINAPI * Real_RegEnumKeyExA)(HKEY a0,
								   DWORD a1,
								   LPSTR a2,
								   LPDWORD a3,
								   LPDWORD a4,
								   LPSTR a5,
								   LPDWORD a6,
struct _FILETIME* a7)
	= RegEnumKeyExA;

LONG (WINAPI * Real_RegEnumKeyExW)(HKEY a0,
								   DWORD a1,
								   LPWSTR a2,
								   LPDWORD a3,
								   LPDWORD a4,
								   LPWSTR a5,
								   LPDWORD a6,
struct _FILETIME* a7)
	= RegEnumKeyExW;

LONG (WINAPI * Real_RegEnumValueA)(HKEY a0,
								   DWORD a1,
								   LPSTR a2,
								   LPDWORD a3,
								   LPDWORD a4,
								   LPDWORD a5,
								   LPBYTE a6,
								   LPDWORD a7)
								   = RegEnumValueA;

LONG (WINAPI * Real_RegEnumValueW)(HKEY a0,
								   DWORD a1,
								   LPWSTR a2,
								   LPDWORD a3,
								   LPDWORD a4,
								   LPDWORD a5,
								   LPBYTE a6,
								   LPDWORD a7)
								   = RegEnumValueW;

LONG (WINAPI * Real_RegOpenKeyExA)(HKEY a0,
								   LPCSTR a1,
								   DWORD a2,
								   REGSAM a3,
								   PHKEY a4)
								   = RegOpenKeyExA;

LONG (WINAPI * Real_RegOpenKeyExW)(HKEY a0,
								   LPCWSTR a1,
								   DWORD a2,
								   REGSAM a3,
								   PHKEY a4)
								   = RegOpenKeyExW;

LONG (WINAPI * Real_RegQueryInfoKeyA)(HKEY a0,
									  LPSTR a1,
									  LPDWORD a2,
									  LPDWORD a3,
									  LPDWORD a4,
									  LPDWORD a5,
									  LPDWORD a6,
									  LPDWORD a7,
									  LPDWORD a8,
									  LPDWORD a9,
									  LPDWORD a10,
struct _FILETIME* a11)
	= RegQueryInfoKeyA;

LONG (WINAPI * Real_RegQueryInfoKeyW)(HKEY a0,
									  LPWSTR a1,
									  LPDWORD a2,
									  LPDWORD a3,
									  LPDWORD a4,
									  LPDWORD a5,
									  LPDWORD a6,
									  LPDWORD a7,
									  LPDWORD a8,
									  LPDWORD a9,
									  LPDWORD a10,
struct _FILETIME* a11)
	= RegQueryInfoKeyW;

LONG (WINAPI * Real_RegQueryValueExA)(HKEY a0,
									  LPCSTR a1,
									  LPDWORD a2,
									  LPDWORD a3,
									  LPBYTE a4,
									  LPDWORD a5)
									  = RegQueryValueExA;

LONG (WINAPI * Real_RegQueryValueExW)(HKEY a0,
									  LPCWSTR a1,
									  LPDWORD a2,
									  LPDWORD a3,
									  LPBYTE a4,
									  LPDWORD a5)
									  = RegQueryValueExW;

LONG (WINAPI * Real_RegSetValueExA)(HKEY a0,
									LPCSTR a1,
									DWORD a2,
									DWORD a3,
									const BYTE* a4,
									DWORD a5)
									= RegSetValueExA;

LONG (WINAPI * Real_RegSetValueExW)(HKEY a0,
									LPCWSTR a1,
									DWORD a2,
									DWORD a3,
									const BYTE* a4,
									DWORD a5)
									= RegSetValueExW;


LONG (WINAPI * Real_RegCloseKey)(HKEY a0)
= RegCloseKey;

LONG (WINAPI * Real_RegFlushKey)(HKEY a0)
= RegFlushKey;

LONG (WINAPI * Real_RegCreateKeyA)(HKEY a0, LPCSTR a1, PHKEY a2)
= RegCreateKeyA;

LONG (WINAPI * Real_RegCreateKeyW)(HKEY a0, LPCWSTR a1, PHKEY a2)
= RegCreateKeyW;

LONG (WINAPI * Real_RegOpenKeyA)(HKEY a0, LPCSTR a1, PHKEY a2)
= RegOpenKeyA;

LONG (WINAPI * Real_RegOpenKeyW)(HKEY a0, LPCWSTR a1, PHKEY a2)
= RegOpenKeyW;

LONG (WINAPI * Real_RegQueryValueA)(HKEY a0, LPCSTR a1, LPSTR a2, PLONG a3)
= RegQueryValueA;

LONG (WINAPI * Real_RegQueryValueW)(HKEY a0, LPCWSTR a1, LPWSTR a2, PLONG a3)
= RegQueryValueW;

LONG (WINAPI * Real_RegSetValueW)(HKEY a0, LPCWSTR a1, DWORD a2, LPCWSTR a3, DWORD a4)
= RegSetValueW;

LONG (WINAPI * Real_RegSetValueA)(HKEY a0, LPCSTR a1, DWORD a2, LPCSTR a3, DWORD a4)
= RegSetValueA;



HRESULT WINAPI Mine_CoCreateInstance(IN REFCLSID rclsid, IN LPUNKNOWN pUnkOuter,
									 IN DWORD dwClsContext, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	if(CFilterMapper2::m_pFilterMapper2)
	{
		CheckPointer(ppv, E_POINTER);

		if(rclsid == CLSID_FilterMapper)
		{
			TRACE_SVP5(_T("Mine_CoCreateInstance Unknown REGDB_E_CLASSNOTREG ") );
			ASSERT(0);
			return REGDB_E_CLASSNOTREG; // sorry...
		}

		if(rclsid == CLSID_FilterMapper2)
		{
			if(pUnkOuter) {
				TRACE_SVP5(_T("Mine_CoCreateInstance Unknown CLASS_E_NOAGGREGATION ") );
				return CLASS_E_NOAGGREGATION;
			}

			if(riid == __uuidof(IUnknown))
			{
				CFilterMapper2::m_pFilterMapper2->AddRef();
				*ppv = (IUnknown*)CFilterMapper2::m_pFilterMapper2;
				return S_OK;
			}
			else if(riid == __uuidof(IFilterMapper2))
			{
				CFilterMapper2::m_pFilterMapper2->AddRef();
				*ppv = (IFilterMapper2*)CFilterMapper2::m_pFilterMapper2;
				return S_OK;
			}
			else
			{
				TRACE_SVP5(_T("Mine_CoCreateInstance Unknown riid %s"), CStringFromGUID(riid) );
				return E_NOINTERFACE;
			}

		}

		TRACE_SVP5(_T("Mine_CoCreateInstance Unknown %s"), CStringFromGUID(rclsid));
	}
	/*
	if( GUIDFromCString(_T("{DB43B405-43AA-4F01-82D8-D84D47E6019C}")) == rclsid){
			CFilterMapper2::m_pFilterMapper2->AddRef();
			*ppv = (IUnknown*)CFilterMapper2::m_pFilterMapper2;
			return S_OK;
		}*/
	
	/*	else
	{
	if(rclsid == CLSID_FilterMapper2)
	{
	CFilterMapper2* pFM2 = new CFilterMapper2(true, false, pUnkOuter);
	CComPtr<IUnknown> pUnk = (IUnknown*)pFM2;
	return pUnk->QueryInterface(riid, ppv);
	}
	}
	*/
	if(!pUnkOuter)
		if(rclsid == CLSID_VideoMixingRenderer || rclsid == CLSID_VideoMixingRenderer9
			|| rclsid == CLSID_VideoRenderer || rclsid == CLSID_VideoRendererDefault
			|| rclsid == CLSID_OverlayMixer)// || rclsid == CLSID_OverlayMixer2 - where is this declared?)
		{
			CMacrovisionKicker* pMK = new CMacrovisionKicker(NAME("CMacrovisionKicker"), NULL);
			CComPtr<IUnknown> pUnk = (IUnknown*)(INonDelegatingUnknown*)pMK;
			CComPtr<IUnknown> pInner;
			HRESULT hr;
			if(SUCCEEDED(hr = Real_CoCreateInstance(rclsid, pUnk, dwClsContext, __uuidof(IUnknown), (void**)&pInner)))
			{
				pMK->SetInner(pInner);
				return pUnk->QueryInterface(riid, ppv);
			}
		}

		long ret;
		
		//if( GUIDFromCString(_T("{DB43B405-43AA-4F01-82D8-D84D47E6019C}")) == rclsid){
			/*
			TRACE_SVP5(_T("CoGetInstanceFromFile OGM.dll"));
						MULTI_QI mqi [1] ;
			
						mqi [0].pIID = &riid ;
						mqi [0].pItf = NULL ;
						mqi [0].hr = 0 ;
			
						ret = CoGetInstanceFromFile(NULL, (CLSID*)&rclsid , pUnkOuter , dwClsContext , 0 , L"D:\\-=SVN=-\\ogm.dll", 1 ,mqi);*/
			//rclsid = CLSID_VideoMixingRenderer;//(CLSID) ;
		//	ret = Real_CoCreateInstance(GUIDFromCString(_T("{DB43B405-43AA-4F01-82D8-D84D47E6019D}")), pUnkOuter, dwClsContext, riid, ppv);
		//}else{
			ret = Real_CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
		//}

		
		
		TRACE_SVP5(_T("Mine_CoCreateInstance %s %x"), CStringFromGUID(rclsid) , ret);

		return ret;
}

#define FAKEHKEY (HKEY)0x12345678

LONG WINAPI Mine_RegCloseKey(HKEY a0)
{
	if(CFilterMapper2::m_pFilterMapper2 && a0 == FAKEHKEY) {return ERROR_SUCCESS;}
	return Real_RegCloseKey(a0);
}
LONG WINAPI Mine_RegFlushKey(HKEY a0)
{
	if(CFilterMapper2::m_pFilterMapper2 && a0 == FAKEHKEY) {return ERROR_SUCCESS;}
	return Real_RegFlushKey(a0);
}
LONG WINAPI Mine_RegCreateKeyA(HKEY a0, LPCSTR a1, PHKEY a2)
{
	TRACE_SVP("Mine_RegCreateKeyA %s" , a1);
	if(CFilterMapper2::m_pFilterMapper2) {TRACE_SVP("Mine_RegCreateKeyA %s" , a1);*a2 = FAKEHKEY; return ERROR_SUCCESS;}

	if(a1){ //IVM is sux
		if( _strcmpi(a1, "Software\\GNU\\ffdshow") == 0 || 
			_strcmpi(a1, "Software\\Gabest\\Media Player Classic\\Settings") == 0 ||
			_strcmpi(a1, "Software\\KMPlayer\\KMP2.0\\OptionList\\KMPWizard") == 0 ||
			_strcmpi(a1, "Software\\KMPlayer\\KMP2.0\\OptionArea") == 0 
			){
				*a2 = FAKEHKEY;
				return ERROR_SUCCESS;
		}
	}
	return Real_RegCreateKeyA(a0, a1, a2);
}
LONG WINAPI Mine_RegCreateKeyW(HKEY a0, LPCWSTR a1, PHKEY a2)
{
	/*
	TRACE_SVP5(L"Mine_RegCreateKeyW %s" , a1);
		if(a1){
			if( _wcsicmp(L"InprocServer32", a1) == 0 ||
				 wcsstr(a1, L"CLSID\\")
				){
				return Real_RegCreateKeyW(a0, a1, a2);
			}
			
		}*/
	
	
	if(CFilterMapper2::m_pFilterMapper2) {*a2 = FAKEHKEY; return ERROR_SUCCESS;}
	return Real_RegCreateKeyW(a0, a1, a2);
}
LONG WINAPI Mine_RegCreateKeyExA(HKEY a0, LPCSTR a1, DWORD a2, LPSTR a3, DWORD a4, REGSAM a5, LPSECURITY_ATTRIBUTES a6, PHKEY a7, LPDWORD a8)
{
	TRACE_SVP("Mine_RegCreateKeyExA %s" , a1);
	if(CFilterMapper2::m_pFilterMapper2) {*a7 = FAKEHKEY; return ERROR_SUCCESS;}
	return Real_RegCreateKeyExA(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}
LONG WINAPI Mine_RegCreateKeyExW(HKEY a0, LPCWSTR a1, DWORD a2, LPWSTR a3, DWORD a4, REGSAM a5, LPSECURITY_ATTRIBUTES a6, PHKEY a7, LPDWORD a8)
{
	TRACE_SVP5(L"Mine_RegCreateKeyExW %s" , a1);
	if(a1 && ( wcsstr(a1, L"MtContain.CMtContainer")	||
		wcsstr(a1, L"Media Type\\Extensions\\.mk") ||
		 _wcsicmp(a1, L"Media Type\\{E436EB83-524F-11CE-9F53-0020AF0BA770}\\{49952F4C-3EDC-4A9B-8906-1DE02A3D4BC2}") == 0
		 || _wcsicmp(a1, L"Media Type\\Extensions\\.csf") == 0
		 ||  wcsstr(a1, L"SOFTWARE\\Microsoft\\MediaPlayer\\Player\\Extensions\\")
		 )){

		TRACE_SVP5(L"FAKING IT" );
		*a7 = FAKEHKEY; return ERROR_SUCCESS;
	}
	if(CFilterMapper2::m_pFilterMapper2) {*a7 = FAKEHKEY; return ERROR_SUCCESS;}
	return Real_RegCreateKeyExW(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}
LONG WINAPI Mine_RegDeleteKeyA(HKEY a0, LPCSTR a1)
{
	if(CFilterMapper2::m_pFilterMapper2 && (a0 == FAKEHKEY || (int)a0 < 0)) {return ERROR_SUCCESS;}
	return Real_RegDeleteKeyA(a0, a1);
}
LONG WINAPI Mine_RegDeleteKeyW(HKEY a0, LPCWSTR a1)
{
	if(CFilterMapper2::m_pFilterMapper2 && (a0 == FAKEHKEY || (int)a0 < 0)) {return ERROR_SUCCESS;}
	return Real_RegDeleteKeyW(a0, a1);
}
LONG WINAPI Mine_RegDeleteValueA(HKEY a0, LPCSTR a1)
{
	if(CFilterMapper2::m_pFilterMapper2 && (a0 == FAKEHKEY || (int)a0 < 0)) {return ERROR_SUCCESS;}
	return Real_RegDeleteValueA(a0, a1);
}
LONG WINAPI Mine_RegDeleteValueW(HKEY a0, LPCWSTR a1)
{
	if(CFilterMapper2::m_pFilterMapper2 && (a0 == FAKEHKEY || (int)a0 < 0)) {return ERROR_SUCCESS;}
	return Real_RegDeleteValueW(a0, a1);
}
LONG WINAPI Mine_RegEnumKeyExA(HKEY a0, DWORD a1, LPSTR a2, LPDWORD a3, LPDWORD a4, LPSTR a5, LPDWORD a6, struct _FILETIME* a7)
{
	TRACE_SVP( "Mine_RegEnumKeyExA %s  %s ",  a2 , a5);
	if(CFilterMapper2::m_pFilterMapper2 && a0 == FAKEHKEY) {return ERROR_NO_MORE_ITEMS;}
	return Real_RegEnumKeyExA(a0, a1, a2, a3, a4, a5, a6, a7);
}
LONG WINAPI Mine_RegEnumKeyExW(HKEY a0, DWORD a1, LPWSTR a2, LPDWORD a3, LPDWORD a4, LPWSTR a5, LPDWORD a6, struct _FILETIME* a7)
{
	TRACE_SVP5( L"Mine_RegEnumKeyExW %s %s ",  a2 , a5);
	if(CFilterMapper2::m_pFilterMapper2 && a0 == FAKEHKEY) {return ERROR_NO_MORE_ITEMS;}
	return Real_RegEnumKeyExW(a0, a1, a2, a3, a4, a5, a6, a7);
}
LONG WINAPI Mine_RegEnumValueA(HKEY a0, DWORD a1, LPSTR a2, LPDWORD a3, LPDWORD a4, LPDWORD a5, LPBYTE a6, LPDWORD a7)
{
	
	if(CFilterMapper2::m_pFilterMapper2 && a0 == FAKEHKEY) {return ERROR_NO_MORE_ITEMS;}
	if( ( FAKEHKEY+14 ) == a0 && a2){
		if(a1 == 0){
			strcpy_s(a2, *a3, "ts.1");
			*a3 = 5;
			*a5 = REG_SZ;
			strcpy_s((char*)a6, *a7, "ff,47,{B841F346-4835-4de8-AA5E-2E7CD2D4C435}");
			*a7 = 45;
			return  ERROR_SUCCESS ;
		}
		if(a1 == 1){
			strcpy_s(a2, *a3, "ogm.1");
			*a3 = 6;
			*a5 = REG_SZ;
			strcpy_s((char*)a6, *a7, "ffffffff,4f676753,{DB43B405-43AA-4f01-82D8-D84D47E6019C}");
			*a7 = 57;
			return  ERROR_SUCCESS ;
		}
        if(a1 == 2){
            strcpy_s(a2, *a3, "mp4.1");
            *a3 = 6;
            *a5 = REG_SZ;
            strcpy_s((char*)a6, *a7, "00000000ffffffff,0000000066747970,{B3DE7EDC-0CD4-4d07-B1C5-92219CD475CC}");
            *a7 = 73;
            return  ERROR_SUCCESS ;
        }
        if(a1 == 3){
            strcpy_s(a2, *a3, "mp4.2");
            *a3 = 6;
            *a5 = REG_SZ;
            strcpy_s((char*)a6, *a7, "00000000ffffffff,000000006d6f6f76,{B3DE7EDC-0CD4-4d07-B1C5-92219CD475CC}");
            *a7 = 73;
            return  ERROR_SUCCESS ;
        }
        if(a1 == 4){
            strcpy_s(a2, *a3, "mp4.3");
            *a3 = 6;
            *a5 = REG_SZ;
            strcpy_s((char*)a6, *a7, "00000000ffffffff,000000006d646174,{B3DE7EDC-0CD4-4d07-B1C5-92219CD475CC}");
            *a7 = 73;
            return  ERROR_SUCCESS ;
        }
        if(a1 == 5){
            strcpy_s(a2, *a3, "avi.1");
            *a3 = 6;
            *a5 = REG_SZ;
            strcpy_s((char*)a6, *a7,  "ffffffff00000000ffffffff,524946460000000041564920,{51A00247-40A8-4845-9F17-7DBFCC9A8783}");
            *a7 = 89;
            return  ERROR_SUCCESS ;
        }
		return ERROR_FILE_NOT_FOUND;
	}
	long ret =  Real_RegEnumValueA(a0, a1, a2, a3, a4, a5, a6, a7);
	if(ret == ERROR_SUCCESS){
		//TRACE_SVP( "Mine_RegEnumValueA %x %x %s %x  %x  %x  %x  %x  %x  %x ", a0, a1, a2, *a3, *a4, *a5, *a6, *a7 , ret);
	}else{
		TRACE_SVP( "Mine_RegEnumValueA %x %x %s %x  %x  %x  %x  %x  %x  %x ", a0, a1, a2, a3, a4, a5, a6, a7 , ret);
	}
	
	return ret;
}
LONG WINAPI Mine_RegEnumValueW(HKEY a0, DWORD a1, LPWSTR a2, LPDWORD a3, LPDWORD a4, LPDWORD a5, LPBYTE a6, LPDWORD a7)
{
	TRACE_SVP5( L"Mine_RegEnumValueW %s ",  a2);
	if(CFilterMapper2::m_pFilterMapper2 && a0 == FAKEHKEY) {return ERROR_NO_MORE_ITEMS;}
	return Real_RegEnumValueW(a0, a1, a2, a3, a4, a5, a6, a7);
}
LONG WINAPI Mine_RegOpenKeyA(HKEY a0, LPCSTR a1, PHKEY a2)
{
	TRACE_SVP( "Mine_RegOpenKeyA2 %s %u ",  a1, a2);
	
	if(CFilterMapper2::m_pFilterMapper2) {TRACE_SVP("Mine_RegOpenKeyA %s" , a1);*a2 = FAKEHKEY; return ERROR_SUCCESS;}

	if(a1){ //IVM is sux
		if( _strcmpi(a1, "Software\\GNU\\ffdshow") == 0 || 
			_strcmpi(a1, "Software\\Gabest\\Media Player Classic\\Settings") == 0 ||
			_strcmpi(a1, "Software\\KMPlayer\\KMP2.0\\OptionList\\KMPWizard") == 0 ||
			_strcmpi(a1, "Software\\KMPlayer\\KMP2.0\\OptionArea") == 0 
			){
				*a2 = FAKEHKEY;
				return ERROR_SUCCESS;
		}
	}

	LONG ret =  Real_RegOpenKeyA(a0, a1, a2);
	
	
	return ret;
}
LONG WINAPI Mine_RegOpenKeyW(HKEY a0, LPCWSTR a1, PHKEY a2)
{
	TRACE_SVP5(L"Mine_RegOpenKeyW %s" , a1);
	if(CFilterMapper2::m_pFilterMapper2) {*a2 = FAKEHKEY; return ERROR_SUCCESS;}
	return Real_RegOpenKeyW(a0, a1, a2);
}
LONG WINAPI Mine_RegOpenKeyExA(HKEY a0, LPCSTR a1, DWORD a2, REGSAM a3, PHKEY a4)
{
	TRACE_SVP( "Mine_RegOpenKeyExA  %s %u ",  a1, a2);
	if( a1 ){
		if( _strcmpi(a1, "Software\\CoreCodec\\CoreAVC Pro") == 0){
			TRACE_SVP( "CoreAVC");
			*a4 = FAKEHKEY+1;
			 return ERROR_SUCCESS;
		}
        if( _strcmpi(a1, "Software\\CoreCodec\\CoreAVC Pro 2.x") == 0){
            TRACE_SVP( "CoreAVC 2");
            *a4 = FAKEHKEY+3;
            return ERROR_SUCCESS;
        }
		
		else if( _strcmpi(a1, "CLSID\\{083863F1-70DE-11D0-BD40-00A0C911CE86}\\Instance\\{09571A4B-F1FE-4C60-9760-DE6D310C7C31}") == 0) {
					*a4 = FAKEHKEY+2;
					 return ERROR_SUCCESS;
		}
		
		
	}
	

	if(CFilterMapper2::m_pFilterMapper2 && (a3&(KEY_SET_VALUE|KEY_CREATE_SUB_KEY))) {TRACE_SVP("Mine_RegOpenKeyExA %s" , a1);*a4 = FAKEHKEY; return ERROR_SUCCESS;}
	LONG ret = Real_RegOpenKeyExA(a0, a1, a2, a3, a4);

	//TRACE_SVP( "Mine_RegOpenKeyExA %s %u %u",  a1, a2, *a4);
	return ret;
}
LONG WINAPI Mine_RegOpenKeyExW(HKEY a0, LPCWSTR a1, DWORD a2, REGSAM a3, PHKEY a4)
{
	TRACE_SVP5( L"Mine_RegOpenKeyExW  %s %x ",  a1, a2);
	if(CFilterMapper2::m_pFilterMapper2 && (a3&(KEY_SET_VALUE|KEY_CREATE_SUB_KEY))) {*a4 = FAKEHKEY; return ERROR_SUCCESS;}
	/*if(a0 >= FAKEHKEY && a0 <= (FAKEHKEY + 100) ){
		TRACE_SVP5(L"Real_RegOpenKeyExW FAKEHKEY %s" , a1);
		*a4 = a0++;
		return ERROR_SUCCESS;
	}
	if( a1 && _wcsicmp(_T("CLSID\\{B841F346-4835-4DE8-AA5E-2E7CD2D4C435}"), a1) == 0 ){
		*a4 = FAKEHKEY + 3;
		TRACE_SVP5(L"Real_RegOpenKeyExW %s" , a1);
		return ERROR_SUCCESS;
	}
	if( a1 && _wcsicmp(_T("Media Type\\{E436EB83-524F-11CE-9F53-0020AF0BA770}\\{49952F4C-3EDC-4A9B-8906-1DE02A3D4BC2}"), a1) == 0 ){
		TRACE_SVP5(L"Real_RegOpenKeyExW %s" , a1);
	}
	if(  a1 && _wcsicmp(_T("CLSID\\{DB43B405-43AA-4f01-82D8-D84D47E6019C}"), a1) == 0 ){ //ogm
		TRACE_SVP5(L"Real_RegOpenKeyExW %s" , a1);
	}
	*/
	if(  a1 && _wcsicmp(_T("Software\\HaaliMkx\\Input"), a1) == 0 ){ 
		*a4 = FAKEHKEY+14;
		return ERROR_SUCCESS;
	}
	LONG ret = Real_RegOpenKeyExW(a0, a1, a2, a3, a4);
	TRACE_SVP5(L"Mine_RegOpenKeyExW %x %s %x" , a0, a1 , ret);
	return ret;
}
LONG WINAPI Mine_RegQueryInfoKeyA(HKEY a0, LPSTR a1, LPDWORD a2, LPDWORD a3, LPDWORD a4, LPDWORD a5, LPDWORD a6, LPDWORD a7, LPDWORD a8, LPDWORD a9, LPDWORD a10, struct _FILETIME* a11)
{
	TRACE_SVP("Mine_RegQueryInfoKeyA %s" , a1);
	if(CFilterMapper2::m_pFilterMapper2 && a0 == FAKEHKEY) {return ERROR_INVALID_HANDLE;}
	return Real_RegQueryInfoKeyA(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}
LONG WINAPI Mine_RegQueryInfoKeyW(HKEY a0, LPWSTR a1, LPDWORD a2, LPDWORD a3, LPDWORD a4, LPDWORD a5, LPDWORD a6, LPDWORD a7, LPDWORD a8, LPDWORD a9, LPDWORD a10, struct _FILETIME* a11)
{
	TRACE_SVP5(L"Mine_RegQueryInfoKeyW %s" , a1);
	if(CFilterMapper2::m_pFilterMapper2 && a0 == FAKEHKEY) {return ERROR_INVALID_HANDLE;}
	return Real_RegQueryInfoKeyW(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}
LONG WINAPI Mine_RegQueryValueA(HKEY a0, LPCSTR a1, LPSTR a2, PLONG a3)
{
	TRACE_SVP("Mine_RegQueryValueA %s" , a1);
	if(CFilterMapper2::m_pFilterMapper2 && a0 == FAKEHKEY) {*a3 = 0; return ERROR_SUCCESS;}
	LONG ret = Real_RegQueryValueA(a0, a1, a2, a3);
	TRACE_SVP( "Mine_RegQueryValueA %s %s",  a1, a2);
	return ret;
}
LONG WINAPI Mine_RegQueryValueW(HKEY a0, LPCWSTR a1, LPWSTR a2, PLONG a3)
{
	//SVP_LogMsg5(_T("Mine_RegQueryValueW %s %s "), a1 , a2);
	TRACE_SVP5(L"Mine_RegQueryValueW %s" , a1);
	if(CFilterMapper2::m_pFilterMapper2 && a0 == FAKEHKEY) {*a3 = 0; return ERROR_SUCCESS;}
	return Real_RegQueryValueW(a0, a1, a2, a3);
}
LONG WINAPI Mine_RegQueryValueExA(HKEY a0, LPCSTR a1, LPDWORD a2, LPDWORD a3, LPBYTE a4, LPDWORD a5)
{
	TRACE_SVP("Mine_RegQueryValueExA %s" , a1);
	if(a1){
        if ( (a0 == (FAKEHKEY+1) || a0 == (FAKEHKEY+3) )){
		//*a3 = REG_SZ;
		TRACE_SVP( "Mine_RegQueryValueExA Serial %s %u %u",  a1, *a5, a4);
		if(*a5 < 40){
			a4 = (LPBYTE)new char[40];
			*a5 = 40;
		}
        if( _strcmpi(a1, "Serial") == 0){
            if( a0 == (FAKEHKEY+3))
                strcpy_s((char *)a4, *a5, "AFGD2R-3VRY6U-RBY69G-34NQAX-RUGX00");
            else
                strcpy_s((char *)a4, *a5, "03JUN-10K9Y-CORE-0CLQV-JOTFL");
        }
        if( _strcmpi(a1, "User") == 0){
            if( a0 == (FAKEHKEY+3))
                strcpy_s((char *)a4, *a5, "HERiTAGE");
            else
		        strcpy_s((char *)a4, *a5, "HERiTAGE");
        }
		return ERROR_SUCCESS;
		}
		if(_strcmpi(a1, "AVC 7x Deblock") == 0){
			*(DWORD*)a4 = 1;
			return ERROR_SUCCESS;
		}
		
		if(_strcmpi(a1, "AVC 7x Deinterlace") == 0){
			*(DWORD*)a4 = 1;
			return ERROR_SUCCESS;
		}
		if(_strcmpi(a1, "AVC 7x HwDeinterlace") == 0){
			*(DWORD*)a4 = 0;
			return ERROR_SUCCESS;
		}
		
		if(_strcmpi(a1, "AVC 7x DisplayOrder") == 0){
			*(DWORD*)a4 = 0;
			return ERROR_SUCCESS;
		}
		if(_strcmpi(a1, "AVC 7x Logo") == 0){
			*(DWORD*)a4 = 0;
			return ERROR_SUCCESS;
		}
	}
		
	
	if(CFilterMapper2::m_pFilterMapper2 && a0 == FAKEHKEY) {*a5 = 0; return ERROR_SUCCESS;}
	LONG ret = Real_RegQueryValueExA(a0, a1, a2, a3, a4, a5);
	if(a1 && a0 == (FAKEHKEY+2)){
		TRACE_SVP( "Mine_RegQueryValueExA %s %u %u %s",  a1, a3,  a4, a4);
	}
	if(a1 && _strcmpi(a1, "UIUseHVA") == 0){
		TRACE_SVP( "Mine_RegQueryValueExA %s %u %u ",  a1, a3,  a4);
		*(DWORD*)a4 = 0;
		return ERROR_SUCCESS;
	}
	return ret;
}
LONG WINAPI Mine_RegQueryValueExW(HKEY a0, LPCWSTR a1, LPDWORD a2, LPDWORD a3, LPBYTE a4, LPDWORD a5)
{
	if(CFilterMapper2::m_pFilterMapper2 && a0 == FAKEHKEY) { *a5 = 0; return ERROR_SUCCESS;}

	if( a1){

		
		if(wcscmp(L"ui.trayicon" , a1 ) == 0){
			memset((void*)a4, 0, 4);
			*a3 = REG_DWORD;
			return ERROR_SUCCESS;
		}
		//
		//return ERROR_FILE_NOT_FOUND;
	}
	

	LONG ret = Real_RegQueryValueExW(a0, a1, a2, a3, a4, a5);
	TRACE_SVP5(_T("Mine_RegQueryValueExW %s %s %x "), a1 , a2, a3);
		if(a4 ){
			
				
				if(*a5 > 4){
					TRACE_SVP5(_T("Mine_RegQueryValueExW Got SZ %s %x  "), a4 , ret);
				}else if(*a5 == 4){
					TRACE_SVP5(_T("Mine_RegQueryValueExW Got %x  %d %x %x"), *(DWORD*)a4 , *a5 , a0,  ret);
				}
			
		}

	
	
	/*
	if(a1 && ( a0 == (FAKEHKEY+2) || a0 == (FAKEHKEY+1) )){
			SVP_LogMsg5(_T("Mine_RegQueryValueExW %s %u %u %s"),  a1, a3,  a4, a4);
		}*/
	
	return ret;
}
LONG WINAPI Mine_RegSetValueA(HKEY a0, LPCSTR a1, DWORD a2, LPCSTR a3, DWORD a4)
{
	TRACE_SVP("SET RegA %s %d %s" , a1 , a2, a3);
	if(CFilterMapper2::m_pFilterMapper2 && (a0 == FAKEHKEY || (int)a0 < 0)) {return ERROR_SUCCESS;}
	return Real_RegSetValueA(a0, a1, a2, a3, a4);
}
LONG WINAPI Mine_RegSetValueW(HKEY a0, LPCWSTR a1, DWORD a2, LPCWSTR a3, DWORD a4)
{
	TRACE_SVP5(_T("SET RegW %s") , a1);
	if(CFilterMapper2::m_pFilterMapper2 && (a0 == FAKEHKEY || (int)a0 < 0)) {return ERROR_SUCCESS;}
	return Real_RegSetValueW(a0, a1, a2, a3, a4);
}
LONG WINAPI Mine_RegSetValueExA(HKEY a0, LPCSTR a1, DWORD a2, DWORD a3, BYTE* a4, DWORD a5)
{
	TRACE_SVP("SET RegAeX %s" , a1);
	if(CFilterMapper2::m_pFilterMapper2 && (a0 == FAKEHKEY || (int)a0 < 0)) {return ERROR_SUCCESS;}
	return Real_RegSetValueExA(a0, a1, a2, a3, a4, a5);
}
LONG WINAPI Mine_RegSetValueExW(HKEY a0, LPCWSTR a1, DWORD a2, DWORD a3, BYTE* a4, DWORD a5)
{
	TRACE_SVP5(_T("SET Mine_RegSetValueExW %s") , a1);
	if(CFilterMapper2::m_pFilterMapper2 && (a0 == FAKEHKEY || (int)a0 < 0)) {return ERROR_SUCCESS;}
	return Real_RegSetValueExW(a0, a1, a2, a3, a4, a5);
}

//
// CFilterMapper2
//

IFilterMapper2* CFilterMapper2::m_pFilterMapper2 = NULL;

bool CFilterMapper2::fInitialized = false;

void CFilterMapper2::Init()
{
	if(!fInitialized)
	{
		DetourAttach(&(PVOID&)Real_CoCreateInstance, (PVOID)Mine_CoCreateInstance);
		DetourAttach(&(PVOID&)Real_RegCloseKey, (PVOID)Mine_RegCloseKey);
		DetourAttach(&(PVOID&)Real_RegFlushKey, (PVOID)Mine_RegFlushKey);
		DetourAttach(&(PVOID&)Real_RegCreateKeyA, (PVOID)Mine_RegCreateKeyA);
		DetourAttach(&(PVOID&)Real_RegCreateKeyW, (PVOID)Mine_RegCreateKeyW);
		DetourAttach(&(PVOID&)Real_RegCreateKeyExA, (PVOID)Mine_RegCreateKeyExA);
		DetourAttach(&(PVOID&)Real_RegCreateKeyExW, (PVOID)Mine_RegCreateKeyExW);
		DetourAttach(&(PVOID&)Real_RegDeleteKeyA, (PVOID)Mine_RegDeleteKeyA);
		DetourAttach(&(PVOID&)Real_RegDeleteKeyW, (PVOID)Mine_RegDeleteKeyW);
		DetourAttach(&(PVOID&)Real_RegDeleteValueA, (PVOID)Mine_RegDeleteValueA);
		DetourAttach(&(PVOID&)Real_RegDeleteValueW, (PVOID)Mine_RegDeleteValueW);
		DetourAttach(&(PVOID&)Real_RegEnumKeyExA, (PVOID)Mine_RegEnumKeyExA);
		DetourAttach(&(PVOID&)Real_RegEnumKeyExW, (PVOID)Mine_RegEnumKeyExW);
		DetourAttach(&(PVOID&)Real_RegEnumValueA, (PVOID)Mine_RegEnumValueA);
		DetourAttach(&(PVOID&)Real_RegEnumValueW, (PVOID)Mine_RegEnumValueW);
		DetourAttach(&(PVOID&)Real_RegOpenKeyA, (PVOID)Mine_RegOpenKeyA);
		DetourAttach(&(PVOID&)Real_RegOpenKeyW, (PVOID)Mine_RegOpenKeyW);
		DetourAttach(&(PVOID&)Real_RegOpenKeyExA, (PVOID)Mine_RegOpenKeyExA);
		DetourAttach(&(PVOID&)Real_RegOpenKeyExW, (PVOID)Mine_RegOpenKeyExW);
		DetourAttach(&(PVOID&)Real_RegQueryInfoKeyA, (PVOID)Mine_RegQueryInfoKeyA);
		DetourAttach(&(PVOID&)Real_RegQueryInfoKeyW, (PVOID)Mine_RegQueryInfoKeyW);
		DetourAttach(&(PVOID&)Real_RegQueryValueA, (PVOID)Mine_RegQueryValueA);
		DetourAttach(&(PVOID&)Real_RegQueryValueW, (PVOID)Mine_RegQueryValueW);
		DetourAttach(&(PVOID&)Real_RegQueryValueExA, (PVOID)Mine_RegQueryValueExA);
		DetourAttach(&(PVOID&)Real_RegQueryValueExW, (PVOID)Mine_RegQueryValueExW);
		DetourAttach(&(PVOID&)Real_RegSetValueA, (PVOID)Mine_RegSetValueA);
		DetourAttach(&(PVOID&)Real_RegSetValueW, (PVOID)Mine_RegSetValueW);
		DetourAttach(&(PVOID&)Real_RegSetValueExA, (PVOID)Mine_RegSetValueExA);
		DetourAttach(&(PVOID&)Real_RegSetValueExW, (PVOID)Mine_RegSetValueExW);

		fInitialized = true;
	}
}

CFilterMapper2::CFilterMapper2(bool fRefCounted, bool fAllowUnreg, LPUNKNOWN pUnkOuter)
: CUnknown(NAME("CFilterMapper2"), pUnkOuter)
, m_fRefCounted(fRefCounted), m_fAllowUnreg(fAllowUnreg)
{
	m_cRef = fRefCounted ? 0 : 1;

	Init();

	HRESULT hr = Real_CoCreateInstance(
		CLSID_FilterMapper2, (IUnknown*)(INonDelegatingUnknown*)this, CLSCTX_ALL, 
		__uuidof(IUnknown), (void**)&m_pFM2);
	if(FAILED(hr) || !m_pFM2)
	{
		ASSERT(0);
		return;
	}
}

CFilterMapper2::~CFilterMapper2()
{
	POSITION pos = m_filters.GetHeadPosition();
	while(pos) delete m_filters.GetNext(pos);
}

STDMETHODIMP CFilterMapper2::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if(riid == __uuidof(IFilterMapper2))
		return GetInterface((IFilterMapper2*)this, ppv);

	HRESULT hr = m_pFM2 ? m_pFM2->QueryInterface(riid, ppv) : E_NOINTERFACE;

	return 
		SUCCEEDED(hr) ? hr :
		__super::NonDelegatingQueryInterface(riid, ppv);
}
void CFilterMapper2::Register(CString path)
{
	RegisterReal(path.GetBuffer());
	path.ReleaseBuffer();

}
void CFilterMapper2::RegisterReal(LPCTSTR path)
{
	HMODULE h = NULL;
	__try{
		h =  LoadLibrary(path);
	}__except(EXCEPTION_EXECUTE_HANDLER) {  }
	if( h )
	{
		typedef HRESULT (__stdcall * PDllRegisterServer)();
		if(PDllRegisterServer p = (PDllRegisterServer)GetProcAddress(h, "DllRegisterServer"))
		{
			ASSERT(CFilterMapper2::m_pFilterMapper2 == NULL);

			CFilterMapper2::m_pFilterMapper2 = this;
			m_path = path;
			__try{
				p();
			}__except(EXCEPTION_EXECUTE_HANDLER) {  }
			m_path.Empty();
			CFilterMapper2::m_pFilterMapper2 = NULL;
		}
		__try{
			FreeLibrary(h);
		}__except(EXCEPTION_EXECUTE_HANDLER) {  }
	}
}

// IFilterMapper2

STDMETHODIMP CFilterMapper2::CreateCategory(REFCLSID clsidCategory, DWORD dwCategoryMerit, LPCWSTR Description)
{
	TRACE_SVP5(_T("CFilterMapper2::CreateCategory %s") , m_path);
	if(!m_path.IsEmpty())
	{
		return S_OK;
	}
	else if(CComQIPtr<IFilterMapper2> pFM2 = m_pFM2)
	{
		return pFM2->CreateCategory(clsidCategory, dwCategoryMerit, Description);
	}

	return E_NOTIMPL;
}

STDMETHODIMP CFilterMapper2::UnregisterFilter(const CLSID* pclsidCategory, const OLECHAR* szInstance, REFCLSID Filter)
{
	TRACE_SVP5(_T("CFilterMapper2::UnregisterFilter %s") , m_path);
	if(!m_path.IsEmpty())
	{
		return S_OK;
	}
	else if(CComQIPtr<IFilterMapper2> pFM2 = m_pFM2)
	{
		return m_fAllowUnreg 
			? pFM2->UnregisterFilter(pclsidCategory, szInstance, Filter) 
			: S_OK;
	}

	return E_NOTIMPL;
}

STDMETHODIMP CFilterMapper2::RegisterFilter(REFCLSID clsidFilter, LPCWSTR Name, IMoniker** ppMoniker, const CLSID* pclsidCategory, const OLECHAR* szInstance, const REGFILTER2* prf2)
{
	TRACE_SVP5(_T("CFilterMapper2::RegisterFilter %s") , m_path);
	if(!m_path.IsEmpty())
	{
		if(FilterOverride* f = new FilterOverride)
		{
			f->fDisabled = false;
			f->type = FilterOverride::EXTERNAL;
			f->path = m_path;
			f->name = CStringW(Name);
			f->clsid = clsidFilter;
			f->iLoadType = FilterOverride::MERIT;
			f->dwMerit = prf2->dwMerit;

			if(prf2->dwVersion == 1)
			{
				for(ULONG i = 0; i < prf2->cPins; i++)
				{
					const REGFILTERPINS& rgPin = prf2->rgPins[i];
					if(rgPin.bOutput) continue;

					for(UINT i = 0; i < rgPin.nMediaTypes; i++)
					{
						if(!rgPin.lpMediaType[i].clsMajorType || !rgPin.lpMediaType[i].clsMinorType) break;
						f->guids.AddTail(*rgPin.lpMediaType[i].clsMajorType);
						f->guids.AddTail(*rgPin.lpMediaType[i].clsMinorType);
					}
				}
			}
			else if(prf2->dwVersion == 2)
			{
				for(ULONG i = 0; i < prf2->cPins2; i++)
				{
					const REGFILTERPINS2& rgPin = prf2->rgPins2[i];
					if(rgPin.dwFlags&REG_PINFLAG_B_OUTPUT) continue;

					for(UINT i = 0; i < rgPin.nMediaTypes; i++)
					{
						if(!rgPin.lpMediaType[i].clsMajorType || !rgPin.lpMediaType[i].clsMinorType) break;
						f->guids.AddTail(*rgPin.lpMediaType[i].clsMajorType);
						f->guids.AddTail(*rgPin.lpMediaType[i].clsMinorType);
					}
				}
			}

			f->backup.AddTailList(&f->guids);

			m_filters.AddTail(f);
		}

		return S_OK;
	}
	else if(CComQIPtr<IFilterMapper2> pFM2 = m_pFM2)
	{
		return pFM2->RegisterFilter(clsidFilter, Name, ppMoniker, pclsidCategory, szInstance, prf2);
	}

	return E_NOTIMPL;
}

STDMETHODIMP CFilterMapper2::EnumMatchingFilters(IEnumMoniker** ppEnum, DWORD dwFlags, BOOL bExactMatch, DWORD dwMerit, 
												 BOOL bInputNeeded, DWORD cInputTypes, const GUID* pInputTypes, const REGPINMEDIUM* pMedIn, const CLSID* pPinCategoryIn, BOOL bRender, 
												 BOOL bOutputNeeded, DWORD cOutputTypes, const GUID* pOutputTypes, const REGPINMEDIUM* pMedOut, const CLSID* pPinCategoryOut)
{
	TRACE_SVP5(_T("CFilterMapper2::EnumMatchingFilters %s") , m_path);
	if(CComQIPtr<IFilterMapper2> pFM2 = m_pFM2)
	{
		TRACE_SVP5(_T("Real CFilterMapper2::EnumMatchingFilters %s") , m_path);
		pFM2->EnumMatchingFilters(ppEnum, dwFlags, bExactMatch, dwMerit, 
			bInputNeeded, cInputTypes, pInputTypes, pMedIn, pPinCategoryIn, bRender, 
			bOutputNeeded, cOutputTypes, pOutputTypes, pMedOut, pPinCategoryOut);
	}

	return E_NOTIMPL;
}
