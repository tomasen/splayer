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

#pragma once

#include <atlbase.h>

#include "ChildView.h"
#include "PlayerSeekBar.h"
#include "PlayerToolBar.h"
#include "PlayerPlaylistBar.h"
#include "PlayerCaptureBar.h"
#include "PlayerShaderEditorBar.h"
#include "PlayerColorControlBar.h"
#include "ChkDefPlayerControlBar.h"
#include "TransparentControlBar.h"
#include "PPageFileInfoSheet.h"
#include "OpenCapDeviceDlg.h"
#include "PlayerToolTopBar.h"
#include "PlayerFloatToolBar.h"
#include "PlayerChannelNormalizer.h"
#include "PlayerEQControlBar.h"
#include "SVPSubVoteControlBar.h"
#include "FileDropTarget.h"

#include "..\..\subpic\ISubPic.h"

#include "IGraphBuilder2.h"

#include "RealMediaGraph.h"
#include "QuicktimeGraph.h"
#include "ShockwaveGraph.h"

#include "..\..\..\include\IChapterInfo.h"
#include "..\..\..\include\IKeyFrameInfo.h"
#include "..\..\..\include\IBufferInfo.h"
#include <D3d9.h>
#include <Vmr9.h>
#include <evr.h>
#include <evr9.h>
#include "NEWOSDWnd.h"

#include "SUIButton.h"

#include "../../filters/misc/SyncClock/Interfaces.h"

#include "../../filters/transform/svpfilter/ISVPSubFilter.h"

#include "..\..\..\lib\lyriclib\lyriclib.h"
#include "SVPLycShowBox.h"

#include "Controller/SnapUploadController.h"

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

class CGraphThread : public CWinThread
{
	CMainFrame* m_pMainFrame;

	DECLARE_DYNCREATE(CGraphThread);

public:
	CGraphThread() : m_pMainFrame(NULL) {}

	void SetMainFrame(CMainFrame* pMainFrame) {m_pMainFrame = pMainFrame;}

	BOOL InitInstance();
	int ExitInstance();

	enum {TM_EXIT=WM_APP, TM_OPEN, TM_CLOSE};
	DECLARE_MESSAGE_MAP()
	afx_msg void OnExit(WPARAM wParam, LPARAM lParam);
	afx_msg void OnOpen(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose(WPARAM wParam, LPARAM lParam);
};
/*
class CKeyFrameFinderThread : public CWinThread, public CCritSec
{
	DECLARE_DYNCREATE(CKeyFrameFinderThread);

public:
	CKeyFrameFinderThread() {}
	
	CUIntArray m_kfs; // protected by (CCritSec*)this

	BOOL InitInstance();
	int ExitInstance();

	enum {TM_EXIT=WM_APP, TM_INDEX, TM_BREAK};
	DECLARE_MESSAGE_MAP()
	afx_msg void OnExit(WPARAM wParam, LPARAM lParam);
	afx_msg void OnIndex(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBreak(WPARAM wParam, LPARAM lParam);
};
*/
interface ISubClock;

class CMainFrame : public CFrameWnd, public CDropTarget 
{
	friend class CPPageFileInfoSheet;
	friend class CPPageLogo;

	// TODO: wrap these graph objects into a class to make it look cleaner

	DWORD m_dwRegister;

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
	CStringW m_VidDispName, m_AudDispName;
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

	void SetBalance(int balance);

	
	friend class CTextPassThruFilter;

	// windowing

	CRect m_lastWindowRect;
	CPoint m_lastMouseMove;

	CRect m_rcDesktop;

	
	void SetDefaultWindowRect(int iMonitor = 0);
	void RestoreDefaultWindowRect();
	void ZoomVideoWindow(double scale = -1);
	UINT GetBottomSubOffset();
	CRect GetWhereTheTansparentToolBarShouldBe(CRect rcView);
	double GetZoomAutoFitScale();

	void SetAlwaysOnTop(int i, BOOL setSetting = TRUE);

	// dynamic menus

	void SetupOpenCDSubMenu();
	void SetupFiltersSubMenu();
	void SetupAudioSwitcherSubMenu();
	void SetupSubtitlesSubMenu(int subid = 1);
	void SetupNavAudioSubMenu();
	void SetupNavSubtitleSubMenu();
	void SetupNavAngleSubMenu();
	void SetupNavChaptersSubMenu();
	void SetupFavoritesSubMenu();
	void SetupRecentFileSubMenu();
	void SetupShadersSubMenu();

	void SetupNavStreamSelectSubMenu(CMenu* pSub, UINT id, DWORD dwSelGroup);
	void OnNavStreamSelectSubMenu(UINT id, DWORD dwSelGroup);

	CMenu m_popupmain;
	CMenu m_opencds, m_openmore, m_playback_resmenu, m_playbackmenu;
	CMenu m_filters, m_subtitles, m_subtitles2,  m_audiodevices , m_subtoolmenu;
	CStringArray m_AudioDevice;
	CAutoPtrArray<CMenu> m_filterpopups;
	CMenu m_navsubtitle, m_navangle;
	CMenu m_navchapters, m_navtitles;
	CMenu m_favorites, m_recentfiles;
	CMenu m_shaders;

	HMONITOR m_HLastMonitor;
	CInterfaceArray<IUnknown, &IID_IUnknown> m_pparray;
	CInterfaceArray<IAMStreamSelect> m_ssarray;

	// chapters (file mode)
	CComPtr<IDSMChapterBag> m_pCB;
	void SetupChapters();
	
	void SetupAudioDeviceSubMenu();
	//

	void SetupIViAudReg();

	void AddTextPassThruFilter();
	
	CCritSec m_PaintLock;
	int m_nLoops;
	int m_nLoopSetting;

	bool m_fCustomGraph;
	bool m_fRealMediaGraph, m_fShockwaveGraph, m_fQuicktimeGraph;

	CComPtr<ISubClock> m_pSubClock;

	int m_fFrameSteppingActive;
	int m_VolumeBeforeFrameStepping;

	bool m_fEndOfStream;

	bool m_fBuffering;

	bool m_fLiveWM;

	bool m_fUpdateInfoBar;
	
	time_t  m_tPlayStartTime;
	time_t  m_tPlayPauseTime;
	time_t  m_tLastLogTick;
	CString m_fnCurPlayingFile;
	CString m_fnsAlreadyUploadedSubfile;
	
	CString m_playingmsg, m_closingmsg;
	CString m_lastUrl;

	REFERENCE_TIME m_rtDurationOverride;

	CComPtr<IUnknown> m_pProv;

	void CleanGraph();

	CComPtr<IBaseFilter> pAudioDubSrc;

	void ShowOptions(int idPage = 0);

	bool GetDIB(BYTE** ppData, long& size, bool fSilent = false);
	void SaveDIB(LPCTSTR fn, BYTE* pData, long size);
	BOOL IsRendererCompatibleWithSaveImage();
	void SaveImage(LPCTSTR fn = NULL);
	void SaveThumbnails(LPCTSTR fn);

	//


private:
	CString m_szTitle;
	CString getCurPlayingSubfile(int * iSubDelayMS = NULL,int subid = 0 );
	CPoint m_pLastClickPoint;
	CPoint m_pDragFuncStartPoint;
	int m_iRedrawAfterCloseCounter;
	CSize m_original_size_of_current_video;
	CSize m_last_size_of_current_kind_of_video;
public:
	//CCpuId m_CPU;
	void SetupSVPAudioMenu();
	CMenu m_navaudio, m_audios, m_popup;
	void SetupEQPersetMenu();
	CMenu m_eqperset_menu;
	void MenuMerge(CMenu* Org, CMenu* New);

	CComPtr<IGraphBuilder2> pGB;

	enum
	{
		TIMER_STREAMPOSPOLLER = 1, 
		TIMER_STREAMPOSPOLLER2, 
		TIMER_FULLSCREENCONTROLBARHIDER, 
		TIMER_FULLSCREENMOUSEHIDER, 
		TIMER_STATS,
		TIMER_LEFTCLICK,
		TIMER_STATUSERASER,
		TIMER_STATUSCHECKER,
		TIMER_MOUSELWOWN,
		TIMER_RECENTFOCUSED,
		TIMER_STATUSBARHIDER,
		TIMER_START_CHECKUPDATER,
		TIMER_DELETE_CUR_FILE,
		TIMER_DELETE_CUR_FOLDER,
		TIMER_TRANSPARENTTOOLBARSTAT,
		TIMER_REDRAW_WINDOW,
        TIMER_IDLE_TASK,
        TIMER_LOADING
	};


	void ShowControls(int nCS, bool fSave = true);
	int m_notshowtoolbarforawhile;
	// subtitles
	CComPtr<IVMRMixerControl9>	m_pMC;
	CComPtr<IMFVideoDisplayControl>	m_pMFVDC;
	void		SetVMR9ColorControl(float Brightness, float Contrast, float Hue, float Saturation, BOOL silent = false);

    CWinThread* m_ThreadSVPSub;
	void SVPSubDownloadByVPath(CString szVPath, CAtlList<CString>* szaStatMsgs = NULL);
	void SVP_UploadSubFileByVideoAndSubFilePath(CString fnVideoFilePath, CString szSubPath, int iDelayMS = 0, CAtlList<CString>* szaStatMsgs = NULL, CStringArray* szaPostTerms = NULL);
	CAtlList<CString> m_statusmsgs;

	CSeekBarTip m_tip;

	BOOL m_bSubDownloading;
	BOOL m_bSubUploading;

	CCritSec m_csSubLock;
	CCritSec m_csSubLock2;
	CCritSec m_csOpenClose;
	CInterfaceList<ISubStream> m_pSubStreams;
	CInterfaceList<ISubStream> m_pSubStreams2;
	int m_iSubtitleSel; // if(m_iSubtitleSel&(1<<31)): disabled
	int m_iSubtitleSel2;
	DWORD_PTR m_nSubtitleId;
	DWORD_PTR m_nSubtitleId2;

	void OnShowCurrentPlayingFileInOSD();
	CString GetStatusMessage();
	bool IsMuted() {return m_wndToolBar.GetVolume() == -10000;}
	int GetVolume() {return m_wndToolBar.m_volctrl.GetPos();}

public:
	CMainFrame();

	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:
	int m_iPlaybackMode;
	bool  m_bMustUseExternalTimer;
	bool m_haveSubVoted;
	UINT lastShowCurrentPlayingFileTime;
	ITaskbarList3* pTBL  ;
	int m_lTransparentToolbarStat;
	int  m_lTransparentToolbarPosStat;
	CPoint m_lTransparentToolbarPosOffset;
	
	bool m_fFullScreen;
	bool m_fScreenHigherThanVideo;
	bool m_fPlaylistBeforeToggleFullScreen;
	int m_WndSizeInited;
	bool m_fHideCursor;

	CComPtr<IBaseFilter> m_pRefClock; // Adjustable reference clock. GothSync
	CComPtr<ISyncClock> m_pSyncClock;
    
    bool IsMenuUp();

	bool IsFrameLessWindow() {return(m_fFullScreen || AfxGetAppSettings().fHideCaptionMenu);}
	bool IsCaptionMenuHidden() {return(!m_fFullScreen && AfxGetAppSettings().fHideCaptionMenu);}
	bool IsSomethingLoaded() {return(m_iMediaLoadState != MLS_CLOSED);}
    bool IsSomethingLoading() {return(m_iMediaLoadState == MLS_LOADING);}
	bool IsSubLoaded();
	bool IsPlaylistEmpty() {return(m_wndPlaylistBar.GetCount() == 0);}
	bool IsInteractiveVideo() {return(AfxGetAppSettings().fIntRealMedia && m_fRealMediaGraph || m_fShockwaveGraph);}

	bool m_fAudioOnly;
	bool m_fLastIsAudioOnly;

	CControlBar* m_pLastBar;

	CString GetCurPlayingFileName();

protected: 
	enum {MLS_CLOSED, MLS_LOADING, MLS_LOADED, MLS_CLOSING};
	int m_iMediaLoadState;

	dispmode m_dmBeforeFullscreen;

	DVD_DOMAIN m_iDVDDomain;
	DWORD m_iDVDTitle;
	int m_iSpeedLevel;

	double m_ZoomX, m_ZoomY, m_PosX, m_PosY;
	int m_AngleX, m_AngleY, m_AngleZ;

// Operations
	bool OpenMediaPrivate(CAutoPtr<OpenMediaData> pOMD);
	void CloseMediaPrivate();
	

	void OpenCreateGraphObject(OpenMediaData* pOMD);
	HRESULT OpenMMSUrlStream(CString szFn);
	void OpenFile(OpenFileData* pOFD);
	void OpenDVD(OpenDVDData* pODD);
	void OpenCapture(OpenDeviceData* pODD);
	void OpenCustomizeGraph();
	void OpenSetupVideo();
	void OpenSetupAudio();
  // TODO: this may should move to Controller?
  std::map<int, std::wstring> m_clipinfo;
  void SetClipInfo(int id, std::wstring &value_in)
  {
    m_clipinfo[id] = value_in;
  }
  std::wstring GetClipInfo(UINT id) 
  {
    std::map<int, std::wstring>::iterator it = m_clipinfo.find(id);
    if(it != m_clipinfo.end())
      return it->second;
    return L"";
  };
	void OpenSetupClipInfo();

	void OpenSetupCaptureBar();
	void OpenSetupWindowTitle(CString fn = _T(""));

	friend class CGraphThread;
	CGraphThread* m_pGraphThread;

	CAtlArray<REFERENCE_TIME> m_kfs;

	bool m_fOpeningAborted;

public:
	void OpenCurPlaylistItem(REFERENCE_TIME rtStart = 0);
	void OpenMedia(CAutoPtr<OpenMediaData> pOMD);
	void CloseMedia();

	int SVPMessageBox(LPCTSTR lpszText, UINT nType = MB_OK,	UINT ulTimeout = 2000 , UINT idWMCommandMsg = 0);
	
	void AddCurDevToPlaylist();

	bool m_fTrayIcon;
	void ShowTrayIcon(bool fShow);
	void SetTrayTip(CString str);

	CSize GetVideoSize();
	void ToggleFullscreen(bool fToNearest, bool fSwitchScreenResWhenHasTo);
	void MoveVideoWindow(bool fShowStats = false);
	void RepaintVideo();

	OAFilterState GetMediaState();
	REFERENCE_TIME GetPos(), GetDur();
	void SeekTo(REFERENCE_TIME rt, int fSeekToKeyFrame = -99, REFERENCE_TIME maxStep = 0); // -1 => bwd , 1 => fwd

	bool LoadSubtitle(CString fn, int sub_delay_ms = 0, BOOL bIsForPlayList = false);
	void UpdateSubtitle(bool fApplyDefStyle = true);
	void UpdateSubtitle2(bool fApplyDefStyle = true);
	void SetSubtitle(ISubStream* pSubStream, bool fApplyDefStyle = true, bool bShowOSD = true);
	void SetSubtitle2(ISubStream* pSubStream, bool fApplyDefStyle = true, bool bShowOSD = true);
	void ReplaceSubtitle(ISubStream* pSubStreamOld, ISubStream* pSubStreamNew, int secondSub = 0);
	void InvalidateSubtitle(DWORD_PTR nSubtitleId = -1, REFERENCE_TIME rtInvalidate = -1);
	void ReloadSubtitle();

	BOOL m_bCheckingUpdater;
	// shaders
	CAtlList<CString> m_shaderlabels;
	void SetShaders( BOOL silent = false);
	void UpdateShaders(CString label);

	//A-B Control
	REFERENCE_TIME m_aRefTime;
	REFERENCE_TIME m_bRefTime;
	int ABControlOn;

	// capturing
	bool m_fCapturing;
	HRESULT BuildCapture(IPin* pPin, IBaseFilter* pBF[3], const GUID& majortype, AM_MEDIA_TYPE* pmt); // pBF: 0 buff, 1 enc, 2 mux, pmt is for 1 enc
	bool BuildToCapturePreviewPin(
		IBaseFilter* pVidCap, IPin** pVidCapPin, IPin** pVidPrevPin, 
		IBaseFilter* pAudCap, IPin** pAudCapPin, IPin** pAudPrevPin);
	bool BuildGraphVideoAudio(int fVPreview, bool fVCapture, int fAPreview, bool fACapture);
	bool DoCapture(), StartCapture(), StopCapture();

	bool DoAfterPlaybackEvent();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void RecalcLayout(BOOL bNotify = TRUE);

// Implementation
public:
	virtual ~CMainFrame();
	CChildView m_wndView;

	CMenu m_ABMenu;


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members


	CTransparentControlBar m_wndTransparentControlBar;
	CPlayerChannelNormalizer m_wndChannelNormalizerBar;
  ChkDefPlayerControlBar m_chkdefplayercontrolbar;
	CPlayerEQControlBar	 m_wndPlayerEQControlBar;
	CSVPSubVoteControlBar	m_wndSubVoteControlBar;
	CList<CControlBar*> m_bars;

	CPlayerPlaylistBar m_wndPlaylistBar;
	CPlayerCaptureBar m_wndCaptureBar;
	CPlayerShaderEditorBar m_wndShaderEditorBar;
	CList<CSizingControlBar*> m_dockingbars;

	
	CFileDropTarget m_fileDropTarget;
	// TODO
	DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	DROPEFFECT OnDropEx(COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
	void OnDragLeave();
	DROPEFFECT OnDragScroll(DWORD dwKeyState, CPoint point);

	friend class CPPagePlayback; // TODO
	friend class CMPlayerCApp; // TODO

	void LoadControlBar(CControlBar* pBar, UINT defDockBarID);
	void RestoreFloatingControlBars();
	void SaveControlBars();

// Generated message map functions

	DECLARE_MESSAGE_MAP()

public:
	CPlayerSeekBar m_wndSeekBar;
	CPlayerToolBar m_wndToolBar;

	CPlayerFloatToolBar* m_wndFloatToolBar;
	CNEWOSDWnd m_wndNewOSD;
	CPlayerColorControlBar m_wndColorControlBar;

	//Mouse Relate
	void PreFocused();

	CPlayerToolTopBar m_wndToolTopBar;
	int m_nLogDPIY;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();

	afx_msg LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEnterMenuLoop( BOOL bIsTrackPopupMenu );
	afx_msg void OnExitMenuLoop( BOOL bIsTrackPopupMenu );


	afx_msg LRESULT OnTaskBarRestart(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyIcon(WPARAM, LPARAM);

	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    int m_lMinFrameWidth;
	afx_msg void OnMove(int x, int y);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnDisplayChange();

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg LRESULT OnAppCommand(WPARAM wParam, LPARAM lParam);

	afx_msg void OnTimer(UINT nIDEvent);

	afx_msg LRESULT OnGraphNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRepaintRenderLess(WPARAM wParam, LPARAM lParam);
    BOOL m_is_resume_from_last_exit_point;
	afx_msg LRESULT OnResumeFromState(WPARAM wParam, LPARAM lParam);

	BOOL OnButton(UINT id, UINT nFlags, CPoint point);
	bool GetNoResponseRect(CRgn& pRgn);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg LRESULT OnXButtonDown(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnXButtonUp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnXButtonDblClk(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMouseMoveIn(WPARAM /*wparam*/, LPARAM /*lparam*/) ;
	afx_msg LRESULT OnMouseMoveOut(WPARAM /*wparam*/, LPARAM /*lparam*/) ;
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	void HideFloatTransparentBar();
	afx_msg LRESULT OnNcHitTest(CPoint point);

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct); 
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct); 

	BOOL OnMenu(CMenu* pMenu);
	afx_msg void OnMenuPlayerShort();
	afx_msg void OnMenuPlayerLong();
	afx_msg void OnMenuFilters();
	//afx_msg void OnColorControlButtonReset();
	//afx_msg void OnColorControlButtonEnable();
	afx_msg void OnEnableDX9();
	void ReRenderOrLoadMedia(BOOL bNoMoreDXVAForThisMedia = FALSE);
	//afx_msg void OnColorControlUpdateButtonReset(CCmdUI* pCmdUI);
	//afx_msg void OnColorControlUpdateButtonEnable(CCmdUI* pCmdUI);

	afx_msg void OnResetSetting();
	afx_msg void OnSetHotkey();

	afx_msg void OnUpdatePlayerStatus(CCmdUI* pCmdUI);

	afx_msg void OnFilePostOpenmedia();
	afx_msg void OnUpdateFilePostOpenmedia(CCmdUI* pCmdUI);
	afx_msg void OnFilePostClosemedia();
	afx_msg void OnUpdateFilePostClosemedia(CCmdUI* pCmdUI);

	afx_msg void OnBossKey();

	afx_msg void OnABControl(UINT nID);

	afx_msg void OnStreamAudio(UINT nID);
	afx_msg void OnStreamSub(UINT nID);
	afx_msg void OnStreamSubOnOff();
	afx_msg void OnOgmAudio(UINT nID);
	afx_msg void OnOgmSub(UINT nID);
	afx_msg void OnDvdAngle(UINT nID);
	afx_msg void OnDvdAudio(UINT nID);
	afx_msg void OnDvdSub(UINT nID);
	afx_msg void OnDvdSubOnOff();


	// menu item handlers

	afx_msg void OnFileOpenUrlStream();
	afx_msg void OnFileOpenQuick();
	afx_msg void OnFileOpenmedia();
	afx_msg void OnFileOpenFolder();
	afx_msg void OnUpdateFileOpen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateABControl(CCmdUI* pCmdUI);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnFileOpendvd();
	afx_msg void OnFileOpenBdvd();
	afx_msg void OnFileOpendevice();
	afx_msg void OnFileOpenCD(UINT nID);
	afx_msg void OnDropFiles(HDROP hDropInfo); // no menu item
	afx_msg void OnFileSaveAs();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveImage();
	afx_msg void OnFileCopyImageToCLipBoard();
	afx_msg void OnFileSaveImageAuto();
	afx_msg void OnUpdateFileSaveImage(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveThumbnails();
	afx_msg void OnUpdateFileSaveThumbnails(CCmdUI* pCmdUI);
	afx_msg void OnShowEQControl();
	afx_msg void OnShowChannelControl();
	afx_msg void OnFileConvert();
	afx_msg void OnUpdateFileConvert(CCmdUI* pCmdUI);
	afx_msg void OnFileLoadsubtitle();
	afx_msg void OnFileLoadsubtitle2();
	afx_msg void OnUpdateFileLoadsubtitle(CCmdUI* pCmdUI);
	afx_msg void OnFileSavesubtitle();
	afx_msg void OnUpdateFileSavesubtitle(CCmdUI* pCmdUI);
	afx_msg void OnFileISDBSearch();
	afx_msg void OnUpdateFileISDBSearch(CCmdUI* pCmdUI);
	afx_msg void OnFileISDBUpload();
	afx_msg void OnUpdateFileISDBUpload(CCmdUI* pCmdUI);
	afx_msg void OnFileISDBDownload();
	afx_msg void OnUpdateFileISDBDownload(CCmdUI* pCmdUI);
	afx_msg void OnFileProperties();
	afx_msg void OnUpdateFileProperties(CCmdUI* pCmdUI);
	afx_msg void OnFileClosePlaylist();
	afx_msg void OnFileCloseMedia(); // no menu item
	afx_msg void OnUpdateFileClose(CCmdUI* pCmdUI);

	afx_msg void OnViewCaptionmenu();
	afx_msg void OnUpdateViewCaptionmenu(CCmdUI* pCmdUI);
	afx_msg void OnViewControlBar(UINT nID);
	afx_msg void OnUpdateViewControlBar(CCmdUI* pCmdUI);
	afx_msg void OnViewPlaylist();
	afx_msg void OnUpdateViewPlaylist(CCmdUI* pCmdUI);
	afx_msg void OnViewCapture();
	afx_msg void OnUpdateViewCapture(CCmdUI* pCmdUI);
	afx_msg void OnViewShaderEditor();
	afx_msg void OnUpdateViewShaderEditor(CCmdUI* pCmdUI);
	afx_msg void OnViewMinimal();
	afx_msg void OnUpdateViewMinimal(CCmdUI* pCmdUI);
	afx_msg void OnViewCompact();
	afx_msg void OnUpdateViewCompact(CCmdUI* pCmdUI);
	afx_msg void OnViewNormal();
	afx_msg void OnUpdateViewNormal(CCmdUI* pCmdUI);
	afx_msg void OnViewFullscreen();
	afx_msg void OnViewFullscreenSecondary();
	afx_msg void OnUpdateViewFullscreen(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom(UINT nID);
	afx_msg void OnUpdateViewZoom(CCmdUI* pCmdUI);
	afx_msg void OnViewZoomAutoFit();
	afx_msg void OnViewDefaultVideoFrame(UINT nID);
	afx_msg void OnUpdateViewDefaultVideoFrame(CCmdUI* pCmdUI);
	afx_msg void OnViewKeepaspectratio();
	afx_msg void OnUpdateViewKeepaspectratio(CCmdUI* pCmdUI);
	afx_msg void OnViewCompMonDeskARDiff();
	afx_msg void OnUpdateViewCompMonDeskARDiff(CCmdUI* pCmdUI);
	afx_msg void OnViewPanNScan(UINT nID);
	afx_msg void OnEQPerset(UINT nID);
	afx_msg void OnEQPersetReset();
	afx_msg void OnUpdateEQPerset(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewPanNScan(CCmdUI* pCmdUI);
	afx_msg void OnViewPanNScanPresets(UINT nID);
	afx_msg void OnUpdateViewPanNScanPresets(CCmdUI* pCmdUI);
	afx_msg void OnViewRotate(UINT nID);
	afx_msg void OnUpdateViewRotate(CCmdUI* pCmdUI);
	afx_msg void OnViewAspectRatio(UINT nID);
	afx_msg void OnUpdateViewAspectRatio(CCmdUI* pCmdUI);
	afx_msg void OnViewAspectRatioNext();
	afx_msg void OnViewOntop(UINT nID);
	afx_msg void OnUpdateViewOntop(CCmdUI* pCmdUI);
	afx_msg void OnViewOptions();
	afx_msg void OnSetAudioNumberOfSpeaker();
	afx_msg void OnSwitchAudioDevice();
	afx_msg void OnShowDrawStats();

	afx_msg void OnPlayPlay();
	afx_msg void OnPlayPause();
	afx_msg void OnPlayPauseI();
	afx_msg void OnPlayPlaypause();
	afx_msg void OnPlayStop();
    afx_msg void OnPlayStopManual();
	afx_msg void OnPlayStopDummy();
	afx_msg void OnUpdatePlayPauseStop(CCmdUI* pCmdUI);
	
	afx_msg void OnPlayMenuLoopSetting(UINT nID); 
	afx_msg void OnUpdatePlayMenuLoopSetting(CCmdUI* pCmdUI);

	afx_msg void OnPlaySubDelay(UINT nID); //SubDelay Button
	afx_msg void OnUpdatePlaySubDelay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSubtitleFontChange(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLanguage(CCmdUI* pCmdUI);
	afx_msg void OnPlaySub2Delay(UINT nID);
	afx_msg void OnUpdatePlaySub2Delay(CCmdUI* pCmdUI);

	afx_msg void OnPlayFramestep(UINT nID);
	afx_msg void OnChangeVSyncOffset(UINT nID);
	afx_msg void OnChangeResizer(UINT nID);
	afx_msg void OnUpdatePlayFramestep(CCmdUI* pCmdUI);
	afx_msg void OnUpdateChangeResizer(CCmdUI* pCmdUI);
	UINT m_lastSeekAction;
	afx_msg void OnPlaySeek(UINT nID);
	afx_msg void OnSmartSeek(UINT nID);
	afx_msg void OnPlaySeekKey(UINT nID); // no menu item
	afx_msg void OnUpdatePlaySeek(CCmdUI* pCmdUI);
	afx_msg void OnPlayGoto();
	afx_msg void OnUpdateGoto(CCmdUI* pCmdUI);
	afx_msg void OnPlayChangeRate(UINT nID);
	afx_msg void OnUpdatePlayChangeRate(CCmdUI* pCmdUI);
	afx_msg void OnPlayResetRate();
	afx_msg void OnUpdatePlayResetRate(CCmdUI* pCmdUI);
	afx_msg void OnPlayChangeAudDelay(UINT nID);
	afx_msg void OnUpdatePlayChangeAudDelay(CCmdUI* pCmdUI);
	afx_msg void OnPlayFilters(UINT nID);
	afx_msg void OnUpdatePlayFilters(CCmdUI* pCmdUI);
	afx_msg void OnPlayShaders(UINT nID);
	afx_msg void OnUpdatePlayShaders(CCmdUI* pCmdUI);
	afx_msg void OnPlayAudio(UINT nID);
	afx_msg void OnUpdatePlayAudio(CCmdUI* pCmdUI);
	afx_msg void OnToogleSubtitle();
	afx_msg void OnPlaySubtitles(UINT nID);
	afx_msg void OnUpdatePlaySubtitles(CCmdUI* pCmdUI);
	afx_msg void OnPlayLanguage(UINT nID);
	afx_msg void OnUpdatePlayLanguage(CCmdUI* pCmdUI);
	afx_msg void OnPlayVolume(UINT nID);
	afx_msg void OnPlayVolumeBoost(UINT nID);
	afx_msg void OnUpdatePlayVolumeBoost(CCmdUI* pCmdUI);
	afx_msg void OnAfterplayback(UINT nID);
	afx_msg void OnUpdateAfterplayback(CCmdUI* pCmdUI);
	
	afx_msg void OnSubtitleDelay(UINT nID);
	afx_msg void OnSubtitleDelay2(UINT nID);
	afx_msg void OnSubtitleMove(UINT nID);
	afx_msg void OnSubtitleFontChange(UINT nID);
	
	afx_msg void OnRenderModeChange(UINT nID);
	afx_msg void OnUpdateRenderModeChange(CCmdUI* pCmdUI);

	afx_msg void OnNavigateSkip(UINT nID);
	afx_msg void OnUpdateNavigateSkip(CCmdUI* pCmdUI);
	afx_msg void OnNavigateSkipPlaylistItem(UINT nID);
	afx_msg void OnUpdateNavigateSkipPlaylistItem(CCmdUI* pCmdUI);
	afx_msg void OnNavigateMenu(UINT nID);
	afx_msg void OnUpdateNavigateMenu(CCmdUI* pCmdUI);
	afx_msg void OnNavigateAudio(UINT nID);
	afx_msg void OnNavigateSubpic(UINT nID);
	afx_msg void OnNavigateAngle(UINT nID);
	afx_msg void OnNavigateChapters(UINT nID);
	afx_msg void OnNavigateMenuItem(UINT nID);
	afx_msg void OnUpdateNavigateMenuItem(CCmdUI* pCmdUI);

	afx_msg void OnPlayListRandom();
	afx_msg void OnUpdatePlayListRandom(CCmdUI* pCmdUI);

	afx_msg void OnFavoritesAdd();
	afx_msg void OnTopBtnFileExit();
    UINT m_l_been_playing_sec;
	void OnFavoritesAddReal( BOOL bRecent = FALSE , BOOL bForceDel = FALSE );
	afx_msg void OnUpdateFavoritesAdd(CCmdUI* pCmdUI);
	afx_msg void OnFavoritesOrganize();
	afx_msg void OnUpdateFavoritesOrganize(CCmdUI* pCmdUI);
	afx_msg void OnRecentFile(UINT nID);
	afx_msg void OnUpdateRecentFile(CCmdUI* pCmdUI);
	afx_msg void OnFavoritesFile(UINT nID);
	afx_msg void OnUpdateFavoritesFile(CCmdUI* pCmdUI);
	afx_msg void OnFavoritesDVD(UINT nID);
	afx_msg void OnUpdateFavoritesDVD(CCmdUI* pCmdUI);
	afx_msg void OnFavoritesDevice(UINT nID);
	afx_msg void OnUpdateFavoritesDevice(CCmdUI* pCmdUI);

    afx_msg void OnSeekToSpecialPos(UINT nID);

	afx_msg void OnHelpHomepage();
	afx_msg void OnHelpDocumentation();

	afx_msg void OnClose();
	
	void		SetSubtitleDelay(int delay_ms);
	void		SetSubtitleDelay2(int delay_ms);
	afx_msg void OnAdvOptions();
	afx_msg void OnManualcheckupdate();
	afx_msg void OnSvpsubMenuenable();
	afx_msg void OnUpdateSvpsubMenuenable(CCmdUI *pCmdUI);
	afx_msg void OnSmartDragEnable();
	afx_msg void OnUpdateSmartDragEnable(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMenuDVDNav(CCmdUI *pCmdUI);
	UINT m_iAudioChannelMaping;
	afx_msg void OnAudioChannalMapMenu(UINT nID);
	afx_msg void OnUpdateChannalMapMenu(CCmdUI *pCmdUI);
	void SetupSubMenuToolbar();
	afx_msg void OnThemeChangeMenu(UINT nID);
	afx_msg void OnUpdateThemeChangeMenu(CCmdUI *pCmdUI);


	afx_msg void OnAudioDeviceChange(UINT nID);
	afx_msg void OnUpdateAudioDeviceChange(CCmdUI *pCmdUI);
	afx_msg void OnVisitbbs();
	afx_msg void OnSendemail();
	afx_msg void OnCheckDefaultPlayer();
  afx_msg void OnCheckAndSetDefaultPlayer();
	afx_msg void OnVisitcontactinfo();
	afx_msg void OnDonate();
	afx_msg void OnJointeam();
	afx_msg void OnLanguage(UINT nID);
	afx_msg void OnColorControl(UINT nID);

	afx_msg void OnShowSUBVoteControlBar();
	afx_msg void OnShowEQControlBar();
	afx_msg void OnShowChannelNormalizerBar();
	afx_msg void OnShowColorControlBar();
	afx_msg void OnShowTranparentControlBar();
	afx_msg void OnRecentFileClear();
	afx_msg void OnRecentFileEnable();
	afx_msg void OnRecentFileDisable();
	afx_msg void OnUpdateShowColorControlBar(CCmdUI *pCmdUI);
	afx_msg void OnSetsnapshotpath();

	/*NEW UI*/
	LRESULT OnNcPaint( WPARAM wParam, LPARAM lParam );
	LRESULT OnNcActivate( WPARAM wParam, LPARAM lParam);
	LRESULT OnNcLButtonDown(  WPARAM wParam, LPARAM lParam);
	LRESULT OnNcLButtonUp(  WPARAM wParam, LPARAM lParam);
	LRESULT OnNcHitTestNewUI(  WPARAM wParam, LPARAM lParam);
	LRESULT OnNcCalcSizeNewUI(  WPARAM wParam, LPARAM lParam);
	LRESULT OnStatusMessage(  WPARAM wParam, LPARAM lParam);
	LRESULT OnSuggestVolume(  WPARAM wParam, LPARAM lParam);
	LRESULT OnFailedInDXVA(  WPARAM wParam, LPARAM lParam);
	bool m_bAllowVolumeSuggestForThisPlaylist;
	LRESULT OnImeSetContext(  WPARAM wParam, LPARAM lParam);
	

	//afx_msg void OnNcCalcSize( BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	void RedrawNonClientArea();
	DWORD GetUIStat(UINT n_CmdID);
	//void PreMultiplyBitmap(CBitmap& bmp);
	void rePosOSD();
	int m_iOSDAlign  ;
	void SendStatusMessage(CString msg, int nTimeOut, int iAlign = 0);
	CMenu m_mainMenu;
	BOOL m_bDxvaInUse;
	CString m_DXVAMode;
	BOOL m_bEVRInUse;

	//Lyric Thing
    CWinThread* m_lyricDownloadThread;
	CLyricLib m_Lyric;
	CStringArray m_LyricFilePaths;
	SVPLycShowBox* m_wndLycShowBox;
private:
	CBitmap m_bmpCaption,m_bmpBCorner;//, m_bmpClose, m_bmpMaximize, m_bmpMinimize, m_bmpRestore, m_bmpMenu;
	CSUIBtnList m_btnList;
	CRgn m_rgn;
	CFont m_hft;
	BOOL m_bHasDrawShadowText;
	//long m_nBoxStatus[4];
	/*NEW UI END*/
	CString fnDelPending;
public:
	CString GetAnEasyToUnderstoodSubtitleName(CString szName);
	CString GetAnEasyToUnderstoodAudioStreamName(CString szName);

	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChangebackground();
	afx_msg void OnSubsetfontboth();
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnDeletecurfile();
	afx_msg void OnDelcurfolder();
	afx_msg void OnUpdateDeleteCurs(CCmdUI *pCmdUI);
	afx_msg void OnToggleSPDIF();
	afx_msg void OnSubMenuToolbar();
	afx_msg void OnUpdateToggleSPDIF(CCmdUI *pCmdUI);
	afx_msg void OnDebugreport();
	afx_msg void OnMenuAudio();
	afx_msg void OnMenuVideo();
	afx_msg void OnUpdateDebugreport(CCmdUI *pCmdUI);
	afx_msg BOOL OnTtnNeedText(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSetAutoLoadSubtitle();
	afx_msg void OnUpdateSetAutoLoadSubtitle(CCmdUI *pCmdUI);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnPaint();
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);

	void OnSettingFinished();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

  afx_msg void OnAudioSettingUpdated();
private:
  void _HandleTimer_Stats();
  void _HandleTimer_StreamPosPoller();

  SnapUploadController  m_snapupload;
};
