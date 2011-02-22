/*
 * $Id: DX9AllocatorPresenter.cpp 1260 2009-08-30 07:09:32Z ar-jar $
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
#include "mplayerc.h"
#include <atlbase.h>
#include <atlcoll.h>
#include "..\..\DSUtil\DSUtil.h"
#include <strsafe.h> // Required in CGenlock

#include <Videoacc.h>

#include <initguid.h>
#include "DX9AllocatorPresenter.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <Vmr9.h>
#include "..\..\SubPic\DX9SubPic.h"
#include <RealMedia\pntypes.h>
#include <RealMedia\pnwintyp.h>
#include <RealMedia\pncom.h>
#include <RealMedia\rmavsurf.h>
#include "IQTVideoSurface.h"
#include <moreuuids.h>

#include "MacrovisionKicker.h"
#include "IPinHook.h"

#include "PixelShaderCompiler.h"
#include "MainFrm.h"

#include "AllocatorCommon.h"

#define FRAMERATE_MAX_DELTA			3000

CCritSec g_ffdshowReceive;
bool queueu_ffdshow_support = false;

CString GetWindowsErrorMessage(HRESULT _Error, HMODULE _Module)
{
	switch (_Error)
	{
	case D3DERR_WRONGTEXTUREFORMAT               : return _T("D3DERR_WRONGTEXTUREFORMAT");
	case D3DERR_UNSUPPORTEDCOLOROPERATION        : return _T("D3DERR_UNSUPPORTEDCOLOROPERATION");
	case D3DERR_UNSUPPORTEDCOLORARG              : return _T("D3DERR_UNSUPPORTEDCOLORARG");
	case D3DERR_UNSUPPORTEDALPHAOPERATION        : return _T("D3DERR_UNSUPPORTEDALPHAOPERATION");
	case D3DERR_UNSUPPORTEDALPHAARG              : return _T("D3DERR_UNSUPPORTEDALPHAARG");
	case D3DERR_TOOMANYOPERATIONS                : return _T("D3DERR_TOOMANYOPERATIONS");
	case D3DERR_CONFLICTINGTEXTUREFILTER         : return _T("D3DERR_CONFLICTINGTEXTUREFILTER");
	case D3DERR_UNSUPPORTEDFACTORVALUE           : return _T("D3DERR_UNSUPPORTEDFACTORVALUE");
	case D3DERR_CONFLICTINGRENDERSTATE           : return _T("D3DERR_CONFLICTINGRENDERSTATE");
	case D3DERR_UNSUPPORTEDTEXTUREFILTER         : return _T("D3DERR_UNSUPPORTEDTEXTUREFILTER");
	case D3DERR_CONFLICTINGTEXTUREPALETTE        : return _T("D3DERR_CONFLICTINGTEXTUREPALETTE");
	case D3DERR_DRIVERINTERNALERROR              : return _T("D3DERR_DRIVERINTERNALERROR");
	case D3DERR_NOTFOUND                         : return _T("D3DERR_NOTFOUND");
	case D3DERR_MOREDATA                         : return _T("D3DERR_MOREDATA");
	case D3DERR_DEVICELOST                       : return _T("D3DERR_DEVICELOST");
	case D3DERR_DEVICENOTRESET                   : return _T("D3DERR_DEVICENOTRESET");
	case D3DERR_NOTAVAILABLE                     : return _T("D3DERR_NOTAVAILABLE");
	case D3DERR_OUTOFVIDEOMEMORY                 : return _T("D3DERR_OUTOFVIDEOMEMORY");
	case D3DERR_INVALIDDEVICE                    : return _T("D3DERR_INVALIDDEVICE");
	case D3DERR_INVALIDCALL                      : return _T("D3DERR_INVALIDCALL");
	case D3DERR_DRIVERINVALIDCALL                : return _T("D3DERR_DRIVERINVALIDCALL");
	case D3DERR_WASSTILLDRAWING                  : return _T("D3DERR_WASSTILLDRAWING");
	case D3DOK_NOAUTOGEN                         : return _T("D3DOK_NOAUTOGEN");
	case D3DERR_DEVICEREMOVED                    : return _T("D3DERR_DEVICEREMOVED");
	case S_NOT_RESIDENT                          : return _T("S_NOT_RESIDENT");
	case S_RESIDENT_IN_SHARED_MEMORY             : return _T("S_RESIDENT_IN_SHARED_MEMORY");
	case S_PRESENT_MODE_CHANGED                  : return _T("S_PRESENT_MODE_CHANGED");
	case S_PRESENT_OCCLUDED                      : return _T("S_PRESENT_OCCLUDED");
	case D3DERR_DEVICEHUNG                       : return _T("D3DERR_DEVICEHUNG");
	}

	CString errmsg;
	LPVOID lpMsgBuf;
	if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_HMODULE,
		_Module, _Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL))
	{
		errmsg = (LPCTSTR)lpMsgBuf;
		LocalFree(lpMsgBuf);
	}
	CString Temp;
	Temp.Format(L"0x%08x ", _Error);
	return Temp + errmsg;
}

bool IsVMR9InGraph(IFilterGraph* pFG)
{
	BeginEnumFilters(pFG, pEF, pBF)
		if(CComQIPtr<IVMRWindowlessControl9>(pBF)) return(true);
	EndEnumFilters
	return(false);
}

using namespace DSObjects;

HRESULT CreateAP9(const CLSID& clsid, HWND hWnd, ISubPicAllocatorPresenterRender** ppAP)
{
	CheckPointer(ppAP, E_POINTER);

	*ppAP = NULL;
    SVP_LogMsg5(L"CreateAP9");
	HRESULT hr = E_FAIL;
	CString Error; 
	if(clsid == CLSID_VMR9AllocatorPresenter && !(*ppAP = DNew CVMR9AllocatorPresenter(hWnd, hr))
	|| clsid == CLSID_RM9AllocatorPresenter && !(*ppAP = DNew CRM9AllocatorPresenter(hWnd, hr))
	|| clsid == CLSID_QT9AllocatorPresenter && !(*ppAP = DNew CQT9AllocatorPresenter(hWnd, hr))
	|| clsid == CLSID_DXRAllocatorPresenter && !(*ppAP = DNew CDXRAllocatorPresenter(hWnd, hr))
	|| clsid == CLSID_madVRAllocatorPresenter && !(*ppAP = DNew CmadVRAllocatorPresenter(hWnd, hr)))
		return E_OUTOFMEMORY;

	if(*ppAP == NULL)
		return E_FAIL;

	(*ppAP)->AddRef();

	if(FAILED(hr))
	{
		Error += L"\n";
		Error += GetWindowsErrorMessage(hr, NULL);
		
		//MessageBox(hWnd, Error, L"Error creating DX9 presenter object", MB_OK|MB_ICONERROR);
        SVP_LogMsg5(  L"Error creating DX9 presenter object %s", Error );
		(*ppAP)->Release();
		*ppAP = NULL;
	}
	else if (!Error.IsEmpty())
	{
		//MessageBox(hWnd, Error, L"Warning creating DX9 presenter object", MB_OK|MB_ICONWARNING);
        SVP_LogMsg5(  L"Warning creating DX9 presenter object %s", Error );
	}
	return hr;
}

const wchar_t *GetD3DFormatStr(D3DFORMAT Format)
{
	switch (Format)
	{
	case D3DFMT_R8G8B8 : return L"R8G8B8";
	case D3DFMT_A8R8G8B8 : return L"A8R8G8B8";
	case D3DFMT_X8R8G8B8 : return L"X8R8G8B8";
	case D3DFMT_R5G6B5 : return L"R5G6B5";
	case D3DFMT_X1R5G5B5 : return L"X1R5G5B5";
	case D3DFMT_A1R5G5B5 : return L"A1R5G5B5";
	case D3DFMT_A4R4G4B4 : return L"A4R4G4B4";
	case D3DFMT_R3G3B2 : return L"R3G3B2";
	case D3DFMT_A8 : return L"A8";
	case D3DFMT_A8R3G3B2 : return L"A8R3G3B2";
	case D3DFMT_X4R4G4B4 : return L"X4R4G4B4";
	case D3DFMT_A2B10G10R10: return L"A2B10G10R10";
	case D3DFMT_A8B8G8R8 : return L"A8B8G8R8";
	case D3DFMT_X8B8G8R8 : return L"X8B8G8R8";
	case D3DFMT_G16R16 : return L"G16R16";
	case D3DFMT_A2R10G10B10: return L"A2R10G10B10";
	case D3DFMT_A16B16G16R16 : return L"A16B16G16R16";
	case D3DFMT_A8P8 : return L"A8P8";
	case D3DFMT_P8 : return L"P8";
	case D3DFMT_L8 : return L"L8";
	case D3DFMT_A8L8 : return L"A8L8";
	case D3DFMT_A4L4 : return L"A4L4";
	case D3DFMT_V8U8 : return L"V8U8";
	case D3DFMT_L6V5U5 : return L"L6V5U5";
	case D3DFMT_X8L8V8U8 : return L"X8L8V8U8";
	case D3DFMT_Q8W8V8U8 : return L"Q8W8V8U8";
	case D3DFMT_V16U16 : return L"V16U16";
	case D3DFMT_A2W10V10U10: return L"A2W10V10U10";
	case D3DFMT_UYVY : return L"UYVY";
	case D3DFMT_R8G8_B8G8: return L"R8G8_B8G8";
	case D3DFMT_YUY2 : return L"YUY2";
	case D3DFMT_G8R8_G8B8: return L"G8R8_G8B8";
	case D3DFMT_DXT1 : return L"DXT1";
	case D3DFMT_DXT2 : return L"DXT2";
	case D3DFMT_DXT3 : return L"DXT3";
	case D3DFMT_DXT4 : return L"DXT4";
	case D3DFMT_DXT5 : return L"DXT5";
	case D3DFMT_D16_LOCKABLE : return L"D16_LOCKABLE";
	case D3DFMT_D32: return L"D32";
	case D3DFMT_D15S1: return L"D15S1";
	case D3DFMT_D24S8: return L"D24S8";
	case D3DFMT_D24X8: return L"D24X8";
	case D3DFMT_D24X4S4: return L"D24X4S4";
	case D3DFMT_D16: return L"D16";
	case D3DFMT_D32F_LOCKABLE: return L"D32F_LOCKABLE";
	case D3DFMT_D24FS8 : return L"D24FS8";
	case D3DFMT_D32_LOCKABLE : return L"D32_LOCKABLE";
	case D3DFMT_S8_LOCKABLE: return L"S8_LOCKABLE";
	case D3DFMT_L16: return L"L16";
	case D3DFMT_VERTEXDATA : return L"VERTEXDATA";
	case D3DFMT_INDEX16: return L"INDEX16";
	case D3DFMT_INDEX32: return L"INDEX32";
	case D3DFMT_Q16W16V16U16 : return L"Q16W16V16U16";
	case D3DFMT_MULTI2_ARGB8 : return L"MULTI2_ARGB8";
	case D3DFMT_R16F : return L"R16F";
	case D3DFMT_G16R16F: return L"G16R16F";
	case D3DFMT_A16B16G16R16F: return L"A16B16G16R16F";
	case D3DFMT_R32F : return L"R32F";
	case D3DFMT_G32R32F: return L"G32R32F";
	case D3DFMT_A32B32G32R32F: return L"A32B32G32R32F";
	case D3DFMT_CxV8U8 : return L"CxV8U8";
	case D3DFMT_A1 : return L"A1";
	case D3DFMT_BINARYBUFFER : return L"BINARYBUFFER";
	}
	return L"Unknown";
}

#pragma pack(push, 1)
template<int texcoords>
struct MYD3DVERTEX {float x, y, z, rhw; struct {float u, v;} t[texcoords];};
template<>
struct MYD3DVERTEX<0> 
{
	float x, y, z, rhw; 
	DWORD Diffuse;
};
#pragma pack(pop)

template<int texcoords>
static void AdjustQuad(MYD3DVERTEX<texcoords>* v, double dx, double dy)
{
	double offset = 0.5;

	for(int i = 0; i < 4; i++)
	{
		v[i].x -= offset;
		v[i].y -= offset;
		
		for(int j = 0; j < max(texcoords-1, 1); j++)
		{
			v[i].t[j].u -= offset*dx;
			v[i].t[j].v -= offset*dy;
		}

		if(texcoords > 1)
		{
			v[i].t[texcoords-1].u -= offset;
			v[i].t[texcoords-1].v -= offset;
		}
	}
}

template<int texcoords>
static HRESULT TextureBlt(CComPtr<IDirect3DDevice9> pD3DDev, MYD3DVERTEX<texcoords> v[4], D3DTEXTUREFILTERTYPE filter = D3DTEXF_LINEAR)
{
	if(!pD3DDev) return E_POINTER;

	DWORD FVF = 0;
	switch(texcoords)
	{
	case 1: FVF = D3DFVF_TEX1; break;
	case 2: FVF = D3DFVF_TEX2; break;
	case 3: FVF = D3DFVF_TEX3; break;
	case 4: FVF = D3DFVF_TEX4; break;
	case 5: FVF = D3DFVF_TEX5; break;
	case 6: FVF = D3DFVF_TEX6; break;
	case 7: FVF = D3DFVF_TEX7; break;
	case 8: FVF = D3DFVF_TEX8; break;
	default: return E_FAIL;
	}

	HRESULT hr;
    do
	{
        hr = pD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        hr = pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
		hr = pD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
		hr = pD3DDev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    	hr = pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		hr = pD3DDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE); 
		hr = pD3DDev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE); 
		hr = pD3DDev->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_RED); 

		for(int i = 0; i < texcoords; i++)
		{
			hr = pD3DDev->SetSamplerState(i, D3DSAMP_MAGFILTER, filter);
			hr = pD3DDev->SetSamplerState(i, D3DSAMP_MINFILTER, filter);
			hr = pD3DDev->SetSamplerState(i, D3DSAMP_MIPFILTER, filter);

			hr = pD3DDev->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			hr = pD3DDev->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		}

        hr = pD3DDev->SetFVF(D3DFVF_XYZRHW | FVF);

		MYD3DVERTEX<texcoords> tmp = v[2]; v[2] = v[3]; v[3] = tmp;
		hr = pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(v[0]));	

		for(int i = 0; i < texcoords; i++)
		{
			pD3DDev->SetTexture(i, NULL);
		}

		return S_OK;
    }
	while(0);
    return E_FAIL;
}

static HRESULT DrawRect(CComPtr<IDirect3DDevice9> pD3DDev, MYD3DVERTEX<0> v[4])
{
	if(!pD3DDev) return E_POINTER;

    do
	{
        HRESULT hr = pD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        hr = pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
		hr = pD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
		hr = pD3DDev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    	hr = pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		hr = pD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); 
		hr = pD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA); 
		hr = pD3DDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE); 
		hr = pD3DDev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE); 

		hr = pD3DDev->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_RED); 

        hr = pD3DDev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX0 | D3DFVF_DIFFUSE);

		MYD3DVERTEX<0> tmp = v[2]; v[2] = v[3]; v[3] = tmp;
		hr = pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(v[0]));	

		return S_OK;
    }
	while(0);
    return E_FAIL;
}

CDX9AllocatorPresenter::CDX9AllocatorPresenter(HWND hWnd, HRESULT& hr, bool bIsEVR ):
	ISubPicAllocatorPresenterImpl(hWnd, hr ),
	m_ScreenSize(0, 0),
	m_bicubicA(0),
	m_nDXSurface(1),
	m_nVMR9Surfaces(0),
	m_iVMR9Surface(0),
	m_nCurSurface(0),
	m_bSnapToVSync(false),
	m_bInterlaced(0),
	m_nUsedBuffer(0),
	m_bNeedPendingResetDevice(0),
	m_bPendingResetDevice(0),
	m_bIsEVR(bIsEVR),
	m_TextScale(1.0),
	m_pDirectDraw(NULL),
	m_dMainThreadId(0),
	m_bNeedCheckSample(true),
	m_hEvtQuit(INVALID_HANDLE_VALUE),
	m_bIsFullscreen(0),
	m_pD3DXLoadSurfaceFromMemory(NULL),
	m_pD3DXCreateLine(NULL),
	m_pD3DXCreateFont(NULL),
	m_pD3DXCreateSprite(NULL),
	m_uSyncGlitches(0),
	m_pGenlock(NULL),
	m_lAudioLag(0),
	m_lAudioLagMin(10000),
	m_lAudioLagMax(-10000),
	m_pAudioStats(NULL),
	m_nNextJitter(0),
	m_nNextSyncOffset(0),
	m_llLastSyncTime(0),
	m_fAvrFps(0.0),
	m_fJitterStdDev(0.0),
	m_fSyncOffsetStdDev(0.0),
	m_fSyncOffsetAvr(0.0),
	m_llHysteresis(0),
	m_uD3DRefreshRate(0),
	m_dD3DRefreshCycle(0),
	m_dDetectedScanlineTime(0.0),
	m_dEstRefreshCycle(0.0),
	m_rtFrameCycle(0.0),
	m_dFrameCycle(0.0),
	m_dOptimumDisplayCycle(0.0),
	m_dCycleDifference(1.0),
	m_VSyncDetectThread(NULL)
{
	if(FAILED(hr)) 
	{
		SVP_LogMsg5( L"ISubPicAllocatorPresenterImpl failed"); 
		return;
	}

	HINSTANCE hDll;
	hDll = AfxGetMyApp()->GetD3X9Dll();
	if(hDll)
	{
		(FARPROC&)m_pD3DXLoadSurfaceFromMemory = GetProcAddress(hDll, "D3DXLoadSurfaceFromMemory");
		(FARPROC&)m_pD3DXCreateLine = GetProcAddress(hDll, "D3DXCreateLine");
		(FARPROC&)m_pD3DXCreateFont = GetProcAddress(hDll, "D3DXCreateFontW");
		(FARPROC&)m_pD3DXCreateSprite = GetProcAddress(hDll, "D3DXCreateSprite");		
	}
	else
	{
		SVP_LogMsg5( L"No D3DX9 dll found. To enable stats, shaders and complex resizers, please install the latest DirectX End-User Runtime.");
	}

	m_pDwmIsCompositionEnabled = NULL;
	m_pDwmEnableComposition = NULL;
	m_hDWMAPI = LoadLibrary(L"dwmapi.dll");
	if (m_hDWMAPI)
	{
		(FARPROC &)m_pDwmIsCompositionEnabled = GetProcAddress(m_hDWMAPI, "DwmIsCompositionEnabled");
		(FARPROC &)m_pDwmEnableComposition = GetProcAddress(m_hDWMAPI, "DwmEnableComposition");
	}

	m_hD3D9 = LoadLibrary(L"d3d9.dll");
	if (m_hD3D9)
		(FARPROC &)m_pDirect3DCreate9Ex = GetProcAddress(m_hD3D9, "Direct3DCreate9Ex");
	else
		m_pDirect3DCreate9Ex = NULL;

	ZeroMemory(&m_VMR9AlphaBitmap, sizeof(m_VMR9AlphaBitmap));

	AppSettings& s = AfxGetAppSettings();
	if (s.m_RenderSettings.iVMRDisableDesktopComposition)
	{
		m_bDesktopCompositionDisabled = true;
		if (m_pDwmEnableComposition) m_pDwmEnableComposition(0);
	}
	else
	{
		m_bDesktopCompositionDisabled = false;
	}

	m_pGenlock = new CGenlock(s.m_RenderSettings.fTargetSyncOffset, s.m_RenderSettings.fControlLimit, s.m_RenderSettings.iLineDelta, s.m_RenderSettings.iColumnDelta, s.m_RenderSettings.fCycleDelta, 0); // Must be done before CreateDevice
	hr = CreateDevice();
	memset (m_pllJitter, 0, sizeof(m_pllJitter));
	memset (m_pllSyncOffset, 0, sizeof(m_pllSyncOffset));
}

CDX9AllocatorPresenter::~CDX9AllocatorPresenter() 
{
	CAutoLock threadLock(&m_csTread);
	if(m_VSyncDetectThread){
		//WaitForSingleObject(m_VSyncDetectThread->m_hThread, 1000);
		//TerminateThread(m_VSyncDetectThread->m_hThread, 0);
		m_VSyncDetectThread = NULL;
	}
	if (m_bDesktopCompositionDisabled)
	{
		m_bDesktopCompositionDisabled = false;
		if (m_pDwmEnableComposition)
			m_pDwmEnableComposition(1);
	}

	m_pFont = NULL;
	m_pLine = NULL;
    m_pD3DDev = NULL;
	m_pD3DDevEx = NULL;
	m_pPSC.Free();
	m_pD3D = NULL;
	m_pD3DEx = NULL;
	if (m_hDWMAPI)
	{
		FreeLibrary(m_hDWMAPI);
		m_hDWMAPI = NULL;
	}
	if (m_hD3D9)
	{
		FreeLibrary(m_hD3D9);
		m_hD3D9 = NULL;
	}
	m_pAudioStats = NULL;
	if (m_pGenlock)
	{
		delete m_pGenlock;
		m_pGenlock = NULL;
	}
}

void CDX9AllocatorPresenter::ResetStats()
{
	m_pGenlock->ResetStats();
	m_lAudioLag = 0;
	m_lAudioLagMin = 10000;
	m_lAudioLagMax = -10000;
	m_MinJitter = MAXLONG64;
	m_MaxJitter = MINLONG64;
	m_MinSyncOffset = MAXLONG64;
	m_MaxSyncOffset = MINLONG64;
	m_uSyncGlitches = 0;
}

void CDX9AllocatorPresenter::ThreadBeginDetectVSync(){
	CAutoLock threadLock(&m_csTread);
	ResetStats();
	EstimateRefreshTimings();
}

bool CDX9AllocatorPresenter::SettingsNeedResetDevice()
{
	AppSettings& s = AfxGetAppSettings();
	CMPlayerCApp::Settings::CRendererSettingsEVR & New = AfxGetAppSettings().m_RenderSettings;
	CMPlayerCApp::Settings::CRendererSettingsEVR & Current = m_LastRendererSettings;

	bool bRet = false;
	if (m_bIsFullscreen)
	{
		bRet = bRet || New.iVMR9FullscreenGUISupport != Current.iVMR9FullscreenGUISupport;
	}
	else
	{
		if (Current.iVMRDisableDesktopComposition)
		{
			if (!m_bDesktopCompositionDisabled)
			{
				m_bDesktopCompositionDisabled = true;
				if (m_pDwmEnableComposition)
					m_pDwmEnableComposition(0);
			}
		}
		else
		{
			if (m_bDesktopCompositionDisabled)
			{
				m_bDesktopCompositionDisabled = false;
				if (m_pDwmEnableComposition)
					m_pDwmEnableComposition(1);
			}
		}
	}
	if (m_bIsEVR) bRet = bRet || New.iEVRHighColorResolution != Current.iEVRHighColorResolution;		
	m_LastRendererSettings = s.m_RenderSettings;
	return bRet;
}
void CDX9AllocatorPresenter::ResetGothSyncVars(){
	m_dD3DRefreshCycle = 0; // Display refresh cycle ms
	m_lNextSampleWait = 0; // Waiting time for next sample in EVR
	m_llSampleTime =0;
	m_llLastSampleTime = 0; // Present time for the current sample
	m_llHysteresis = 0; // If != 0 then a "snap to vsync" is active, see EVR
	m_rtEstVSyncTime = 0; // Next vsync time in reference clock "coordinates"
	m_dDetectedScanlineTime = 0; // Time for one (horizontal) scan line. Extracted at stream start and used to calculate vsync time
	m_pRefClock = 0; // The reference clock. Used in Paint()
	m_lShiftToNearest = 0;
	m_lShiftToNearestPrev = 0; // Correction to sample presentation time in sync to nearest
	m_bVideoSlowerThanDisplay = 0; // True if this fact is detected in sync to nearest
	m_bSnapToVSync = 0; // True if framerate is low enough so that snap to vsync makes sense
	m_llLastSyncTime = 0;

	m_lOverWaitCounter = 0;

	m_MinJitter = MAXLONG64;
	m_MaxJitter = MINLONG64;
	m_MinSyncOffset = MAXLONG64;
	m_MaxSyncOffset = MINLONG64;

	m_bInterlaced = 0;
	m_nUsedBuffer = 0;
	m_bNeedPendingResetDevice = 0;
	m_bPendingResetDevice = 0;

	m_TextScale = 0.7;


	memset (m_pllJitter, 0, sizeof(m_pllJitter));
	memset (m_pllSyncOffset, 0, sizeof(m_pllSyncOffset));

}
HRESULT CDX9AllocatorPresenter::CreateDevice( )
{
	AppSettings& s = AfxGetAppSettings();
	m_LastRendererSettings = s.m_RenderSettings;

	m_pPSC.Free();
    m_pD3DDev = NULL;
	m_pD3DDevEx = NULL;
	m_pDirectDraw = NULL;

	m_pResizerPixelShader[0] = 0;
	m_pResizerPixelShader[1] = 0;
	m_pResizerPixelShader[2] = 0;
	m_pResizerPixelShader[3] = 0;

	POSITION pos = m_pPixelShadersScreenSpace.GetHeadPosition();
	while(pos)
	{
		CExternalPixelShader &Shader = m_pPixelShadersScreenSpace.GetNext(pos);
		Shader.m_pPixelShader = NULL;
	}
	pos = m_pPixelShaders.GetHeadPosition();
	while(pos)
	{
		CExternalPixelShader &Shader = m_pPixelShaders.GetNext(pos);
		Shader.m_pPixelShader = NULL;
	}
    //SVP_LogMsg5(L"CDX9AllocatorPresenter::CreateDevice start");
	m_pD3DEx = NULL;
	m_pD3D = NULL;

	if (m_pDirect3DCreate9Ex)
	{
		m_pDirect3DCreate9Ex(D3D_SDK_VERSION, &m_pD3DEx);
		if(!m_pD3DEx) 
		{
			m_pDirect3DCreate9Ex(D3D9b_SDK_VERSION, &m_pD3DEx);
		}
	}
	if(!m_pD3DEx) 
	{
		m_pD3D.Attach(Direct3DCreate9(D3D_SDK_VERSION));
		if(!m_pD3D) 
		{
			m_pD3D.Attach(Direct3DCreate9(D3D9b_SDK_VERSION));
		}
		if(!m_pD3D) 
		{
			SVP_LogMsg5( L"Failed to create D3D9\n");
			return E_UNEXPECTED;
		}
	}
	else
		m_pD3D = m_pD3DEx;
	{

		ResetGothSyncVars();
	}

	D3DDISPLAYMODE d3ddm;
	HRESULT hr;
	ZeroMemory(&d3ddm, sizeof(d3ddm));
	UINT CurrentMonitor = GetAdapter(m_pD3D);
	if(FAILED(m_pD3D->GetAdapterDisplayMode(CurrentMonitor, &d3ddm)))
	{
		SVP_LogMsg5( L"GetAdapterDisplayMode failed\n");
		return E_UNEXPECTED;
	}
	m_lastMonitor = m_pD3D->GetAdapterMonitor(CurrentMonitor);

	m_uD3DRefreshRate = d3ddm.RefreshRate;
	DOUBLE dTargetSyncOffset = 500.0/m_uD3DRefreshRate ;
	if(s.m_RenderSettings.fTargetSyncOffset != 1.0 ){
		dTargetSyncOffset *= s.m_RenderSettings.fTargetSyncOffset;
	}
	m_pGenlock->SetTargetSyncOffset(dTargetSyncOffset);
	m_dD3DRefreshCycle = 1000.0 / (double)m_uD3DRefreshRate; // In ms
	m_ScreenSize.SetSize(d3ddm.Width, d3ddm.Height);
	m_pGenlock->SetDisplayResolution(d3ddm.Width, d3ddm.Height);

	m_ScreenSizeCurrent = m_ScreenSize;
	if(s.fbSmoothMutilMonitor)
		EnumDisplayMonitors(NULL, NULL, MonitorEnumProcDxDetect, (LPARAM)&m_ScreenSize);

	SVP_LogMsg5(_T("m_ScreenSize DX9 %d %d ") , m_ScreenSize.cx, m_ScreenSize.cy);

    D3DPRESENT_PARAMETERS pp;
    ZeroMemory(&pp, sizeof(pp));

	BOOL bCompositionEnabled = false;
	if (m_pDwmIsCompositionEnabled) m_pDwmIsCompositionEnabled(&bCompositionEnabled);

	m_bCompositionEnabled = bCompositionEnabled != 0;
	m_bHighColorResolution = s.m_RenderSettings.iEVRHighColorResolution && m_bIsEVR;

	if (m_bIsFullscreen)
	{
		pp.Windowed = false; 
		pp.BackBufferWidth = d3ddm.Width; 
		pp.BackBufferHeight = d3ddm.Height; 
		pp.hDeviceWindow = m_hWnd;
		pp.BackBufferCount = 3; 
		pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		pp.Flags = D3DPRESENTFLAG_VIDEO;
		if (s.m_RenderSettings.iVMR9FullscreenGUISupport && !m_bHighColorResolution)
			pp.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		if (m_bHighColorResolution)
			pp.BackBufferFormat = D3DFMT_A2R10G10B10;
		else
			pp.BackBufferFormat = d3ddm.Format;
		
		m_D3DDevExError = L"No m_pD3DEx";
		if (m_pD3DEx)
		{
			D3DDISPLAYMODEEX DisplayMode;
			ZeroMemory(&DisplayMode, sizeof(DisplayMode));
			DisplayMode.Size = sizeof(DisplayMode);
			m_pD3DEx->GetAdapterDisplayModeEx(GetAdapter(m_pD3DEx), &DisplayMode, NULL);

			DisplayMode.Format = pp.BackBufferFormat;
			pp.FullScreen_RefreshRateInHz = DisplayMode.RefreshRate;

			hr = m_pD3DEx->CreateDeviceEx(
								GetAdapter(m_pD3D), D3DDEVTYPE_HAL, m_hWnd,
								D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED,
								&pp, &DisplayMode, &m_pD3DDevEx);

			m_D3DDevExError = GetWindowsErrorMessage(hr, m_hD3D9);
			if (m_pD3DDevEx)
			{
				m_pD3DDev = m_pD3DDevEx;
				m_BackbufferType = pp.BackBufferFormat;
				m_DisplayType = DisplayMode.Format;
			}
		}
		if (!m_pD3DDev)
		{
			hr = m_pD3D->CreateDevice(
								GetAdapter(m_pD3D), D3DDEVTYPE_HAL, m_hWnd,
								D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED, //D3DCREATE_MANAGED 
								&pp, &m_pD3DDev);
			if (m_pD3DDev)
			{
				m_BackbufferType = pp.BackBufferFormat;
				m_DisplayType = d3ddm.Format;
			}
		}
		if (m_pD3DDev && s.m_RenderSettings.iVMR9FullscreenGUISupport && !m_bHighColorResolution)
		{
			m_pD3DDev->SetDialogBoxMode(true);
		}
		ASSERT(SUCCEEDED (hr));
	}
	else // Windowed
	{
		pp.Windowed = TRUE;
		pp.hDeviceWindow = m_hWnd;
		pp.SwapEffect = D3DSWAPEFFECT_COPY;
		pp.Flags = D3DPRESENTFLAG_VIDEO;
		pp.BackBufferCount = 1; 
		pp.BackBufferWidth = m_ScreenSize.cx;
		pp.BackBufferHeight = m_ScreenSize.cy;
		m_BackbufferType = d3ddm.Format;
		m_DisplayType = d3ddm.Format;
		if (m_bHighColorResolution)
		{
			m_BackbufferType = D3DFMT_A2R10G10B10;
			pp.BackBufferFormat = D3DFMT_A2R10G10B10;
		}
		SVP_LogMsg5(L"Set D3DPRESENT_INTERVAL %d %d",bCompositionEnabled, ((s.fVMRGothSyncFix || s.fVMRSyncFix)));

		if (bCompositionEnabled)
		{
			// Desktop composition takes care of the VSYNC
			pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;//D3DPRESENT_INTERVAL_IMMEDIATE;
			s.fVMRGothSyncFix = 0;
			s.fVMRSyncFix = 0;
			s.m_RenderSettings.bSynchronizeVideo = 0;
			s.m_RenderSettings.bSynchronizeNearest = 0;

		}
		else
		{
			s.m_RenderSettings.bSynchronizeNearest = s.fVMRGothSyncFix;
			if(s.fVMRGothSyncFix || s.fVMRSyncFix)
				pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
			else
				pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}
		if (m_pD3DEx)
		{
			hr = m_pD3DEx->CreateDeviceEx(
								GetAdapter(m_pD3D), D3DDEVTYPE_HAL, m_hWnd,
								D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED,
								&pp, NULL, &m_pD3DDevEx);
			if (m_pD3DDevEx)
				m_pD3DDev = m_pD3DDevEx;
		}
		else
		{
			hr = m_pD3D->CreateDevice(
							GetAdapter(m_pD3D), D3DDEVTYPE_HAL, m_hWnd,
							D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED,
							&pp, &m_pD3DDev);
		}
	}
//SVP_LogMsg5(L"CDX9AllocatorPresenter::CreateDevice end");
	if (m_pD3DDevEx)
	{
		m_pD3DDevEx->SetGPUThreadPriority(7);
	}

	if(FAILED(hr))
	{
		SVP_LogMsg5( L"CreateDevice failed %x\n", hr);
		return hr;
	}

	m_pPSC.Attach(DNew CPixelShaderCompiler(m_pD3DDev, true));
	m_filter = D3DTEXF_NONE;
    ZeroMemory(&m_caps, sizeof(m_caps));
	m_pD3DDev->GetDeviceCaps(&m_caps);

	if((m_caps.StretchRectFilterCaps&D3DPTFILTERCAPS_MINFLINEAR)
	&& (m_caps.StretchRectFilterCaps&D3DPTFILTERCAPS_MAGFLINEAR))
		m_filter = D3DTEXF_LINEAR;

	m_bicubicA = 0;

	CComPtr<ISubPicProvider> pSubPicProvider;
	if(m_pSubPicQueue) m_pSubPicQueue->GetSubPicProvider(&pSubPicProvider);

	CComPtr<ISubPicProvider> pSubPicProvider2;
	if(m_pSubPicQueue2) m_pSubPicQueue2->GetSubPicProvider(&pSubPicProvider2);


	CSize size;
	switch(AfxGetAppSettings().nSPCMaxRes)
	{
	case 0: default: size = m_ScreenSize; break;
	case 1: size.SetSize(1024, 768); break;
	case 2: size.SetSize(800, 600); break;
	case 3: size.SetSize(640, 480); break;
	case 4: size.SetSize(512, 384); break;
	case 5: size.SetSize(384, 288); break;
	case 6: size.SetSize(2560, 1600); break;
	case 7: size.SetSize(1920, 1080); break;
	case 8: size.SetSize(1320, 900); break;
	case 9: size.SetSize(1280, 720); break;
	}


	if(m_pAllocator)
	{
		m_pAllocator->ChangeDevice(m_pD3DDev);
	}
	else
	{
		m_pAllocator = new CDX9SubPicAllocator(m_pD3DDev, size, AfxGetAppSettings().fSPCPow2Tex);
        if(!m_pAllocator){
            SVP_LogMsg5(L"Create SubPicAllocator Failed");
			return E_FAIL;
        }
	}

	hr = S_OK;
	m_pSubPicQueue = AfxGetAppSettings().nSPCSize > 0 
		? (ISubPicQueue*)new CSubPicQueue(AfxGetAppSettings().nSPCSize, m_pAllocator, &hr)
		: (ISubPicQueue*)new CSubPicQueueNoThread(m_pAllocator, &hr);
	if(!m_pSubPicQueue || FAILED(hr))
		return E_FAIL;

	m_pFont = NULL;
	if (m_pD3DXCreateFont)
	{
		int MinSize = 1600;
		int CurrentSize = min(m_ScreenSize.cx, MinSize);
		double Scale = double(CurrentSize) / double(MinSize);
		m_TextScale = Scale;
		m_pD3DXCreateFont(m_pD3DDev,
							 -24.0*Scale,
							 -11.0*Scale,
							 CurrentSize < 800 ? FW_NORMAL : FW_BOLD,
							 0,
							 FALSE,
							 DEFAULT_CHARSET,
							 OUT_DEFAULT_PRECIS,
							 ANTIALIASED_QUALITY,
							 FIXED_PITCH | FF_DONTCARE,
							 L"Lucida Console",
							 &m_pFont);
	}
	m_pSprite = NULL;
	if (m_pD3DXCreateSprite) m_pD3DXCreateSprite( m_pD3DDev, &m_pSprite);
	m_pLine = NULL;
	if (m_pD3DXCreateLine) m_pD3DXCreateLine (m_pD3DDev, &m_pLine);

	HRESULT hr2 = S_OK;
	m_pSubPicQueue2 = AfxGetAppSettings().nSPCSize > 0 
		? (ISubPicQueue*)new CSubPicQueue(AfxGetAppSettings().nSPCSize, m_pAllocator, &hr2)
		: (ISubPicQueue*)new CSubPicQueueNoThread(m_pAllocator, &hr2);

    if( ( !m_pSubPicQueue && !m_pSubPicQueue2 ) || ( FAILED(hr) && FAILED(hr2) )){
        SVP_LogMsg5(L"m_pSubPicQueue2 Fail");	
        return E_FAIL;
    }

	if(m_pSubPicQueue && pSubPicProvider) m_pSubPicQueue->SetSubPicProvider(pSubPicProvider);

	if(m_pSubPicQueue2 && pSubPicProvider2) m_pSubPicQueue2->SetSubPicProvider(pSubPicProvider2);

	return S_OK;
} 

HRESULT CDX9AllocatorPresenter::AllocSurfaces(D3DFORMAT Format)
{
    CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	AppSettings& s = AfxGetAppSettings();

	for(int i = 0; i < m_nDXSurface+2; i++)
	{
		m_pVideoTexture[i] = NULL;
		m_pVideoSurface[i] = NULL;
	}

	m_pScreenSizeTemporaryTexture[0] = NULL;
	m_pScreenSizeTemporaryTexture[1] = NULL;
	m_SurfaceType = Format;

	HRESULT hr;
	if(s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE2D || s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D)
	{
		int nTexturesNeeded = s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D ? m_nDXSurface+2 : 1;

		for(int i = 0; i < nTexturesNeeded; i++)
		{
			if(FAILED(hr = m_pD3DDev->CreateTexture(
				m_NativeVideoSize.cx, m_NativeVideoSize.cy, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pVideoTexture[i], NULL)))
				return hr;

			if(FAILED(hr = m_pVideoTexture[i]->GetSurfaceLevel(0, &m_pVideoSurface[i])))
				return hr;
		}
		if(s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE2D)
		{
			for(int i = 0; i < m_nDXSurface+2; i++)
			{
				m_pVideoTexture[i] = NULL;
			}
		}
	}
	else
	{
		if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(m_NativeVideoSize.cx, m_NativeVideoSize.cy, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pVideoSurface[m_nCurSurface], NULL)))
			return hr;
	}

	hr = m_pD3DDev->ColorFill(m_pVideoSurface[m_nCurSurface], NULL, 0);
	return S_OK;
}

void CDX9AllocatorPresenter::DeleteSurfaces()
{
    CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	for(int i = 0; i < m_nDXSurface+2; i++)
	{
		m_pVideoTexture[i] = NULL;
		m_pVideoSurface[i] = NULL;
	}
}

UINT CDX9AllocatorPresenter::GetAdapter(IDirect3D9* pD3D)
{
	if(m_hWnd == NULL || pD3D == NULL) return D3DADAPTER_DEFAULT;

	HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	if(hMonitor == NULL) return D3DADAPTER_DEFAULT;

	for(UINT adp = 0, num_adp = pD3D->GetAdapterCount(); adp < num_adp; ++adp)
	{
		HMONITOR hAdpMon = pD3D->GetAdapterMonitor(adp);
		if(hAdpMon == hMonitor) return adp;
	}
	return D3DADAPTER_DEFAULT;
}

// ISubPicAllocatorPresenter

STDMETHODIMP CDX9AllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
	return E_NOTIMPL;
}

static bool ClipToSurface(IDirect3DSurface9* pSurface, CRect& s, CRect& d)   
{   
	D3DSURFACE_DESC d3dsd;   
	ZeroMemory(&d3dsd, sizeof(d3dsd));   
	if(FAILED(pSurface->GetDesc(&d3dsd)))   
		return(false);   

	int w = d3dsd.Width, h = d3dsd.Height;   
	int sw = s.Width(), sh = s.Height();   
	int dw = d.Width(), dh = d.Height();   

	if(d.left >= w || d.right < 0 || d.top >= h || d.bottom < 0   
	|| sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0)   
	{   
		s.SetRectEmpty();   
		d.SetRectEmpty();   
		return(true);   
	}   
	if(d.right > w) {s.right -= (d.right-w)*sw/dw; d.right = w;}   
	if(d.bottom > h) {s.bottom -= (d.bottom-h)*sh/dh; d.bottom = h;}   
	if(d.left < 0) {s.left += (0-d.left)*sw/dw; d.left = 0;}   
	if(d.top < 0) {s.top += (0-d.top)*sh/dh; d.top = 0;}   
	return(true);
}

HRESULT CDX9AllocatorPresenter::InitResizers(float bicubicA, bool bNeedScreenSizeTexture)
{
	HRESULT hr;
	do
	{
		if (bicubicA)
		{
			if (!m_pResizerPixelShader[0])
				break;
			if (!m_pResizerPixelShader[1])
				break;
			if (!m_pResizerPixelShader[2])
				break;
			if (!m_pResizerPixelShader[3])
				break;
			if (m_bicubicA != bicubicA)
				break;
			if (!m_pScreenSizeTemporaryTexture[0])
				break;
			if (bNeedScreenSizeTexture)
			{
				if (!m_pScreenSizeTemporaryTexture[1])
					break;
			}
		}
		else
		{
			if (!m_pResizerPixelShader[0])
				break;
			if (bNeedScreenSizeTexture)
			{
				if (!m_pScreenSizeTemporaryTexture[0])
					break;
				if (!m_pScreenSizeTemporaryTexture[1])
					break;
			}
		}
		return S_OK;
	}
	while (0);

	//SVP_LogMsg5(L"Compiler Resize Shader");
	m_bicubicA = bicubicA;
	m_pScreenSizeTemporaryTexture[0] = NULL;
	m_pScreenSizeTemporaryTexture[1] = NULL;

	for(int i = 0; i < countof(m_pResizerPixelShader); i++)
		m_pResizerPixelShader[i] = NULL;

	if(m_caps.PixelShaderVersion < D3DPS_VERSION(2, 0)) return E_FAIL;

	LPCSTR pProfile = m_caps.PixelShaderVersion >= D3DPS_VERSION(3, 0) ? "ps_3_0" : "ps_2_0";

	CStringA str;
	if(!LoadResource(IDF_SHADER_RESIZER, str, _T("FILE"))) return E_FAIL;

	CStringA A;
	A.Format("(%f)", bicubicA);
	str.Replace("_The_Value_Of_A_Is_Set_Here_", A);
	if( m_caps.PixelShaderVersion >= D3DPS_VERSION(3, 0) )
		str.Replace("+0.001", "");
	LPCSTR pEntries[] = {"main_bilinear", "main_bicubic1pass", "main_bicubic2pass_pass1", "main_bicubic2pass_pass2"};

	ASSERT(countof(pEntries) == countof(m_pResizerPixelShader));
	for(int i = 0; i < countof(pEntries); i++)
	{
		CString ErrorMessage;
		CString DissAssembly;
		hr = m_pPSC->CompileShader(str, pEntries[i], pProfile, 0, &m_pResizerPixelShader[i], &DissAssembly, &ErrorMessage);
		if(FAILED(hr)) 
		{
			TRACE("%ws", ErrorMessage.GetString());
			ASSERT (0);
			return hr;
		}
	}
	if(m_bicubicA || bNeedScreenSizeTexture)
	{
		if(FAILED(m_pD3DDev->CreateTexture(
			min(m_ScreenSize.cx, (int)m_caps.MaxTextureWidth), min(max(m_ScreenSize.cy, m_NativeVideoSize.cy), (int)m_caps.MaxTextureHeight), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
			D3DPOOL_DEFAULT, &m_pScreenSizeTemporaryTexture[0], NULL)))
		{
			ASSERT(0);
			m_pScreenSizeTemporaryTexture[0] = NULL; // will do 1 pass then
		}
	}
	if(m_bicubicA || bNeedScreenSizeTexture)
	{
		if(FAILED(m_pD3DDev->CreateTexture(
			min(m_ScreenSize.cx, (int)m_caps.MaxTextureWidth), min(max(m_ScreenSize.cy, m_NativeVideoSize.cy), (int)m_caps.MaxTextureHeight), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
			D3DPOOL_DEFAULT, &m_pScreenSizeTemporaryTexture[1], NULL)))
		{
			ASSERT(0);
			m_pScreenSizeTemporaryTexture[1] = NULL; // will do 1 pass then
		}
	}
	return S_OK;
}

HRESULT CDX9AllocatorPresenter::TextureCopy(CComPtr<IDirect3DTexture9> pTexture)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
		return E_FAIL;

	float w = (float)desc.Width;
	float h = (float)desc.Height;
	MYD3DVERTEX<1> v[] =
	{
		{0, 0, 0.5f, 2.0f, 0, 0},
		{w, 0, 0.5f, 2.0f, 1, 0},
		{0, h, 0.5f, 2.0f, 0, 1},
		{w, h, 0.5f, 2.0f, 1, 1},
	};
	for(int i = 0; i < countof(v); i++)
	{
		v[i].x -= 0.5;
		v[i].y -= 0.5;
	}
	hr = m_pD3DDev->SetTexture(0, pTexture);
	return TextureBlt(m_pD3DDev, v, D3DTEXF_LINEAR);
}

HRESULT CDX9AllocatorPresenter::DrawRect(DWORD _Color, DWORD _Alpha, const CRect &_Rect)
{
	DWORD Color = D3DCOLOR_ARGB(_Alpha, GetRValue(_Color), GetGValue(_Color), GetBValue(_Color));
	MYD3DVERTEX<0> v[] =
	{
		{float(_Rect.left), float(_Rect.top), 0.5f, 2.0f, Color},
		{float(_Rect.right), float(_Rect.top), 0.5f, 2.0f, Color},
		{float(_Rect.left), float(_Rect.bottom), 0.5f, 2.0f, Color},
		{float(_Rect.right), float(_Rect.bottom), 0.5f, 2.0f, Color},
	};
	for(int i = 0; i < countof(v); i++)
	{
		v[i].x -= 0.5;
		v[i].y -= 0.5;
	}
	return ::DrawRect(m_pD3DDev, v);
}

HRESULT CDX9AllocatorPresenter::TextureResize(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], D3DTEXTUREFILTERTYPE filter, const CRect &SrcRect)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
		return E_FAIL;

	float w = (float)desc.Width;
	float h = (float)desc.Height;

	float dx = 1.0f/w;
	float dy = 1.0f/h;
	float dx2 = 1.0/w;
	float dy2 = 1.0/h;

	MYD3DVERTEX<1> v[] =
	{
		{dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z,  SrcRect.left * dx2, SrcRect.top * dy2},
		{dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z,  SrcRect.right * dx2, SrcRect.top * dy2},
		{dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z,  SrcRect.left * dx2, SrcRect.bottom * dy2},
		{dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z,  SrcRect.right * dx2, SrcRect.bottom * dy2},
	};
	AdjustQuad(v, 0, 0);
	hr = m_pD3DDev->SetTexture(0, pTexture);
	hr = m_pD3DDev->SetPixelShader(NULL);
	hr = TextureBlt(m_pD3DDev, v, filter);
	return hr;
}

HRESULT CDX9AllocatorPresenter::TextureResizeBilinear(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], const CRect &SrcRect)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
		return E_FAIL;

	float w = (float)desc.Width;
	float h = (float)desc.Height;

	float dx = 1.0f/w;
	float dy = 1.0f/h;
	float tx0 = SrcRect.left;
	float tx1 = SrcRect.right;
	float ty0 = SrcRect.top;
	float ty1 = SrcRect.bottom;

	MYD3DVERTEX<1> v[] =
	{
		{dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z,  tx0, ty0},
		{dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z,  tx1, ty0},
		{dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z,  tx0, ty1},
		{dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z,  tx1, ty1},
	};
	AdjustQuad(v, 1.0, 1.0);
	float fConstData[][4] = {{0.5f / w, 0.5f / h, 0, 0}, {1.0f / w, 1.0f / h, 0, 0}, {1.0f / w, 0, 0, 0}, {0, 1.0f / h, 0, 0}, {w, h, 0, 0}};
	hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));
	hr = m_pD3DDev->SetTexture(0, pTexture);
	hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShader[0]);
	hr = TextureBlt(m_pD3DDev, v, D3DTEXF_POINT);
	m_pD3DDev->SetPixelShader(NULL);
	return hr;
}

HRESULT CDX9AllocatorPresenter::TextureResizeBicubic1pass(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], const CRect &SrcRect)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
		return E_FAIL;

	double w = (double)desc.Width;
	double h = (double)desc.Height;

	double sw = SrcRect.Width();
	double sh = SrcRect.Height();

	double dx = 1.0f/w;
	double dy = 1.0f/h;

	float dx2 = 1.0f/w;
	float dy2 = 1.0f/h;
	float tx0 = SrcRect.left;
	float tx1 = SrcRect.right;
	float ty0 = SrcRect.top;
	float ty1 = SrcRect.bottom;

	MYD3DVERTEX<1> v[] =
	{
		{dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z,  tx0, ty0},
		{dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z,  tx1, ty0},
		{dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z,  tx0, ty1},
		{dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z,  tx1, ty1},
	};
	AdjustQuad(v, 1.0, 1.0);
	hr = m_pD3DDev->SetTexture(0, pTexture);
	float fConstData[][4] = {{0.5f / w, 0.5f / h, 0, 0}, {1.0f / w, 1.0f / h, 0, 0}, {1.0f / w, 0, 0, 0}, {0, 1.0f / h, 0, 0}, {w, h, 0, 0}};
	hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));
	hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShader[1]);
	hr = TextureBlt(m_pD3DDev, v, D3DTEXF_POINT);
	m_pD3DDev->SetPixelShader(NULL);
	return hr;
}

HRESULT CDX9AllocatorPresenter::TextureResizeBicubic2pass(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], const CRect &SrcRect)
{
	// The 2 pass sampler is incorrect in that it only does bilinear resampling in the y direction.
	return TextureResizeBicubic1pass(pTexture, dst, SrcRect);

	HRESULT hr;

	// rotated?
	if(dst[0].z != dst[1].z || dst[2].z != dst[3].z || dst[0].z != dst[3].z
	|| dst[0].y != dst[1].y || dst[0].x != dst[2].x || dst[2].y != dst[3].y || dst[1].x != dst[3].x)
		return TextureResizeBicubic1pass(pTexture, dst, SrcRect);

	D3DSURFACE_DESC desc;
	if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
		return E_FAIL;

	float Tex0_Width = desc.Width;
	float Tex0_Height = desc.Height;

	double dx0 = 1.0/desc.Width;
	double dy0 = 1.0/desc.Height;

	CSize SrcTextSize = CSize(desc.Width, desc.Height);
	double w = (double)SrcRect.Width();
	double h = (double)SrcRect.Height();

	CRect dst1(0, 0, (int)(dst[3].x - dst[0].x), (int)h);

	if(!m_pScreenSizeTemporaryTexture[0] || FAILED(m_pScreenSizeTemporaryTexture[0]->GetLevelDesc(0, &desc)))
		return TextureResizeBicubic1pass(pTexture, dst, SrcRect);

	float Tex1_Width = desc.Width;
	float Tex1_Height = desc.Height;

	double dx1 = 1.0/desc.Width;
	double dy1 = 1.0/desc.Height;

	double dw = (double)dst1.Width() / desc.Width;
	double dh = (double)dst1.Height() / desc.Height;

	float dx2 = 1.0f/SrcTextSize.cx;
	float dy2 = 1.0f/SrcTextSize.cy;
	float tx0 = SrcRect.left;
	float tx1 = SrcRect.right;
	float ty0 = SrcRect.top;
	float ty1 = SrcRect.bottom;

	float tx0_2 = 0;
	float tx1_2 = dst1.Width();
	float ty0_2 = 0;
	float ty1_2 = h;

	if(dst1.Width() > (int)desc.Width || dst1.Height() > (int)desc.Height)
		return TextureResizeBicubic1pass(pTexture, dst, SrcRect);

	MYD3DVERTEX<1> vx[] =
	{
		{(float)dst1.left, (float)dst1.top,		0.5f, 2.0f, tx0, ty0},
		{(float)dst1.right, (float)dst1.top,	0.5f, 2.0f, tx1, ty0},
		{(float)dst1.left, (float)dst1.bottom,	0.5f, 2.0f, tx0, ty1},
		{(float)dst1.right, (float)dst1.bottom, 0.5f, 2.0f, tx1, ty1},
	};
	AdjustQuad(vx, 1.0, 0.0);		// Casimir666 : bug ici, génére des bandes verticales! TODO : pourquoi ??????
	MYD3DVERTEX<1> vy[] =
	{
		{dst[0].x, dst[0].y, dst[0].z, 1.0/dst[0].z, tx0_2, ty0_2},
		{dst[1].x, dst[1].y, dst[1].z, 1.0/dst[1].z, tx1_2, ty0_2},
		{dst[2].x, dst[2].y, dst[2].z, 1.0/dst[2].z, tx0_2, ty1_2},
		{dst[3].x, dst[3].y, dst[3].z, 1.0/dst[3].z, tx1_2, ty1_2},
	};
	AdjustQuad(vy, 0.0, 1.0);
	hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShader[2]);
	{
		float fConstData[][4] = {{0.5f / Tex0_Width, 0.5f / Tex0_Height, 0, 0}, {1.0f / Tex0_Width, 1.0f / Tex0_Height, 0, 0}, {1.0f / Tex0_Width, 0, 0, 0}, {0, 1.0f / Tex0_Height, 0, 0}, {Tex0_Width, Tex0_Height, 0, 0}};
		hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));
	}
	hr = m_pD3DDev->SetTexture(0, pTexture);
	CComPtr<IDirect3DSurface9> pRTOld;
	hr = m_pD3DDev->GetRenderTarget(0, &pRTOld);
	CComPtr<IDirect3DSurface9> pRT;
	hr = m_pScreenSizeTemporaryTexture[0]->GetSurfaceLevel(0, &pRT);
	hr = m_pD3DDev->SetRenderTarget(0, pRT);
	hr = TextureBlt(m_pD3DDev, vx, D3DTEXF_POINT);
	hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShader[3]);
	{
		float fConstData[][4] = {{0.5f / Tex1_Width, 0.5f / Tex1_Height, 0, 0}, {1.0f / Tex1_Width, 1.0f / Tex1_Height, 0, 0}, {1.0f / Tex1_Width, 0, 0, 0}, {0, 1.0f / Tex1_Height, 0, 0}, {Tex1_Width, Tex1_Height, 0, 0}};
		hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));
	}
	hr = m_pD3DDev->SetTexture(0, m_pScreenSizeTemporaryTexture[0]);
	hr = m_pD3DDev->SetRenderTarget(0, pRTOld);
	hr = TextureBlt(m_pD3DDev, vy, D3DTEXF_POINT);
	m_pD3DDev->SetPixelShader(NULL);
	return hr;
}

HRESULT CDX9AllocatorPresenter::AlphaBlt(RECT* pSrc, RECT* pDst, CComPtr<IDirect3DTexture9> pTexture)
{
	if(!pSrc || !pDst)
		return E_POINTER;

	CRect src(*pSrc), dst(*pDst);

	HRESULT hr;

    do
	{
		D3DSURFACE_DESC d3dsd;
		ZeroMemory(&d3dsd, sizeof(d3dsd));
		if(FAILED(pTexture->GetLevelDesc(0, &d3dsd)) /*|| d3dsd.Type != D3DRTYPE_TEXTURE*/)
			break;

        float w = (float)d3dsd.Width;
        float h = (float)d3dsd.Height;

		struct
		{
			float x, y, z, rhw;
			float tu, tv;
		}
		pVertices[] =
		{
			{(float)dst.left, (float)dst.top, 0.5f, 2.0f, (float)src.left / w, (float)src.top / h},
			{(float)dst.right, (float)dst.top, 0.5f, 2.0f, (float)src.right / w, (float)src.top / h},
			{(float)dst.left, (float)dst.bottom, 0.5f, 2.0f, (float)src.left / w, (float)src.bottom / h},
			{(float)dst.right, (float)dst.bottom, 0.5f, 2.0f, (float)src.right / w, (float)src.bottom / h},
		};

        hr = m_pD3DDev->SetTexture(0, pTexture);

		DWORD abe, sb, db;
		hr = m_pD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &abe);
		hr = m_pD3DDev->GetRenderState(D3DRS_SRCBLEND, &sb);
		hr = m_pD3DDev->GetRenderState(D3DRS_DESTBLEND, &db);

        hr = m_pD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        hr = m_pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
		hr = m_pD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
    	hr = m_pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        hr = m_pD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE); // pre-multiplied src and ...
        hr = m_pD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA); // ... inverse alpha channel for dst

		hr = m_pD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        hr = m_pD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        hr = m_pD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

        hr = m_pD3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        hr = m_pD3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        hr = m_pD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		hr = m_pD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		hr = m_pD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

        hr = m_pD3DDev->SetPixelShader(NULL);

        hr = m_pD3DDev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
		hr = m_pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertices, sizeof(pVertices[0]));

		m_pD3DDev->SetTexture(0, NULL);

    	m_pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, abe);
        m_pD3DDev->SetRenderState(D3DRS_SRCBLEND, sb);
        m_pD3DDev->SetRenderState(D3DRS_DESTBLEND, db);

		return S_OK;
    }
	while(0);
    return E_FAIL;
}

// Update the array m_pllJitter with a new vsync period. Calculate min, max and stddev.
void CDX9AllocatorPresenter::SyncStats(LONGLONG syncTime)
{
	m_nNextJitter = (m_nNextJitter+1) % NB_JITTER;
	m_pllJitter[m_nNextJitter] = syncTime - m_llLastSyncTime;
	double syncDeviation = ((double)m_pllJitter[m_nNextJitter] - m_fJitterMean) / 10000.0;
	if (abs(syncDeviation) > (GetDisplayCycle() / 2))
		m_uSyncGlitches++;

	LONGLONG llJitterSum = 0;
	LONGLONG llJitterSumAvg = 0;
	for (int i=0; i<NB_JITTER; i++)
	{
		LONGLONG Jitter = m_pllJitter[i];
		llJitterSum += Jitter;
		llJitterSumAvg += Jitter;
	}
	m_fJitterMean = double(llJitterSumAvg) / NB_JITTER;
	double DeviationSum = 0;

	for (int i=0; i<NB_JITTER; i++)
	{
		LONGLONG DevInt = m_pllJitter[i] - m_fJitterMean;
		double Deviation = DevInt;
		DeviationSum += Deviation*Deviation;
		m_MaxJitter = max(m_MaxJitter, DevInt);
		m_MinJitter = min(m_MinJitter, DevInt);
	}

	m_fJitterStdDev = sqrt(DeviationSum/NB_JITTER);
	m_fAvrFps = 10000000.0/(double(llJitterSum)/NB_JITTER);
	m_llLastSyncTime = syncTime;
}

// Collect the difference between periodEnd and periodStart in an array, calculate mean and stddev.
void CDX9AllocatorPresenter::SyncOffsetStats(LONGLONG syncOffset)
{
	m_nNextSyncOffset = (m_nNextSyncOffset+1) % NB_JITTER;
	m_pllSyncOffset[m_nNextSyncOffset] = syncOffset;

	LONGLONG AvrageSum = 0;
	for (int i=0; i<NB_JITTER; i++)
	{
		LONGLONG Offset = m_pllSyncOffset[i];
		AvrageSum += Offset;
		m_MaxSyncOffset = max(m_MaxSyncOffset, Offset);
		m_MinSyncOffset = min(m_MinSyncOffset, Offset);
	}
	double MeanOffset = double(AvrageSum)/NB_JITTER;
	double DeviationSum = 0;
	for (int i=0; i<NB_JITTER; i++)
	{
		double Deviation = double(m_pllSyncOffset[i]) - MeanOffset;
		DeviationSum += Deviation*Deviation;
	}
	double StdDev = sqrt(DeviationSum/NB_JITTER);

	m_fSyncOffsetAvr = MeanOffset;
	m_fSyncOffsetStdDev = StdDev;
}

void CDX9AllocatorPresenter::UpdateAlphaBitmap()
{
	m_VMR9AlphaBitmapData.Free();

	if ((m_VMR9AlphaBitmap.dwFlags & VMRBITMAP_DISABLE) == 0)
	{
		HBITMAP			hBitmap = (HBITMAP)GetCurrentObject (m_VMR9AlphaBitmap.hdc, OBJ_BITMAP);
		if (!hBitmap)
			return;
		DIBSECTION		info = {0};
		if (!::GetObject(hBitmap, sizeof( DIBSECTION ), &info ))
			return;

		m_VMR9AlphaBitmapRect = CRect(0, 0, info.dsBm.bmWidth, info.dsBm.bmHeight);
		m_VMR9AlphaBitmapWidthBytes = info.dsBm.bmWidthBytes;

		if (m_VMR9AlphaBitmapData.Allocate(info.dsBm.bmWidthBytes * info.dsBm.bmHeight))
		{
			memcpy((BYTE *)m_VMR9AlphaBitmapData, info.dsBm.bmBits, info.dsBm.bmWidthBytes * info.dsBm.bmHeight);
		}
	}
}
UINT __cdecl ThreadDX9AllocatorRedetectVSync( LPVOID lpParam ) 
{ 
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if(pFrame){
		CAutoLock mOpenCloseLock(&pFrame->m_csOpenClose);
		CDX9AllocatorPresenter* pDX9 = (CDX9AllocatorPresenter*)lpParam;
		pDX9->ThreadBeginDetectVSync();
	}
	return 0; 
}
// Present a sample (frame) using DirectX.
STDMETHODIMP_(bool) CDX9AllocatorPresenter::Paint(bool fAll)
{
	AppSettings& s = AfxGetAppSettings();
	D3DRASTER_STATUS rasterStatus;
	REFERENCE_TIME rtSyncOffset = 0;
	double msSyncOffset = 0.0;
	REFERENCE_TIME rtCurRefTime = 0;
	
	CMPlayerCApp * pApp = AfxGetMyApp();
	CAutoLock cRenderLock(&m_RenderLock);
	
	if(m_pRefClock && s.fVMRGothSyncFix && s.m_RenderSettings.bSynchronizeNearest){
		m_pD3DDev->GetRasterStatus(0, &rasterStatus);	
		m_uScanLineEnteringPaint = rasterStatus.ScanLine;
		if(m_pRefClock){
			m_pRefClock->GetTime(&rtCurRefTime);
		}
		msSyncOffset = (m_ScreenSizeCurrent.cy - m_uScanLineEnteringPaint) * m_dDetectedScanlineTime;
		rtSyncOffset = REFERENCE_TIME(10000.0 * msSyncOffset);
		m_rtEstVSyncTime = rtCurRefTime + rtSyncOffset;
		
	}

	if(m_WindowRect.right <= m_WindowRect.left || m_WindowRect.bottom <= m_WindowRect.top
		|| m_NativeVideoSize.cx <= 0 || m_NativeVideoSize.cy <= 0
		|| !m_pVideoSurface)
	{
		return(false);
	}

	HRESULT hr;
	CRect rSrcVid(CPoint(0, 0), m_NativeVideoSize);
	CRect rDstVid(m_VideoRect);
	CRect rSrcPri(CPoint(0, 0), m_WindowRect.Size());
	CRect rDstPri(m_WindowRect);

	m_pD3DDev->BeginScene();
	CComPtr<IDirect3DSurface9> pBackBuffer;
	m_pD3DDev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
	m_pD3DDev->SetRenderTarget(0, pBackBuffer);
	hr = m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);
	if(!rDstVid.IsRectEmpty())
	{
		if(m_pVideoTexture[m_nCurSurface])
		{
			CComPtr<IDirect3DTexture9> pVideoTexture = m_pVideoTexture[m_nCurSurface];
			// If there is a pixel shader
			if(m_pVideoTexture[m_nDXSurface] && m_pVideoTexture[m_nDXSurface+1] && !m_pPixelShaders.IsEmpty())
			{
				static __int64 counter = 0;
				static long start = clock();
				long stop = clock();
				long diff = stop - start;
				if(diff >= 10*60*CLOCKS_PER_SEC) start = stop; // reset after 10 min (ps float has its limits in both range and accuracy)
				int src = m_nCurSurface, dst = m_nDXSurface;
				D3DSURFACE_DESC desc;
				m_pVideoTexture[src]->GetLevelDesc(0, &desc);
				float fConstData[][4] = 
				{
					{(float)desc.Width, (float)desc.Height, (float)(counter++), (float)diff / CLOCKS_PER_SEC},
					{1.0f / desc.Width, 1.0f / desc.Height, 0, 0},
				};

				hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));
				CComPtr<IDirect3DSurface9> pRT;
				hr = m_pD3DDev->GetRenderTarget(0, &pRT);
				POSITION pos = m_pPixelShaders.GetHeadPosition();
				while(pos)
				{
					pVideoTexture = m_pVideoTexture[dst];

					hr = m_pD3DDev->SetRenderTarget(0, m_pVideoSurface[dst]);
					CExternalPixelShader &Shader = m_pPixelShaders.GetNext(pos);
					if (!Shader.m_pPixelShader)
						Shader.Compile(m_pPSC);
					hr = m_pD3DDev->SetPixelShader(Shader.m_pPixelShader);
					TextureCopy(m_pVideoTexture[src]);
					src		= dst;
					if(++dst >= m_nDXSurface+2) dst = m_nDXSurface;
				}
				hr = m_pD3DDev->SetRenderTarget(0, pRT);
				hr = m_pD3DDev->SetPixelShader(NULL);
			}
			Vector dst[4];
			Transform(rDstVid, dst);
			DWORD iDX9Resizer = s.iDX9Resizer;
			float A = 0;
			switch(iDX9Resizer)
			{
			case 3: A = -0.60f; break;
			case 4: A = -0.751f; break;	// FIXME : 0.75 crash recent D3D, or eat CPU 
			case 5: A = -1.00f; break;
			case 7: 
				{
					if(m_WindowRect.Width() > m_NativeVideoSize.cx)
						A = -0.751f ;
					else
						A = -0.60f ;

				}
				break;
			}
			bool bScreenSpacePixelShaders = !m_pPixelShadersScreenSpace.IsEmpty();
			hr = InitResizers(A, bScreenSpacePixelShaders);
			if (!m_pScreenSizeTemporaryTexture[0] || !m_pScreenSizeTemporaryTexture[1])
				bScreenSpacePixelShaders = false;
			if (bScreenSpacePixelShaders)
			{
				CComPtr<IDirect3DSurface9> pRT;
				hr = m_pScreenSizeTemporaryTexture[1]->GetSurfaceLevel(0, &pRT);
				if (hr != S_OK)
					bScreenSpacePixelShaders = false;
				if (bScreenSpacePixelShaders)
				{
					hr = m_pD3DDev->SetRenderTarget(0, pRT);
					if (hr != S_OK)
						bScreenSpacePixelShaders = false;
					hr = m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);
				}
			}
			if(iDX9Resizer == 0 || iDX9Resizer == 1)
			{
				D3DTEXTUREFILTERTYPE Filter = iDX9Resizer == 0 ? D3DTEXF_POINT : D3DTEXF_LINEAR;
				if (rSrcVid.Size() == rDstVid.Size())
					Filter = D3DTEXF_POINT;
				hr = TextureResize(pVideoTexture, dst, Filter, rSrcVid);
			}
			else if(iDX9Resizer == 2)
			{
				hr = TextureResizeBilinear(pVideoTexture, dst, rSrcVid);
			}
			else if(iDX9Resizer >= 3)
			{
				hr = TextureResizeBicubic2pass(pVideoTexture, dst, rSrcVid);
			}
			if (bScreenSpacePixelShaders)
			{
				static __int64 counter = 555;
				static long start = clock() + 333;

				long stop = clock() + 333;
				long diff = stop - start;

				if(diff >= 10*60*CLOCKS_PER_SEC) start = stop; // reset after 10 min (ps float has its limits in both range and accuracy)

				D3DSURFACE_DESC desc;
				m_pScreenSizeTemporaryTexture[0]->GetLevelDesc(0, &desc);

				float fConstData[][4] = 
				{
					{(float)desc.Width, (float)desc.Height, (float)(counter++), (float)diff / CLOCKS_PER_SEC},
					{1.0f / desc.Width, 1.0f / desc.Height, 0, 0},
				};

				hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));

				int src = 1, dst = 0, itmp = 0;

				POSITION pos = m_pPixelShadersScreenSpace.GetHeadPosition();
				while(pos)
				{
					if (m_pPixelShadersScreenSpace.GetTailPosition() == pos)
					{
						m_pD3DDev->SetRenderTarget(0, pBackBuffer);
					}
					else
					{
						CComPtr<IDirect3DSurface9> pRT;
						hr = m_pScreenSizeTemporaryTexture[dst]->GetSurfaceLevel(0, &pRT);
						m_pD3DDev->SetRenderTarget(0, pRT);
					}

					CExternalPixelShader &Shader = m_pPixelShadersScreenSpace.GetNext(pos);
					if (!Shader.m_pPixelShader) Shader.Compile(m_pPSC);
					hr = m_pD3DDev->SetPixelShader(Shader.m_pPixelShader);
					TextureCopy(m_pScreenSizeTemporaryTexture[src]);
					itmp = src;
					src = dst;
					dst = itmp;
				}
				hr = m_pD3DDev->SetPixelShader(NULL);
			}
		}
		else
		{
			if(pBackBuffer)
			{
				ClipToSurface(pBackBuffer, rSrcVid, rDstVid);
				// rSrcVid has to be aligned on mod2 for yuy2->rgb conversion with StretchRect
				rSrcVid.left &= ~1; rSrcVid.right &= ~1;
				rSrcVid.top &= ~1; rSrcVid.bottom &= ~1;
				hr = m_pD3DDev->StretchRect(m_pVideoSurface[m_nCurSurface], rSrcVid, pBackBuffer, rDstVid, m_filter);
				if(FAILED(hr)) return false;
			}
		}
	}
	AlphaBltSubPic(rSrcPri.Size());
	if (0 && m_VMR9AlphaBitmap.dwFlags & VMRBITMAP_UPDATE)
	{
		CAutoLock BitMapLock(&m_VMR9AlphaBitmapLock);
		CRect		rcSrc (m_VMR9AlphaBitmap.rSrc);
		m_pOSDTexture	= NULL;
		m_pOSDSurface	= NULL;
		if ((m_VMR9AlphaBitmap.dwFlags & VMRBITMAP_DISABLE) == 0 && (BYTE *)m_VMR9AlphaBitmapData)
		{
			if( (m_pD3DXLoadSurfaceFromMemory != NULL) &&
				SUCCEEDED(hr = m_pD3DDev->CreateTexture(rcSrc.Width(), rcSrc.Height(), 1, 
				D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
				D3DPOOL_DEFAULT, &m_pOSDTexture, NULL)) )
			{
				if (SUCCEEDED (hr = m_pOSDTexture->GetSurfaceLevel(0, &m_pOSDSurface)))
				{
					hr = m_pD3DXLoadSurfaceFromMemory (m_pOSDSurface, NULL, NULL, (BYTE *)m_VMR9AlphaBitmapData, D3DFMT_A8R8G8B8, m_VMR9AlphaBitmapWidthBytes,
						NULL, &m_VMR9AlphaBitmapRect, D3DX_FILTER_NONE, m_VMR9AlphaBitmap.clrSrcKey);
				}
				if (FAILED (hr))
				{
					m_pOSDTexture	= NULL;
					m_pOSDSurface	= NULL;
				}
			}
		}
		m_VMR9AlphaBitmap.dwFlags ^= VMRBITMAP_UPDATE;
	}
	if (pApp->m_fDisplayStats) DrawStats();
	if (0 && m_pOSDTexture) AlphaBlt(rSrcPri, rDstPri, m_pOSDTexture);
	m_pD3DDev->EndScene();
    //SVP_LogMsg5(L"Before Present %x",m_pD3DDevEx);
	if (m_pD3DDevEx)
	{
		if (m_bIsFullscreen)
			hr = m_pD3DDevEx->PresentEx(NULL, NULL, NULL, NULL, NULL);
		else
			hr = m_pD3DDevEx->PresentEx(rSrcPri, rDstPri, NULL, NULL, NULL);
	}
	else
	{
		if (m_bIsFullscreen)
			hr = m_pD3DDev->Present(NULL, NULL, NULL, NULL);
		else
			hr = m_pD3DDev->Present(rSrcPri, rDstPri, NULL, NULL);
	}
    //SVP_LogMsg5(L"After Present %x",hr);
	// Calculate timing statistics
	if (m_pRefClock) 
		m_pRefClock->GetTime(&rtCurRefTime); // To check if we called Present too late to hit the right vsync
	else
		rtCurRefTime = 0;
	
	if(s.fVMRGothSyncFix && s.m_RenderSettings.bSynchronizeNearest){
		SyncStats(max(m_rtEstVSyncTime, rtCurRefTime)); // Max of estimate and real. Sometimes Present may actually return immediately so we need the estimate as a lower bound
		SyncOffsetStats(-rtSyncOffset); // Minus because we want time to flow downward in the graph in DrawStats
	
		//SVP_LogMsg3(" m_rtEstVSyncTime Cost %f %f %f" , double (rtEndEst - rtStartEst), double (rtEndEst - rtCurRefTime), double(m_rtEstVSyncTime));
		// Adjust sync
		if (s.m_RenderSettings.bSynchronizeVideo) m_pGenlock->ControlClock(msSyncOffset);
		else if (s.m_RenderSettings.bSynchronizeDisplay) m_pGenlock->ControlDisplay(msSyncOffset);
		else m_pGenlock->UpdateStats(msSyncOffset); // No sync or sync to nearest neighbor
	}

	// Check how well audio is matching rate (if at all)
	DWORD tmp;
	if (m_pAudioStats != NULL)
	{
		m_pAudioStats->GetStatParam(AM_AUDREND_STAT_PARAM_SLAVE_ACCUMERROR, &m_lAudioLag, &tmp);
		m_lAudioLagMin = min((long)m_lAudioLag, m_lAudioLagMin);
		m_lAudioLagMax = max((long)m_lAudioLag, m_lAudioLagMax);
		m_pAudioStats->GetStatParam(AM_AUDREND_STAT_PARAM_SLAVE_MODE, &m_lAudioSlaveMode, &tmp);
	}

	if (pApp->m_fResetStats)
	{
		ResetStats();
		pApp->m_fResetStats = false;
	}

	bool fResetDevice = m_bPendingResetDevice;
	if(hr == D3DERR_DEVICELOST && m_pD3DDev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET || hr == S_PRESENT_MODE_CHANGED)
		fResetDevice = true;
	if (SettingsNeedResetDevice()) fResetDevice = true;

	BOOL bCompositionEnabled = false;
	if (m_pDwmIsCompositionEnabled) m_pDwmIsCompositionEnabled(&bCompositionEnabled);
	if ((bCompositionEnabled != 0) != m_bCompositionEnabled)
	{
		if (m_bIsFullscreen)
		{
			m_bCompositionEnabled = (bCompositionEnabled != 0);
		}
		else
			fResetDevice = true;
	}

	//if(s.fResetDevice)
	{
		D3DDEVICE_CREATION_PARAMETERS Parameters;
		UINT CurrentMonitor = GetAdapter(m_pD3D);
		if(SUCCEEDED(m_pD3DDev->GetCreationParameters(&Parameters))){
			HMONITOR hOrgMonitor = m_pD3D->GetAdapterMonitor(Parameters.AdapterOrdinal);
			HMONITOR hCurMonitor = m_pD3D->GetAdapterMonitor(CurrentMonitor);

				if( m_lastMonitor != hCurMonitor)
				{
					if(!AfxGetAppSettings().fbSmoothMutilMonitor/*s.fResetDevice*/)
					{
						SVP_LogMsg5(_T("SUCCEEDED(m_pD3DDev->GetCreationParameters(&Parameters)) && m_pD3D->GetAdapterMonitor(Parameters.AdapterOrdinal) != m_pD3D->GetAdapterMonitor(GetAdapter(m_pD3D)))") );
						fResetDevice = true;
					}else{
						D3DDISPLAYMODE d3ddm;
						HRESULT hr;
						ZeroMemory(&d3ddm, sizeof(d3ddm));
						if(FAILED(m_pD3D->GetAdapterDisplayMode(CurrentMonitor, &d3ddm)))
						{
							//_Error += L"GetAdapterDisplayMode failed\n";

						}else{
							//int l_OldSizeY = m_ScreenSizeCurrent.cy;
							m_ScreenSizeCurrent.SetSize(d3ddm.Width, d3ddm.Height);
							if(s.m_RenderSettings.bSynchronizeNearest){
								m_VSyncDetectThread = AfxBeginThread(ThreadDX9AllocatorRedetectVSync, (LPVOID)this, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
								m_VSyncDetectThread->m_pMainWnd = AfxGetMainWnd();
								m_VSyncDetectThread->ResumeThread();
							}


							//	EstimateRefreshTimings();

							m_uD3DRefreshRate = d3ddm.RefreshRate;
							DOUBLE dTargetSyncOffset = 500.0/m_uD3DRefreshRate ;
							if(s.m_RenderSettings.fTargetSyncOffset != 1.0 ){
								dTargetSyncOffset *= s.m_RenderSettings.fTargetSyncOffset;
							}

							m_pGenlock->SetTargetSyncOffset(dTargetSyncOffset);

						}


						m_pGenlock->SetMonitor(CurrentMonitor);
						m_pGenlock->GetTiming();


					}

				}
				m_lastMonitor = hCurMonitor;
		}

		
	}

	if(fResetDevice)
	{
		if (m_bNeedPendingResetDevice)
		{
			m_bPendingResetDevice = true;
		}
		else
		{
			if (m_dMainThreadId && m_dMainThreadId == GetCurrentThreadId())
			{
				m_bPendingResetDevice = false;
				ResetDevice();
			}
			else
				m_bPendingResetDevice = true;
		}
	}
	return(true);
}

bool CDX9AllocatorPresenter::ResetDevice()
{
	DeleteSurfaces();
	HRESULT hr;
	CString Error;
	if(FAILED(hr = CreateDevice( )) || FAILED(hr = AllocSurfaces())) return false;
	m_pGenlock->SetMonitor(GetAdapter(m_pD3D));
	m_pGenlock->GetTiming();
	OnResetDevice();
	return true;
}

void CDX9AllocatorPresenter::DrawText(const RECT &rc, const CString &strText, int _Priority)
{
	if (_Priority < 1) return;
	int Quality = 1;
	D3DXCOLOR Color1( 1.0f, 0.2f, 0.2f, 1.0f );
	D3DXCOLOR Color0( 0.0f, 0.0f, 0.0f, 1.0f );
	RECT Rect1 = rc;
	RECT Rect2 = rc;
	if (Quality == 1)
		OffsetRect(&Rect2 , 2, 2);
	else
		OffsetRect(&Rect2 , -1, -1);
	if (Quality > 0)
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	OffsetRect (&Rect2 , 1, 0);
	if (Quality > 3)
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	OffsetRect (&Rect2 , 1, 0);
	if (Quality > 2)
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	OffsetRect (&Rect2 , 0, 1);
	if (Quality > 3)
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	OffsetRect (&Rect2 , 0, 1);
	if (Quality > 1)
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	OffsetRect (&Rect2 , -1, 0);
	if (Quality > 3)
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	OffsetRect (&Rect2 , -1, 0);
	if (Quality > 2)
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	OffsetRect (&Rect2 , 0, -1);
	if (Quality > 3)
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	m_pFont->DrawText( m_pSprite, strText, -1, &Rect1, DT_NOCLIP, Color1);
}

void CDX9AllocatorPresenter::DrawStats()
{
	AppSettings& s = AfxGetAppSettings();
	CMPlayerCApp * pApp = AfxGetMyApp();
	int bDetailedStats = 2;
	switch (pApp->m_fDisplayStats)
	{
	case 1: bDetailedStats = 2; break;
	case 2: bDetailedStats = 1; break;
	case 3: bDetailedStats = 0; break;
	}	

	LONGLONG llMaxJitter = m_MaxJitter;
	LONGLONG llMinJitter = m_MinJitter;
	LONGLONG llMaxSyncOffset = m_MaxSyncOffset;
	LONGLONG llMinSyncOffset = m_MinSyncOffset;

	RECT rc = {20, 20, 520, 520 };
	if (m_pFont && m_pSprite)
	{
		m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
		CString	strText;
		int TextHeight = 25.0*m_TextScale + 0.5;

		strText.Format(L"Frames drawn from stream start: %d | Time from stream start: %.0f ms", m_pcFramesDrawn, m_llSampleTime / 10000.0);
		DrawText(rc, strText, 1);
		OffsetRect(&rc, 0, TextHeight);

		strText.Format(L"Frame cycle from video header: %.3f ms | Frame rate from video header: %.3f fps", m_dFrameCycle, m_fps);
		DrawText(rc, strText, 1);
		OffsetRect(&rc, 0, TextHeight);

		strText.Format(L"Frame cycle from sample time stamps: %.3f ms", (m_llSampleTime - m_llLastSampleTime) / 10000.0);
		DrawText(rc, strText, 1);
		OffsetRect(&rc, 0, TextHeight);

		strText.Format(L"Measured closest match display cycle: %.3f ms | Measured base display cycle: %.3f ms", m_dOptimumDisplayCycle, m_dEstRefreshCycle);
		DrawText(rc, strText, 1);
		OffsetRect(&rc, 0, TextHeight);

		strText.Format(L"Display cycle - frame cycle mismatch: %.3f %%", 100 * m_dCycleDifference);
		DrawText(rc, strText, 1);
		OffsetRect(&rc, 0, TextHeight);

		strText.Format(L"Actual frame cycle: %+5.3f ms [%+.3f ms, %+.3f ms] | Actual frame rate: %.3f fps", m_fJitterMean / 10000.0, (double(llMinJitter)/10000.0), (double(llMaxJitter)/10000.0), 10000000.0 / m_fJitterMean);
		DrawText(rc, strText, 1);
		OffsetRect(&rc, 0, TextHeight);

		strText.Format(L"Display cycle from Windows: %.3f ms | Display refresh rate from Windows: %d Hz", m_dD3DRefreshCycle, m_uD3DRefreshRate);
		DrawText(rc, strText, 1);
		OffsetRect(&rc, 0, TextHeight);

		if (m_pGenlock->powerstripTimingExists)
		{
			strText.Format(L"Display cycle from Powerstrip: %.3f ms | Display refresh rate from Powerstrip: %.3f Hz", 1000.0 / m_pGenlock->curDisplayFreq, m_pGenlock->curDisplayFreq);
			DrawText(rc, strText, 1);
			OffsetRect(&rc, 0, TextHeight);
		}


		if (bDetailedStats > 1)
		{
			if ((m_caps.Caps & D3DCAPS_READ_SCANLINE) == 0)
			{
				strText.Format(L"Graphics device does not support scan line access. No sync is possible");
				DrawText(rc, strText, 1);
				OffsetRect(&rc, 0, TextHeight);
			}

			strText.Format(L"Video resolution: %d x %d | Aspect ratio: %d x %d", m_NativeVideoSize.cx, m_NativeVideoSize.cy, m_AspectRatio.cx, m_AspectRatio.cy);
			DrawText(rc, strText, 1);
			OffsetRect(&rc, 0, TextHeight);

			strText.Format(L"Display resolution: %d x %d", m_ScreenSize.cx, m_ScreenSize.cy);
			DrawText(rc, strText, 1);
			OffsetRect(&rc, 0, TextHeight);

			if (s.m_RenderSettings.bSynchronizeDisplay || s.m_RenderSettings.bSynchronizeVideo)
			{
				if (s.m_RenderSettings.bSynchronizeDisplay && !m_pGenlock->PowerstripRunning())
				{
					strText.Format(L"PowerStrip is not running. No display sync is possible.");
					DrawText(rc, strText, 1);
					OffsetRect(&rc, 0, TextHeight);
				}
				else
				{
					strText.Format(L"Sync adjustment: %d | # of adjustments: %d", m_pGenlock->adjDelta, (m_pGenlock->clockAdjustmentsMade + m_pGenlock->displayAdjustmentsMade) / 2);
					DrawText(rc, strText, 1);
					OffsetRect(&rc, 0, TextHeight);
				}
			}
		}

		strText.Format(L"Average sync offset: %+5.1f ms [%.1f ms, %.1f ms]", m_fSyncOffsetAvr/10000.0, -m_pGenlock->maxSyncOffset, -m_pGenlock->minSyncOffset);
		DrawText(rc, strText, 1);
		OffsetRect(&rc, 0, TextHeight);

		if ((bDetailedStats > 1) && m_pAudioStats && s.m_RenderSettings.bSynchronizeVideo)
		{
			strText.Format(L"Audio lag: %3d ms [%d ms, %d ms] | %s", m_lAudioLag, m_lAudioLagMin, m_lAudioLagMax, (m_lAudioSlaveMode == 4) ? _T("Audio renderer is matching rate (for analog sound output)") : _T("Audio renderer is not matching rate"));
			DrawText(rc, strText, 1);
			OffsetRect(&rc, 0, TextHeight);
		}

		//if (m_bIsEVR)
		{
			strText.Format(L"Sample waiting time: %03d ms | %d", m_lNextSampleWait, m_lOverWaitCounter);
			DrawText(rc, strText, 1);
			OffsetRect(&rc, 0, TextHeight);
			if (s.m_RenderSettings.bSynchronizeNearest)
			{
				strText.Format(L"Sample paint time correction: %+02d ms %s", m_lShiftToNearest, (m_llHysteresis == 0) ? L"| Snap to vsync : No " : L"| Snap to vsync : Yes");
				DrawText(rc, strText, 1);
				OffsetRect(&rc, 0, TextHeight);

			}
		}

		strText.Format(L"# of sync glitches: %d", m_uSyncGlitches);
		DrawText(rc, strText, 1);
		OffsetRect(&rc, 0, TextHeight);

		if (bDetailedStats > 1)
		{
			strText.Format(L"Settings: ");

			if (m_bIsEVR)
				strText += "EVR ";
			else
				strText += "VMR9 ";

			if (m_bIsFullscreen)
				strText += "D3DFS ";

			if (s.m_RenderSettings.iVMR9FullscreenGUISupport)
				strText += "FSGui ";

			if (s.m_RenderSettings.iVMRDisableDesktopComposition)
				strText += "DisDC ";

			if (s.m_RenderSettings.iVMRFlushGPUBeforeVSync)
				strText += "GPUFlushBV ";

			if (s.m_RenderSettings.iVMRFlushGPUAfterPresent)
				strText += "GPUFlushAP ";

			if (s.m_RenderSettings.iVMRFlushGPUWait)
				strText += "GPUFlushWt ";

			/*
				if (s.m_RenderSettings.iVMR9VSync)
								strText += "VS ";
				
							if (s.m_RenderSettings.fVMR9AlterativeVSync)
								strText += "AltVS ";
				
							if (s.m_RenderSettings.iVMR9VSyncAccurate)
								strText += "AccVS ";*/
				

			if (s.m_RenderSettings.bSynchronizeVideo)
				strText += "SyncVideo ";

			if (s.m_RenderSettings.bSynchronizeDisplay)
				strText += "SyncDisplay ";

			if (s.m_RenderSettings.bSynchronizeNearest)
				strText += "SyncNearest ";
/*

			if (s.m_RenderSettings.iVMR9VSyncOffset)
				strText.AppendFormat(L"VSOfst(%d) ", s.m_RenderSettings.iVMR9VSyncOffset);

			if (m_bIsEVR)
			{
				if (s.m_RenderSettings.iEVRHighColorResolution)
					strText += "10bit ";
				if (s.m_RenderSettings.iEVREnableFrameTimeCorrection)
					strText += "FTC ";
				if (s.m_RenderSettings.iEVROutputRange == 0)
					strText += "0-255 ";
				else if (s.m_RenderSettings.iEVROutputRange == 1)
					strText += "16-235 ";
			}
*/
			DrawText(rc, strText, 1);
			OffsetRect(&rc, 0, TextHeight);

			strText.Format(L"%s: %s", GetDXVAVersion(), GetDXVADecoderDescription());
			DrawText(rc, strText, 1);
			OffsetRect(&rc, 0, TextHeight);

			strText.Format(L"DirectX SDK: %d", AfxGetMyApp()->GetDXSdkRelease());
			DrawText(rc, strText, 1);
			OffsetRect(&rc, 0, TextHeight);

			for (int i=0; i<6; i++)
			{
				if (m_strStatsMsg[i][0])
				{
					DrawText(rc, m_strStatsMsg[i], 1);
					OffsetRect(&rc, 0, TextHeight);
				}
			}
		}
		OffsetRect(&rc, 0, TextHeight); // Extra "line feed"
		m_pSprite->End();
	}

	if (m_pLine && bDetailedStats)
	{
		D3DXVECTOR2	Points[NB_JITTER];
		int nIndex;

		int DrawWidth = 625;
		int DrawHeight = 250;
		int Alpha = 80;
		int StartX = rc.left;
		int StartY = rc.top;

		DrawRect(RGB(0, 0, 0), Alpha, CRect(StartX, StartY, StartX + DrawWidth, StartY + DrawHeight));
		m_pLine->SetWidth(2.5); 
		m_pLine->SetAntialias(1);
		m_pLine->Begin();

		for (int i = 0; i <= DrawHeight; i += 5)
		{
			Points[0].x = (FLOAT)StartX;
			Points[0].y = (FLOAT)(StartY + i);
			Points[1].x = (FLOAT)(StartX + ((i + 25) % 25 ? 50 : 625));
			Points[1].y = (FLOAT)(StartY + i);
			m_pLine->Draw (Points, 2, D3DCOLOR_XRGB(100, 100, 255));
		}

		for (int i = 0; i < DrawWidth; i += 125) // Every 25:th sample
		{
			Points[0].x = (FLOAT)(StartX + i);
			Points[0].y = (FLOAT)(StartY + DrawHeight / 2);
			Points[1].x = (FLOAT)(StartX + i);
			Points[1].y = (FLOAT)(StartY + DrawHeight / 2 + 10);
			m_pLine->Draw (Points, 2, D3DCOLOR_XRGB(100, 100, 255));
		}

		for (int i = 0; i < NB_JITTER; i++)
		{
			nIndex = (m_nNextJitter + 1 + i) % NB_JITTER;
			if (nIndex < 0)
				nIndex += NB_JITTER;
			double Jitter = m_pllJitter[nIndex] - m_fJitterMean;
			Points[i].x  = (FLOAT)(StartX + (i * 5));
			Points[i].y  = (FLOAT)(StartY + (Jitter / 2000.0 + 125.0));
		}		
		m_pLine->Draw(Points, NB_JITTER, D3DCOLOR_XRGB(255, 100, 100));

		for (int i = 0; i < NB_JITTER; i++)
		{
			nIndex = (m_nNextSyncOffset + 1 + i) % NB_JITTER;
			if (nIndex < 0)
				nIndex += NB_JITTER;
			Points[i].x  = (FLOAT)(StartX + (i * 5));
			Points[i].y  = (FLOAT)(StartY + ((m_pllSyncOffset[nIndex]) / 2000 + 125));
		}		
		m_pLine->Draw(Points, NB_JITTER, D3DCOLOR_XRGB(100, 200, 100));

		m_pLine->End();
	}
}

void CDX9AllocatorPresenter::EstimateRefreshTimings()
{
	if(!AfxGetAppSettings().fVMRGothSyncFix){
		m_dDetectedScanlineTime = 0.001;
		m_dEstRefreshCycle = 0.01;
		return;
	}

	if (m_pD3DDev)
	{
		SVP_LogMsg5(L"EstimateRefreshTimings Start");
		CMPlayerCApp *pApp = AfxGetMyApp();
		D3DRASTER_STATUS rasterStatus;
		if(!m_pD3DDev) return;
		m_pD3DDev->GetRasterStatus(0, &rasterStatus);
		while (rasterStatus.ScanLine != 0) {
			if(!m_pD3DDev) return;
			m_pD3DDev->GetRasterStatus(0, &rasterStatus);
		}
		while (rasterStatus.ScanLine == 0){
			if(!m_pD3DDev) return;
			m_pD3DDev->GetRasterStatus(0, &rasterStatus);
		}
		if(!m_pD3DDev) return;
		m_pD3DDev->GetRasterStatus(0, &rasterStatus);
		LONGLONG startTime = pApp->GetPerfCounter();
		UINT startLine = rasterStatus.ScanLine;
		LONGLONG endTime = 0;
		LONGLONG time = 0;
		UINT endLine = 0;
		UINT line = 0;
		bool done = false;
		while (!done) // Estimate time for one scan line
		{
			if(!m_pD3DDev) return;
			m_pD3DDev->GetRasterStatus(0, &rasterStatus);
			line = rasterStatus.ScanLine;
			time = pApp->GetPerfCounter();
			if (line > 0)
			{
				if(endLine > line){
					//if this looped to another vsync cycle SVP_LogMsg5(L"Shit Est");
					startTime = time;
					startLine = line;
				}
				endLine = line;
				endTime = time;
			}
			else
				done = true;
		}
		m_dDetectedScanlineTime = (double)(endTime - startTime) / (double)((endLine - startLine) * 10000.0);

		// Estimate the display refresh rate from the vsyncs
		if(!m_pD3DDev) return;
		m_pD3DDev->GetRasterStatus(0, &rasterStatus);
		while (rasterStatus.ScanLine != 0){
			if(!m_pD3DDev) return;
			m_pD3DDev->GetRasterStatus(0, &rasterStatus);
		}
		// Now we're at the start of a vsync
		startTime = pApp->GetPerfCounter();
		UINT i;
		for (i = 1; i <= 50; i++)
		{
			if(!m_pD3DDev) return;
			m_pD3DDev->GetRasterStatus(0, &rasterStatus);
			while (rasterStatus.ScanLine == 0){
				if(!m_pD3DDev) return;
				m_pD3DDev->GetRasterStatus(0, &rasterStatus);
			}
			while (rasterStatus.ScanLine != 0){
				if(!m_pD3DDev) return;
				m_pD3DDev->GetRasterStatus(0, &rasterStatus);
			}
			// Now we're at the next vsync
		}
		endTime = pApp->GetPerfCounter();
		m_dEstRefreshCycle = (double)(endTime - startTime) / ((i - 1) * 10000.0);

		SVP_LogMsg5(L"Got ScanlineTime %f %f %f" ,
			m_dDetectedScanlineTime, m_dDetectedScanlineTime*m_ScreenSizeCurrent.cy, m_dEstRefreshCycle);

	}
}

STDMETHODIMP CDX9AllocatorPresenter::GetDIB(BYTE* lpDib, DWORD* size)
{
	CheckPointer(size, E_POINTER);

	HRESULT hr;

	D3DSURFACE_DESC desc;
	memset(&desc, 0, sizeof(desc));
	m_pVideoSurface[m_nCurSurface]->GetDesc(&desc);

	DWORD required = sizeof(BITMAPINFOHEADER) + (desc.Width * desc.Height * 32 >> 3);
	if(!lpDib) {*size = required; return S_OK;}
	if(*size < required) return E_OUTOFMEMORY;
	*size = required;

	CComPtr<IDirect3DSurface9> pSurface = m_pVideoSurface[m_nCurSurface];
	D3DLOCKED_RECT r;
	if(FAILED(hr = pSurface->LockRect(&r, NULL, D3DLOCK_READONLY)))
	{
		pSurface = NULL;
		if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &pSurface, NULL))
		|| FAILED(hr = m_pD3DDev->GetRenderTargetData(m_pVideoSurface[m_nCurSurface], pSurface))
		|| FAILED(hr = pSurface->LockRect(&r, NULL, D3DLOCK_READONLY)))
			return hr;
	}

	BITMAPINFOHEADER* bih = (BITMAPINFOHEADER*)lpDib;
	memset(bih, 0, sizeof(BITMAPINFOHEADER));
	bih->biSize = sizeof(BITMAPINFOHEADER);
	bih->biWidth = desc.Width;
	bih->biHeight = desc.Height;
	bih->biBitCount = 32;
	bih->biPlanes = 1;
	bih->biSizeImage = bih->biWidth * bih->biHeight * bih->biBitCount >> 3;

	BitBltFromRGBToRGB(
		bih->biWidth, bih->biHeight, 
		(BYTE*)(bih + 1), bih->biWidth*bih->biBitCount>>3, bih->biBitCount,
		(BYTE*)r.pBits + r.Pitch*(desc.Height-1), -(int)r.Pitch, 32);

	pSurface->UnlockRect();

	return S_OK;
}

STDMETHODIMP CDX9AllocatorPresenter::SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget)
{
	return SetPixelShader2(pSrcData, pTarget, false);
}

STDMETHODIMP CDX9AllocatorPresenter::SetPixelShader2(LPCSTR pSrcData, LPCSTR pTarget, bool bScreenSpace)
{
	CAutoLock cRenderLock(&m_RenderLock);

	CAtlList<CExternalPixelShader> *pPixelShaders;
	if (bScreenSpace)
		pPixelShaders = &m_pPixelShadersScreenSpace;
	else
		pPixelShaders = &m_pPixelShaders;

	if(!pSrcData && !pTarget)
	{
		pPixelShaders->RemoveAll();
		m_pD3DDev->SetPixelShader(NULL);
		return S_OK;
	}

	if(!pSrcData || !pTarget)
		return E_INVALIDARG;

	CExternalPixelShader Shader;
	Shader.m_SourceData = pSrcData;
	Shader.m_SourceTarget = pTarget;
	
	CComPtr<IDirect3DPixelShader9> pPixelShader;

	HRESULT hr = Shader.Compile(m_pPSC);
	if(FAILED(hr)) 
		return hr;

	pPixelShaders->AddTail(Shader);
	//Paint(true);
	return S_OK;
}

// CVMR9AllocatorPresenter

#define MY_USER_ID 0x6ABE51

CVMR9AllocatorPresenter::CVMR9AllocatorPresenter(HWND hWnd, HRESULT& hr) 
	: CDX9AllocatorPresenter(hWnd, hr, false)
	, m_bUseInternalTimer(false)
{
}

STDMETHODIMP CVMR9AllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

	//SVP_LogMsg5(CStringFromGUID(riid));
	return 
		QI(IVMRSurfaceAllocator9)
		QI(IVMRImagePresenter9)
		QI(IVMRWindowlessControl9)
		QI(ISubPicAllocatorPresenter)
		QI(ISubPicAllocatorPresenterRender)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CVMR9AllocatorPresenter::CreateDevice( )
{
	HRESULT hr = __super::CreateDevice();
	if(FAILED(hr)) return hr;
	if(m_pIVMRSurfAllocNotify)
	{
		HMONITOR hMonitor = m_pD3D->GetAdapterMonitor(GetAdapter(m_pD3D));
		if(FAILED(hr = m_pIVMRSurfAllocNotify->ChangeD3DDevice(m_pD3DDev, hMonitor)))
		{
			//_Error += L"m_pIVMRSurfAllocNotify->ChangeD3DDevice failed";
			return(false);
		}
	}
	return hr;
}

void CVMR9AllocatorPresenter::DeleteSurfaces()
{
    CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	m_pSurfaces.RemoveAll();
	return __super::DeleteSurfaces();
}

// ISubPicAllocatorPresenter

class COuterVMR9:
	public CUnknown,
	public IVideoWindow,
	public IBasicVideo2,
	public IVMRWindowlessControl,
	public IVMRffdshow9,
	public IVMRMixerBitmap9
{
	CComPtr<IUnknown> m_pVMR;
	VMR9AlphaBitmap* m_pVMR9AlphaBitmap;
	CDX9AllocatorPresenter *m_pAllocatorPresenter;

public:
	COuterVMR9(const TCHAR* pName, LPUNKNOWN pUnk, VMR9AlphaBitmap* pVMR9AlphaBitmap, CDX9AllocatorPresenter *_pAllocatorPresenter) : CUnknown(pName, pUnk)
	{
		m_pVMR.CoCreateInstance(CLSID_VideoMixingRenderer9, GetOwner());
		m_pVMR9AlphaBitmap = pVMR9AlphaBitmap;
		m_pAllocatorPresenter = _pAllocatorPresenter;
	}

	~COuterVMR9()
	{
		m_pVMR = NULL;
	}

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv)
	{
		HRESULT hr;
		if(riid == __uuidof(IVMRMixerBitmap9))
			return GetInterface((IVMRMixerBitmap9*)this, ppv);

		hr = m_pVMR ? m_pVMR->QueryInterface(riid, ppv) : E_NOINTERFACE;
		if(m_pVMR && FAILED(hr))
		{
			if(riid == __uuidof(IVideoWindow))
				return GetInterface((IVideoWindow*)this, ppv);
			if(riid == __uuidof(IBasicVideo))
				return GetInterface((IBasicVideo*)this, ppv);
			if(riid == __uuidof(IBasicVideo2))
				return GetInterface((IBasicVideo2*)this, ppv);
			if(riid == __uuidof(IVMRffdshow9)) // Support ffdshow queueing. We show ffdshow that this is patched Media Player Classic.
				return GetInterface((IVMRffdshow9*)this, ppv);
/*			if(riid == __uuidof(IVMRWindowlessControl))
				return GetInterface((IVMRWindowlessControl*)this, ppv);
*/
		}

		return SUCCEEDED(hr) ? hr : __super::NonDelegatingQueryInterface(riid, ppv);
	}

	// IVMRWindowlessControl

	STDMETHODIMP GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			return pWC9->GetNativeVideoSize(lpWidth, lpHeight, lpARWidth, lpARHeight);
		}

		return E_NOTIMPL;
	}
	STDMETHODIMP GetMinIdealVideoSize(LONG* lpWidth, LONG* lpHeight) {return E_NOTIMPL;}
	STDMETHODIMP GetMaxIdealVideoSize(LONG* lpWidth, LONG* lpHeight) {return E_NOTIMPL;}
	STDMETHODIMP SetVideoPosition(const LPRECT lpSRCRect, const LPRECT lpDSTRect) {return E_NOTIMPL;}
    STDMETHODIMP GetVideoPosition(LPRECT lpSRCRect, LPRECT lpDSTRect)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			return pWC9->GetVideoPosition(lpSRCRect, lpDSTRect);
		}

		return E_NOTIMPL;
	}
	STDMETHODIMP GetAspectRatioMode(DWORD* lpAspectRatioMode)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			*lpAspectRatioMode = VMR_ARMODE_NONE;
			return S_OK;
		}

		return E_NOTIMPL;
	}
	STDMETHODIMP SetAspectRatioMode(DWORD AspectRatioMode) {return E_NOTIMPL;}
	STDMETHODIMP SetVideoClippingWindow(HWND hwnd) {return E_NOTIMPL;}
	STDMETHODIMP RepaintVideo(HWND hwnd, HDC hdc) {return E_NOTIMPL;}
	STDMETHODIMP DisplayModeChanged() {return E_NOTIMPL;}
	STDMETHODIMP GetCurrentImage(BYTE** lpDib) {return E_NOTIMPL;}
	STDMETHODIMP SetBorderColor(COLORREF Clr) {return E_NOTIMPL;}
	STDMETHODIMP GetBorderColor(COLORREF* lpClr) {return E_NOTIMPL;}
	STDMETHODIMP SetColorKey(COLORREF Clr) {return E_NOTIMPL;}
	STDMETHODIMP GetColorKey(COLORREF* lpClr) {return E_NOTIMPL;}

	// IVideoWindow
	STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) {return E_NOTIMPL;}
	STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) {return E_NOTIMPL;}
	STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {return E_NOTIMPL;}
	STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {return E_NOTIMPL;}
    STDMETHODIMP put_Caption(BSTR strCaption) {return E_NOTIMPL;}
    STDMETHODIMP get_Caption(BSTR* strCaption) {return E_NOTIMPL;}
	STDMETHODIMP put_WindowStyle(long WindowStyle) {return E_NOTIMPL;}
	STDMETHODIMP get_WindowStyle(long* WindowStyle) {return E_NOTIMPL;}
	STDMETHODIMP put_WindowStyleEx(long WindowStyleEx) {return E_NOTIMPL;}
	STDMETHODIMP get_WindowStyleEx(long* WindowStyleEx) {return E_NOTIMPL;}
	STDMETHODIMP put_AutoShow(long AutoShow) {return E_NOTIMPL;}
	STDMETHODIMP get_AutoShow(long* AutoShow) {return E_NOTIMPL;}
	STDMETHODIMP put_WindowState(long WindowState) {return E_NOTIMPL;}
	STDMETHODIMP get_WindowState(long* WindowState) {return E_NOTIMPL;}
	STDMETHODIMP put_BackgroundPalette(long BackgroundPalette) {return E_NOTIMPL;}
	STDMETHODIMP get_BackgroundPalette(long* pBackgroundPalette) {return E_NOTIMPL;}
	STDMETHODIMP put_Visible(long Visible) {return E_NOTIMPL;}
	STDMETHODIMP get_Visible(long* pVisible) {return E_NOTIMPL;}
	STDMETHODIMP put_Left(long Left) {return E_NOTIMPL;}
	STDMETHODIMP get_Left(long* pLeft) {return E_NOTIMPL;}
	STDMETHODIMP put_Width(long Width) {return E_NOTIMPL;}
	STDMETHODIMP get_Width(long* pWidth)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			CRect s, d;
			HRESULT hr = pWC9->GetVideoPosition(&s, &d);
			*pWidth = d.Width();
			return hr;
		}

		return E_NOTIMPL;
	}
	STDMETHODIMP put_Top(long Top) {return E_NOTIMPL;}
	STDMETHODIMP get_Top(long* pTop) {return E_NOTIMPL;}
	STDMETHODIMP put_Height(long Height) {return E_NOTIMPL;}
	STDMETHODIMP get_Height(long* pHeight)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			CRect s, d;
			HRESULT hr = pWC9->GetVideoPosition(&s, &d);
			*pHeight = d.Height();
			return hr;
		}

		return E_NOTIMPL;
	}
	STDMETHODIMP put_Owner(OAHWND Owner) {return E_NOTIMPL;}
	STDMETHODIMP get_Owner(OAHWND* Owner) {return E_NOTIMPL;}
	STDMETHODIMP put_MessageDrain(OAHWND Drain) {return E_NOTIMPL;}
	STDMETHODIMP get_MessageDrain(OAHWND* Drain) {return E_NOTIMPL;}
	STDMETHODIMP get_BorderColor(long* Color) {return E_NOTIMPL;}
	STDMETHODIMP put_BorderColor(long Color) {return E_NOTIMPL;}
	STDMETHODIMP get_FullScreenMode(long* FullScreenMode) {return E_NOTIMPL;}
	STDMETHODIMP put_FullScreenMode(long FullScreenMode) {return E_NOTIMPL;}
    STDMETHODIMP SetWindowForeground(long Focus) {return E_NOTIMPL;}
    STDMETHODIMP NotifyOwnerMessage(OAHWND hwnd, long uMsg, LONG_PTR wParam, LONG_PTR lParam) {return E_NOTIMPL;}
    STDMETHODIMP SetWindowPosition(long Left, long Top, long Width, long Height) {return E_NOTIMPL;}
	STDMETHODIMP GetWindowPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight) {return E_NOTIMPL;}
	STDMETHODIMP GetMinIdealImageSize(long* pWidth, long* pHeight) {return E_NOTIMPL;}
	STDMETHODIMP GetMaxIdealImageSize(long* pWidth, long* pHeight) {return E_NOTIMPL;}
	STDMETHODIMP GetRestorePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight) {return E_NOTIMPL;}
	STDMETHODIMP HideCursor(long HideCursor) {return E_NOTIMPL;}
	STDMETHODIMP IsCursorHidden(long* CursorHidden) {return E_NOTIMPL;}

	// IBasicVideo2
    STDMETHODIMP get_AvgTimePerFrame(REFTIME* pAvgFrameCycle) {return E_NOTIMPL;}
    STDMETHODIMP get_BitRate(long* pBitRate) {return E_NOTIMPL;}
    STDMETHODIMP get_BitErrorRate(long* pBitErrorRate) {return E_NOTIMPL;}
    STDMETHODIMP get_VideoWidth(long* pVideoWidth) {return E_NOTIMPL;}
    STDMETHODIMP get_VideoHeight(long* pVideoHeight) {return E_NOTIMPL;}
    STDMETHODIMP put_SourceLeft(long SourceLeft) {return E_NOTIMPL;}
    STDMETHODIMP get_SourceLeft(long* pSourceLeft) {return E_NOTIMPL;}
    STDMETHODIMP put_SourceWidth(long SourceWidth) {return E_NOTIMPL;}
    STDMETHODIMP get_SourceWidth(long* pSourceWidth) {return E_NOTIMPL;}
    STDMETHODIMP put_SourceTop(long SourceTop) {return E_NOTIMPL;}
    STDMETHODIMP get_SourceTop(long* pSourceTop) {return E_NOTIMPL;}
    STDMETHODIMP put_SourceHeight(long SourceHeight) {return E_NOTIMPL;}
    STDMETHODIMP get_SourceHeight(long* pSourceHeight) {return E_NOTIMPL;}
    STDMETHODIMP put_DestinationLeft(long DestinationLeft) {return E_NOTIMPL;}
    STDMETHODIMP get_DestinationLeft(long* pDestinationLeft) {return E_NOTIMPL;}
    STDMETHODIMP put_DestinationWidth(long DestinationWidth) {return E_NOTIMPL;}
    STDMETHODIMP get_DestinationWidth(long* pDestinationWidth) {return E_NOTIMPL;}
    STDMETHODIMP put_DestinationTop(long DestinationTop) {return E_NOTIMPL;}
    STDMETHODIMP get_DestinationTop(long* pDestinationTop) {return E_NOTIMPL;}
    STDMETHODIMP put_DestinationHeight(long DestinationHeight) {return E_NOTIMPL;}
    STDMETHODIMP get_DestinationHeight(long* pDestinationHeight) {return E_NOTIMPL;}
    STDMETHODIMP SetSourcePosition(long Left, long Top, long Width, long Height) {return E_NOTIMPL;}
    STDMETHODIMP GetSourcePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight)
	{
		// DVD Nav. bug workaround fix
		{
			*pLeft = *pTop = 0;
			return GetVideoSize(pWidth, pHeight);
		}
/*
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			CRect s, d;
			HRESULT hr = pWC9->GetVideoPosition(&s, &d);
			*pLeft = s.left;
			*pTop = s.top;
			*pWidth = s.Width();
			*pHeight = s.Height();
			return hr;
		}
*/
		return E_NOTIMPL;
	}
    STDMETHODIMP SetDefaultSourcePosition() {return E_NOTIMPL;}
    STDMETHODIMP SetDestinationPosition(long Left, long Top, long Width, long Height) {return E_NOTIMPL;}
    STDMETHODIMP GetDestinationPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			CRect s, d;
			HRESULT hr = pWC9->GetVideoPosition(&s, &d);
			*pLeft = d.left;
			*pTop = d.top;
			*pWidth = d.Width();
			*pHeight = d.Height();
			return hr;
		}

		return E_NOTIMPL;
	}
    STDMETHODIMP SetDefaultDestinationPosition() {return E_NOTIMPL;}
    STDMETHODIMP GetVideoSize(long* pWidth, long* pHeight)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			LONG aw, ah;
//			return pWC9->GetNativeVideoSize(pWidth, pHeight, &aw, &ah);
			// DVD Nav. bug workaround fix
			HRESULT hr = pWC9->GetNativeVideoSize(pWidth, pHeight, &aw, &ah);
			*pWidth = *pHeight * aw / ah;
			return hr;
		}

		return E_NOTIMPL;
	}
	// IVMRffdshow9
	STDMETHODIMP support_ffdshow()
	{
		queueu_ffdshow_support = true;
		return S_OK;
	}

    STDMETHODIMP GetVideoPaletteEntries(long StartIndex, long Entries, long* pRetrieved, long* pPalette) {return E_NOTIMPL;}
    STDMETHODIMP GetCurrentImage(long* pBufferSize, long* pDIBImage) {return E_NOTIMPL;}
    STDMETHODIMP IsUsingDefaultSource() {return E_NOTIMPL;}
    STDMETHODIMP IsUsingDefaultDestination() {return E_NOTIMPL;}

	STDMETHODIMP GetPreferredAspectRatio(long* plAspectX, long* plAspectY)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			LONG w, h;
			return pWC9->GetNativeVideoSize(&w, &h, plAspectX, plAspectY);
		}

		return E_NOTIMPL;
	}

	// IVMRMixerBitmap9
	STDMETHODIMP GetAlphaBitmapParameters(VMR9AlphaBitmap* pBmpParms)
	{
		CheckPointer(pBmpParms, E_POINTER);
		CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
		memcpy (pBmpParms, m_pVMR9AlphaBitmap, sizeof(VMR9AlphaBitmap));
		return S_OK;
	}
	
	STDMETHODIMP SetAlphaBitmap(const VMR9AlphaBitmap*  pBmpParms)
	{
		CheckPointer(pBmpParms, E_POINTER);
		CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
		memcpy (m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
		m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
		m_pAllocatorPresenter->UpdateAlphaBitmap();
		return S_OK;
	}

	STDMETHODIMP UpdateAlphaBitmapParameters(const VMR9AlphaBitmap* pBmpParms)
	{
		CheckPointer(pBmpParms, E_POINTER);
		CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
		memcpy (m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
		m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
		m_pAllocatorPresenter->UpdateAlphaBitmap();
		return S_OK;
	}
};

STDMETHODIMP CVMR9AllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
    CheckPointer(ppRenderer, E_POINTER);

	*ppRenderer = NULL;

	HRESULT hr;

	do
	{
		CMacrovisionKicker* pMK = DNew CMacrovisionKicker(NAME("CMacrovisionKicker"), NULL);
		CComPtr<IUnknown> pUnk = (IUnknown*)(INonDelegatingUnknown*)pMK;

		COuterVMR9 *pOuter = DNew COuterVMR9(NAME("COuterVMR9"), pUnk, &m_VMR9AlphaBitmap, this);


		pMK->SetInner((IUnknown*)(INonDelegatingUnknown*)pOuter);
		CComQIPtr<IBaseFilter> pBF = pUnk;

		CComPtr<IPin> pPin = GetFirstPin(pBF);
		CComQIPtr<IMemInputPin> pMemInputPin = pPin;
		m_bUseInternalTimer = HookNewSegmentAndReceive((IPinC*)(IPin*)pPin, (IMemInputPinC*)(IMemInputPin*)pMemInputPin);

		if(CComQIPtr<IAMVideoAccelerator> pAMVA = pPin)
			HookAMVideoAccelerator((IAMVideoAcceleratorC*)(IAMVideoAccelerator*)pAMVA);

		CComQIPtr<IVMRFilterConfig9> pConfig = pBF;
		if(!pConfig)
			break;

		AppSettings& s = AfxGetAppSettings();

		if(s.fVMR9MixerMode)
		{
			if(FAILED(hr = pConfig->SetNumberOfStreams(1)))
				break;

			if(CComQIPtr<IVMRMixerControl9> pMC = pBF)
			{
				DWORD dwPrefs;
				pMC->GetMixingPrefs(&dwPrefs);  

				// See http://msdn.microsoft.com/en-us/library/dd390928(VS.85).aspx
				dwPrefs |= MixerPref9_NonSquareMixing;
				dwPrefs |= MixerPref9_NoDecimation;
				if(s.fVMR9MixerYUV && !AfxGetMyApp()->IsVista())
				{
					dwPrefs &= ~MixerPref9_RenderTargetMask; 
					dwPrefs |= MixerPref9_RenderTargetYUV;
				}
				pMC->SetMixingPrefs(dwPrefs);		
			}
		}

		if(FAILED(hr = pConfig->SetRenderingMode(VMR9Mode_Renderless)))
			break;

		CComQIPtr<IVMRSurfaceAllocatorNotify9> pSAN = pBF;
		if(!pSAN)
			break;

		if(FAILED(hr = pSAN->AdviseSurfaceAllocator(MY_USER_ID, static_cast<IVMRSurfaceAllocator9*>(this)))
		|| FAILED(hr = AdviseNotify(pSAN)))
			break;

		*ppRenderer = (IUnknown*)pBF.Detach();

		return S_OK;
	}
	while(0);

    return E_FAIL;
}

STDMETHODIMP_(void) CVMR9AllocatorPresenter::SetTime(REFERENCE_TIME rtNow)
{
	__super::SetTime(rtNow);
}

// IVMRSurfaceAllocator9

STDMETHODIMP CVMR9AllocatorPresenter::InitializeDevice(DWORD_PTR dwUserID, VMR9AllocationInfo* lpAllocInfo, DWORD* lpNumBuffers)
{

	if(!lpAllocInfo || !lpNumBuffers)
		return E_POINTER;

	if(!m_pIVMRSurfAllocNotify)
		return E_FAIL;

	if((GetAsyncKeyState(VK_CONTROL)&0x80000000))
	if(lpAllocInfo->Format == '21VY' || lpAllocInfo->Format == '024I')
		return E_FAIL;

	DeleteSurfaces();

	int nOriginal = *lpNumBuffers;

	if (*lpNumBuffers == 1)
	{
		*lpNumBuffers = 4;
		m_nVMR9Surfaces = 4;
	}
	else
		m_nVMR9Surfaces = 0;
	m_pSurfaces.SetCount(*lpNumBuffers);

	int w = lpAllocInfo->dwWidth;
	int h = abs((int)lpAllocInfo->dwHeight);

	HRESULT hr;

	if(lpAllocInfo->dwFlags & VMR9AllocFlag_3DRenderTarget)
		lpAllocInfo->dwFlags |= VMR9AllocFlag_TextureSurface;

	hr = m_pIVMRSurfAllocNotify->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, &m_pSurfaces[0]);
	if(FAILED(hr)) return hr;

	m_pSurfaces.SetCount(*lpNumBuffers);

	m_NativeVideoSize = m_AspectRatio = CSize(w, h);
	m_bNeedCheckSample = true;
	int arx = lpAllocInfo->szAspectRatio.cx, ary = lpAllocInfo->szAspectRatio.cy;
	if(arx > 0 && ary > 0) m_AspectRatio.SetSize(arx, ary);

	if(FAILED(hr = AllocSurfaces()))
		return hr;

	if(!(lpAllocInfo->dwFlags & VMR9AllocFlag_TextureSurface))
	{
		// test if the colorspace is acceptable
		if(FAILED(hr = m_pD3DDev->StretchRect(m_pSurfaces[0], NULL, m_pVideoSurface[m_nCurSurface], NULL, D3DTEXF_NONE)))
		{
			DeleteSurfaces();
			return E_FAIL;
		}
	}

	hr = m_pD3DDev->ColorFill(m_pVideoSurface[m_nCurSurface], NULL, 0);

	if (m_nVMR9Surfaces && m_nVMR9Surfaces != *lpNumBuffers)
		m_nVMR9Surfaces = *lpNumBuffers;
	*lpNumBuffers = min(nOriginal, *lpNumBuffers);
	m_iVMR9Surface = 0;

	return hr;
}

STDMETHODIMP CVMR9AllocatorPresenter::TerminateDevice(DWORD_PTR dwUserID)
{
    DeleteSurfaces();
    return S_OK;
}

STDMETHODIMP CVMR9AllocatorPresenter::GetSurface(DWORD_PTR dwUserID, DWORD SurfaceIndex, DWORD SurfaceFlags, IDirect3DSurface9** lplpSurface)
{
    if(!lplpSurface)
		return E_POINTER;

	if(SurfaceIndex >= m_pSurfaces.GetCount()) 
        return E_FAIL;

	CAutoLock cRenderLock(&m_RenderLock);

	if (m_nVMR9Surfaces)
	{
		++m_iVMR9Surface;
		m_iVMR9Surface = m_iVMR9Surface % m_nVMR9Surfaces;
		(*lplpSurface = m_pSurfaces[m_iVMR9Surface + SurfaceIndex])->AddRef();
	}
	else
	{
		m_iVMR9Surface = SurfaceIndex;
		(*lplpSurface = m_pSurfaces[SurfaceIndex])->AddRef();
	}

	return S_OK;
}

STDMETHODIMP CVMR9AllocatorPresenter::AdviseNotify(IVMRSurfaceAllocatorNotify9* lpIVMRSurfAllocNotify)
{
    CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	m_pIVMRSurfAllocNotify = lpIVMRSurfAllocNotify;

	HRESULT hr;
    HMONITOR hMonitor = m_pD3D->GetAdapterMonitor(GetAdapter(m_pD3D));
    if(FAILED(hr = m_pIVMRSurfAllocNotify->SetD3DDevice(m_pD3DDev, hMonitor)))
		return hr;

    return S_OK;
}
void CVMR9AllocatorPresenter::ThreadStartPresenting(){
	//SVP_LogMsg5(L"StartPresenting Start ");

	CAutoLock threadLock(&m_csTread);

	//ResetGothSyncVars();
	
	AppSettings& s = AfxGetAppSettings();
	
	m_pGenlock->SetMonitor(GetAdapter(m_pD3D));
	if (!m_pGenlock->powerstripTimingExists) m_pGenlock->GetTiming(); // StartPresenting seems to get called more often than StopPresenting

	ResetStats();
	if(m_dDetectedScanlineTime <= 0.0){
		EstimateRefreshTimings();
	}
	if (m_rtFrameCycle > 0.0) m_dCycleDifference = GetCycleDifference(); // Might have moved to another display

	//if (m_pRefClock) m_pRefClock->GetTime(&rtEndEst);

	//SVP_LogMsg5(L"StartPresenting End ");
}
UINT __cdecl ThreadVMR9AllocatorPresenterStartPresenting( LPVOID lpParam ) 
{ 
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if(pFrame){
		CAutoLock mOpenCloseLock(&pFrame->m_csOpenClose);
		CVMR9AllocatorPresenter* pVMRA = (CVMR9AllocatorPresenter*)lpParam;
		pVMRA->ThreadStartPresenting();
	}
	return 0; 
}
// IVMRImagePresenter9

STDMETHODIMP CVMR9AllocatorPresenter::StartPresenting(DWORD_PTR dwUserID)
{
    CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);
	
	m_lOverWaitCounter = 0;
	AppSettings& s = AfxGetAppSettings();
	m_pcFramesDrawn = 0;

	if (s.m_RenderSettings.bSynchronizeVideo)
		m_pGenlock->AdviseSyncClock(((CMainFrame*)(AfxGetApp()->m_pMainWnd))->m_pSyncClock);

	{
		CComPtr<IBaseFilter> pVMR9;
		FILTER_INFO filterInfo;
		ZeroMemory(&filterInfo, sizeof(filterInfo));
		m_pIVMRSurfAllocNotify->QueryInterface (__uuidof(IBaseFilter), (void**)&pVMR9);
		pVMR9->QueryFilterInfo(&filterInfo); // This addref's the pGraph member

		BeginEnumFilters(filterInfo.pGraph, pEF, pBF)
			if(CComQIPtr<IAMAudioRendererStats> pAS = pBF)
			{
				m_pAudioStats = pAS;
			};
		EndEnumFilters

		pVMR9->GetSyncSource(&m_pRefClock);
		if (filterInfo.pGraph) filterInfo.pGraph->Release();
	}
	hEventGoth = CreateEvent(NULL, TRUE, FALSE, NULL);

	if(m_dDetectedScanlineTime <= 0.0){
		m_VSyncDetectThread = AfxBeginThread(ThreadVMR9AllocatorPresenterStartPresenting, (LPVOID)this, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
		// a workaround: when you call AfxGetMainWnd in a work thread, you get the windows associated with the thread, not the MainWindow. 
		// set the m_pMainWnd of the thread object to the MainWindow in the main thread can work around the issue.
		m_VSyncDetectThread->m_pMainWnd = AfxGetMainWnd();
		m_VSyncDetectThread->ResumeThread();
	}

	return S_OK;
}

STDMETHODIMP CVMR9AllocatorPresenter::StopPresenting(DWORD_PTR dwUserID)
{
	m_pGenlock->ResetTiming();
	m_pRefClock = NULL;
	if(hEventGoth){
		CloseHandle(hEventGoth);
		hEventGoth = NULL;
	}
	return S_OK;
}

STDMETHODIMP CVMR9AllocatorPresenter::PresentImage(DWORD_PTR dwUserID, VMR9PresentationInfo* lpPresInfo)
{
	CheckPointer(m_pIVMRSurfAllocNotify, E_UNEXPECTED);

	//REFERENCE_TIME rtStartEst = 1000, rtEndEst = 0;
	//if (m_pRefClock) m_pRefClock->GetTime(&rtStartEst);

	m_dMainThreadId = GetCurrentThreadId();
	m_llLastSampleTime = m_llSampleTime;
	m_llSampleTime = lpPresInfo->rtStart;
	if (m_rtFrameCycle == 0 || m_bNeedCheckSample)
	{
		m_bNeedCheckSample = false;
		CComPtr<IBaseFilter> pVMR9;
		CComPtr<IPin> pPin;
		CMediaType mt;
		
		if (SUCCEEDED (m_pIVMRSurfAllocNotify->QueryInterface (__uuidof(IBaseFilter), (void**)&pVMR9)) &&
			SUCCEEDED (pVMR9->FindPin(L"VMR Input0", &pPin)) &&
			SUCCEEDED (pPin->ConnectionMediaType(&mt)) )
		{
			ExtractAvgTimePerFrame(&mt, m_rtFrameCycle);
			m_dFrameCycle = m_rtFrameCycle / 10000.0;
			if (m_rtFrameCycle > 0.0)
			{
				m_fps = 10000000.0 / m_rtFrameCycle;
				m_dCycleDifference = GetCycleDifference();

				if (abs(m_dCycleDifference) < 0.05) // If less than 5%
					m_bSnapToVSync = true;
				else
					m_bSnapToVSync = false;
			}
			m_bInterlaced = ExtractInterlaced(&mt);

			CSize NativeVideoSize = m_NativeVideoSize;
			CSize AspectRatio = m_AspectRatio;
			if (mt.formattype==FORMAT_VideoInfo || mt.formattype==FORMAT_MPEGVideo)
			{
				VIDEOINFOHEADER *vh = (VIDEOINFOHEADER*)mt.pbFormat;

				NativeVideoSize = CSize(vh->bmiHeader.biWidth, abs(vh->bmiHeader.biHeight));
				if (vh->rcTarget.right - vh->rcTarget.left > 0)
					NativeVideoSize.cx = vh->rcTarget.right - vh->rcTarget.left;
				else if (vh->rcSource.right - vh->rcSource.left > 0)
					NativeVideoSize.cx = vh->rcSource.right - vh->rcSource.left;

				if (vh->rcTarget.bottom - vh->rcTarget.top > 0)
					NativeVideoSize.cy = vh->rcTarget.bottom - vh->rcTarget.top;
				else if (vh->rcSource.bottom - vh->rcSource.top > 0)
					NativeVideoSize.cy = vh->rcSource.bottom - vh->rcSource.top;
			}
			else if (mt.formattype==FORMAT_VideoInfo2 || mt.formattype==FORMAT_MPEG2Video)
			{
				VIDEOINFOHEADER2 *vh = (VIDEOINFOHEADER2*)mt.pbFormat;

				if (vh->dwPictAspectRatioX && vh->dwPictAspectRatioY)
					AspectRatio = CSize(vh->dwPictAspectRatioX, vh->dwPictAspectRatioY);

				NativeVideoSize = CSize(vh->bmiHeader.biWidth, abs(vh->bmiHeader.biHeight));
				if (vh->rcTarget.right - vh->rcTarget.left > 0)
					NativeVideoSize.cx = vh->rcTarget.right - vh->rcTarget.left;
				else if (vh->rcSource.right - vh->rcSource.left > 0)
					NativeVideoSize.cx = vh->rcSource.right - vh->rcSource.left;

				if (vh->rcTarget.bottom - vh->rcTarget.top > 0)
					NativeVideoSize.cy = vh->rcTarget.bottom - vh->rcTarget.top;
				else if (vh->rcSource.bottom - vh->rcSource.top > 0)
					NativeVideoSize.cy = vh->rcSource.bottom - vh->rcSource.top;
			}
			if (m_NativeVideoSize != NativeVideoSize || m_AspectRatio != AspectRatio)
			{
				m_NativeVideoSize = NativeVideoSize;
				m_AspectRatio = AspectRatio;
				AfxGetApp()->m_pMainWnd->PostMessage(WM_REARRANGERENDERLESS);
			}
		}
	}

    HRESULT hr;

	if(!lpPresInfo || !lpPresInfo->lpSurf)
		return E_POINTER;


	AppSettings& s = AfxGetAppSettings();
	if(m_rtFrameCycle > 0 && s.fVMRGothSyncFix && m_rtFrameCycle < 300000){
		REFERENCE_TIME rtCurTime, rtInterleave;
		if (m_pRefClock) {
			m_pRefClock->GetTime(&rtCurTime);
			rtInterleave = rtCurTime - m_lastFrameArrivedTime;
			m_lastFrameArrivedTime = rtCurTime;
			if(rtInterleave > m_rtFrameCycle*6/5 && rtInterleave < m_rtFrameCycle * 3 && m_lastFramePainted){
				//m_pcFramesDropped++;
				m_lastFrameArrivedTime = rtCurTime;
				m_lastFramePainted = false;
				//SVP_LogMsg5(L"droped %f %f %d", double(m_rtFrameCycle) , double(rtInterleave) , m_pcFramesDropped);
				return S_OK;
			}
			//SVP_LogMsg5(L"Paint");
		}
	}
	m_lastFramePainted = true;

	CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	CComPtr<IDirect3DTexture9> pTexture;
	lpPresInfo->lpSurf->GetContainer(IID_IDirect3DTexture9, (void**)&pTexture);

	if(pTexture)
	{
		m_pVideoSurface[m_nCurSurface] = lpPresInfo->lpSurf;
		if(m_pVideoTexture[m_nCurSurface]) 
			m_pVideoTexture[m_nCurSurface] = pTexture;
	}
	else
	{
		hr = m_pD3DDev->StretchRect(lpPresInfo->lpSurf, NULL, m_pVideoSurface[m_nCurSurface], NULL, D3DTEXF_NONE);
	}
	

	if( lpPresInfo->rtEnd > lpPresInfo->rtStart)
	{
		if(m_pSubPicQueue)
		{
			m_pSubPicQueue->SetFPS(m_fps);

		}

		if(m_pSubPicQueue2)
		{
			m_pSubPicQueue2->SetFPS(m_fps);
		}

		if( !s.bExternalSubtitleTime && (m_pSubPicQueue || m_pSubPicQueue2))
		{
			//SVP_LogMsg5(L"SetTime %f %f",m_fps, (double)g_tSegmentStart + g_tSampleStart);
			__super::SetTime(g_tSegmentStart + g_tSampleStart);
			
		}
	}

	CSize VideoSize = m_NativeVideoSize;
	int arx = lpPresInfo->szAspectRatio.cx, ary = lpPresInfo->szAspectRatio.cy;
	if(arx > 0 && ary > 0) VideoSize.cx = VideoSize.cy*arx/ary;
	if(VideoSize != GetVideoSize())
	{
		m_AspectRatio.SetSize(arx, ary);
		AfxGetApp()->m_pMainWnd->PostMessage(WM_REARRANGERENDERLESS);
	}

	if (AfxGetMyApp()->m_fTearingTest)
	{
		RECT rcTearing;
		
		rcTearing.left = m_nTearingPos;
		rcTearing.top = 0;
		rcTearing.right = rcTearing.left + 4;
		rcTearing.bottom = m_NativeVideoSize.cy;
		m_pD3DDev->ColorFill(m_pVideoSurface[m_nCurSurface], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));

		rcTearing.left	= (rcTearing.right + 15) % m_NativeVideoSize.cx;
		rcTearing.right	= rcTearing.left + 4;
		m_pD3DDev->ColorFill (m_pVideoSurface[m_nCurSurface], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));

		m_nTearingPos = (m_nTearingPos + 7) % m_NativeVideoSize.cx;
	}

	
	while(s.fVMRGothSyncFix && s.m_RenderSettings.bSynchronizeNearest ){//

		
		REFERENCE_TIME rtRefClockTimeNow = 0; if (m_pRefClock) m_pRefClock->GetTime(&rtRefClockTimeNow); // Reference clock time now
		LONG lLastVsyncTime = (LONG)((m_rtEstVSyncTime - rtRefClockTimeNow) / 10000); // Time of previous vsync relative to now

		LONGLONG llNextSampleWait = (LONGLONG)(((double)lLastVsyncTime + GetDisplayCycle() * (2.0 - s.m_RenderSettings.fTargetSyncOffset)) * 10000); // Next safe time to Paint()
		
		LONGLONG llEachStep = GetDisplayCycle();
		if(m_dFrameCycle > 0 && llEachStep > m_dFrameCycle*8/10)
			s.m_RenderSettings.bSynchronizeNearest = 0;

		llEachStep *= 10000; // While the proposed time is in the past of sample presentation time
		
		if(llEachStep > 0){
			if(llNextSampleWait < 0){
				LONGLONG llStepWeNeed = (-llNextSampleWait) / llEachStep;
				llNextSampleWait += llEachStep*(llStepWeNeed+1);
			}else{
				llNextSampleWait = min(llNextSampleWait , llEachStep);
			}
		}else
			llNextSampleWait = 0;

		m_lNextSampleWait = (LONG)(llNextSampleWait / 10000) ;// min(40,
			
		if (m_lNextSampleWait <= 0){
		//	m_lNextSampleWait = 0;
		}else{
			if(hEventGoth){
				{
					if(WAIT_TIMEOUT != WaitForSingleObject(hEventGoth, m_lNextSampleWait) ){
						m_lOverWaitCounter = 3;
					}
					s.m_RenderSettings.bSynchronizeNearest = 1;
					m_lOverWaitCounter = 0;
				}
				
			}else
				m_lOverWaitCounter = 2;
			
		}

		break;
	}
	
	//if (m_pRefClock) m_pRefClock->GetTime(&rtEndEst);

	//SVP_LogMsg3("Prefsent Cost %f ", double(rtEndEst - rtStartEst));
	Paint(true);
	m_pcFramesDrawn++;
    return S_OK;
}

// IVMRWindowlessControl9
//
// It is only implemented (partially) for the dvd navigator's 
// menu handling, which needs to know a few things about the 
// location of our window.

STDMETHODIMP CVMR9AllocatorPresenter::GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
{
	if(lpWidth) *lpWidth = m_NativeVideoSize.cx;
	if(lpHeight) *lpHeight = m_NativeVideoSize.cy;
	if(lpARWidth) *lpARWidth = m_AspectRatio.cx;
	if(lpARHeight) *lpARHeight = m_AspectRatio.cy;
	return S_OK;
}
STDMETHODIMP CVMR9AllocatorPresenter::GetMinIdealVideoSize(LONG* lpWidth, LONG* lpHeight) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::GetMaxIdealVideoSize(LONG* lpWidth, LONG* lpHeight) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::SetVideoPosition(const LPRECT lpSRCRect, const LPRECT lpDSTRect) {return E_NOTIMPL;} // we have our own method for this
STDMETHODIMP CVMR9AllocatorPresenter::GetVideoPosition(LPRECT lpSRCRect, LPRECT lpDSTRect)
{
	CopyRect(lpSRCRect, CRect(CPoint(0, 0), m_NativeVideoSize));
	CopyRect(lpDSTRect, &m_VideoRect);
	return S_OK;
}
STDMETHODIMP CVMR9AllocatorPresenter::GetAspectRatioMode(DWORD* lpAspectRatioMode)
{
	if(lpAspectRatioMode) *lpAspectRatioMode = AM_ARMODE_STRETCHED;
	return S_OK;
}
STDMETHODIMP CVMR9AllocatorPresenter::SetAspectRatioMode(DWORD AspectRatioMode) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::SetVideoClippingWindow(HWND hwnd) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::RepaintVideo(HWND hwnd, HDC hdc) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::DisplayModeChanged() {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::GetCurrentImage(BYTE** lpDib) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::SetBorderColor(COLORREF Clr) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::GetBorderColor(COLORREF* lpClr)
{
	if(lpClr) *lpClr = 0;
	return S_OK;
}

// CRM9AllocatorPresenter

CRM9AllocatorPresenter::CRM9AllocatorPresenter(HWND hWnd, HRESULT& hr ) 
	: CDX9AllocatorPresenter(hWnd, hr, false)
{
}

STDMETHODIMP CRM9AllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

	return 
		QI2(IRMAVideoSurface)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CRM9AllocatorPresenter::AllocSurfaces()
{
    CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	m_pVideoSurfaceOff = NULL;
	m_pVideoSurfaceYUY2 = NULL;

	HRESULT hr;

	if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(
		m_NativeVideoSize.cx, m_NativeVideoSize.cy, D3DFMT_X8R8G8B8, 
		D3DPOOL_DEFAULT, &m_pVideoSurfaceOff, NULL)))
		return hr;

	m_pD3DDev->ColorFill(m_pVideoSurfaceOff, NULL, 0);

	if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(
		m_NativeVideoSize.cx, m_NativeVideoSize.cy, D3DFMT_YUY2, 
		D3DPOOL_DEFAULT, &m_pVideoSurfaceYUY2, NULL)))
		m_pVideoSurfaceYUY2 = NULL;

	if(m_pVideoSurfaceYUY2)
	{
		m_pD3DDev->ColorFill(m_pVideoSurfaceOff, NULL, 0x80108010);
	}

	return __super::AllocSurfaces();
}

void CRM9AllocatorPresenter::DeleteSurfaces()
{
    CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);
	m_pVideoSurfaceOff = NULL;
	m_pVideoSurfaceYUY2 = NULL;
	__super::DeleteSurfaces();
}

// IRMAVideoSurface

STDMETHODIMP CRM9AllocatorPresenter::Blt(UCHAR* pImageData, RMABitmapInfoHeader* pBitmapInfo, REF(PNxRect) inDestRect, REF(PNxRect) inSrcRect)
{
	if(!m_pVideoSurface || !m_pVideoSurfaceOff)
		return E_FAIL;

	bool fRGB = false;
	bool fYUY2 = false;

	CRect src((RECT*)&inSrcRect), dst((RECT*)&inDestRect), src2(CPoint(0,0), src.Size());
	if(src.Width() > dst.Width() || src.Height() > dst.Height())
		return E_FAIL;

	D3DSURFACE_DESC d3dsd;
	ZeroMemory(&d3dsd, sizeof(d3dsd));
	if(FAILED(m_pVideoSurfaceOff->GetDesc(&d3dsd)))
		return E_FAIL;

	int dbpp = 
		d3dsd.Format == D3DFMT_R8G8B8 || d3dsd.Format == D3DFMT_X8R8G8B8 || d3dsd.Format == D3DFMT_A8R8G8B8 ? 32 : 
		d3dsd.Format == D3DFMT_R5G6B5 ? 16 : 0;

	if(pBitmapInfo->biCompression == '024I')
	{
		DWORD pitch = pBitmapInfo->biWidth;
		DWORD size = pitch*abs(pBitmapInfo->biHeight);

		BYTE* y = pImageData					+ src.top*pitch + src.left;
		BYTE* u = pImageData + size				+ src.top*(pitch/2) + src.left/2;
		BYTE* v = pImageData + size + size/4	+ src.top*(pitch/2) + src.left/2;

		if(m_pVideoSurfaceYUY2)
		{
			D3DLOCKED_RECT r;
			if(SUCCEEDED(m_pVideoSurfaceYUY2->LockRect(&r, src2, 0)))
			{
				BitBltFromI420ToYUY2(src.Width(), src.Height(), (BYTE*)r.pBits, r.Pitch, y, u, v, pitch);
				m_pVideoSurfaceYUY2->UnlockRect();
				fYUY2 = true;
			}
		}
		else
		{
			D3DLOCKED_RECT r;
			if(SUCCEEDED(m_pVideoSurfaceOff->LockRect(&r, src2, 0)))
			{
				BitBltFromI420ToRGB(src.Width(), src.Height(), (BYTE*)r.pBits, r.Pitch, dbpp, y, u, v, pitch);
				m_pVideoSurfaceOff->UnlockRect();
				fRGB = true;
			}
		}
	}
	else if(pBitmapInfo->biCompression == '2YUY')
	{
		DWORD w = pBitmapInfo->biWidth;
		DWORD h = abs(pBitmapInfo->biHeight);
		DWORD pitch = pBitmapInfo->biWidth*2;

		BYTE* yvyu = pImageData + src.top*pitch + src.left*2;

		if(m_pVideoSurfaceYUY2)
		{
			D3DLOCKED_RECT r;
			if(SUCCEEDED(m_pVideoSurfaceYUY2->LockRect(&r, src2, 0)))
			{
				BitBltFromYUY2ToYUY2(src.Width(), src.Height(), (BYTE*)r.pBits, r.Pitch, yvyu, pitch);
				m_pVideoSurfaceYUY2->UnlockRect();
				fYUY2 = true;
			}
		}
		else
		{
			D3DLOCKED_RECT r;
			if(SUCCEEDED(m_pVideoSurfaceOff->LockRect(&r, src2, 0)))
			{
				BitBltFromYUY2ToRGB(src.Width(), src.Height(), (BYTE*)r.pBits, r.Pitch, dbpp, yvyu, pitch);
				m_pVideoSurfaceOff->UnlockRect();
				fRGB = true;
			}
		}
	}
	else if(pBitmapInfo->biCompression == 0 || pBitmapInfo->biCompression == 3
		 || pBitmapInfo->biCompression == 'BGRA')
	{
		DWORD w = pBitmapInfo->biWidth;
		DWORD h = abs(pBitmapInfo->biHeight);
		DWORD pitch = pBitmapInfo->biWidth*pBitmapInfo->biBitCount>>3;

		BYTE* rgb = pImageData + src.top*pitch + src.left*(pBitmapInfo->biBitCount>>3);

		D3DLOCKED_RECT r;
		if(SUCCEEDED(m_pVideoSurfaceOff->LockRect(&r, src2, 0)))
		{
			BYTE* pBits = (BYTE*)r.pBits;
			if(pBitmapInfo->biHeight > 0) {pBits += r.Pitch*(src.Height()-1); r.Pitch = -r.Pitch;}
			BitBltFromRGBToRGB(src.Width(), src.Height(), pBits, r.Pitch, dbpp, rgb, pitch, pBitmapInfo->biBitCount);
			m_pVideoSurfaceOff->UnlockRect();
			fRGB = true;
		}
	}

	if(!fRGB && !fYUY2)
	{
		m_pD3DDev->ColorFill(m_pVideoSurfaceOff, NULL, 0);

		HDC hDC;
		if(SUCCEEDED(m_pVideoSurfaceOff->GetDC(&hDC)))
		{
			CString str;
			str.Format(_T("Sorry, this format is not supported"));

			SetBkColor(hDC, 0);
			SetTextColor(hDC, 0x404040);
			TextOut(hDC, 10, 10, str, str.GetLength());

			m_pVideoSurfaceOff->ReleaseDC(hDC);

			fRGB = true;
		}
	}

	HRESULT hr;
	
	if(fRGB)
		hr = m_pD3DDev->StretchRect(m_pVideoSurfaceOff, src2, m_pVideoSurface[m_nCurSurface], dst, D3DTEXF_NONE);
	if(fYUY2)
		hr = m_pD3DDev->StretchRect(m_pVideoSurfaceYUY2, src2, m_pVideoSurface[m_nCurSurface], dst, D3DTEXF_NONE);
	Paint(true);
	return PNR_OK;
}

STDMETHODIMP CRM9AllocatorPresenter::BeginOptimizedBlt(RMABitmapInfoHeader* pBitmapInfo)
{
    CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);
	DeleteSurfaces();
	m_NativeVideoSize = m_AspectRatio = CSize(pBitmapInfo->biWidth, abs(pBitmapInfo->biHeight));
	if(FAILED(AllocSurfaces())) return E_FAIL;
	return PNR_NOTIMPL;
}

STDMETHODIMP CRM9AllocatorPresenter::OptimizedBlt(UCHAR* pImageBits, REF(PNxRect) rDestRect, REF(PNxRect) rSrcRect)
{
	return PNR_NOTIMPL;
}

STDMETHODIMP CRM9AllocatorPresenter::EndOptimizedBlt()
{
	return PNR_NOTIMPL;
}

STDMETHODIMP CRM9AllocatorPresenter::GetOptimizedFormat(REF(RMA_COMPRESSION_TYPE) ulType)
{
	return PNR_NOTIMPL;
}

STDMETHODIMP CRM9AllocatorPresenter::GetPreferredFormat(REF(RMA_COMPRESSION_TYPE) ulType)
{
	ulType = RMA_I420;
	return PNR_OK;
}

// CQT9AllocatorPresenter

CQT9AllocatorPresenter::CQT9AllocatorPresenter(HWND hWnd, HRESULT& hr) 
	: CDX9AllocatorPresenter(hWnd, hr, false )
{
}

STDMETHODIMP CQT9AllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

	return 
		QI(IQTVideoSurface)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CQT9AllocatorPresenter::AllocSurfaces()
{
	HRESULT hr;

	m_pVideoSurfaceOff = NULL;

	if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(
		m_NativeVideoSize.cx, m_NativeVideoSize.cy, D3DFMT_X8R8G8B8, 
		D3DPOOL_DEFAULT, &m_pVideoSurfaceOff, NULL)))
		return hr;

	return __super::AllocSurfaces();
}

void CQT9AllocatorPresenter::DeleteSurfaces()
{
	m_pVideoSurfaceOff = NULL;

	__super::DeleteSurfaces();
}

// IQTVideoSurface

STDMETHODIMP CQT9AllocatorPresenter::BeginBlt(const BITMAP& bm)
{
    CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);
	DeleteSurfaces();
	m_NativeVideoSize = m_AspectRatio = CSize(bm.bmWidth, abs(bm.bmHeight));
	if(FAILED(AllocSurfaces())) return E_FAIL;
	return S_OK;
}

STDMETHODIMP CQT9AllocatorPresenter::DoBlt(const BITMAP& bm)
{
	if(!m_pVideoSurface || !m_pVideoSurfaceOff)
		return E_FAIL;

	bool fOk = false;

	D3DSURFACE_DESC d3dsd;
	ZeroMemory(&d3dsd, sizeof(d3dsd));
	if(FAILED(m_pVideoSurfaceOff->GetDesc(&d3dsd)))
		return E_FAIL;

	int w = bm.bmWidth;
	int h = abs(bm.bmHeight);
	int bpp = bm.bmBitsPixel;
	int dbpp = 
		d3dsd.Format == D3DFMT_R8G8B8 || d3dsd.Format == D3DFMT_X8R8G8B8 || d3dsd.Format == D3DFMT_A8R8G8B8 ? 32 : 
		d3dsd.Format == D3DFMT_R5G6B5 ? 16 : 0;

	if((bpp == 16 || bpp == 24 || bpp == 32) && w == d3dsd.Width && h == d3dsd.Height)
	{
		D3DLOCKED_RECT r;
		if(SUCCEEDED(m_pVideoSurfaceOff->LockRect(&r, NULL, 0)))
		{
			BitBltFromRGBToRGB(
				w, h,
				(BYTE*)r.pBits, r.Pitch, dbpp,
				(BYTE*)bm.bmBits, bm.bmWidthBytes, bm.bmBitsPixel);
			m_pVideoSurfaceOff->UnlockRect();
			fOk = true;
		}
	}

	if(!fOk)
	{
		m_pD3DDev->ColorFill(m_pVideoSurfaceOff, NULL, 0);

		HDC hDC;
		if(SUCCEEDED(m_pVideoSurfaceOff->GetDC(&hDC)))
		{
			CString str;
			str.Format(_T("Sorry, this color format is not supported"));

			SetBkColor(hDC, 0);
			SetTextColor(hDC, 0x404040);
			TextOut(hDC, 10, 10, str, str.GetLength());

			m_pVideoSurfaceOff->ReleaseDC(hDC);
		}
	}

	m_pD3DDev->StretchRect(m_pVideoSurfaceOff, NULL, m_pVideoSurface[m_nCurSurface], NULL, D3DTEXF_NONE);

	Paint(true);

	return S_OK;
}

// CDXRAllocatorPresenter

CDXRAllocatorPresenter::CDXRAllocatorPresenter(HWND hWnd, HRESULT& hr )
	: ISubPicAllocatorPresenterImpl(hWnd, hr )
	, m_ScreenSize(0, 0)
{
	if(FAILED(hr))
	{
		//_Error += L"ISubPicAllocatorPresenterImpl failed\n";
		return;
	}

	hr = S_OK;
}

CDXRAllocatorPresenter::~CDXRAllocatorPresenter()
{
	if(m_pSRCB)
	{
		// nasty, but we have to let it know about our death somehow
		((CSubRenderCallback*)(ISubRenderCallback*)m_pSRCB)->SetDXRAP(NULL);
	}

	// the order is important here
	m_pSubPicQueue = NULL;
	m_pSubPicQueue2 = NULL;
	m_pAllocator = NULL;
	m_pDXR = NULL;
}

STDMETHODIMP CDXRAllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
/*
	if(riid == __uuidof(IVideoWindow))
		return GetInterface((IVideoWindow*)this, ppv);
	if(riid == __uuidof(IBasicVideo))
		return GetInterface((IBasicVideo*)this, ppv);
	if(riid == __uuidof(IBasicVideo2))
		return GetInterface((IBasicVideo2*)this, ppv);
*/
/*
	if(riid == __uuidof(IVMRWindowlessControl))
		return GetInterface((IVMRWindowlessControl*)this, ppv);
*/

	if(riid != IID_IUnknown && m_pDXR)
	{
		if(SUCCEEDED(m_pDXR->QueryInterface(riid, ppv)))
			return S_OK;
	}

	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CDXRAllocatorPresenter::SetDevice(IDirect3DDevice9* pD3DDev)
{
	CheckPointer(pD3DDev, E_POINTER);

	CSize size;
	switch(AfxGetAppSettings().nSPCMaxRes)
	{
	case 0: default: size = m_ScreenSize; break;
	case 1: size.SetSize(1024, 768); break;
	case 2: size.SetSize(800, 600); break;
	case 3: size.SetSize(640, 480); break;
	case 4: size.SetSize(512, 384); break;
	case 5: size.SetSize(384, 288); break;
	case 6: size.SetSize(2560, 1600); break;
	case 7: size.SetSize(1920, 1080); break;
	case 8: size.SetSize(1320, 900); break;
	case 9: size.SetSize(1280, 720); break;
	}

	if(m_pAllocator)
	{
		m_pAllocator->ChangeDevice(pD3DDev);
	}
	else
	{
		m_pAllocator = DNew CDX9SubPicAllocator(pD3DDev, size, AfxGetAppSettings().fSPCPow2Tex);
		if(!m_pAllocator)
			return E_FAIL;
	}

	HRESULT hr = S_OK;


	m_pSubPicQueue = AfxGetAppSettings().nSPCSize > 0 
		? (ISubPicQueue*)DNew CSubPicQueue(AfxGetAppSettings().nSPCSize, m_pAllocator, &hr)
		: (ISubPicQueue*)DNew CSubPicQueueNoThread(m_pAllocator, &hr);

	HRESULT hr2 = S_OK;
	m_pSubPicQueue2 = AfxGetAppSettings().nSPCSize > 0 
		? (ISubPicQueue*)new CSubPicQueue(AfxGetAppSettings().nSPCSize, m_pAllocator, &hr2)
		: (ISubPicQueue*)new CSubPicQueueNoThread(m_pAllocator, &hr2);

	if( ( !m_pSubPicQueue && !m_pSubPicQueue2 ) || ( FAILED(hr) && FAILED(hr2) ))
		return E_FAIL;

	if(m_pSubPicQueue && m_SubPicProvider) m_pSubPicQueue->SetSubPicProvider(m_SubPicProvider);

	if(m_pSubPicQueue2 && m_SubPicProvider2) m_pSubPicQueue2->SetSubPicProvider(m_SubPicProvider2);


	return S_OK;
}

HRESULT CDXRAllocatorPresenter::Render(
	REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, REFERENCE_TIME atpf,
	int left, int top, int right, int bottom, int width, int height)
{	
	__super::SetPosition(CRect(0, 0, width, height), CRect(left, top, right, bottom)); // needed? should be already set by the player
	SetTime(rtStart);
	if(atpf > 0 && m_pSubPicQueue) m_pSubPicQueue->SetFPS(10000000.0 / atpf);
	if(atpf > 0 && m_pSubPicQueue2) m_pSubPicQueue2->SetFPS(10000000.0 / atpf);
	AlphaBltSubPic(CSize(width, height));
	return S_OK;
}

// ISubPicAllocatorPresenter

STDMETHODIMP CDXRAllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
    CheckPointer(ppRenderer, E_POINTER);

	if(m_pDXR) return E_UNEXPECTED;
	m_pDXR.CoCreateInstance(CLSID_DXR, GetOwner());
	if(!m_pDXR) return E_FAIL;

	CComQIPtr<ISubRender> pSR = m_pDXR;
	if(!pSR) {m_pDXR = NULL; return E_FAIL;}

	m_pSRCB = DNew CSubRenderCallback(this);
	if(FAILED(pSR->SetCallback(m_pSRCB))) {m_pDXR = NULL; return E_FAIL;}

	(*ppRenderer = this)->AddRef();

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &mi))
		m_ScreenSize.SetSize(mi.rcMonitor.right-mi.rcMonitor.left, mi.rcMonitor.bottom-mi.rcMonitor.top);

	return S_OK;
}

STDMETHODIMP_(void) CDXRAllocatorPresenter::SetPosition(RECT w, RECT v)
{
	if(CComQIPtr<IBasicVideo> pBV = m_pDXR)
	{
		pBV->SetDefaultSourcePosition();
		pBV->SetDestinationPosition(v.left, v.top, v.right - v.left, v.bottom - v.top);
	}

	if(CComQIPtr<IVideoWindow> pVW = m_pDXR)
	{
		pVW->SetWindowPosition(w.left, w.top, w.right - w.left, w.bottom - w.top);
	}
}

STDMETHODIMP_(SIZE) CDXRAllocatorPresenter::GetVideoSize(bool fCorrectAR)
{
	SIZE size = {0, 0};

	if(!fCorrectAR)
	{
		if(CComQIPtr<IBasicVideo> pBV = m_pDXR)
			pBV->GetVideoSize(&size.cx, &size.cy);
	}
	else
	{
		if(CComQIPtr<IBasicVideo2> pBV2 = m_pDXR)
			pBV2->GetPreferredAspectRatio(&size.cx, &size.cy);
	}

	return size;
}

STDMETHODIMP_(bool) CDXRAllocatorPresenter::Paint(bool fAll)
{
	return false; // TODO
}

STDMETHODIMP CDXRAllocatorPresenter::GetDIB(BYTE* lpDib, DWORD* size)
{
	HRESULT hr = E_NOTIMPL;
	if(CComQIPtr<IBasicVideo> pBV = m_pDXR)
		hr = pBV->GetCurrentImage((long*)size, (long*)lpDib);
	return hr;
}

STDMETHODIMP CDXRAllocatorPresenter::SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget)
{
	return E_NOTIMPL; // TODO
}

// CmadVRAllocatorPresenter

CmadVRAllocatorPresenter::CmadVRAllocatorPresenter(HWND hWnd, HRESULT& hr )
	: ISubPicAllocatorPresenterImpl(hWnd, hr )
	, m_ScreenSize(0, 0)
{
	if(FAILED(hr))
	{
		//_Error += L"ISubPicAllocatorPresenterImpl failed\n";
		return;
	}

	hr = S_OK;
}

CmadVRAllocatorPresenter::~CmadVRAllocatorPresenter()
{
	if(m_pSRCB)
	{
		// nasty, but we have to let it know about our death somehow
		((CSubRenderCallback*)(ISubRenderCallback*)m_pSRCB)->SetDXRAP(NULL);
	}

	// the order is important here
	m_pSubPicQueue = NULL;
	m_pSubPicQueue2 = NULL;
	m_pAllocator = NULL;
	m_pDXR = NULL;
}

STDMETHODIMP CmadVRAllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
/*
	if(riid == __uuidof(IVideoWindow))
		return GetInterface((IVideoWindow*)this, ppv);
	if(riid == __uuidof(IBasicVideo))
		return GetInterface((IBasicVideo*)this, ppv);
	if(riid == __uuidof(IBasicVideo2))
		return GetInterface((IBasicVideo2*)this, ppv);
*/
/*
	if(riid == __uuidof(IVMRWindowlessControl))
		return GetInterface((IVMRWindowlessControl*)this, ppv);
*/

	if(riid != IID_IUnknown && m_pDXR)
	{
		if(SUCCEEDED(m_pDXR->QueryInterface(riid, ppv)))
			return S_OK;
	}

	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CmadVRAllocatorPresenter::SetDevice(IDirect3DDevice9* pD3DDev)
{
	CheckPointer(pD3DDev, E_POINTER);

	CSize size;
	switch(AfxGetAppSettings().nSPCMaxRes)
	{
	case 0: default: size = m_ScreenSize; break;
	case 1: size.SetSize(1024, 768); break;
	case 2: size.SetSize(800, 600); break;
	case 3: size.SetSize(640, 480); break;
	case 4: size.SetSize(512, 384); break;
	case 5: size.SetSize(384, 288); break;
	case 6: size.SetSize(2560, 1600); break;
	case 7: size.SetSize(1920, 1080); break;
	case 8: size.SetSize(1320, 900); break;
	case 9: size.SetSize(1280, 720); break;
	}

	if(m_pAllocator)
	{
		m_pAllocator->ChangeDevice(pD3DDev);
	}
	else
	{
		m_pAllocator = DNew CDX9SubPicAllocator(pD3DDev, size, AfxGetAppSettings().fSPCPow2Tex);
		if(!m_pAllocator)
			return E_FAIL;
	}

	HRESULT hr = S_OK;

	m_pSubPicQueue = AfxGetAppSettings().nSPCSize > 0 
		? (ISubPicQueue*)DNew CSubPicQueue(AfxGetAppSettings().nSPCSize, m_pAllocator, &hr)
		: (ISubPicQueue*)DNew CSubPicQueueNoThread(m_pAllocator, &hr);

	HRESULT hr2 = S_OK;
	m_pSubPicQueue2 = AfxGetAppSettings().nSPCSize > 0 
		? (ISubPicQueue*)new CSubPicQueue(AfxGetAppSettings().nSPCSize, m_pAllocator, &hr2)
		: (ISubPicQueue*)new CSubPicQueueNoThread(m_pAllocator, &hr2);

	if( ( !m_pSubPicQueue && !m_pSubPicQueue2 ) || ( FAILED(hr) && FAILED(hr2) ))
		return E_FAIL;

	if(m_pSubPicQueue && m_SubPicProvider) m_pSubPicQueue->SetSubPicProvider(m_SubPicProvider);

	if(m_pSubPicQueue2 && m_SubPicProvider2) m_pSubPicQueue2->SetSubPicProvider(m_SubPicProvider2);

	return S_OK;
}

HRESULT CmadVRAllocatorPresenter::Render(
	REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, REFERENCE_TIME atpf,
	int left, int top, int right, int bottom, int width, int height)
{	
	__super::SetPosition(CRect(0, 0, width, height), CRect(left, top, right, bottom)); // needed? should be already set by the player
	SetTime(rtStart);
	if(atpf > 0 && m_pSubPicQueue) m_pSubPicQueue->SetFPS(10000000.0 / atpf);
	if(atpf > 0 && m_pSubPicQueue2) m_pSubPicQueue2->SetFPS(10000000.0 / atpf);
	AlphaBltSubPic(CSize(width, height));
	return S_OK;
}

// ISubPicAllocatorPresenter

STDMETHODIMP CmadVRAllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
    CheckPointer(ppRenderer, E_POINTER);

	if(m_pDXR) return E_UNEXPECTED;
	m_pDXR.CoCreateInstance(CLSID_madVR, GetOwner());
	if(!m_pDXR) return E_FAIL;

	CComQIPtr<ISubRender> pSR = m_pDXR;
	if(!pSR) {m_pDXR = NULL; return E_FAIL;}

	m_pSRCB = DNew CSubRenderCallback(this);
	if(FAILED(pSR->SetCallback(m_pSRCB))) {m_pDXR = NULL; return E_FAIL;}

	(*ppRenderer = this)->AddRef();

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &mi))
		m_ScreenSize.SetSize(mi.rcMonitor.right-mi.rcMonitor.left, mi.rcMonitor.bottom-mi.rcMonitor.top);

	return S_OK;
}

STDMETHODIMP_(void) CmadVRAllocatorPresenter::SetPosition(RECT w, RECT v)
{
	if(CComQIPtr<IBasicVideo> pBV = m_pDXR)
	{
		pBV->SetDefaultSourcePosition();
		pBV->SetDestinationPosition(v.left, v.top, v.right - v.left, v.bottom - v.top);
	}

	if(CComQIPtr<IVideoWindow> pVW = m_pDXR)
	{
		pVW->SetWindowPosition(w.left, w.top, w.right - w.left, w.bottom - w.top);
	}
}

STDMETHODIMP_(SIZE) CmadVRAllocatorPresenter::GetVideoSize(bool fCorrectAR)
{
	SIZE size = {0, 0};

	if(!fCorrectAR)
	{
		if(CComQIPtr<IBasicVideo> pBV = m_pDXR)
			pBV->GetVideoSize(&size.cx, &size.cy);
	}
	else
	{
		if(CComQIPtr<IBasicVideo2> pBV2 = m_pDXR)
			pBV2->GetPreferredAspectRatio(&size.cx, &size.cy);
	}

	return size;
}

STDMETHODIMP_(bool) CmadVRAllocatorPresenter::Paint(bool fAll)
{
	return false; // TODO
}

STDMETHODIMP CmadVRAllocatorPresenter::GetDIB(BYTE* lpDib, DWORD* size)
{
	HRESULT hr = E_NOTIMPL;
	if(CComQIPtr<IBasicVideo> pBV = m_pDXR)
		hr = pBV->GetCurrentImage((long*)size, (long*)lpDib);
	return hr;
}

STDMETHODIMP CmadVRAllocatorPresenter::SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget)
{
	return E_NOTIMPL; // TODO
}

CGenlock::CGenlock(DOUBLE target, DOUBLE limit, INT lineD, INT colD, DOUBLE clockD, UINT mon):
	targetSyncOffset(target), // Target sync offset, typically around 10 ms
	controlLimit(limit), // How much sync offset is allowed to drift from target sync offset before control kicks in
	lineDelta(lineD), // Number of rows used in display frequency adjustment, typically 1 (one)
	columnDelta(colD),  // Number of columns used in display frequency adjustment, typically 1 - 2
	cycleDelta(clockD),  // Delta used in clock speed adjustment. In fractions of 1.0. Typically around 0.001
	monitor(mon) // The monitor to be adjusted if the display refresh rate is the controlled parameter
{
	lowSyncOffset = targetSyncOffset - controlLimit;
	highSyncOffset = targetSyncOffset + controlLimit;
	adjDelta = 0;
	displayAdjustmentsMade = 0;
	clockAdjustmentsMade = 0;
	displayFreqCruise = 0;
	displayFreqFaster = 0;
	displayFreqSlower = 0;
	curDisplayFreq = 0;
	psWnd = NULL;
	liveSource = FALSE;
	powerstripTimingExists = FALSE;
	syncOffsetFifo = new MovingAverage(64);
}

CGenlock::~CGenlock()
{
	ResetTiming();
	if(syncOffsetFifo != NULL)
	{
		delete syncOffsetFifo;
		syncOffsetFifo = NULL;
	}
	syncClock = NULL;
};

BOOL CGenlock::PowerstripRunning()
{
	psWnd = FindWindow(_T("TPShidden"), NULL); 
	if (!psWnd) return FALSE; // Powerstrip is not running
	else return TRUE;
}

// Get the display timing parameters through PowerStrip (if running).
HRESULT CGenlock::GetTiming()
{
	ATOM getTiming; 
	LPARAM lParam = NULL; 
	WPARAM wParam = monitor;
	INT i = 0;
	INT j = 0;
	INT params = 0;
	BOOL done = FALSE;
	TCHAR tmpStr[MAX_LOADSTRING];

	CAutoLock lock(&csGenlockLock);
	if (!PowerstripRunning()) return E_FAIL;

	getTiming = static_cast<ATOM>(SendMessage(psWnd, UM_GETTIMING, wParam, lParam));
	GlobalGetAtomName(getTiming, savedTiming, MAX_LOADSTRING);

	while (params < TIMING_PARAM_CNT)
	{
		while (savedTiming[i] != ',' && savedTiming[i] != '\0')
		{
			tmpStr[j++] = savedTiming[i];
			tmpStr[j] = '\0';
			i++;
		}
		i++; // Skip trailing comma
		j = 0;
		displayTiming[params] = _ttoi(tmpStr);
		displayTimingSave[params] = displayTiming[params];
		params++;
	}

	// The display update frequency is controlled by adding and subtracting pixels form the
	// image. This is done by either subtracting columns or rows or both. Some displays like
	// row adjustments and some column adjustments. One should probably not do both.
	StringCchPrintf(faster, MAX_LOADSTRING, TEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\0"),
		displayTiming[0],
		displayTiming[HFRONTPORCH] - columnDelta,
		displayTiming[2],
		displayTiming[3],
		displayTiming[4],
		displayTiming[VFRONTPORCH] - lineDelta,
		displayTiming[6],
		displayTiming[7],
		displayTiming[PIXELCLOCK],
		displayTiming[9]	
		);

	// Nominal update frequency
	StringCchPrintf(cruise, MAX_LOADSTRING, TEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\0"),
		displayTiming[0],
		displayTiming[HFRONTPORCH],
		displayTiming[2],
		displayTiming[3],
		displayTiming[4],
		displayTiming[VFRONTPORCH],
		displayTiming[6],
		displayTiming[7],
		displayTiming[PIXELCLOCK],
		displayTiming[9]	
		);

	// Lower than nominal update frequency
	StringCchPrintf(slower, MAX_LOADSTRING, TEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\0"),
		displayTiming[0],
		displayTiming[HFRONTPORCH] + columnDelta,
		displayTiming[2],
		displayTiming[3],
		displayTiming[4],
		displayTiming[VFRONTPORCH] + lineDelta,
		displayTiming[6],
		displayTiming[7],
		displayTiming[PIXELCLOCK],
		displayTiming[9]	
		);

	totalColumns = displayTiming[HACTIVE] + displayTiming[HFRONTPORCH] + displayTiming[HSYNCWIDTH] + displayTiming[HBACKPORCH];
	totalLines = displayTiming[VACTIVE] + displayTiming[VFRONTPORCH] + displayTiming[VSYNCWIDTH] + displayTiming[VBACKPORCH];
	pixelClock = 1000 * displayTiming[PIXELCLOCK]; // Pixels/s
	displayFreqCruise = (DOUBLE)pixelClock / (totalLines * totalColumns); // Frames/s
	displayFreqSlower = (DOUBLE)pixelClock / ((totalLines + lineDelta) * (totalColumns + columnDelta));
	displayFreqFaster = (DOUBLE)pixelClock / ((totalLines - lineDelta) * (totalColumns - columnDelta));
	curDisplayFreq = displayFreqCruise;
	GlobalDeleteAtom(getTiming);
	adjDelta = 0;
	powerstripTimingExists = TRUE;
	return S_OK;
}

// Reset display timing parameters to nominal.
HRESULT CGenlock::ResetTiming()
{
	LPARAM lParam = NULL; 
	WPARAM wParam = monitor; 
	ATOM setTiming; 
	LRESULT ret;
	CAutoLock lock(&csGenlockLock);

	if (!PowerstripRunning()) return E_FAIL;

	if (displayAdjustmentsMade > 0)
	{
		setTiming = GlobalAddAtom(cruise); 
		lParam = setTiming;
		ret = SendMessage(psWnd, UM_SETCUSTOMTIMINGFAST, wParam, lParam);
		GlobalDeleteAtom(setTiming);
		curDisplayFreq = displayFreqCruise;
	}
	adjDelta = 0;
	return S_OK;
}

// Reset reference clock speed to nominal.
HRESULT CGenlock::ResetClock()
{
	adjDelta = 0;
	if (syncClock == NULL) return E_FAIL;
	else return syncClock->AdjustClock(1.0);
	return S_OK;
}

HRESULT CGenlock::SetTargetSyncOffset(DOUBLE targetD)
{
	targetSyncOffset = targetD;
	lowSyncOffset = targetD - controlLimit;
	highSyncOffset = targetD + controlLimit;
	return S_OK;
}

HRESULT CGenlock::GetTargetSyncOffset(DOUBLE *targetD)
{
	*targetD = targetSyncOffset;
	return S_OK;
}

HRESULT CGenlock::SetControlLimit(DOUBLE cL)
{
	controlLimit = cL;
	return S_OK;
}

HRESULT CGenlock::GetControlLimit(DOUBLE *cL)
{
	*cL = controlLimit;
	return S_OK;
}

HRESULT CGenlock::SetDisplayResolution(UINT columns, UINT lines)
{
	visibleColumns = columns;
	visibleLines = lines;
	return S_OK;
}

HRESULT CGenlock::AdviseSyncClock(CComPtr<ISyncClock> sC)
{
	if (!sC) return E_FAIL;
	if (syncClock) syncClock = NULL; // Release any outstanding references if this is called repeatedly
	syncClock = sC;
	return S_OK;
}

// Set the monitor to control. This is best done manually as not all monitors can be controlled
// so automatic detection of monitor to control might have unintended effects.
// The PowerStrip API uses zero-based monitor numbers, i.e. the default monitor is 0.
HRESULT CGenlock::SetMonitor(UINT mon)
{
	monitor = mon;
	return S_OK;
}

HRESULT CGenlock::ResetStats()
{
	CAutoLock lock(&csGenlockLock);
	minSyncOffset = 1000000.0;
	maxSyncOffset = -1000000.0;
	displayAdjustmentsMade = 0;
	clockAdjustmentsMade = 0;
	return S_OK;
}

// Synchronize by adjusting display refresh rate
HRESULT CGenlock::ControlDisplay(double syncOffset)
{
	LPARAM lParam = NULL; 
	WPARAM wParam = monitor; 
	ATOM setTiming;

	syncOffsetAvg = syncOffsetFifo->Average(syncOffset);
	minSyncOffset = min(minSyncOffset, syncOffset);
	maxSyncOffset = max(maxSyncOffset, syncOffset);

	if (!PowerstripRunning() || !powerstripTimingExists) return E_FAIL;
	// Adjust as seldom as possible by checking the current controlState before changing it.
	if ((syncOffsetAvg > highSyncOffset) && (adjDelta != 1))
		// Speed up display refresh rate by subtracting pixels from the image.
		{
			adjDelta = 1; // Increase refresh rate
			curDisplayFreq = displayFreqFaster;
			setTiming = GlobalAddAtom(faster);
			lParam = setTiming;
			SendMessage(psWnd, UM_SETCUSTOMTIMINGFAST, wParam, lParam);
			GlobalDeleteAtom(setTiming);
			displayAdjustmentsMade++;
		}
	else
		// Slow down display refresh rate by adding pixels to the image.
		if ((syncOffsetAvg < lowSyncOffset) && (adjDelta != -1))
		{
			adjDelta = -1;
			curDisplayFreq = displayFreqSlower;
			setTiming = GlobalAddAtom(slower);
			lParam = setTiming;
			SendMessage(psWnd, UM_SETCUSTOMTIMINGFAST, wParam, lParam);
			GlobalDeleteAtom(setTiming);
			displayAdjustmentsMade++;
		}
	else
		// Cruise.
		if ((syncOffsetAvg < targetSyncOffset) && (adjDelta == 1))
		{
			adjDelta = 0;
			curDisplayFreq = displayFreqCruise;
			setTiming = GlobalAddAtom(cruise);
			lParam = setTiming;
			SendMessage(psWnd, UM_SETCUSTOMTIMINGFAST, wParam, lParam);
			GlobalDeleteAtom(setTiming);
			displayAdjustmentsMade++;
		}
	else
		if ((syncOffsetAvg > targetSyncOffset) && (adjDelta == -1))
		{
			adjDelta = 0;
			curDisplayFreq = displayFreqCruise;
			setTiming = GlobalAddAtom(cruise);
			lParam = setTiming;
			SendMessage(psWnd, UM_SETCUSTOMTIMINGFAST, wParam, lParam);
			GlobalDeleteAtom(setTiming);
			displayAdjustmentsMade++;
		}
	return S_OK;
}

// Synchronize by adjusting reference clock rate (and therefore video FPS).
// Todo: check so that we don't have a live source
HRESULT CGenlock::ControlClock(double syncOffset)
{
	syncOffsetAvg = syncOffsetFifo->Average(syncOffset);
	minSyncOffset = min(minSyncOffset, syncOffset);
	maxSyncOffset = max(maxSyncOffset, syncOffset);

	if (!syncClock) return E_FAIL;
	// Adjust as seldom as possible by checking the current controlState before changing it.
	if ((syncOffsetAvg > highSyncOffset) && (adjDelta != 1))
		// Slow down video stream.
		{
			adjDelta = 1;
			syncClock->AdjustClock(1.0 - cycleDelta); // Makes the clock move slower by providing smaller increments
			clockAdjustmentsMade++;
		}
	else
		// Speed up video stream.
		if ((syncOffsetAvg < lowSyncOffset) && (adjDelta != -1))
		{
			adjDelta = -1;
			syncClock->AdjustClock(1.0 + cycleDelta);
			clockAdjustmentsMade++;
		}
	else
		// Cruise.
		if ((syncOffsetAvg < targetSyncOffset) && (adjDelta == 1))
		{
			adjDelta = 0;
			syncClock->AdjustClock(1.0);
			clockAdjustmentsMade++;
		}
	else
		if ((syncOffsetAvg > targetSyncOffset) && (adjDelta == -1))
		{
			adjDelta = 0;
			syncClock->AdjustClock(1.0);
			clockAdjustmentsMade++;
		}
	return S_OK;
}

// Don't adjust anything, just update the syncOffset stats
HRESULT CGenlock::UpdateStats(double syncOffset)
{
	syncOffsetAvg = syncOffsetFifo->Average(syncOffset);
	minSyncOffset = min(minSyncOffset, syncOffset);
	maxSyncOffset = max(maxSyncOffset, syncOffset);
	return S_OK;
}