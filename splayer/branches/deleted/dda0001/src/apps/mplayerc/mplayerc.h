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

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif


#define ResStr(id) CString(MAKEINTRESOURCE(id))

#include "resource.h"       // main symbols
#include <afxadv.h>
#include <atlsync.h>
#include "..\..\subtitles\STS.h"
#include "MediaFormats.h"
#include "fakefiltermapper2.h"
#include "..\..\..\lib\splite\libsqlite\libsqlite.h"

//#include "..\..\..\Updater\cupdatenetlib.h"
#include "..\..\filters\switcher\AudioSwitcher\AudioSwitcher.h"

class cupdatenetlib;

#ifdef UNICODE
#define MPC_WND_CLASS_NAME L"MediaPlayerClassicW"
#else
#define MPC_WND_CLASS_NAME "MediaPlayerClassicA"
#endif

enum 
{
	WM_GRAPHNOTIFY = WM_APP+1,
	WM_REARRANGERENDERLESS,
	WM_RESUMEFROMSTATE
};

#define WM_MYMOUSELAST WM_XBUTTONDBLCLK

///////////////

extern void CorrectComboListWidth(CComboBox& box, CFont* pWndFont);
extern HICON LoadIcon(CString fn, bool fSmall);
extern bool LoadType(CString fn, CString& type);
extern bool LoadResource(UINT resid, CStringA& str, LPCTSTR restype);
extern CString GetContentType(CString fn, CAtlList<CString>* redir = NULL);
extern void GetSystemFontWithScale(CFont* pFont, double dDefaultSize = 14.0, int iWeight = FW_NORMAL, CString szTryFontName = _T(""));

struct eq_perset_setting
{
	/* Filter static config */
	CString szPersetName;
	int  i_band;
	float f_preamp;
	float f_amp[MAX_EQ_BAND];   /* Per band amp */
	

};
/////////////////////////////////////////////////////////////////////////////
// CMPlayerCApp:
// See mplayerc.cpp for the implementation of this class
//

// flags for AppSettings::nCS
enum 
{
	CS_NONE=0, 
	CS_SEEKBAR=1, 
	CS_TOOLBAR=CS_SEEKBAR<<1, 
	CS_INFOBAR=CS_TOOLBAR<<1, 
	CS_STATSBAR=CS_INFOBAR<<1, 
	CS_STATUSBAR=CS_STATSBAR<<1, 
	CS_TOOLTOPBAR = CS_STATUSBAR<<1, 
	CS_COLORCONTROLBAR=CS_TOOLTOPBAR<<1, 
	CS_LAST=CS_TOOLTOPBAR
};

enum
{
	CLSW_NONE=0,
	CLSW_OPEN=1,
	CLSW_PLAY=CLSW_OPEN<<1,
	CLSW_CLOSE=CLSW_PLAY<<1,
	CLSW_STANDBY=CLSW_CLOSE<<1,
	CLSW_HIBERNATE=CLSW_STANDBY<<1,
	CLSW_SHUTDOWN=CLSW_HIBERNATE<<1,
	CLSW_LOGOFF=CLSW_SHUTDOWN<<1,
	CLSW_AFTERPLAYBACK_MASK=CLSW_CLOSE|CLSW_STANDBY|CLSW_SHUTDOWN|CLSW_HIBERNATE|CLSW_LOGOFF,
	CLSW_FULLSCREEN=CLSW_LOGOFF<<1,
	CLSW_NEW=CLSW_FULLSCREEN<<1,
	CLSW_HELP=CLSW_NEW<<1,
	CLSW_DVD=CLSW_HELP<<1,
	CLSW_CD=CLSW_DVD<<1,
	CLSW_CAP=CLSW_CD<<1,
	CLSW_ADD=CLSW_CAP<<1,
	CLSW_MINIMIZED=CLSW_ADD<<1,
	CLSW_REGEXTVID=CLSW_MINIMIZED<<1,
	CLSW_REGEXTAUD=CLSW_REGEXTVID<<1,
	CLSW_UNREGEXT=CLSW_REGEXTAUD<<1,
	CLSW_STARTVALID=CLSW_UNREGEXT<<2,
	CLSW_NOFOCUS=CLSW_STARTVALID<<1,
	CLSW_FIXEDSIZE=CLSW_NOFOCUS<<1,
	CLSW_MONITOR=CLSW_FIXEDSIZE<<1,	
	CLSW_ADMINOPTION=CLSW_MONITOR<<1,
	CLSW_GENUIINI=CLSW_ADMINOPTION<<1,
	CLSW_STARTFROMDMP=CLSW_GENUIINI<<1,
	CLSW_HTPCMODE=CLSW_STARTFROMDMP<<1,
    CLSW_STARTFULL=CLSW_HTPCMODE<<1,
	CLSW_UNRECOGNIZEDSWITCH=CLSW_STARTFULL<<1
};

enum
{
	VIDRNDT_DS_DEFAULT,
	VIDRNDT_DS_OLDRENDERER,
	VIDRNDT_DS_OVERLAYMIXER,
	VIDRNDT_DS_VMR7WINDOWED,
	VIDRNDT_DS_VMR9WINDOWED,
	VIDRNDT_DS_VMR7RENDERLESS,
	VIDRNDT_DS_VMR9RENDERLESS,
	VIDRNDT_DS_DXR,
	VIDRNDT_DS_NULL_COMP,
	VIDRNDT_DS_NULL_UNCOMP,
};

enum
{
	VIDRNDT_RM_DEFAULT,
	VIDRNDT_RM_DX7,
	VIDRNDT_RM_DX9,
};

enum
{
	VIDRNDT_QT_DEFAULT,
	VIDRNDT_QT_DX7,
	VIDRNDT_QT_DX9,
};

enum
{
	VIDRNDT_AP_SURFACE,
	VIDRNDT_AP_TEXTURE2D,
	VIDRNDT_AP_TEXTURE3D,
};

#define AUDRNDT_NULL_COMP _T("Null Audio Renderer (Any)")
#define AUDRNDT_NULL_UNCOMP _T("Null Audio Renderer (Uncompressed)")

enum
{
	SRC_CDDA=1, 
	SRC_CDXA=SRC_CDDA<<1,
	SRC_VTS=SRC_CDXA<<1,
	SRC_FLIC=SRC_VTS<<1,
	SRC_D2V=SRC_FLIC<<1,
	SRC_DTSAC3=SRC_D2V<<1,
	SRC_MATROSKA=SRC_DTSAC3<<1,
	SRC_SHOUTCAST=SRC_MATROSKA<<1,
	SRC_REALMEDIA=SRC_SHOUTCAST<<1,
	SRC_AVI=SRC_REALMEDIA<<1,
	SRC_RADGT=SRC_AVI<<1,
	SRC_ROQ=SRC_RADGT<<1,
	SRC_OGG=SRC_ROQ<<1,
	SRC_NUT=SRC_OGG<<1,
	SRC_MPEG=SRC_NUT<<1,
	SRC_DIRAC=SRC_MPEG<<1,
	SRC_MPA=SRC_DIRAC<<1,
	SRC_DSM=SRC_MPA<<1,
	SRC_SUBS=SRC_DSM<<1,
	SRC_MP4=SRC_SUBS<<1,
	SRC_FLV=SRC_MP4<<1,
	SRC_LAST=SRC_FLV<<1
};

enum
{
	TRA_MPEG1  = 1, 
	TRA_MPEG2  = TRA_MPEG1<<1,
	TRA_RV     = TRA_MPEG2<<1,
	TRA_RA     = TRA_RV<<1,
	TRA_MPA    = TRA_RA<<1,
	TRA_LPCM   = TRA_MPA<<1,
	TRA_AC3    = TRA_LPCM<<1,
	TRA_DTS    = TRA_AC3<<1,
	TRA_AAC    = TRA_DTS<<1,
	TRA_PS2AUD = TRA_AAC<<1,
	TRA_DIRAC  = TRA_PS2AUD<<1,
	TRA_VORBIS = TRA_DIRAC<<1,
	TRA_FLAC   = TRA_VORBIS<<1,
	TRA_NELLY  = TRA_FLAC<<1,
	TRA_LAST   = TRA_NELLY<<1
};

enum
{
	MPCDXVA_H264  = 1,
	MPCDXVA_VC1   = MPCDXVA_H264<<1,
	MPCDXVA_LAST  = MPCDXVA_VC1<<1
};

enum
{
	FFM_H264    = 1,
	FFM_VC1     = FFM_H264<<1,	
	FFM_FLV4    = FFM_VC1<<1,
	FFM_VP62    = FFM_FLV4<<1,
	FFM_XVID    = FFM_VP62<<1,
	FFM_DIVX    = FFM_XVID<<1,
	FFM_MSMPEG4 = FFM_DIVX<<1,
	FFM_WMV     = FFM_MSMPEG4<<1,
	FFM_SVQ3    = FFM_WMV<<1,
	FFM_H263    = FFM_SVQ3<<1,
	FFM_THEORA  = FFM_H263<<1,
	FFM_AMVV    = FFM_THEORA<<1,	
	FFM_LAST    = FFM_AMVV<<1
};

enum
{
	DVS_HALF, 
	DVS_NORMAL, 
	DVS_DOUBLE, 
	DVS_STRETCH, 
	DVS_FROMINSIDE, 
	DVS_FROMOUTSIDE
};

typedef enum 
{
	FAV_FILE,
	FAV_DVD,
	FAV_DEVICE
} favtype;

#pragma pack(push, 1)
typedef struct
{
	bool fValid;
	CSize size; 
	int bpp, freq;
} dispmode;
#pragma pack(pop)

#include <afxsock.h>

extern void GetCurDispMode(dispmode& dm);
extern bool GetDispMode(int i, dispmode& dm);
extern void SetDispMode(dispmode& dm);

class CMPlayerCApp : public CWinApp
{
	ATL::CMutex m_mutexOneInstance;

	CAtlList<CString> m_cmdln;
	void PreProcessCommandLine();
	BOOL SendCommandLine(HWND hWnd, BOOL bPostMessage = false);
	HINSTANCE				m_hD3DX9Dll;
	int						m_nDXSdkRelease;

public:
	CMPlayerCApp();
	UINT GetBottomSubOffset();


	BOOL (__stdcall * m_pGetLayeredWindowAttributes)(__in HWND hwnd,__out_opt COLORREF* pcrKey,	__out_opt BYTE* pbAlpha,__out_opt DWORD* pdwFlags);
	HRESULT (__stdcall * m_pDwmIsCompositionEnabled)(__out BOOL* pfEnabled);
	HRESULT (__stdcall * m_pDwmEnableComposition)(UINT uCompositionAction);
	HRESULT (__stdcall * m_pDwmExtendFrameIntoClientArea)( HWND hWnd,const MARGINS *pMarInset);
	HTHEME (__stdcall * m_pOpenThemeData)(  HWND hwnd,LPCWSTR pszClassList);
	HRESULT (__stdcall * m_pGetThemeSysFont)( HTHEME hTheme,int iFontID,LOGFONTW *plf);
	HRESULT (__stdcall * m_pCloseThemeData)(HTHEME hTheme);
	HRESULT (__stdcall * m_pDrawThemeTextEx)( HTHEME hTheme, HDC hdc,int iPartId,int iStateId,LPCWSTR pszText,int iCharCount,DWORD dwFlags,LPRECT pRect,const DTTOPTS *pOptions);
	HRESULT (__stdcall * m_pDwmDefWindowProc)( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);
	HRESULT (__stdcall * m_pDirect3DCreate9Ex)(UINT SDKVersion, LPVOID**);

	// === CASIMIR666 : Ajout CMPlayerCApp
	bool		m_fTearingTest;
	int			m_fDisplayStats;
	int			m_fResetStats; // Set to reset the presentation statistics

	CString		m_strD3DX9Version;
	LONGLONG	m_PerfFrequency;
	
	void ShowCmdlnSwitches();

	bool StoreSettingsToIni();
	bool StoreSettingsToRegistry();
	CString GetIniPath();
	bool IsIniValid();
	void RemoveAllSetting();

	BOOL RegDelnode (HKEY hKeyRoot, LPTSTR lpSubKey);
	BOOL RegDelnodeRecurse (HKEY hKeyRoot, LPTSTR lpSubKey);

	bool GetAppDataPath(CString& path);
	
	bool m_bGotResponse;
	static int	m_isVista;
	static HMODULE	m_hResDll;
	static void					SetLanguage (int nLanguage);
	static LPCTSTR				GetSatelliteDll(int nLang);
	static bool	IsVista();
	static bool	IsWin7();
	static int	m_bCanUseCUDA;
	static bool	CanUseCUDA();
	static int	m_bHasEVRSupport;
	static bool	HasEVRSupport();
	static void GainAdminPrivileges(UINT idd, BOOL bWait = true);
	static int GetNumberOfSpeakers(LPCGUID lpcGUID = NULL, HWND hWnd = NULL);
	static bool	IsVSFilterInstalled();
	HINSTANCE					GetD3X9Dll();
	LONGLONG					GetPerfCounter();
	int							GetDXSdkRelease() { return m_nDXSdkRelease; };


    void ClearRecentFileListForWin7();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMPlayerCApp)
	private:
	BOOL  m_bSystemParametersInfo[4] ;

	public:
	void InitInstanceThreaded(INT64 CLS64);
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	class Settings
	{
		friend class CMPlayerCApp;

		bool fInitialized;

		class CRecentFileAndURLList : public CRecentFileList
		{
		public:
			CRecentFileAndURLList(UINT nStart, LPCTSTR lpszSection,
				LPCTSTR lpszEntryFormat, int nSize,
				int nMaxDispLen = AFX_ABBREV_FILENAME_LEN);

			virtual void Add(LPCTSTR lpszPathName); // we have to override CRecentFileList::Add because the original version can't handle URLs
		};

	public:
		// cmdline params
		int nCLSwitches;
		BOOL bGenUIINIOnExit;
		CAtlList<CString> slFiles, slDubs, slSubs, slFilters;
		__int64 rtStart;
		CSize fixedWindowSize;
		bool HasFixedWindowSize() {return fixedWindowSize.cx > 0 || fixedWindowSize.cy > 0;}
		// int iFixedWidth, iFixedHeight;
		int iMonitor;

		void ParseCommandLine(CAtlList<CString>& cmdln);

		bool fXpOrBetter;
		bool bDisableCenterBigOpenBmp;
		int iDXVer;
		UINT iAdminOption;

		int nCS;
		UINT tLastCheckUpdater;
		UINT tCheckUpdaterInterleave;
		bool fCheckFileAsscOnStartup;
		CString szSVPSubPerf;
		int fVMDetected ;
		CString szStartUPCheckExts;
		bool fPopupStartUpExtCheck;
		bool fHideCaptionMenu;
		int iDefaultVideoSize;
		bool fKeepAspectRatio;
		bool fCompMonDeskARDiff;
		int autoResumePlay;
		int lHardwareDecoderFailCount;

		int iDecSpeakers;
		int bUseWaveOutDeviceByDefault;
		int bNotAutoCheckSpeaker;
		int iSS;

		bool bShowControlBar;
		bool bNotChangeFontToYH;

		CRecentFileAndURLList MRU;
		CRecentFileAndURLList MRUDub;
		CRecentFileAndURLList MRUUrl;

		CAutoPtrList<FilterOverride> filters;

		CString CheckSVPSubExts;

		BOOL bSetTempChannelMaping;
		BOOL bSaveSVPSubWithVideo;
		BOOL bAeroGlass;
		BOOL bAeroGlassAvalibility;
		BOOL bTransControl;
		int iDSVideoRendererType;
		int iRMVideoRendererType;
		int iQTVideoRendererType;
		int iSVPRenderType;
		
		bool bExternalSubtitleTime;
		bool bDisableSoftCAVC;
		bool bDisableSoftCAVCForce;
		bool bDontNeedSVPSubFilter;
		bool bNoMoreDXVA;
		bool bNoMoreDXVAForThisFile;

		int iAPSurfaceUsage;
		bool fVMRSyncFix;
		bool fVMRGothSyncFix;
		int useGPUAcel;
		CStringArray szaGPUStrings;
		CString optionDecoder;
		int useGPUCUDA;
		int useFFMPEGWMV;
		int autoDownloadSVPSub;
        int autoIconvSubBig2GB;
        int autoIconvSubGB2BIG;
		int iDX9Resizer;
		bool fVMR9MixerMode;
		bool fVMR9MixerYUV;
		bool fFasterSeeking;
		int				iLanguage;

		int nVolume;
		int nBalance;
		bool fMute;
		int nLoops;
		bool fLoopForever;
		bool fRewind;
		int iZoomLevel;
		// int iVideoRendererType; 
		CStringW AudioRendererDisplayName;
		bool fAutoloadAudio;
		bool fAutoloadSubtitles;
		bool fAutoloadSubtitles2;
		bool fBlockVSFilter;
		bool fEnableWorkerThreadForOpening;
		bool fReportFailedPins;
		bool fUploadFailedPinsInfo;

		class CRendererSettingsEVR
		{
		public:
			//Genlock settings
			int bSynchronizeVideo;
			int bSynchronizeDisplay;
			int bSynchronizeNearest;
			int iLineDelta;
			int iColumnDelta;
			double fCycleDelta;
			double fTargetSyncOffset;
			double fControlLimit;
			int iVMRFlushGPUBeforeVSync;
			int iVMRFlushGPUWait;
			int iVMRFlushGPUAfterPresent;

			int iVMRDisableDesktopComposition;
			int iVMR9FullscreenGUISupport;
			int iEVRHighColorResolution;

			int iEVROutputRange;
			CRendererSettingsEVR()
			{
				bSynchronizeVideo = 0;
				bSynchronizeDisplay = 0;
				bSynchronizeNearest = 1;
				iLineDelta = 0;
				iColumnDelta = 0;
				iVMRFlushGPUBeforeVSync = 0;
				iVMRFlushGPUWait = 0;
				iVMRFlushGPUAfterPresent = 0;
				fCycleDelta = 0.0012;
				fTargetSyncOffset = 1.9;//10.0;
				fControlLimit = 2.0;

				iVMRDisableDesktopComposition = 0;
				iVMR9FullscreenGUISupport = 0;
				iEVRHighColorResolution = 0;
				iEVROutputRange = 0;
			}
			
		};

		CRendererSettingsEVR m_RenderSettings;

		double dGSubFontRatio;
		bool bUsePowerDVD;
		bool fAllowMultipleInst;
		int iTitleBarTextStyle;
		bool fTitleBarTextTitle;
		int iOnTop;
		bool fTrayIcon;
		bool fRememberZoomLevel;
		bool fShowBarsWhenFullScreen;
		int nShowBarsWhenFullScreenTimeOut;
		bool bRGBOnly;
		dispmode dmFullscreenRes;
		bool fExitFullScreenAtTheEnd;
		bool fRememberWindowPos;
		bool fRememberWindowSize;
		bool fSnapToDesktopEdges;
		CRect rcLastWindowPos;
		UINT lastWindowType;
		CSize AspectRatio;
		bool fKeepHistory;

        bool bDontDeleteOldSubFileAutomaticly;

		CString szUELastPanel;
		
		bool bHasCUDAforCoreAVC;
		bool bSupportFFGPU;
		bool bDisableEVR;
		bool useSmartDrag;
		bool onlyUseInternalDec;

		CString sDVDPath;
		bool fUseDVDPath;
		LCID idMenuLang, idAudioLang, idSubtitlesLang;
		bool fAutoSpeakerConf;
		bool fbUseSPDIF;
		bool fbSmoothMutilMonitor;

		STSStyle subdefstyle;
		STSStyle subdefstyle2;
		bool fOverridePlacement;
		bool fOverridePlacement2;
		int nHorPos, nVerPos;
		int nHorPos2, nVerPos2;
		int nSPCSize;
		int nSPCMaxRes;
		int nSubDelayInterval;
		int nSubDelayInterval2;
		bool fSPCPow2Tex;
		bool fEnableSubtitles;
		bool fEnableSubtitles2;

		CString sSubStreamName1;
		CString sSubStreamName2;

		bool fUseInternalTSSpliter;

		bool fDisabeXPToolbars;
		bool fUseWMASFReader;
		int fForceRGBrender;
		int nJumpDistS;
		int nJumpDistM;
		int nJumpDistL;
		bool fFreeWindowResizing;
		bool fNotifyMSN;
		bool fNotifyGTSdll;

		int				iEvrBuffers;

		float			dBrightness;
		float			dContrast;
		float			dHue;
		float			dSaturation;

		bool bIsIVM;
		CString szCurrentExtension;
		bool fEnableAudioSwitcher;
		bool fDownSampleTo441;
		bool fAudioTimeShift;
		int tAudioTimeShift;
		bool fCustomChannelMapping;
		bool fCustomSpeakers;
		
		float pSpeakerToChannelMap2[MAX_INPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_NORMALIZE_CHANNELS]; //Meaning [Total Channel Number] [Speaker] = 1 << Channel
		float pSpeakerToChannelMap2Custom[MAX_INPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_NORMALIZE_CHANNELS];
		float pSpeakerToChannelMapOffset[MAX_INPUT_CHANNELS][MAX_NORMALIZE_CHANNELS];
		float pEQBandControlCustom[MAX_EQ_BAND+1];
		
		int pEQBandControlPerset;

		CAtlMap<DWORD, eq_perset_setting > eqPerset;

		bool fAudioNormalize;
		bool fAudioNormalizeRecover;
		float AudioBoost;

		bool fIntRealMedia;
		// bool fRealMediaRenderless;
		int iQuickTimeRenderer;
		float RealMediaQuickTimeFPS;

		CStringArray m_pnspresets;

		HACCEL hAccel;
		//int disableSmartDrag;

		CString szOEMTitle;

		bool fWinLirc;
		CString WinLircAddr;
// 		CWinLircClient WinLircClient;
		bool fUIce;
		CString UIceAddr;
// 		CUIceClient UIceClient;

		CMediaFormats Formats;
		
		UINT SrcFilters, TraFilters, DXVAFilters, FFmpegFilters;
		BOOL bDVXACompat;

		CString logofn;
		UINT logoid;
		UINT logostretch;
		bool logoext;

		BOOL fBUltraFastMode;
		bool fHideCDROMsSubMenu;

		DWORD priority;
		bool launchfullscreen;
		bool htpcmode;
        bool startAsFullscreen;

		BOOL fEnableWebServer;
		int nWebServerPort;
		bool fWebServerPrintDebugInfo;
		bool fWebServerUseCompression;
		bool fWebServerLocalhostOnly;
		CString WebRoot, WebDefIndex;
		BYTE lAeroTransparent;
		BOOL bOldLumaControl;
		CString WebServerCGI;
		CString SVPSubStoreDir;

		CString SnapShotPath, SnapShotExt;
		int ThumbRows, ThumbCols, ThumbWidth;

		CString ISDb;

		struct Shader {CString label, target, srcdata;};
		CAtlList<Shader> m_shaders;
		CString m_shadercombine;
		int m_lTransparentToolbarPosOffset;
		int m_lTransparentToolbarPosSavedOffset;
		
		CString	strShaderList;
		CString szFGMLog;
		CAtlMap<CString, COLORREF , CStringElementTraits<CString>> colorsTheme;
		
	public:
		Settings();
		virtual ~Settings();
		void ThreadedLoading();
		void SetNumberOfSpeakers(int iSS, int iNumberOfSpeakers);
		void RegGlobalAccelKey(HWND hWnd = NULL);
		void UpdateData(bool fSave);
		BOOL bUserAeroUI();
        BOOL bIsChineseUIUser();
		BOOL bShouldUseGPUAcel();
		CString GetSVPSubStorePath();
		void InitChannelMap();
		void InitEQPerset();
		void ChangeChannelMapByCustomSetting();
		BOOL bUserAeroTitle();
		BOOL bShouldUseEVR();

		void GetFav(favtype ft, CAtlList<CString>& sl, BOOL bRecent = FALSE);
		void SetFav(favtype ft, CAtlList<CString>& sl, BOOL bRecent = FALSE);
		void DelFavByFn(favtype ft, BOOL bRecent, CString szMatch);
		void AddFav(favtype ft, CString s, BOOL bRecent = FALSE, CString szMatch = _T(""));
		COLORREF GetColorFromTheme(CString clrName, COLORREF clrDefault);

	} m_s;

public:

	SQLITE3* sqlite_setting; 
	SQLITE3* sqlite_local_record; 

	cupdatenetlib* m_cnetupdater;
	DECLARE_MESSAGE_MAP()
	afx_msg void OnAppAbout();
	afx_msg void OnFileExit();
	afx_msg void OnHelpShowcommandlineswitches();
	bool m_bMouseIn;
	bool m_bMouseInOutUnknown;
	bool m_bGenerateMouseInOutMessages;
	virtual BOOL PumpMessage();

public:

	// Retrieve an integer value from INI file or registry.
	UINT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);

	// Sets an integer value to INI file or registry.
	BOOL WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);

	// Retrieve a string value from INI file or registry.
	CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPCTSTR lpszDefault = NULL);

	// Sets a string value to INI file or registry.
	BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPCTSTR lpszValue);

	// Retrieve an arbitrary binary value from INI file or registry.
	BOOL GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPBYTE* ppData, UINT* pBytes);

	// Sets an arbitrary binary value to INI file or registry.
	BOOL WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPBYTE pData, UINT nBytes);
	
};
extern const UINT WM_MOUSEMOVEIN;
extern const UINT WM_MOUSEMOVEOUT;

#define AfxGetMyApp() ((CMPlayerCApp*)AfxGetApp())
#define AfxGetAppSettings() ((CMPlayerCApp*)AfxGetApp())->m_s
#define AppSettings CMPlayerCApp::Settings
