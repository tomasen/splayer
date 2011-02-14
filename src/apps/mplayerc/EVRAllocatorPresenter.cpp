/*
 * $Id: EVRAllocatorPresenter.cpp 1260 2009-08-30 07:09:32Z ar-jar $
 *
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
#include <Videoacc.h>
#include <initguid.h>
#include "..\..\SubPic\ISubPic.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <Vmr9.h>
#include <evr.h>
#include <mfapi.h> 
#include <Mferror.h>
#include "..\..\SubPic\DX9SubPic.h"
#include "IQTVideoSurface.h"
#include <moreuuids.h>
#include "MacrovisionKicker.h"
#include "IPinHook.h"
#include "PixelShaderCompiler.h"
#include "MainFrm.h"
#include "AllocatorCommon.h"
#include "EVRAllocatorPresenter.h"

//#define  SVP_LogMsg5 __noop
//#define  SVP_LogMsg6 __noop
typedef enum 
{
	MSG_MIXERIN,
	MSG_MIXEROUT
} EVR_STATS_MSG;

// dxva.dll
typedef HRESULT (__stdcall *PTR_DXVA2CreateDirect3DDeviceManager9)(UINT* pResetToken, IDirect3DDeviceManager9** ppDeviceManager);

// mf.dll
typedef HRESULT (__stdcall *PTR_MFCreatePresentationClock)(IMFPresentationClock** ppPresentationClock);

// evr.dll
typedef HRESULT (__stdcall *PTR_MFCreateDXSurfaceBuffer)(REFIID riid, IUnknown* punkSurface, BOOL fBottomUpWhenLinear, IMFMediaBuffer** ppBuffer);
typedef HRESULT (__stdcall *PTR_MFCreateVideoSampleFromSurface)(IUnknown* pUnkSurface, IMFSample** ppSample);
typedef HRESULT (__stdcall *PTR_MFCreateVideoMediaType)(const MFVIDEOFORMAT* pVideoFormat, IMFVideoMediaType** ppIVideoMediaType);

// avrt.dll
typedef HANDLE  (__stdcall *PTR_AvSetMmThreadCharacteristicsW)(LPCWSTR TaskName, LPDWORD TaskIndex);
typedef BOOL	(__stdcall *PTR_AvSetMmThreadPriority)(HANDLE AvrtHandle, AVRT_PRIORITY Priority);
typedef BOOL	(__stdcall *PTR_AvRevertMmThreadCharacteristics)(HANDLE AvrtHandle);

// Guid to tag IMFSample with DirectX surface index
static const GUID GUID_SURFACE_INDEX = { 0x30c8e9f6, 0x415, 0x4b81, { 0xa3, 0x15, 0x1, 0xa, 0xc6, 0xa9, 0xda, 0x19 } };

#define CheckHR(exp) {if(FAILED(hr = exp)) return hr;}

MFOffset MakeOffset(float v)
{
    MFOffset offset;
    offset.value = short(v);
    offset.fract = WORD(65536 * (v-offset.value));
    return offset;
}

MFVideoArea MakeArea(float x, float y, DWORD width, DWORD height)
{
    MFVideoArea area;
    area.OffsetX = MakeOffset(x);
    area.OffsetY = MakeOffset(y);
    area.Area.cx = width;
    area.Area.cy = height;
    return area;
}

class CEVRAllocatorPresenter;

class COuterEVR:
	public CUnknown,
	public IVMRffdshow9,
	public IVMRMixerBitmap9,
	public IBaseFilter
{
	CComPtr<IUnknown> m_pEVR;
	VMR9AlphaBitmap *m_pVMR9AlphaBitmap;
	CEVRAllocatorPresenter *m_pAllocatorPresenter;

public:
	// IBaseFilter
    virtual HRESULT STDMETHODCALLTYPE EnumPins(__out  IEnumPins **ppEnum)
	{
		CComPtr<IBaseFilter> pEVRBase;
		if (m_pEVR)
			m_pEVR->QueryInterface(&pEVRBase);
		if (pEVRBase)
			return pEVRBase->EnumPins(ppEnum);
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE FindPin(LPCWSTR Id, __out  IPin **ppPin)
	{
		CComPtr<IBaseFilter> pEVRBase;
		if (m_pEVR)
			m_pEVR->QueryInterface(&pEVRBase);
		if (pEVRBase)
			return pEVRBase->FindPin(Id, ppPin);
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE QueryFilterInfo(__out  FILTER_INFO *pInfo)
	{
		CComPtr<IBaseFilter> pEVRBase;
		if (m_pEVR)
			m_pEVR->QueryInterface(&pEVRBase);
		if (pEVRBase)
			return pEVRBase->QueryFilterInfo(pInfo);
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE JoinFilterGraph(__in_opt  IFilterGraph *pGraph, __in_opt  LPCWSTR pName)
	{
		CComPtr<IBaseFilter> pEVRBase;
		if (m_pEVR)
			m_pEVR->QueryInterface(&pEVRBase);
		if (pEVRBase)
			return pEVRBase->JoinFilterGraph(pGraph, pName);
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE QueryVendorInfo(__out  LPWSTR *pVendorInfo)
	{
		CComPtr<IBaseFilter> pEVRBase;
		if (m_pEVR)
			m_pEVR->QueryInterface(&pEVRBase);
		if (pEVRBase)
			return pEVRBase->QueryVendorInfo(pVendorInfo);
		return E_NOTIMPL;
	}

    virtual HRESULT STDMETHODCALLTYPE Stop( void)
	{
		CComPtr<IBaseFilter> pEVRBase;
		if (m_pEVR)
			m_pEVR->QueryInterface(&pEVRBase);
		if (pEVRBase)
			return pEVRBase->Stop();
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE Pause( void)
	{
		CComPtr<IBaseFilter> pEVRBase;
		if (m_pEVR)
			m_pEVR->QueryInterface(&pEVRBase);
		if (pEVRBase)
			return pEVRBase->Pause();
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE Run( REFERENCE_TIME tStart)
	{
		CComPtr<IBaseFilter> pEVRBase;
		if (m_pEVR)
			m_pEVR->QueryInterface(&pEVRBase);
		if (pEVRBase)
			return pEVRBase->Run(tStart);
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE GetState(DWORD dwMilliSecsTimeout, __out FILTER_STATE *State);
    
    virtual HRESULT STDMETHODCALLTYPE SetSyncSource(__in_opt  IReferenceClock *pClock)
	{
		CComPtr<IBaseFilter> pEVRBase;
		if (m_pEVR)
			m_pEVR->QueryInterface(&pEVRBase);
		if (pEVRBase)
			return pEVRBase->SetSyncSource(pClock);
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE GetSyncSource(__deref_out_opt  IReferenceClock **pClock)
	{
		CComPtr<IBaseFilter> pEVRBase;
		if (m_pEVR)
			m_pEVR->QueryInterface(&pEVRBase);
		if (pEVRBase)
			return pEVRBase->GetSyncSource(pClock);
		return E_NOTIMPL;
	}

    virtual HRESULT STDMETHODCALLTYPE GetClassID(__RPC__out CLSID *pClassID)
	{
		CComPtr<IBaseFilter> pEVRBase;
		if (m_pEVR)
			m_pEVR->QueryInterface(&pEVRBase);
		if (pEVRBase)
			return pEVRBase->GetClassID(pClassID);
		return E_NOTIMPL;
	}

	COuterEVR(const TCHAR* pName, LPUNKNOWN pUnk, HRESULT& hr, VMR9AlphaBitmap* pVMR9AlphaBitmap, CEVRAllocatorPresenter *pAllocatorPresenter) : CUnknown(pName, pUnk)
	{
		hr = m_pEVR.CoCreateInstance(CLSID_EnhancedVideoRenderer, GetOwner());
		m_pVMR9AlphaBitmap = pVMR9AlphaBitmap;
		m_pAllocatorPresenter = pAllocatorPresenter;
	}

	~COuterEVR();

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv)
	{
		HRESULT hr;

		if(riid == __uuidof(IVMRMixerBitmap9))
			return GetInterface((IVMRMixerBitmap9*)this, ppv);

		if (riid == __uuidof(IBaseFilter))
		{
			return GetInterface((IBaseFilter*)this, ppv);
		}

		if (riid == __uuidof(IMediaFilter))
		{
			return GetInterface((IMediaFilter*)this, ppv);
		}
		if (riid == __uuidof(IPersist))
		{
			return GetInterface((IPersist*)this, ppv);
		}
		if (riid == __uuidof(IBaseFilter))
		{
			return GetInterface((IBaseFilter*)this, ppv);
		}

		hr = m_pEVR ? m_pEVR->QueryInterface(riid, ppv) : E_NOINTERFACE;
		if(m_pEVR && FAILED(hr))
		{
			if(riid == __uuidof(IVMRffdshow9)) // Support ffdshow queueing. We show ffdshow that this is patched Media Player Classic.
				return GetInterface((IVMRffdshow9*)this, ppv);
		}

		return SUCCEEDED(hr) ? hr : __super::NonDelegatingQueryInterface(riid, ppv);
	}

	// IVMRffdshow9
	STDMETHODIMP support_ffdshow()
	{
		queueu_ffdshow_support = true;
		return S_OK;
	}

	// IVMRMixerBitmap9
	STDMETHODIMP GetAlphaBitmapParameters(VMR9AlphaBitmap* pBmpParms);
	STDMETHODIMP SetAlphaBitmap(const VMR9AlphaBitmap*  pBmpParms);
	STDMETHODIMP UpdateAlphaBitmapParameters(const VMR9AlphaBitmap* pBmpParms);
};

class CEVRAllocatorPresenter: 
	public CDX9AllocatorPresenter,
	public IMFGetService,
	public IMFTopologyServiceLookupClient,
	public IMFVideoDeviceID,
	public IMFVideoPresenter,
	public IDirect3DDeviceManager9,
	public IMFAsyncCallback,
	public IQualProp,
	public IMFRateSupport,				
	public IMFVideoDisplayControl,
	public IEVRTrustedVideoPlugin

{
public:
	CEVRAllocatorPresenter(HWND hWnd, HRESULT& hr );
	~CEVRAllocatorPresenter(void);

	void ThreadBeginStreaming();

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	STDMETHODIMP CreateRenderer(IUnknown** ppRenderer);
	STDMETHODIMP_(bool) Paint(bool fAll);
	STDMETHODIMP GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight);
	STDMETHODIMP InitializeDevice(AM_MEDIA_TYPE*	pMediaType);

	// IMFClockStateSink
	STDMETHODIMP OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset);        
	STDMETHODIMP STDMETHODCALLTYPE OnClockStop(MFTIME hnsSystemTime);
	STDMETHODIMP STDMETHODCALLTYPE OnClockPause(MFTIME hnsSystemTime);
	STDMETHODIMP STDMETHODCALLTYPE OnClockRestart(MFTIME hnsSystemTime);
	STDMETHODIMP STDMETHODCALLTYPE OnClockSetRate(MFTIME hnsSystemTime, float flRate);

	// IBaseFilter delegate
    bool GetState( DWORD dwMilliSecsTimeout, FILTER_STATE *State, HRESULT &_ReturnValue);

	// IQualProp (EVR statistics window). These are incompletely implemented currently
    STDMETHODIMP get_FramesDroppedInRenderer(int *pcFrames);
    STDMETHODIMP get_FramesDrawn(int *pcFramesDrawn);
    STDMETHODIMP get_AvgFrameRate(int *piAvgFrameRate);
    STDMETHODIMP get_Jitter(int *iJitter);
    STDMETHODIMP get_AvgSyncOffset(int *piAvg);
    STDMETHODIMP get_DevSyncOffset(int *piDev);

	// IMFRateSupport
    STDMETHODIMP GetSlowestRate(MFRATE_DIRECTION eDirection, BOOL fThin, float *pflRate);
    STDMETHODIMP GetFastestRate(MFRATE_DIRECTION eDirection, BOOL fThin, float *pflRate);
    STDMETHODIMP IsRateSupported(BOOL fThin, float flRate, float *pflNearestSupportedRate);
	float GetMaxRate(BOOL bThin);

	// IMFVideoPresenter
	STDMETHODIMP ProcessMessage(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam);
	STDMETHODIMP GetCurrentMediaType(__deref_out  IMFVideoMediaType **ppMediaType);

	// IMFTopologyServiceLookupClient        
	STDMETHODIMP InitServicePointers(__in  IMFTopologyServiceLookup *pLookup);
	STDMETHODIMP ReleaseServicePointers();

	// IMFVideoDeviceID
	STDMETHODIMP GetDeviceID(__out  IID *pDeviceID);

	// IMFGetService
	STDMETHODIMP GetService (__RPC__in REFGUID guidService, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID *ppvObject);

	// IMFAsyncCallback
	STDMETHODIMP GetParameters(__RPC__out DWORD *pdwFlags, /* [out] */ __RPC__out DWORD *pdwQueue);
	STDMETHODIMP Invoke(__RPC__in_opt IMFAsyncResult *pAsyncResult);

	// IMFVideoDisplayControl
    STDMETHODIMP GetNativeVideoSize(SIZE *pszVideo, SIZE *pszARVideo);    
    STDMETHODIMP GetIdealVideoSize(SIZE *pszMin, SIZE *pszMax);
    STDMETHODIMP SetVideoPosition(const MFVideoNormalizedRect *pnrcSource, const LPRECT prcDest);
    STDMETHODIMP GetVideoPosition(MFVideoNormalizedRect *pnrcSource, LPRECT prcDest);
    STDMETHODIMP SetAspectRatioMode(DWORD dwAspectRatioMode);
    STDMETHODIMP GetAspectRatioMode(DWORD *pdwAspectRatioMode);
    STDMETHODIMP SetVideoWindow(HWND hwndVideo);
    STDMETHODIMP GetVideoWindow(HWND *phwndVideo);
    STDMETHODIMP RepaintVideo( void);
    STDMETHODIMP GetCurrentImage(BITMAPINFOHEADER *pBih, BYTE **pDib, DWORD *pcbDib, LONGLONG *pTimeStamp);
    STDMETHODIMP SetBorderColor(COLORREF Clr);
    STDMETHODIMP GetBorderColor(COLORREF *pClr);
    STDMETHODIMP SetRenderingPrefs(DWORD dwRenderFlags);
    STDMETHODIMP GetRenderingPrefs(DWORD *pdwRenderFlags);
    STDMETHODIMP SetFullscreen(BOOL fFullscreen);
    STDMETHODIMP GetFullscreen(BOOL *pfFullscreen);

	// IEVRTrustedVideoPlugin
    STDMETHODIMP IsInTrustedVideoMode(BOOL *pYes);
    STDMETHODIMP CanConstrict(BOOL *pYes);
    STDMETHODIMP SetConstriction(DWORD dwKPix);
    STDMETHODIMP DisableImageExport(BOOL bDisable);

	// IDirect3DDeviceManager9
	STDMETHODIMP ResetDevice(IDirect3DDevice9 *pDevice,UINT resetToken);        
	STDMETHODIMP OpenDeviceHandle(HANDLE *phDevice);
	STDMETHODIMP CloseDeviceHandle(HANDLE hDevice);        
    STDMETHODIMP TestDevice(HANDLE hDevice);
	STDMETHODIMP LockDevice(HANDLE hDevice, IDirect3DDevice9 **ppDevice, BOOL fBlock);
	STDMETHODIMP UnlockDevice(HANDLE hDevice, BOOL fSaveState);
	STDMETHODIMP GetVideoService(HANDLE hDevice, REFIID riid, void **ppService);

protected:
	void OnResetDevice();
	MFCLOCK_STATE m_LastClockState;

private:
	typedef enum
	{
		Started = State_Running,
		Stopped = State_Stopped,
		Paused = State_Paused,
		Shutdown = State_Running + 1
	} RENDER_STATE;

	COuterEVR *m_pOuterEVR;
	CComPtr<IMFClock> m_pClock;
	CComPtr<IDirect3DDeviceManager9> m_pD3DManager;
	CComPtr<IMFTransform> m_pMixer;
	CComPtr<IMediaEventSink> m_pSink;
	CComPtr<IMFVideoMediaType> m_pMediaType;
	MFVideoAspectRatioMode m_dwVideoAspectRatioMode;
	MFVideoRenderPrefs m_dwVideoRenderPrefs;
	COLORREF m_BorderColor;

	HANDLE m_hEvtQuit; // Stop rendering thread event
    HANDLE m_hEvtSampleNotify; // Stop rendering thread event
    bool m_SampleNotified;
    bool m_HasSampleNotified;
	bool m_bEvtQuit;
	HANDLE m_hEvtFlush; // Discard all buffers
	bool m_bEvtFlush;

	bool m_bUseInternalTimer;
	int32 m_LastSetOutputRange;
	bool m_bPendingRenegotiate;
	bool m_bPendingMediaFinished;
	bool m_bPrerolled; // true if first sample has been displayed.

	HANDLE m_hRenderThread;
	HANDLE m_hMixerThread;
	RENDER_STATE m_nRenderState;
	
	CCritSec m_SampleQueueLock;
   // CCritSec m_FreeSampleQueueLock;
   // CCritSec m_ScheduleSampleQueueLock;
	CCritSec m_ImageProcessingLock;

	CInterfaceList<IMFSample, &IID_IMFSample> m_FreeSamples;
	CInterfaceList<IMFSample, &IID_IMFSample> m_ScheduledSamples;
	IMFSample *m_pCurrentDisplaydSample;
	UINT m_nResetToken;
	int m_nStepCount;

    HRESULT ProcessOutputSafe( DWORD dwFlags, DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER *pOutputSamples, DWORD *pdwStatus) ;

	bool GetSampleFromMixer();
	void MixerThread();
	static DWORD WINAPI MixerThreadStatic(LPVOID lpParam);
	void RenderThread();
	static DWORD WINAPI RenderThreadStatic(LPVOID lpParam);

	void StartWorkerThreads();
	void StopWorkerThreads();
	HRESULT CheckShutdown() const;
	void CompleteFrameStep(bool bCancel);

	void RemoveAllSamples();
	HRESULT BeginStreaming();
	HRESULT GetFreeSample(IMFSample** ppSample);
	HRESULT GetScheduledSample(IMFSample** ppSample, int &_Count);
	void MoveToFreeList(IMFSample* pSample, bool bTail);
	void MoveToScheduledList(IMFSample* pSample, bool _bSorted);
	void FlushSamples();
	void FlushSamplesInternal();

	HRESULT RenegotiateMediaType();
	HRESULT IsMediaTypeSupported(IMFMediaType* pMixerType);
	HRESULT CreateProposedOutputType(IMFMediaType* pMixerType, IMFMediaType** pType);
	HRESULT SetMediaType(IMFMediaType* pType);

	// Functions pointers for Vista/.NET3 specific library
	PTR_DXVA2CreateDirect3DDeviceManager9 pfDXVA2CreateDirect3DDeviceManager9;
	PTR_MFCreateDXSurfaceBuffer pfMFCreateDXSurfaceBuffer;
	PTR_MFCreateVideoSampleFromSurface pfMFCreateVideoSampleFromSurface;
	PTR_MFCreateVideoMediaType pfMFCreateVideoMediaType;
											
	PTR_AvSetMmThreadCharacteristicsW pfAvSetMmThreadCharacteristicsW;
	PTR_AvSetMmThreadPriority pfAvSetMmThreadPriority;
	PTR_AvRevertMmThreadCharacteristics pfAvRevertMmThreadCharacteristics;
};

HRESULT STDMETHODCALLTYPE COuterEVR::GetState( DWORD dwMilliSecsTimeout, __out  FILTER_STATE *State)
{
	HRESULT ReturnValue;
	CComPtr<IBaseFilter> pEVRBase;
	if (m_pEVR)
		m_pEVR->QueryInterface(&pEVRBase);
	if (pEVRBase)
		return pEVRBase->GetState(dwMilliSecsTimeout, State);
	return E_NOTIMPL;
}

STDMETHODIMP COuterEVR::GetAlphaBitmapParameters(VMR9AlphaBitmap* pBmpParms)
{
	CheckPointer(pBmpParms, E_POINTER);
	CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
	memcpy (pBmpParms, m_pVMR9AlphaBitmap, sizeof(VMR9AlphaBitmap));
	return S_OK;
}

STDMETHODIMP COuterEVR::SetAlphaBitmap(const VMR9AlphaBitmap*  pBmpParms)
{
	CheckPointer(pBmpParms, E_POINTER);
	CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
	memcpy (m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
	m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
	m_pAllocatorPresenter->UpdateAlphaBitmap();
	return S_OK;
}

STDMETHODIMP COuterEVR::UpdateAlphaBitmapParameters(const VMR9AlphaBitmap* pBmpParms)
{
	CheckPointer(pBmpParms, E_POINTER);
	CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
	memcpy (m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
	m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
	m_pAllocatorPresenter->UpdateAlphaBitmap();
	return S_OK;
}

COuterEVR::~COuterEVR()
{
}

CString GetWindowsErrorMessage(HRESULT _Error, HMODULE _Module);

HRESULT CreateEVR(const CLSID& clsid, HWND hWnd, ISubPicAllocatorPresenterRender** ppAP)
{
	HRESULT		hr = E_FAIL;
	if (clsid == CLSID_EVRAllocatorPresenter)
	{
		CString Error;
		*ppAP	= DNew CEVRAllocatorPresenter(hWnd, hr );
		(*ppAP)->AddRef();

		if(FAILED(hr))
		{
			Error += L"\n";
			Error += GetWindowsErrorMessage(hr, NULL);
			//MessageBox(hWnd, Error, L"Error creating EVR Custom renderer", MB_OK | MB_ICONERROR);
			(*ppAP)->Release();
			*ppAP = NULL;
		}
		else if (!Error.IsEmpty())
		{
			//MessageBox(hWnd, Error, L"Warning creating EVR Custom renderer", MB_OK|MB_ICONWARNING);
		}
	}
	return hr;
}

CEVRAllocatorPresenter::CEVRAllocatorPresenter(HWND hWnd, HRESULT& hr )
	: CDX9AllocatorPresenter(hWnd, hr, true )
{
	HMODULE		hLib;
	AppSettings& s = AfxGetAppSettings();

	m_nResetToken = 0;
	m_hRenderThread  = INVALID_HANDLE_VALUE;
	m_hMixerThread= INVALID_HANDLE_VALUE;
	m_hEvtFlush = INVALID_HANDLE_VALUE;
	m_hEvtQuit = INVALID_HANDLE_VALUE;
    m_hEvtSampleNotify = INVALID_HANDLE_VALUE;
	m_bEvtQuit = 0;
    m_SampleNotified = true;
    m_HasSampleNotified = 0;
	m_bEvtFlush = 0;

	m_bNeedPendingResetDevice = true;

	if (FAILED (hr)) 
	{
		//_Error += L"DX9AllocatorPresenter failed\n";
		return;
	}

	// Load EVR specifics DLLs
	hLib = LoadLibrary (L"dxva2.dll");
	pfDXVA2CreateDirect3DDeviceManager9	= hLib ? (PTR_DXVA2CreateDirect3DDeviceManager9) GetProcAddress (hLib, "DXVA2CreateDirect3DDeviceManager9") : NULL;
	
	// Load EVR functions
	hLib = LoadLibrary (L"evr.dll");
	pfMFCreateDXSurfaceBuffer = hLib ? (PTR_MFCreateDXSurfaceBuffer)GetProcAddress (hLib, "MFCreateDXSurfaceBuffer") : NULL;
	pfMFCreateVideoSampleFromSurface = hLib ? (PTR_MFCreateVideoSampleFromSurface)GetProcAddress (hLib, "MFCreateVideoSampleFromSurface") : NULL;
	pfMFCreateVideoMediaType = hLib ? (PTR_MFCreateVideoMediaType)GetProcAddress (hLib, "MFCreateVideoMediaType") : NULL;

	if (!pfDXVA2CreateDirect3DDeviceManager9 || !pfMFCreateDXSurfaceBuffer || !pfMFCreateVideoSampleFromSurface || !pfMFCreateVideoMediaType)
	{
		/*
			if (!pfDXVA2CreateDirect3DDeviceManager9)
						_Error += L"Could not find DXVA2CreateDirect3DDeviceManager9 (dxva2.dll)\n";
					if (!pfMFCreateDXSurfaceBuffer)
						_Error += L"Could not find MFCreateDXSurfaceBuffer (evr.dll)\n";
					if (!pfMFCreateVideoSampleFromSurface)
						_Error += L"Could not find MFCreateVideoSampleFromSurface (evr.dll)\n";
					if (!pfMFCreateVideoMediaType)
						_Error += L"Could not find MFCreateVideoMediaType (evr.dll)\n";*/
			
		hr = E_FAIL;
		return;
	}

	// Load Vista specific DLLs
	hLib = LoadLibrary (L"avrt.dll");
	pfAvSetMmThreadCharacteristicsW = hLib ? (PTR_AvSetMmThreadCharacteristicsW) GetProcAddress (hLib, "AvSetMmThreadCharacteristicsW") : NULL;
	pfAvSetMmThreadPriority = hLib ? (PTR_AvSetMmThreadPriority) GetProcAddress (hLib, "AvSetMmThreadPriority") : NULL;
	pfAvRevertMmThreadCharacteristics = hLib ? (PTR_AvRevertMmThreadCharacteristics) GetProcAddress (hLib, "AvRevertMmThreadCharacteristics") : NULL;

	// Init DXVA manager
	hr = pfDXVA2CreateDirect3DDeviceManager9(&m_nResetToken, &m_pD3DManager);
	if (SUCCEEDED (hr)) 
	{
		hr = m_pD3DManager->ResetDevice(m_pD3DDev, m_nResetToken);
		if (!SUCCEEDED (hr)) 
		{
			//_Error += L"m_pD3DManager->ResetDevice failed\n";
		}
	}
	//else
	//	_Error += L"DXVA2CreateDirect3DDeviceManager9 failed\n";

	CComPtr<IDirectXVideoDecoderService> pDecoderService;
	HANDLE hDevice;
	if (SUCCEEDED (m_pD3DManager->OpenDeviceHandle(&hDevice)) &&
		SUCCEEDED (m_pD3DManager->GetVideoService (hDevice, __uuidof(IDirectXVideoDecoderService), (void**)&pDecoderService)))
	{
		HookDirectXVideoDecoderService (pDecoderService);
		m_pD3DManager->CloseDeviceHandle (hDevice);
	}

	// Bufferize frame only with 3D texture
	if (s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D)
		m_nDXSurface	= max (min (s.iEvrBuffers, MAX_PICTURE_SLOTS-2), 4);
	else
		m_nDXSurface = 1;


	m_nRenderState = Shutdown;
	m_bUseInternalTimer = false;
	m_LastSetOutputRange = -1;
	m_bPendingRenegotiate = false;
	m_bPendingMediaFinished = false;
	m_pCurrentDisplaydSample = NULL;
	m_nStepCount = 0;
	m_dwVideoAspectRatioMode = MFVideoARMode_PreservePicture;
	m_dwVideoRenderPrefs = (MFVideoRenderPrefs)0;
	m_BorderColor = RGB (0,0,0);
	m_pOuterEVR = NULL;
	m_bPrerolled = false;
}

CEVRAllocatorPresenter::~CEVRAllocatorPresenter(void)
{
	StopWorkerThreads();
	m_pMediaType = NULL;
	m_pClock = NULL;
	m_pD3DManager = NULL;
}

HRESULT CEVRAllocatorPresenter::CheckShutdown() const 
{
    if (m_nRenderState == Shutdown) return MF_E_SHUTDOWN;
    else return S_OK;
}

void CEVRAllocatorPresenter::StartWorkerThreads()
{
	DWORD dwThreadId;
	if (m_nRenderState == Shutdown)
	{
		m_hEvtQuit = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_hEvtFlush = CreateEvent(NULL, TRUE, FALSE, NULL);
        m_hEvtSampleNotify = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_hMixerThread = ::CreateThread(NULL, 0, MixerThreadStatic, (LPVOID)this, 0, &dwThreadId);
		SetThreadPriority(m_hMixerThread, THREAD_PRIORITY_HIGHEST);
		m_hRenderThread = ::CreateThread(NULL, 0, RenderThreadStatic, (LPVOID)this, 0, &dwThreadId);
		SetThreadPriority(m_hRenderThread, THREAD_PRIORITY_TIME_CRITICAL);
		m_nRenderState = Stopped;
	}
}

void CEVRAllocatorPresenter::StopWorkerThreads()
{
	if (m_nRenderState != Shutdown)
	{
        m_SampleNotified = true;
        SetEvent(m_hEvtSampleNotify);
        SetEvent (m_hEvtFlush);
		m_bEvtFlush = true;
		SetEvent (m_hEvtQuit);
		m_bEvtQuit = true;
        if ((m_hRenderThread != INVALID_HANDLE_VALUE) && (WaitForSingleObject (m_hRenderThread, 1000) == WAIT_TIMEOUT))
		{
			ASSERT (FALSE);
			TerminateThread (m_hRenderThread, 0xDEAD);
		}
		if (m_hRenderThread != INVALID_HANDLE_VALUE) CloseHandle (m_hRenderThread);
		if ((m_hMixerThread != INVALID_HANDLE_VALUE) && (WaitForSingleObject (m_hMixerThread, 1000) == WAIT_TIMEOUT))
		{
			ASSERT (FALSE);
			TerminateThread (m_hMixerThread, 0xDEAD);
		}
		if (m_hMixerThread != INVALID_HANDLE_VALUE) CloseHandle (m_hMixerThread);

        if (m_hEvtSampleNotify != INVALID_HANDLE_VALUE) CloseHandle (m_hEvtSampleNotify);

		if (m_hEvtFlush != INVALID_HANDLE_VALUE) CloseHandle (m_hEvtFlush);
		if (m_hEvtQuit != INVALID_HANDLE_VALUE) CloseHandle (m_hEvtQuit);
        
        m_SampleNotified = true;
        m_HasSampleNotified = 0;
		m_bEvtFlush = false;
		m_bEvtQuit = false;
	}
	m_nRenderState = Shutdown;
}

STDMETHODIMP CEVRAllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
    CheckPointer(ppRenderer, E_POINTER);
	*ppRenderer = NULL;
	HRESULT hr = E_FAIL;

	do
	{
		CMacrovisionKicker* pMK = DNew CMacrovisionKicker(NAME("CMacrovisionKicker"), NULL);
		CComPtr<IUnknown> pUnk = (IUnknown*)(INonDelegatingUnknown*)pMK;

		COuterEVR *pOuterEVR = DNew COuterEVR(NAME("COuterEVR"), pUnk, hr, &m_VMR9AlphaBitmap, this);
		m_pOuterEVR = pOuterEVR;

		pMK->SetInner((IUnknown*)(INonDelegatingUnknown*)pOuterEVR);
		CComQIPtr<IBaseFilter> pBF = pUnk;

		if (FAILED(hr)) break;

		// Set EVR custom presenter
		CComPtr<IMFVideoPresenter> pVP;
		CComPtr<IMFVideoRenderer> pMFVR;
		CComQIPtr<IMFGetService, &__uuidof(IMFGetService)> pMFGS = pBF;

		hr = pMFGS->GetService (MR_VIDEO_RENDER_SERVICE, IID_IMFVideoRenderer, (void**)&pMFVR);

		if(SUCCEEDED(hr)) hr = QueryInterface(__uuidof(IMFVideoPresenter), (void**)&pVP);
		if(SUCCEEDED(hr)) hr = pMFVR->InitializeRenderer(NULL, pVP);

		CComPtr<IPin> pPin = GetFirstPin(pBF);
		CComQIPtr<IMemInputPin> pMemInputPin = pPin;
		
		m_bUseInternalTimer = HookNewSegmentAndReceive((IPinC*)(IPin*)pPin, (IMemInputPinC*)(IMemInputPin*)pMemInputPin);
		if(FAILED(hr))
			*ppRenderer = NULL;
		else
			*ppRenderer = pBF.Detach();
	} while (0);

	return hr;
}

STDMETHODIMP_(bool) CEVRAllocatorPresenter::Paint(bool fAll)
{
	return __super::Paint(fAll);
}

STDMETHODIMP CEVRAllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	////SVP_LogMsg5(CStringFromGUID(riid));

	HRESULT		hr;
	if(riid == __uuidof(IMFClockStateSink))
		hr = GetInterface((IMFClockStateSink*)this, ppv);
	else if(riid == __uuidof(IMFVideoPresenter))
		hr = GetInterface((IMFVideoPresenter*)this, ppv);
	else if(riid == __uuidof(IMFTopologyServiceLookupClient))
		hr = GetInterface((IMFTopologyServiceLookupClient*)this, ppv);
	else if(riid == __uuidof(IMFVideoDeviceID))
		hr = GetInterface((IMFVideoDeviceID*)this, ppv);
	else if(riid == __uuidof(IMFGetService))
		hr = GetInterface((IMFGetService*)this, ppv);
	else if(riid == __uuidof(IMFAsyncCallback))
		hr = GetInterface((IMFAsyncCallback*)this, ppv);
	else if(riid == __uuidof(IMFVideoDisplayControl))
		hr = GetInterface((IMFVideoDisplayControl*)this, ppv);
	else if(riid == __uuidof(IEVRTrustedVideoPlugin))
		hr = GetInterface((IEVRTrustedVideoPlugin*)this, ppv);
	else if(riid == IID_IQualProp)
		hr = GetInterface((IQualProp*)this, ppv);
	else if(riid == __uuidof(IMFRateSupport))
		hr = GetInterface((IMFRateSupport*)this, ppv);
	else if(riid == __uuidof(IDirect3DDeviceManager9))
		hr = m_pD3DManager->QueryInterface (__uuidof(IDirect3DDeviceManager9), (void**) ppv);
	else
		hr = __super::NonDelegatingQueryInterface(riid, ppv);

	return hr;
}

// IMFClockStateSink
STDMETHODIMP CEVRAllocatorPresenter::OnClockStart(MFTIME hnsSystemTime,  LONGLONG llClockStartOffset)
{
    //SVP_LogMsg5( L"CEVRAllocatorPresenter::OnClockStart");
	m_nRenderState = Started;
	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::OnClockStop(MFTIME hnsSystemTime)
{
    SVP_LogMsg5( L"CEVRAllocatorPresenter::OnClockStop");
	m_nRenderState = Stopped;
	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::OnClockPause(MFTIME hnsSystemTime)
{
    SVP_LogMsg5( L"CEVRAllocatorPresenter::OnClockPause");
	m_nRenderState = Paused;
	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::OnClockRestart(MFTIME hnsSystemTime)
{
    SVP_LogMsg5( L"CEVRAllocatorPresenter::OnClockRestart");
	m_nRenderState	= Started;
	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::OnClockSetRate(MFTIME hnsSystemTime, float flRate)
{
    SVP_LogMsg5( L"CEVRAllocatorPresenter::OnClockSetRate");
	return E_NOTIMPL;
}

// IBaseFilter delegate
bool CEVRAllocatorPresenter::GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *State, HRESULT &_ReturnValue)
{
	switch(m_nRenderState)
	{
		case Started: *State = State_Running; break;
		case Paused: *State = State_Paused; break;
		case Stopped: *State = State_Stopped; break;
		default: *State = State_Stopped; _ReturnValue = E_FAIL;
	}
	_ReturnValue = S_OK;
	return true;
}

// IQualProp
STDMETHODIMP CEVRAllocatorPresenter::get_FramesDroppedInRenderer(int *pcFrames)
{
	*pcFrames = m_pcFramesDropped;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::get_FramesDrawn(int *pcFramesDrawn)
{
	*pcFramesDrawn = m_pcFramesDrawn;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::get_AvgFrameRate(int *piAvgFrameRate)
{
	*piAvgFrameRate = (int)(m_fAvrFps * 100);
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::get_Jitter(int *iJitter)
{
	*iJitter = (int)((m_fJitterStdDev/10000.0) + 0.5);
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::get_AvgSyncOffset(int *piAvg)
{
	*piAvg = (int)((m_fSyncOffsetAvr/10000.0) + 0.5);
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::get_DevSyncOffset(int *piDev)
{
	*piDev = (int)((m_fSyncOffsetStdDev/10000.0) + 0.5);
	return S_OK;
}

// IMFRateSupport
STDMETHODIMP CEVRAllocatorPresenter::GetSlowestRate(MFRATE_DIRECTION eDirection, BOOL fThin, float *pflRate)
{
	*pflRate = 0;
	return S_OK;
}
    
STDMETHODIMP CEVRAllocatorPresenter::GetFastestRate(MFRATE_DIRECTION eDirection, BOOL fThin, float *pflRate)
{
	HRESULT		hr = S_OK;
	float		fMaxRate = 0.0f;

	CAutoLock lock(this);

	CheckPointer(pflRate, E_POINTER);
	CheckHR(CheckShutdown());
    
	// Get the maximum forward rate.
	fMaxRate = GetMaxRate(fThin);

	// For reverse playback, swap the sign.
	if (eDirection == MFRATE_REVERSE)
		fMaxRate = -fMaxRate;

	*pflRate = fMaxRate;
	return hr;
}
    
STDMETHODIMP CEVRAllocatorPresenter::IsRateSupported(BOOL fThin, float flRate, float *pflNearestSupportedRate)
{
    // fRate can be negative for reverse playback.
    // pfNearestSupportedRate can be NULL.

    CAutoLock lock(this);

    HRESULT hr = S_OK;
    float   fMaxRate = 0.0f;
    float   fNearestRate = flRate;   // Default.

	CheckPointer (pflNearestSupportedRate, E_POINTER);
    CheckHR(hr = CheckShutdown());

    // Find the maximum forward rate.
    fMaxRate = GetMaxRate(fThin);

    if (fabsf(flRate) > fMaxRate)
    {
        // The (absolute) requested rate exceeds the maximum rate.
        hr = MF_E_UNSUPPORTED_RATE;

        // The nearest supported rate is fMaxRate.
        fNearestRate = fMaxRate;
        if (flRate < 0)
        {
            // For reverse playback, swap the sign.
            fNearestRate = -fNearestRate;
        }
    }
    // Return the nearest supported rate if the caller requested it.
    if (pflNearestSupportedRate != NULL) *pflNearestSupportedRate = fNearestRate;
    return hr;
}

float CEVRAllocatorPresenter::GetMaxRate(BOOL bThin)
{
	float fMaxRate = FLT_MAX;  // Default.
	UINT32 fpsNumerator = 0, fpsDenominator = 0;
	UINT MonitorRateHz = 0; 

	if (!bThin && (m_pMediaType != NULL))
	{
		// Non-thinned: Use the frame rate and monitor refresh rate.
        
		// Frame rate:
		MFGetAttributeRatio(m_pMediaType, MF_MT_FRAME_RATE, 
			&fpsNumerator, &fpsDenominator);

		// Monitor refresh rate:
		MonitorRateHz = m_uD3DRefreshRate; // D3DDISPLAYMODE

		if (fpsDenominator && fpsNumerator && MonitorRateHz)
		{
			// Max Rate = Refresh Rate / Frame Rate
			fMaxRate = (float)MulDiv(MonitorRateHz, fpsDenominator, fpsNumerator);
		}
	}
	return fMaxRate;
}

void CEVRAllocatorPresenter::CompleteFrameStep(bool bCancel)
{
	if (m_nStepCount > 0)
	{
		if (bCancel || (m_nStepCount == 1)) 
		{
			m_pSink->Notify(EC_STEP_COMPLETE, bCancel ? TRUE : FALSE, 0);
			m_nStepCount = 0;
		}
		else
			m_nStepCount--;
	}
}

// IMFVideoPresenter
STDMETHODIMP CEVRAllocatorPresenter::ProcessMessage(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam)
{
	HRESULT hr = S_OK;
	AppSettings& s = AfxGetAppSettings();
    //SVP_LogMsg6("ProcessMessage %x", eMessage);
	switch (eMessage)
	{
	case MFVP_MESSAGE_BEGINSTREAMING : // The EVR switched from stopped to paused. The presenter should allocate resources
		hr = BeginStreaming();
		break;

	case MFVP_MESSAGE_CANCELSTEP:
		CompleteFrameStep (true);
		break;

	case MFVP_MESSAGE_ENDOFSTREAM : 
		m_bPendingMediaFinished = true;
		break;

	case MFVP_MESSAGE_ENDSTREAMING :
		{
			m_pGenlock->ResetTiming();
			m_pRefClock = NULL;
		}
		break;

	case MFVP_MESSAGE_FLUSH :
		SetEvent(m_hEvtFlush);
        ResetEvent(m_hEvtSampleNotify);
        if(m_HasSampleNotified)
            m_SampleNotified = 0;
		m_bEvtFlush = true;
        //SVP_LogMsg6("MFVP_MESSAGE_FLUSH");
		while (WaitForSingleObject(m_hEvtFlush, 1) == WAIT_OBJECT_0);
        //SVP_LogMsg6("MFVP_MESSAGE_FLUSH OUT");
		break;

	case MFVP_MESSAGE_INVALIDATEMEDIATYPE:
		m_bPendingRenegotiate = true;
		while (*((volatile bool *)&m_bPendingRenegotiate)) Sleep(1);
		break;

	case MFVP_MESSAGE_PROCESSINPUTNOTIFY:
        SetEvent(m_hEvtSampleNotify);
        m_HasSampleNotified = true;
        m_SampleNotified = true;
        //SVP_LogMsg6("MFVP_MESSAGE_PROCESSINPUTNOTIFY");
        //while (WaitForSingleObject(m_hEvtSampleNotify, 1) == WAIT_OBJECT_0);
		break;

	case MFVP_MESSAGE_STEP:
		m_nStepCount = ulParam;
		hr = S_OK;
		break;

	default :
        //SVP_LogMsg6("MFVP_MESSAGE_ UNKNOW MESSAGE %x", eMessage);
		ASSERT(FALSE);
		break;
	}
	return hr;
}


HRESULT CEVRAllocatorPresenter::IsMediaTypeSupported(IMFMediaType* pMixerType)
{
	HRESULT hr;
	AM_MEDIA_TYPE* pAMMedia;
	UINT nInterlaceMode;

	CheckHR (pMixerType->GetRepresentation(FORMAT_VideoInfo2, (void**)&pAMMedia));
	CheckHR (pMixerType->GetUINT32 (MF_MT_INTERLACE_MODE, &nInterlaceMode));

    //SVP_LogMsg5(L"MF_MT_INTERLACE_MODE %d",nInterlaceMode);

	if ( (pAMMedia->majortype != MEDIATYPE_Video)) hr = MF_E_INVALIDMEDIATYPE;
	pMixerType->FreeRepresentation(FORMAT_VideoInfo2, (void*)pAMMedia);
	return hr;
}

HRESULT CEVRAllocatorPresenter::CreateProposedOutputType(IMFMediaType* pMixerType, IMFMediaType** pType)
{
	HRESULT hr;
	AM_MEDIA_TYPE *pAMMedia = NULL;
	LARGE_INTEGER i64Size;
	MFVIDEOFORMAT *VideoFormat;

	CheckHR(pMixerType->GetRepresentation(FORMAT_MFVideoFormat, (void**)&pAMMedia));
	
	VideoFormat = (MFVIDEOFORMAT*)pAMMedia->pbFormat;
	hr = pfMFCreateVideoMediaType(VideoFormat, &m_pMediaType);

	m_AspectRatio.cx = VideoFormat->videoInfo.PixelAspectRatio.Numerator;
	m_AspectRatio.cy = VideoFormat->videoInfo.PixelAspectRatio.Denominator;

	if (SUCCEEDED (hr))
	{
		i64Size.HighPart = VideoFormat->videoInfo.dwWidth;
		i64Size.LowPart	 = VideoFormat->videoInfo.dwHeight;
		m_pMediaType->SetUINT64(MF_MT_FRAME_SIZE, i64Size.QuadPart);
		m_pMediaType->SetUINT32(MF_MT_PAN_SCAN_ENABLED, 0);
		AppSettings& s = AfxGetAppSettings();
		
		if (s.m_RenderSettings.iEVROutputRange == 1)
			m_pMediaType->SetUINT32(MF_MT_VIDEO_NOMINAL_RANGE, MFNominalRange_16_235);
		else
			m_pMediaType->SetUINT32(MF_MT_VIDEO_NOMINAL_RANGE, MFNominalRange_0_255);

        //m_pMediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive );

		m_LastSetOutputRange = s.m_RenderSettings.iEVROutputRange;
		i64Size.HighPart = m_AspectRatio.cx;
		i64Size.LowPart  = m_AspectRatio.cy;
		m_pMediaType->SetUINT64(MF_MT_PIXEL_ASPECT_RATIO, i64Size.QuadPart);

		MFVideoArea Area = MakeArea(0, 0, VideoFormat->videoInfo.dwWidth, VideoFormat->videoInfo.dwHeight);
		m_pMediaType->SetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*)&Area, sizeof(MFVideoArea));
	}

	////SVP_LogMsg6("CreateProposedOutputType %d %d %d %d ", m_AspectRatio.cx , m_AspectRatio.cy, VideoFormat->videoInfo.dwWidth, VideoFormat->videoInfo.dwHeight );
	if(min(m_AspectRatio.cx, m_AspectRatio.cy) <= 0){
		m_AspectRatio.cx = m_AspectRatio.cy = 1;
	}
	m_AspectRatio.cx *= VideoFormat->videoInfo.dwWidth;
	m_AspectRatio.cy *= VideoFormat->videoInfo.dwHeight;

	bool bDoneSomething = true;

	//如果 m_AspectRatio.cx 为 0 会无限循环
	for(int k = 0; k < 100; k++)
	{
		if(!bDoneSomething){
			break;
		}
		bDoneSomething = false;
		INT MinNum = min(m_AspectRatio.cx, m_AspectRatio.cy);
		if(MinNum < 2){
			break;
		}
		INT i;
		for (i = 2; i < MinNum+1; ++i)
		{
			if (m_AspectRatio.cx%i == 0 && m_AspectRatio.cy%i ==0)
				break;
		}
		if (i != MinNum + 1)
		{
			m_AspectRatio.cx = m_AspectRatio.cx / i;
			m_AspectRatio.cy = m_AspectRatio.cy / i;
			bDoneSomething = true;
		}
	}

	
	pMixerType->FreeRepresentation(FORMAT_MFVideoFormat, (void*)pAMMedia);
	m_pMediaType->QueryInterface(__uuidof(IMFMediaType), (void**) pType);

	return hr;
}

HRESULT CEVRAllocatorPresenter::SetMediaType(IMFMediaType* pType)
{
	HRESULT hr;
	AM_MEDIA_TYPE* pAMMedia = NULL;
	CString strTemp;

	CheckPointer(pType, E_POINTER);
	CheckHR(pType->GetRepresentation(FORMAT_VideoInfo2, (void**)&pAMMedia));
	
	hr = InitializeDevice(pAMMedia);
	if (SUCCEEDED(hr))
	{
		strTemp = GetMediaTypeName(pAMMedia->subtype);
		strTemp.Replace(L"MEDIASUBTYPE_", L"");
		m_strStatsMsg[MSG_MIXEROUT].Format (L"Mixer output: %s", strTemp);
	}

	pType->FreeRepresentation(FORMAT_VideoInfo2, (void*)pAMMedia);

	return hr;
}

LONGLONG GetMediaTypeMerit(IMFMediaType *pMediaType)
{
	AM_MEDIA_TYPE *pAMMedia = NULL;
	MFVIDEOFORMAT *VideoFormat;

	HRESULT hr;
	CheckHR(pMediaType->GetRepresentation  (FORMAT_MFVideoFormat, (void**)&pAMMedia));
	VideoFormat = (MFVIDEOFORMAT*)pAMMedia->pbFormat;

	LONGLONG Merit = 0;
	switch (VideoFormat->surfaceInfo.Format)
	{
		case FCC('NV12'): Merit = 90000000; break;
		case FCC('YV12'): Merit = 80000000; break;
		case FCC('YUY2'): Merit = 70000000; break;
		case FCC('UYVY'): Merit = 60000000; break;

		case D3DFMT_X8R8G8B8: // Never opt for RGB
		case D3DFMT_A8R8G8B8: 
		case D3DFMT_R8G8B8: 
		case D3DFMT_R5G6B5: 
			Merit = 0; 
			break;
		default: Merit = 1000; break;
	}
	pMediaType->FreeRepresentation(FORMAT_MFVideoFormat, (void*)pAMMedia);
	return Merit;
}

typedef struct
{
  const int Format;
  const LPCTSTR Description;
} D3DFORMAT_TYPE;

extern const D3DFORMAT_TYPE	D3DFormatType[];

LPCTSTR FindD3DFormat(const D3DFORMAT Format);

LPCTSTR GetMediaTypeFormatDesc(IMFMediaType *pMediaType)
{
	AM_MEDIA_TYPE *pAMMedia = NULL;
	MFVIDEOFORMAT *VideoFormat;
	HRESULT hr;

	hr = pMediaType->GetRepresentation(FORMAT_MFVideoFormat, (void**)&pAMMedia);
	VideoFormat = (MFVIDEOFORMAT*)pAMMedia->pbFormat;
	LPCTSTR Type = FindD3DFormat((D3DFORMAT)VideoFormat->surfaceInfo.Format);
	pMediaType->FreeRepresentation (FORMAT_MFVideoFormat, (void*)pAMMedia);
	return Type;
}

HRESULT CEVRAllocatorPresenter::RenegotiateMediaType()
{
    HRESULT hr = S_OK;

    CComPtr<IMFMediaType> pMixerType;
    CComPtr<IMFMediaType> pType;

    if (!m_pMixer) return MF_E_INVALIDREQUEST;

	CInterfaceArray<IMFMediaType> ValidMixerTypes;
    // Loop through all of the mixer's proposed output types.
    DWORD iTypeIndex = 0;
    while ((hr != MF_E_NO_MORE_TYPES))
    {
        pMixerType  = NULL;
        pType = NULL;
		m_pMediaType = NULL;

        // Step 1. Get the next media type supported by mixer.
        hr = m_pMixer->GetOutputAvailableType(0, iTypeIndex++, &pMixerType);
        if (FAILED(hr))
        {
            break;
        }

        // Step 2. Check if we support this media type.
        if (SUCCEEDED(hr))
            hr = IsMediaTypeSupported(pMixerType);

        if (SUCCEEDED(hr))
	        hr = CreateProposedOutputType(pMixerType, &pType);
	
        // Step 4. Check if the mixer will accept this media type.
        if (SUCCEEDED(hr))
            hr = m_pMixer->SetOutputType(0, pType, MFT_SET_TYPE_TEST_ONLY);

        if (SUCCEEDED(hr))
		{
			LONGLONG Merit = GetMediaTypeMerit(pType);

			int nTypes = ValidMixerTypes.GetCount();
			int iInsertPos = 0;
			for (int i = 0; i < nTypes; ++i)
			{
				LONGLONG ThisMerit = GetMediaTypeMerit(ValidMixerTypes[i]);
				if (Merit > ThisMerit)
				{
					iInsertPos = i;
					break;
				}
				else
					iInsertPos = i+1;
			}
			ValidMixerTypes.InsertAt(iInsertPos, pType);
		}
    }


	int nValidTypes = ValidMixerTypes.GetCount();
	for (int i = 0; i < nValidTypes; ++i)
	{
		pType = ValidMixerTypes[i];
	}

	for (int i = 0; i < nValidTypes; ++i)
	{
		pType = ValidMixerTypes[i];
		hr = SetMediaType(pType);
        if (SUCCEEDED(hr))
        {
            hr = m_pMixer->SetOutputType(0, pType, 0);
            // If something went wrong, clear the media type.
            if (FAILED(hr))
            {
                SetMediaType(NULL);
            }
			else
				break;
        }
	}

    pMixerType = NULL;
    pType = NULL;
    return hr;
}
HRESULT CEVRAllocatorPresenter::ProcessOutputSafe( DWORD dwFlags, DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER *pOutputSamples, DWORD *pdwStatus) 
{
    __try{
        return m_pMixer->ProcessOutput(0 , 1, pOutputSamples, pdwStatus);
    }__except(EXCEPTION_EXECUTE_HANDLER){
        //SVP_LogMsg6( "ProcessOutputSafe Crash ");
        return E_FAIL;
    }
}
bool CEVRAllocatorPresenter::GetSampleFromMixer()
{
	MFT_OUTPUT_DATA_BUFFER Buffer;
	HRESULT hr = S_OK;
	DWORD dwStatus;
	UINT dwSurface;
	bool newSample = false;

	while (hr == S_OK) // Get as many frames as there are and that we have samples for
    {
        //SVP_LogMsg6( "GetFreeSample Free Samples Before %d ",  m_FreeSamples.GetCount());
		CComPtr<IMFSample> pSample;
		if (FAILED(GetFreeSample(&pSample))) // All samples are taken for the moment. Better luck next time
		{
            //SVP_LogMsg6( "GetFreeSample Failed %d %x", m_FreeSamples.GetCount(), hr);
			hr = E_FAIL;
			break;
		}
		else
		{
			memset(&Buffer, 0, sizeof(Buffer));
			Buffer.pSample = pSample;
			pSample->GetUINT32(GUID_SURFACE_INDEX, &dwSurface);
            
            //__try{
			    hr = ProcessOutputSafe(0 , 1, &Buffer, &dwStatus);
            /*}__except(EXCEPTION_EXECUTE_HANDLER){
                hr = E_FAIL;
                break;
            }*/

			if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT) // There are no samples left in the mixer
			{
                //SVP_LogMsg5(L"MF_E_TRANSFORM_NEED_MORE_INPUT");
				MoveToFreeList(pSample, false);
				break;
            }else if (hr == MF_E_TRANSFORM_TYPE_NOT_SET){
                MoveToFreeList(pSample, true);
                //newSample = false;
                //FlushSamples();
                //hr = RenegotiateMediaType();


                //SVP_LogMsg5(L"MF_E_TRANSFORM_TYPE_NOT_SET %x", hr);

                break;
            }
			else if(SUCCEEDED(hr))
			{
				newSample = true;
				if (AfxGetMyApp()->m_fTearingTest)
				{
					RECT rcTearing;

					rcTearing.left = m_nTearingPos;
					rcTearing.top = 0;
					rcTearing.right	= rcTearing.left + 4;
					rcTearing.bottom = m_NativeVideoSize.cy;
					m_pD3DDev->ColorFill(m_pVideoSurface[dwSurface], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));

					rcTearing.left = (rcTearing.right + 15) % m_NativeVideoSize.cx;
					rcTearing.right	= rcTearing.left + 4;
					m_pD3DDev->ColorFill(m_pVideoSurface[dwSurface], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));
					m_nTearingPos = (m_nTearingPos + 7) % m_NativeVideoSize.cx;
				}	
                
				MoveToScheduledList(pSample, false); // Schedule, then go back to see if there is more where that came from
            }else{
                //SVP_LogMsg5(L"ProcessOutputSafe Failed %x", hr);
            }
		}
	}

	return newSample;
}

STDMETHODIMP CEVRAllocatorPresenter::GetCurrentMediaType(__deref_out  IMFVideoMediaType **ppMediaType)
{
    HRESULT hr = S_OK;
    CAutoLock lock(this);

    CheckPointer (ppMediaType, E_POINTER);
    CheckHR (CheckShutdown());

    if (m_pMediaType == NULL)
        CheckHR(MF_E_NOT_INITIALIZED);

    CheckHR(m_pMediaType->QueryInterface( __uuidof(IMFVideoMediaType), (void**)&ppMediaType));

    return hr;
}

// IMFTopologyServiceLookupClient        
STDMETHODIMP CEVRAllocatorPresenter::InitServicePointers(/* [in] */ __in  IMFTopologyServiceLookup *pLookup)
{
	HRESULT						hr;
	DWORD						dwObjects = 1;
    m_llSystemJitter = 0;
    m_systemTime = 0;
	hr = pLookup->LookupService(MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_MIXER_SERVICE, __uuidof (IMFTransform), (void**)&m_pMixer, &dwObjects);
	hr = pLookup->LookupService(MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_RENDER_SERVICE, __uuidof (IMediaEventSink ), (void**)&m_pSink, &dwObjects);
	hr = pLookup->LookupService(MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_RENDER_SERVICE, __uuidof (IMFClock ), (void**)&m_pClock, &dwObjects);
    //SVP_LogMsg5(L"CEVRAllocatorPresenter::InitServicePointers");
	StartWorkerThreads();
	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::ReleaseServicePointers()
{
	StopWorkerThreads();
	m_pMixer = NULL;
	m_pSink = NULL;
	m_pClock = NULL;
	return S_OK;
}

// IMFVideoDeviceID
STDMETHODIMP CEVRAllocatorPresenter::GetDeviceID( __out  IID *pDeviceID)
{
	CheckPointer(pDeviceID, E_POINTER);
	*pDeviceID = IID_IDirect3DDevice9;
	return S_OK;
}

// IMFGetService
STDMETHODIMP CEVRAllocatorPresenter::GetService( __RPC__in REFGUID guidService, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID *ppvObject)
{
	if (guidService == MR_VIDEO_RENDER_SERVICE)
		return NonDelegatingQueryInterface (riid, ppvObject);
	else if (guidService == MR_VIDEO_ACCELERATION_SERVICE)
		return m_pD3DManager->QueryInterface (__uuidof(IDirect3DDeviceManager9), (void**) ppvObject);

	return E_NOINTERFACE;
}


// IMFAsyncCallback
STDMETHODIMP CEVRAllocatorPresenter::GetParameters( __RPC__out DWORD *pdwFlags, __RPC__out DWORD *pdwQueue)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEVRAllocatorPresenter::Invoke( __RPC__in_opt IMFAsyncResult *pAsyncResult)
{
	return E_NOTIMPL;
}

// IMFVideoDisplayControl
STDMETHODIMP CEVRAllocatorPresenter::GetNativeVideoSize(SIZE *pszVideo, SIZE *pszARVideo)
{
	if (pszVideo)
	{
		pszVideo->cx	= m_NativeVideoSize.cx;
		pszVideo->cy	= m_NativeVideoSize.cy;
	}
	if (pszARVideo)
	{
		pszARVideo->cx	= m_AspectRatio.cx; //m_NativeVideoSize.cx *
		pszARVideo->cy	= m_AspectRatio.cy; //m_NativeVideoSize.cy *
	}
	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::GetIdealVideoSize(SIZE *pszMin, SIZE *pszMax)
{
	if (pszMin)
	{
		pszMin->cx	= 1;
		pszMin->cy	= 1;
	}

	if (pszMax)
	{
		D3DDISPLAYMODE	d3ddm;

		ZeroMemory(&d3ddm, sizeof(d3ddm));
		if(SUCCEEDED(m_pD3D->GetAdapterDisplayMode(GetAdapter(m_pD3D), &d3ddm)))
		{
			pszMax->cx	= d3ddm.Width;
			pszMax->cy	= d3ddm.Height;
		}
	}

	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::SetVideoPosition(const MFVideoNormalizedRect *pnrcSource, const LPRECT prcDest)
{
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::GetVideoPosition(MFVideoNormalizedRect *pnrcSource, LPRECT prcDest)
{
	// Always all source rectangle ?
	if (pnrcSource)
	{
		pnrcSource->left	= 0.0;
		pnrcSource->top		= 0.0;
		pnrcSource->right	= 1.0;
		pnrcSource->bottom	= 1.0;
	}

	if (prcDest)
		memcpy (prcDest, &m_VideoRect, sizeof(m_VideoRect));//GetClientRect (m_hWnd, prcDest);

	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::SetAspectRatioMode(DWORD dwAspectRatioMode)
{
	m_dwVideoAspectRatioMode = (MFVideoAspectRatioMode)dwAspectRatioMode;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::GetAspectRatioMode(DWORD *pdwAspectRatioMode)
{
	CheckPointer (pdwAspectRatioMode, E_POINTER);
	*pdwAspectRatioMode = m_dwVideoAspectRatioMode;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::SetVideoWindow(HWND hwndVideo)
{
	ASSERT (m_hWnd == hwndVideo);	// What if not ??
//	m_hWnd = hwndVideo;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::GetVideoWindow(HWND *phwndVideo)
{
	CheckPointer (phwndVideo, E_POINTER);
	*phwndVideo = m_hWnd;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::RepaintVideo()
{
	Paint (true);
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::GetCurrentImage(BITMAPINFOHEADER *pBih, BYTE **pDib, DWORD *pcbDib, LONGLONG *pTimeStamp)
{
	ASSERT (FALSE);
	return E_NOTIMPL;
}
STDMETHODIMP CEVRAllocatorPresenter::SetBorderColor(COLORREF Clr)
{
	m_BorderColor = Clr;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::GetBorderColor(COLORREF *pClr)
{
	CheckPointer (pClr, E_POINTER);
	*pClr = m_BorderColor;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::SetRenderingPrefs(DWORD dwRenderFlags)
{
	m_dwVideoRenderPrefs = (MFVideoRenderPrefs)dwRenderFlags;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::GetRenderingPrefs(DWORD *pdwRenderFlags)
{
	CheckPointer(pdwRenderFlags, E_POINTER);
	*pdwRenderFlags = m_dwVideoRenderPrefs;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::SetFullscreen(BOOL fFullscreen)
{
	ASSERT (FALSE);
	return E_NOTIMPL;
}
STDMETHODIMP CEVRAllocatorPresenter::GetFullscreen(BOOL *pfFullscreen)
{
	ASSERT (FALSE);
	return E_NOTIMPL;
}

// IEVRTrustedVideoPlugin
STDMETHODIMP CEVRAllocatorPresenter::IsInTrustedVideoMode(BOOL *pYes)
{
	CheckPointer(pYes, E_POINTER);
	*pYes = TRUE;
	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::CanConstrict(BOOL *pYes)
{
	CheckPointer(pYes, E_POINTER);
	*pYes = TRUE;
	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::SetConstriction(DWORD dwKPix)
{
	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::DisableImageExport(BOOL bDisable)
{
	return S_OK;
}

// IDirect3DDeviceManager9
STDMETHODIMP CEVRAllocatorPresenter::ResetDevice(IDirect3DDevice9 *pDevice,UINT resetToken)
{
	HRESULT		hr = m_pD3DManager->ResetDevice (pDevice, resetToken);
	return hr;
}

STDMETHODIMP CEVRAllocatorPresenter::OpenDeviceHandle(HANDLE *phDevice)
{
	HRESULT		hr = m_pD3DManager->OpenDeviceHandle (phDevice);
	return hr;
}

STDMETHODIMP CEVRAllocatorPresenter::CloseDeviceHandle(HANDLE hDevice)
{
	HRESULT		hr = m_pD3DManager->CloseDeviceHandle(hDevice);
	return hr;
}

STDMETHODIMP CEVRAllocatorPresenter::TestDevice(HANDLE hDevice)
{
	HRESULT		hr = m_pD3DManager->TestDevice(hDevice);
	return hr;
}

STDMETHODIMP CEVRAllocatorPresenter::LockDevice(HANDLE hDevice, IDirect3DDevice9 **ppDevice, BOOL fBlock)
{
	HRESULT		hr = m_pD3DManager->LockDevice(hDevice, ppDevice, fBlock);
	return hr;
}

STDMETHODIMP CEVRAllocatorPresenter::UnlockDevice(HANDLE hDevice, BOOL fSaveState)
{
	HRESULT		hr = m_pD3DManager->UnlockDevice(hDevice, fSaveState);
	return hr;
}

STDMETHODIMP CEVRAllocatorPresenter::GetVideoService(HANDLE hDevice, REFIID riid, void **ppService)
{
	HRESULT		hr = m_pD3DManager->GetVideoService(hDevice, riid, ppService);

	if (riid == __uuidof(IDirectXVideoDecoderService))
	{
		UINT		nNbDecoder = 5;
		GUID*		pDecoderGuid;
		IDirectXVideoDecoderService*		pDXVAVideoDecoder = (IDirectXVideoDecoderService*) *ppService;
		pDXVAVideoDecoder->GetDecoderDeviceGuids (&nNbDecoder, &pDecoderGuid);
	}
	else if (riid == __uuidof(IDirectXVideoProcessorService))
	{
		IDirectXVideoProcessorService*		pDXVAProcessor = (IDirectXVideoProcessorService*) *ppService;
	}

	return hr;
}

STDMETHODIMP CEVRAllocatorPresenter::GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
{
	// This function should be called...
	ASSERT (FALSE);

	if(lpWidth)		*lpWidth	= m_NativeVideoSize.cx;
	if(lpHeight)	*lpHeight	= m_NativeVideoSize.cy;
	if(lpARWidth)	*lpARWidth	= m_AspectRatio.cx;
	if(lpARHeight)	*lpARHeight	= m_AspectRatio.cy;
	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::InitializeDevice(AM_MEDIA_TYPE* pMediaType)
{
	HRESULT hr;
	CAutoLock lock(this);
	CAutoLock lock2(&m_ImageProcessingLock);
	CAutoLock cRenderLock(&m_RenderLock);

	RemoveAllSamples();
	DeleteSurfaces();

	VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*) pMediaType->pbFormat;
	int w = vih2->bmiHeader.biWidth;
	int h = abs(vih2->bmiHeader.biHeight);

	m_NativeVideoSize = CSize(w, h);
	if (m_bHighColorResolution)
		hr = AllocSurfaces(D3DFMT_A2R10G10B10);
	else
		hr = AllocSurfaces(D3DFMT_X8R8G8B8);
	
	for(int i = 0; i < m_nDXSurface; i++)
	{
		CComPtr<IMFSample> pMFSample;
		hr = pfMFCreateVideoSampleFromSurface(m_pVideoSurface[i], &pMFSample);
		if (SUCCEEDED (hr))
		{
			pMFSample->SetUINT32(GUID_SURFACE_INDEX, i);
			m_FreeSamples.AddTail (pMFSample);
		}
		ASSERT (SUCCEEDED (hr));
	}


	return hr;
}

DWORD WINAPI CEVRAllocatorPresenter::MixerThreadStatic(LPVOID lpParam)
{
	CEVRAllocatorPresenter *pThis = (CEVRAllocatorPresenter*) lpParam;
	pThis->MixerThread();
	return 0;
}

void CEVRAllocatorPresenter::MixerThread()
{
	HANDLE hAvrt;
	HANDLE hEvts[] = {m_hEvtQuit,m_hEvtSampleNotify};
	bool bQuit = false;
    TIMECAPS tc;
	DWORD dwResolution;
	DWORD dwUser = 0;
	DWORD dwTaskIndex = 0;

    timeGetDevCaps(&tc, sizeof(TIMECAPS));
    dwResolution = min(max(tc.wPeriodMin, 0), tc.wPeriodMax);
    dwUser = timeBeginPeriod(dwResolution);

	while (!bQuit)
	{
		DWORD dwObject = WaitForMultipleObjects (countof(hEvts), hEvts, FALSE, 1);
		switch (dwObject)
		{
		case WAIT_OBJECT_0 :
			bQuit = true;
            m_SampleNotified = true;
//			break;
        case WAIT_OBJECT_0 + 1:
            ResetEvent(m_hEvtSampleNotify);
             //SVP_LogMsg6(" MixerThread ResetEvent(m_hEvtSampleNotify); %d",m_SampleNotified);
        case WAIT_TIMEOUT :
            bool bNewSample = false;
        //   
            if(m_SampleNotified )
			{
				
				{
					CAutoLock lock(&m_ImageProcessingLock);
                    CAutoLock lock2(&m_SampleQueueLock);
					bNewSample = GetSampleFromMixer();

                     //SVP_LogMsg6(" MixerThread GetSampleFromMixer %x %d", bNewSample, m_FreeSamples.GetCount());
				}

                if (m_rtFrameCycle == 0 && bNewSample) // Get frame time and type from the input pin				
                {
                    CComPtr<IPin> pPin;
                    CMediaType mt;
                    if (
                        SUCCEEDED(m_pOuterEVR->FindPin(L"EVR Input0", &pPin)) &&
                        SUCCEEDED(pPin->ConnectionMediaType(&mt)))
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

                        //SVP_LogMsg5(L"m_bInterlaced %d",m_bInterlaced);
                    }
                    // Update internal subtitle clock
                    if(m_bUseInternalTimer && m_pSubPicQueue)
                    {
                        m_pSubPicQueue->SetFPS(m_fps);
                    }

                    if(m_bUseInternalTimer && m_pSubPicQueue2)
                    {
                        m_pSubPicQueue2->SetFPS(m_fps);
                    }

                }

            }else{
                 //SVP_LogMsg6(" (m_hEvtSampleNotify Skip);");
            }
				
			break;
		}
	}
	timeEndPeriod (dwResolution);
}

DWORD WINAPI CEVRAllocatorPresenter::RenderThreadStatic(LPVOID lpParam)
{
	CEVRAllocatorPresenter *pThis = (CEVRAllocatorPresenter*)lpParam;
	pThis->RenderThread();
	return 0;
}

// Get samples that have been received and queued up by MixerThread() and present them at the correct time by calling Paint().
void CEVRAllocatorPresenter::RenderThread()
{
	HANDLE hAvrt;
	DWORD dwTaskIndex = 0;
	HANDLE hEvts[] = {m_hEvtQuit, m_hEvtFlush};
	bool bQuit = false;
	TIMECAPS tc;
	DWORD dwResolution;
	LONGLONG llRefClockTime;
	double targetSyncOffset;
	MFTIME systemTime;
	DWORD dwUser = 0;
    LONG orgNextSampleWait = 0;
	DWORD dwObject;
	int samplesLeft = 0;
	CComPtr<IMFSample>pNewSample = NULL; // The sample next in line to be presented

	// Tell Vista Multimedia Class Scheduler we are doing threaded playback (increase priority)
	if (pfAvSetMmThreadCharacteristicsW) hAvrt = pfAvSetMmThreadCharacteristicsW (L"Playback", &dwTaskIndex);
	if (pfAvSetMmThreadPriority) pfAvSetMmThreadPriority (hAvrt, AVRT_PRIORITY_HIGH);

	AppSettings& s = AfxGetAppSettings();
	m_pGenlock->GetTargetSyncOffset(&targetSyncOffset); // Target sync offset from settings

	// Set timer resolution
	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	dwResolution = min(max(tc.wPeriodMin, 0), tc.wPeriodMax);
	dwUser = timeBeginPeriod(dwResolution);

	while (!bQuit)
	{
		pNewSample = NULL;
		m_lNextSampleWait = 1;
		samplesLeft = 0;

		if (m_nRenderState == Started || !m_bPrerolled) // If either streaming or the pre-roll sample
		{
            
			while (1)//SUCCEEDED(GetScheduledSample(&pNewSample, samplesLeft))
			{
                BOOL bGetInTimeProc = false;
                BOOL bBreakTheSampleQueueLoop = false;
                int lJustDroped = 0;
                {
                    CAutoLock lock(&m_SampleQueueLock);
                
                    while (1){
                        // Get the next sample
                        
                        
                        samplesLeft = m_ScheduledSamples.GetCount();
                        if (samplesLeft > 0)
                        {
                            pNewSample = m_ScheduledSamples.RemoveHead().Detach();
                            --samplesLeft;
                        }
                        else{
                            bBreakTheSampleQueueLoop = true;
                            break;
                        }

                        m_llLastSampleTime = m_llSampleTime;
                        if (!m_bPrerolled)
                        {
                            m_bPrerolled = true; // m_bPrerolled is a ticket to show one (1) frame and no more until streaming
                            m_lNextSampleWait = 0; // Present immediately
                        }
                        else if (SUCCEEDED(pNewSample->GetSampleTime(&m_llSampleTime)))
                        {
                            llRefClockTime = 0;
                            systemTime = 0;

                            HRESULT hrc =  m_pClock->GetCorrelatedTime(0, &llRefClockTime, &systemTime); // Get zero-based reference clock time. systemTime is not used for anything here

                           // SVP_LogMsg5(L"ORG   (LONG)((m_llSampleTime - llRefClockTime) / 10000) %d %d %f %f %d %d %x" ,  (LONG)(m_llSampleTime/10000), (llRefClockTime < 0), double(llRefClockTime), double(systemTime), (LONG)(llRefClockTime/10000), (LONG)((m_llSampleTime - llRefClockTime) / 10000), hrc);
                           /* if(m_systemTime != 0 && _abs64(m_systemTime - systemTime) > 100000000 ){ //FAILED(hrc) ||  sometime GetCorrelatedTime just fail :( ?
                               //retry will not help
                               //m_systemTime maybe changed
                                m_llSystemJitter = (m_systemTime - systemTime);
                            }
                            llRefClockTime += m_llSystemJitter;
                           */
                            if(llRefClockTime < -10000000i64){
                                SVP_LogMsg5(L"ORG   (LONG)((m_llSampleTime - llRefClockTime) / 10000) %d %d %f %f %d %d %x" ,  (LONG)(m_llSampleTime/10000), (llRefClockTime < 0), double(llRefClockTime), double(systemTime), (LONG)(llRefClockTime/10000), (LONG)((m_llSampleTime - llRefClockTime) / 10000), hrc);
                                m_bPendingResetDevice = true;
                                llRefClockTime = m_llSampleTime;
                                SVP_LogMsg5(L"Pending reset");
                            }
                            
                            //LONGLONG llDur;
                            //pNewSample->GetSampleDuration(&llDur);
                            //Drop if sample is pasted
                            if( samplesLeft > 0 && lJustDroped < 10 && m_llSampleTime < llRefClockTime - 2000000)
                            {
                                //SVP_LogMsg5(L"GetSampleTime and Drop %f %f %d %x", double(m_llSampleTime), double(llRefClockTime), (LONG)((m_llSampleTime - llRefClockTime) / 10000), m_pClock);

                                //MoveToFreeList(pNewSample, true)
                                {
                                    //CAutoLock lock(&m_SampleQueueLock);
                                     InterlockedDecrement(&m_nUsedBuffer);
                                     m_FreeSamples.AddTail(pNewSample);
                                    pNewSample = NULL;
                                    m_lNextSampleWait = 1;
                                    samplesLeft = 0;
                                    m_pcFramesDropped++;
                                    m_nStepCount = 0;
                                    lJustDroped++;
                                }
                                //SVP_LogMsg5(L"MoveToFreeList Done");
                                continue;

                            }
                            if(m_llSampleTime > llRefClockTime)
                                m_lNextSampleWait = (LONG)((m_llSampleTime - llRefClockTime) / 10000); // Time left until sample is due, in ms
                            else 
                                m_lNextSampleWait = 0;
                            //if(m_lNextSampleWait > 100000)
                            //    m_lNextSampleWait = 0;

                            orgNextSampleWait = m_lNextSampleWait;
                            bGetInTimeProc = true;
                        }
                        break;
                    }
                }
                if(bBreakTheSampleQueueLoop){
                    break;
                }
				 // Get zero-based sample due time
				if(bGetInTimeProc){
					
                    {
					    
                        ////SVP_LogMsg6("m_lNextSampleWait %d %f %f", m_lNextSampleWait, double(m_llSampleTime), double(llRefClockTime) );
					    if (m_lNextSampleWait < 0)
						    m_lNextSampleWait = 0; // We came too late. Race through, discard the sample and get a new one
					    else if (s.fVMRGothSyncFix && s.m_RenderSettings.bSynchronizeNearest ) // Present at the closest "safe" occasion at tergetSyncOffset ms before vsync to avoid tearing
					    {
						    LONG lOldNextSampleWait = m_lNextSampleWait;
						    while(m_lOverWaitCounter < 20){
							    if (m_rtFrameCycle > 0.0){
								    if(targetSyncOffset*15000 > m_rtFrameCycle){
									    targetSyncOffset = (double)m_rtFrameCycle /15000;
									    m_pGenlock->SetTargetSyncOffset(targetSyncOffset);
									    m_lOverWaitCounter = 99;
									    break;
								    }
							    }

							    REFERENCE_TIME rtRefClockTimeNow; if (m_pRefClock) m_pRefClock->GetTime(&rtRefClockTimeNow); // Reference clock time now
							    ////SVP_LogMsg3(" EVR %f %f %f %f ", double(m_llLastSampleTime ) , double(rtRefClockTimeNow),  double(m_llSampleTime) , double(llRefClockTime));

							    LONG lLastVsyncTime = (LONG)((m_rtEstVSyncTime - rtRefClockTimeNow) / 10000); // Time of previous vsync relative to now

							    LONGLONG llNextSampleWait = (LONGLONG)(((double)lLastVsyncTime + GetDisplayCycle() - targetSyncOffset) * 10000); // Next safe time to Paint()
							    // 						while ((llRefClockTime + llNextSampleWait) < (m_llSampleTime + m_llHysteresis)) // While the proposed time is in the past of sample presentation time
							    // 						{
							    // 							llNextSampleWait = llNextSampleWait + (LONGLONG)(GetDisplayCycle() * 10000); // Try the next possible time, one display cycle ahead
							    // 						}
							    //By Tomasen: The while loop seems un-nesssery
							    LONGLONG llEachStep = (GetDisplayCycle() * 10000); // While the proposed time is in the past of sample presentation time
							    if(llEachStep){
								    LONGLONG llHowManyStepWeNeed = ((m_llSampleTime + m_llHysteresis) - (llRefClockTime + llNextSampleWait)) / llEachStep;   // Try the next possible time, one display cycle ahead
								    llNextSampleWait += llEachStep * llHowManyStepWeNeed;
							    }else
								    llNextSampleWait = 10000;
							    m_lNextSampleWait = (LONG)(llNextSampleWait / 10000);
							    m_lShiftToNearestPrev = m_lShiftToNearest;
							    m_lShiftToNearest = (LONG)((llRefClockTime + llNextSampleWait - m_llSampleTime) / 10000); // The adjustment made to get to the sweet point in time, in ms

							    if (m_bSnapToVSync)
							    {
								    LONG lDisplayCycle2 = (LONG)(GetDisplayCycle() / 2.0); // These are a couple of empirically determined constants used the control the "snap" function
								    LONG lDisplayCycle3 = (LONG)(GetDisplayCycle() / 3.0);
								    if ((m_lShiftToNearestPrev - m_lShiftToNearest) > lDisplayCycle2) // If a step down in the m_lShiftToNearest function. Display slower than video. 
								    {
									    m_bVideoSlowerThanDisplay = false;
									    m_llHysteresis = -(LONGLONG)(10000 * lDisplayCycle3);
								    }
								    else if ((m_lShiftToNearest - m_lShiftToNearestPrev) > lDisplayCycle2) // If a step up
								    {
									    m_bVideoSlowerThanDisplay = true;
									    m_llHysteresis = (LONGLONG)(10000 * lDisplayCycle3);
								    }
								    else if ((m_lShiftToNearest < (2 * lDisplayCycle3)) && (m_lShiftToNearest > lDisplayCycle3))
									    m_llHysteresis = 0; // Reset when between 1/3 and 2/3 of the way either way
							    }
							    break;
						    }
						    //if( (m_lNextSampleWait - lOldNextSampleWait) > llEachStep) {
						    //	m_lNextSampleWait = lOldNextSampleWait + llEachStep;
						    //}
					    }
                         ////SVP_LogMsg6("m_lNextSampleWait %d Hhhhhhhhhhhh", m_lNextSampleWait);
					    if((m_lNextSampleWait-orgNextSampleWait) > 200){
						    m_lNextSampleWait = orgNextSampleWait + 200;
						    m_lOverWaitCounter++;
					    }else if(m_lOverWaitCounter != 99)
						    m_lOverWaitCounter = 0;
    					
					    if (m_lNextSampleWait < 0)
						    m_lNextSampleWait = 0;
				    }
                }
				break;
			}
			
		}
       
		// Wait for the next presentation time or a quit or flush event
		dwObject = WaitForMultipleObjects(countof(hEvts), hEvts, FALSE, (DWORD)m_lNextSampleWait); 
		switch (dwObject)
		{
		case WAIT_OBJECT_0: // Quit event
            //SVP_LogMsg5(L"WaitForMultipleObjects Quit event");
			bQuit = true;
			break;

		case WAIT_OBJECT_0 + 1: // Flush event
            //SVP_LogMsg5(L"WaitForMultipleObjects Flush event");
			FlushSamples();
			m_bEvtFlush = false;
			ResetEvent(m_hEvtFlush);
			m_bPrerolled = false;;
            
			break;

		case WAIT_TIMEOUT: // Time to show the sample or something
             //SVP_LogMsg5(L"WaitForMultipleObjects Render event");
			if (m_LastSetOutputRange != -1 && m_LastSetOutputRange != s.m_RenderSettings.iEVROutputRange || m_bPendingRenegotiate)
			{
				FlushSamples();
				RenegotiateMediaType();
				m_bPendingRenegotiate = false;
                //SVP_LogMsg5(L"WaitForMultipleObjects FlushSamples end");
			}

			if (m_bPendingResetDevice)
			{
                 //SVP_LogMsg5(L"WaitForMultipleObjects m_bPendingResetDevice");
				m_bPendingResetDevice = false;
				CAutoLock lock(this);
				CAutoLock lock2(&m_ImageProcessingLock);
				CAutoLock cRenderLock(&m_RenderLock);
				if (pNewSample) MoveToFreeList(pNewSample, true);
				pNewSample = NULL;
				RemoveAllSamples();
				CDX9AllocatorPresenter::ResetDevice();

				for(int i = 0; i < m_nDXSurface; i++)
				{
					CComPtr<IMFSample> pMFSample;
					HRESULT hr = pfMFCreateVideoSampleFromSurface (m_pVideoSurface[i], &pMFSample);
					if (SUCCEEDED (hr))
					{
						pMFSample->SetUINT32(GUID_SURFACE_INDEX, i);
						m_FreeSamples.AddTail(pMFSample);
					}
					ASSERT(SUCCEEDED (hr));
				}
                //SVP_LogMsg5(L"WaitForMultipleObjects m_bPendingResetDevice end");
			}
			else if (m_nStepCount < 0)
			{
				m_nStepCount = 0;
				m_pcFramesDropped++;
                //SVP_LogMsg5(L"WaitForMultipleObjects m_pcFramesDropped++ end");
			}
			else if ((m_nStepCount > 0) && pNewSample)
			{
                
				pNewSample->GetUINT32(GUID_SURFACE_INDEX, (UINT32 *)&m_nCurSurface);
                //SVP_LogMsg5(L"WaitForMultipleObjects Paint %d", m_nCurSurface);
				if (!g_bExternalSubtitleTime) __super::SetTime (g_tSegmentStart + m_llSampleTime);
				Paint(true);
				CompleteFrameStep(false);

                //SVP_LogMsg5(L"WaitForMultipleObjects Paint end");
			}
			else if (pNewSample)
			{
                pNewSample->GetUINT32(GUID_SURFACE_INDEX, (UINT32*)&m_nCurSurface);
				if (!g_bExternalSubtitleTime) __super::SetTime (g_tSegmentStart + m_llSampleTime);
                //SVP_LogMsg5(L"WaitForMultipleObjects Paint2 %d ",m_nCurSurface );
                Paint(true);
				//SVP_LogMsg5(L"WaitForMultipleObjects Paint2 end");
                 m_pcFramesDrawn++;
                
			}
            //SVP_LogMsg5(L"WaitForMultipleObjects Reander end");
			break;
		} // switch
        if (pNewSample) {
            MoveToFreeList(pNewSample, true);
            //SVP_LogMsg6("Sample Freed %d", m_FreeSamples.GetCount());
        }
		pNewSample = NULL;
        ////SVP_LogMsg5(L"WaitForMultipleObjects while event");
	} // while
	timeEndPeriod (dwResolution);
	if (pfAvRevertMmThreadCharacteristics) pfAvRevertMmThreadCharacteristics(hAvrt);

     //SVP_LogMsg5(L"Render Loop Out");
}

void CEVRAllocatorPresenter::OnResetDevice()
{
	HRESULT hr;
	hr = m_pD3DManager->ResetDevice(m_pD3DDev, m_nResetToken);
	if (m_pSink) m_pSink->Notify(EC_DISPLAY_CHANGED, 0, 0);
	
}

void CEVRAllocatorPresenter::RemoveAllSamples()
{
	CAutoLock AutoLock(&m_ImageProcessingLock);
	FlushSamples();
	m_ScheduledSamples.RemoveAll();
	m_FreeSamples.RemoveAll();
	ASSERT(m_nUsedBuffer == 0);
	m_nUsedBuffer = 0;
}

HRESULT CEVRAllocatorPresenter::GetFreeSample(IMFSample** ppSample)
{
	CAutoLock lock(&m_SampleQueueLock);
	HRESULT		hr = S_OK;

	if (m_FreeSamples.GetCount() > 1)	// Cannot use first free buffer (can be currently displayed)
	{
		InterlockedIncrement(&m_nUsedBuffer);
		*ppSample = m_FreeSamples.RemoveHead().Detach();
	}
	else
		hr = MF_E_SAMPLEALLOCATOR_EMPTY;

	return hr;
}

HRESULT CEVRAllocatorPresenter::GetScheduledSample(IMFSample** ppSample, int &_Count)
{
	CAutoLock lock(&m_SampleQueueLock);
	HRESULT		hr = S_OK;

	_Count = m_ScheduledSamples.GetCount();
	if (_Count > 0)
	{
		*ppSample = m_ScheduledSamples.RemoveHead().Detach();
		--_Count;
	}
	else
		hr = MF_E_SAMPLEALLOCATOR_EMPTY;

	return hr;
}

void CEVRAllocatorPresenter::MoveToFreeList(IMFSample* pSample, bool bTail)
{
	CAutoLock lock(&m_SampleQueueLock);
	InterlockedDecrement(&m_nUsedBuffer);
	if (m_bPendingMediaFinished && m_nUsedBuffer == 0)
	{
		m_bPendingMediaFinished = false;
		m_pSink->Notify(EC_COMPLETE, 0, 0);
	}
	if (bTail)
		m_FreeSamples.AddTail(pSample);
	else
		m_FreeSamples.AddHead(pSample);
}

void CEVRAllocatorPresenter::MoveToScheduledList(IMFSample* pSample, bool _bSorted)
{
	if (_bSorted)
	{
		CAutoLock lock(&m_SampleQueueLock);
		m_ScheduledSamples.AddHead(pSample);
	}
	else
	{
		CAutoLock lock(&m_SampleQueueLock);
		m_ScheduledSamples.AddTail(pSample);
	}
}

void CEVRAllocatorPresenter::FlushSamples()
{
	CAutoLock lock(this);
	CAutoLock lock2(&m_SampleQueueLock);
   
    
	
	FlushSamplesInternal();
}

void CEVRAllocatorPresenter::FlushSamplesInternal()
{
	m_bPrerolled = false;
	while (m_ScheduledSamples.GetCount() > 0)
	{
		CComPtr<IMFSample> pMFSample;
		pMFSample = m_ScheduledSamples.RemoveHead();
		MoveToFreeList(pMFSample, true);
		//SVP_LogMsg5(_T("--- m_ScheduledSamples.GetCount: %d\n"), m_ScheduledSamples.GetCount());
	}
}
void CEVRAllocatorPresenter::ThreadBeginStreaming(){
	////SVP_LogMsg5(L"CEVRAllocatorPresenter::BeginStreamingThread");
	CAutoLock threadLock(&m_csTread);
	AppSettings& s = AfxGetAppSettings();	
	
	m_pGenlock->SetMonitor(GetAdapter(m_pD3D));
	m_pGenlock->GetTiming();

	ResetStats();
	if(m_dDetectedScanlineTime <= 0.0){
		EstimateRefreshTimings();
	}
	if (m_rtFrameCycle > 0.0) m_dCycleDifference = GetCycleDifference(); // Might have moved to another display
}

UINT __cdecl ThreadEVRAllocatorPresenterStartPresenting( LPVOID lpParam ) 
{ 
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if(pFrame){
		CAutoLock mOpenCloseLock(&pFrame->m_csOpenClose);
		CEVRAllocatorPresenter* pEVRA = (CEVRAllocatorPresenter*)lpParam;
		pEVRA->ThreadBeginStreaming();
	}
	return 0; 
}
// Called when streaming begins.
HRESULT CEVRAllocatorPresenter::BeginStreaming()
{
	m_pcFramesDropped = 0;
	m_pcFramesDrawn = 0;

    //SVP_LogMsg5(L"CEVRAllocatorPresenter::BeginStreaming1");
	
	AppSettings& s = AfxGetAppSettings();
	
	if (s.m_RenderSettings.bSynchronizeVideo)
		m_pGenlock->AdviseSyncClock(((CMainFrame*)(AfxGetApp()->m_pMainWnd))->m_pSyncClock);
	CComPtr<IBaseFilter> pEVR;
	FILTER_INFO filterInfo;
	ZeroMemory(&filterInfo, sizeof(filterInfo));
	m_pOuterEVR->QueryInterface (__uuidof(IBaseFilter), (void**)&pEVR);
	pEVR->QueryFilterInfo(&filterInfo); // This addref's the pGraph member

	BeginEnumFilters(filterInfo.pGraph, pEF, pBF)
		if(CComQIPtr<IAMAudioRendererStats> pAS = pBF)
		{
			m_pAudioStats = pAS;
		};
	EndEnumFilters
	pEVR->GetSyncSource(&m_pRefClock);
	if (filterInfo.pGraph) filterInfo.pGraph->Release();
	////SVP_LogMsg5(L"CEVRAllocatorPresenter::BeginStreaming2");
	if(m_dDetectedScanlineTime <= 0.0){
		m_VSyncDetectThread = AfxBeginThread(ThreadEVRAllocatorPresenterStartPresenting, (LPVOID)this, THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
			
		m_VSyncDetectThread->m_pMainWnd = AfxGetMainWnd();
		m_VSyncDetectThread->ResumeThread();
	}

	return S_OK;
}
