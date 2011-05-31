#pragma once
// Graph Core Controller
#include "IGraphBuilder2.h"
#include "RealMediaGraph.h"
#include "QuicktimeGraph.h"
#include "ShockwaveGraph.h"

#include "../../../include/IChapterInfo.h"
#include "../../../include/IKeyFrameInfo.h"
#include "../../../include/IBufferInfo.h"
#include <D3d9.h>
#include <Vmr9.h>
#include <evr.h>
#include <evr9.h>

#include "textpassthrufilter.h"

#include "../../filters/misc/SyncClock/Interfaces.h"
#include "../../filters/transform/svpfilter/SVPSubFilter.h"

#include "../../DSUtil/DSMPropertyBag.h"

#include "GraphThread.h"

enum {PM_NONE, PM_FILE, PM_DVD, PM_CAPTURE};

class OpenMediaData
{
public:
  //	OpenMediaData() {}
  virtual ~OpenMediaData() {} // one virtual funct is needed to enable rtti
  CString title;
  CAtlList<CString> subs;
};

class OpenFileData : public OpenMediaData 
{
public:
  OpenFileData() : rtStart(0) {}
  CAtlList<CString> fns; 
  REFERENCE_TIME rtStart;
};

class OpenDVDData : public OpenMediaData 
{
public: 
  //	OpenDVDData() {}
  CString path; 
  CComPtr<IDvdState> pDvdState;
};

class OpenDeviceData : public OpenMediaData
{
public: 
  OpenDeviceData() {vinput = vchannel = ainput = -1;}
  CStringW DisplayName[2];
  int vinput, vchannel, ainput;
};


class CMainFrame;
class CChildView;
class CPlayerCaptureBar;
class CPlayerPlaylistBar;
class CPlayerSeekBar;
class CPlayerToolBar;

class CGraphCore
{
public:
  CGraphCore(void);
  ~CGraphCore(void);

  /************************************************************************/
  // MainFrm

  CString m_fnCurPlayingFile;
  time_t  m_tPlayStartTime;
  bool m_fUpdateInfoBar;
  bool m_fOpeningAborted;
  std::wstring m_playingmsg, m_closingmsg;
  int m_iPlaybackMode;

  virtual CMainFrame* GetMainFrame() = 0;
  virtual CPlayerCaptureBar* GetCaptureBar() = 0;
  virtual CPlayerPlaylistBar* GetPlaylistBar() = 0;
  virtual CPlayerSeekBar* GetSeekBar() = 0;
  virtual CPlayerToolBar* GetToolBar() = 0;
  virtual CChildView* GetVideoView() = 0;

  virtual CString GetAnEasyToUnderstoodSubtitleName(CString szName) = 0;
  virtual void SendStatusMessage(CString msg, int nTimeOut, int iAlign = 0) = 0;
  virtual CString getCurPlayingSubfile(int * iSubDelayMS = NULL,int subid = 0) = 0;
  virtual void OnPlayStop() = 0;
  virtual void SVPSubDownloadByVPath(CString szVPath, CAtlList<CString>* szaStatMsgs = NULL) = 0;
  virtual void OsdMsg_SetShader(CString* appendmsg = NULL) = 0;
  virtual void OpenSetupWindowTitle(CString fn = _T("")) = 0;
  virtual void OnFavoritesAddReal( BOOL bRecent = FALSE , BOOL bForceDel = FALSE ) = 0;
  /************************************************************************/

  enum {MLS_CLOSED, MLS_LOADING, MLS_LOADED, MLS_CLOSING};

  // we need open / play pause / seek / close / vol control / sub control / audio and video switching where

  CGraphThread* m_pGraphThread;

  void CleanGraph();
  CSize GetVideoSize();
  OAFilterState GetMediaState();
  void AddTextPassThruFilter();
  BOOL SetVMR9ColorControl(float Brightness, float Contrast, float Hue,
                           float Saturation, BOOL silent = false);

  //  Playback Control  Functions and Variables
  void SetBalance(int balance);

  // Subtitles
  bool LoadSubtitle(CString fn, int sub_delay_ms = 0, BOOL bIsForPlayList = false);
  void UpdateSubtitle(bool fApplyDefStyle = true);
  void SetSubtitle(ISubStream* pSubStream, bool fApplyDefStyle = true, bool bShowOSD = true);
  void SetSubtitleDelay(int delay_ms);

  void UpdateSubtitle2(bool fApplyDefStyle = true);
  void SetSubtitle2(ISubStream* pSubStream, bool fApplyDefStyle = true, bool bShowOSD = true);
  void ReplaceSubtitle(ISubStream* pSubStreamOld, ISubStream* pSubStreamNew, int secondSub = 0);
  void InvalidateSubtitle(DWORD_PTR nSubtitleId = -1, REFERENCE_TIME rtInvalidate = -1);
  void ReloadSubtitle();

  void SetSubtitleDelay2(int delay_ms);

  int m_iSubtitleSel;
  int m_iSubtitleSel2;
  DWORD_PTR m_nSubtitleId;
  DWORD_PTR m_nSubtitleId2;

  // chapters (file mode)
  void SetupChapters();
  
  // shaders
  CAtlList<CString> m_shaderlabels;
  void SetShaders( BOOL silent = false);
  void UpdateShaders(CString label);

  // Operations
  void OpenMedia(CAutoPtr<OpenMediaData> pOMD);
  void CloseMedia();
  bool OpenMediaPrivate(CAutoPtr<OpenMediaData> pOMD);
  void CloseMediaPrivate();
  void OpenCreateGraphObject(OpenMediaData* pOMD);
  HRESULT OpenMMSUrlStream(CString szFn);
  void OpenFile(OpenFileData* pOFD);
  void OpenDVD(OpenDVDData* pODD);
  void OpenCustomizeGraph();
  void OpenCapture(OpenDeviceData* pODD);
  void OpenSetupVideo();
  void OpenSetupAudio();

  int m_iRedrawAfterCloseCounter;
  DVD_DOMAIN m_iDVDDomain;
  int m_iMediaLoadState;
  CAtlArray<REFERENCE_TIME> m_kfs;
  UINT m_iAudioChannelMaping;
  bool m_fAudioOnly;
  CStringW m_VidDispName, m_AudDispName;
  int m_iSpeedLevel;
  bool m_fEndOfStream;
  bool m_fLiveWM;
  CString m_lastUrl;
  REFERENCE_TIME m_rtDurationOverride;
  BOOL m_is_resume_from_last_exit_point;

  // Capturing
  bool m_fCapturing;
  // pBF: 0 buff, 1 enc, 2 mux, pmt is for 1 enc
  HRESULT BuildCapture(IPin* pPin, IBaseFilter* pBF[3],
                  const GUID& majortype, AM_MEDIA_TYPE* pmt); 
  bool BuildToCapturePreviewPin(IBaseFilter* pVidCap, IPin** pVidCapPin, IPin** pVidPrevPin, 
    IBaseFilter* pAudCap, IPin** pAudCapPin, IPin** pAudPrevPin);
  bool BuildGraphVideoAudio(int fVPreview, bool fVCapture, int fAPreview, bool fACapture);

  //  Snapshot
  void GetSnapShotSliently(const std::vector<std::wstring> &args);
  bool _skip_ui;
  bool GetDIB(BYTE** ppData, long& size, bool fSilent = false);
  void SaveDIB(LPCTSTR fn, BYTE* pData, long size);
  void SaveThumbnails(LPCTSTR fn);
  int m_VolumeBeforeFrameStepping;

  bool m_fCustomGraph;
  bool m_fRealMediaGraph, m_fShockwaveGraph, m_fQuicktimeGraph;

  // thread switch on opened or closed
  BOOL m_fOpenedThruThread;

  CInterfaceArray<IUnknown, &IID_IUnknown> m_pparray;
  CInterfaceArray<IAMStreamSelect> m_ssarray;

  CComQIPtr<IMediaControl> pMC;
  CComQIPtr<IMediaEventEx> pME;
  CComQIPtr<IVideoWindow> pVW;
  CComQIPtr<IBasicVideo> pBV;
  CComQIPtr<IBasicAudio> pBA;
  CComQIPtr<IMediaSeeking> pMS;
  CComQIPtr<IVideoFrameStep> pFS;
  CComQIPtr<IQualProp, &IID_IQualProp> pQP;
  CComQIPtr<IBufferInfo> pBI;
  CComQIPtr<IAMOpenProgress> pAMOP;

  CComQIPtr<IDvdControl2> pDVDC;
  CComQIPtr<IDvdInfo2> pDVDI;

  CComPtr<ICaptureGraphBuilder2> pCGB;
  CComPtr<IBaseFilter> pVidCap, pAudCap;
  CComPtr<IAMVideoCompression> pAMVCCap, pAMVCPrev;
  CComPtr<IAMStreamConfig> pAMVSCCap, pAMVSCPrev, pAMASC;
  CComPtr<IAMCrossbar> pAMXBar;
  CComPtr<IAMTVTuner> pAMTuner;
  CComPtr<IAMDroppedFrames> pAMDF;

  CComPtr<ISVPSubFilter> m_pSVPSub;
  CComPtr<ISubPicAllocatorPresenter> m_pCAP;
  CComPtr<ISubPicAllocatorPresenter2> m_pCAP2;
  CComPtr<ISubPicAllocatorPresenterRender> m_pCAPR ;

  CComPtr<IGraphBuilder2> pGB;

  // chapters (file mode)
  CComPtr<IDSMChapterBag> m_pCB;
  CComPtr<ISubClock> m_pSubClock;

  CComPtr<IUnknown> m_pProv;
  CComPtr<IBaseFilter> pAudioDubSrc;

  CComPtr<IVMRMixerControl9>	m_pMC;
  CComPtr<IMFVideoDisplayControl>	m_pMFVDC;

  // subtitle control
  CCritSec m_csSubLock;
  CCritSec m_csSubLock2;
  CCritSec m_csOpenClose;
  CInterfaceList<ISubStream> m_pSubStreams;
  CInterfaceList<ISubStream> m_pSubStreams2;

  CComPtr<IBaseFilter> m_pRefClock; // Adjustable reference clock. GothSync
  CComPtr<ISyncClock> m_pSyncClock;

private:
  void MediaTypeDlg();
  void ApplyOptionToCaptureBar(OpenDeviceData* p);
};
