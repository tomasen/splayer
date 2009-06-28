// UESettingPanel.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "UESettingPanel.h"
#include "PPageAccelTbl.h"
#include "PPageFormats.h"
#include "PPageLogo.h"
#include "MainFrm.h"
#include "..\..\svplib\svplib.h"
#include "..\..\svplib\svptoolbox.h"

// CUESettingPanel dialog

IMPLEMENT_DYNAMIC(CUESettingPanel, CDHtmlDialog)
CUESettingPanel::CUESettingPanel(IFilterGraph* pFG, CWnd* pParentWnd, UINT idPagein)
: CDHtmlDialog(CUESettingPanel::IDD, CUESettingPanel::IDH, pParentWnd)
{
	this->bOpenAdvancePanel = FALSE;
	m_pASF = FindFilter(__uuidof(CAudioSwitcherFilter), pFG);
	this->idPage = idPagein;
	m_stss = AfxGetAppSettings().subdefstyle;
	m_stss2 = AfxGetAppSettings().subdefstyle2;

}

BEGIN_MESSAGE_MAP(CUESettingPanel, CDHtmlDialog)
END_MESSAGE_MAP()



BEGIN_DHTML_EVENT_MAP(CUESettingPanel)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
	DHTML_EVENT_ONCLICK(_T("ButtonApply"), OnButtonApply)
	DHTML_EVENT_ONCLICK(_T("ButtonAdvanceSetting"), OnButtonAdvanceSetting)
	DHTML_EVENT_ONCLICK(_T("sub1c1"), OnColorSub)
	DHTML_EVENT_ONCLICK(_T("sub1c2"), OnColorSub)
	DHTML_EVENT_ONCLICK(_T("sub1c3"), OnColorSub)
	DHTML_EVENT_ONCLICK(_T("sub1c4"), OnColorSub)
	DHTML_EVENT_ONCLICK(_T("sub2c1"), OnColorSub)
	DHTML_EVENT_ONCLICK(_T("sub2c2"), OnColorSub)
	DHTML_EVENT_ONCLICK(_T("sub2c3"), OnColorSub)
	DHTML_EVENT_ONCLICK(_T("sub2c4"), OnColorSub)
	DHTML_EVENT_ONCLICK(_T("subfont1"), OnFontSetting)
	DHTML_EVENT_ONCLICK(_T("subfont2"), OnFontSetting)
	DHTML_EVENT_ONCLICK(_T("IDFileAss"), OnFileAss)
	DHTML_EVENT_ONCLICK(_T("IDHotKey"), OnHotKey)
	DHTML_EVENT_ONCLICK(_T("IDBGCHG"), OnChangeBG)
	DHTML_EVENT_ONCLICK(_T("ButtonReset"), OnButtonReset)
	
END_DHTML_EVENT_MAP()



CUESettingPanel::~CUESettingPanel()
{
}

void CUESettingPanel::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);

	DDX_DHtml_CheckBox(pDX, _T("chkremhistory") , m_sgs_chkremhistory);
	DDX_DHtml_ElementInnerText(pDX, _T("initvarblock") , m_sgs_initblock);
	DDX_DHtml_ElementInnerText(pDX, _T("haveCUDAforCoreAVC") , m_sgs_CUDAVC);
	DDX_DHtml_ElementInnerText(pDX, _T("canUseFFMPEGGPU") , m_sgs_FFGPU);
	DDX_DHtml_CheckBox(pDX, _T("chkremwinpos"), m_sgi_chkremwinpos);
	DDX_DHtml_CheckBox(pDX, _T("chkcdromenu"), m_sgi_chkcdromenu);
	DDX_DHtml_CheckBox(pDX, _T("chkuseini"), m_sgi_chkuseini);
	DDX_DHtml_CheckBox(pDX, _T("chkfileass"), m_sgi_chkfileass);
	DDX_DHtml_CheckBox(pDX, _T("chkplayrepeat"), m_sgi_chkplayrepeat);
	DDX_DHtml_CheckBox(pDX, _T("chkexitfullscreen"), m_sgi_chkexitfullscreen);
	DDX_DHtml_CheckBox(pDX, _T("chkabnormal"), m_sgi_chkabnormal);
	DDX_DHtml_CheckBox(pDX, _T("noaudioboost"), m_sgi_noaudioboost);
	DDX_DHtml_CheckBox(pDX, _T("chkautozoom"), m_sgi_chkautozoom);
	DDX_DHtml_CheckBox(pDX, _T("chkautodownloadsvpsub"), m_sgi_chkautodownloadsvpsub);
	DDX_DHtml_CheckBox(pDX, _T("chkautoresumeplay"), m_sgi_chkautoresumeplay);
	DDX_DHtml_CheckBox(pDX, _T("chktrayicon"), m_sgi_chktrayicon);

	DDX_DHtml_CheckBox(pDX, _T("chkusesmartdrag"), m_sgi_chkuseSmartDrag);
	
	DDX_DHtml_ElementValue (pDX, _T("stepsmall"), m_sgs_stepsmall);
	DDX_DHtml_ElementValue (pDX, _T("stepmed"), m_sgs_stepmed);
	DDX_DHtml_ElementValue (pDX, _T("stepbig"), m_sgs_stepbig);

	DDX_DHtml_SelectValue( pDX, _T("speaker"), m_sgs_speaker);
	DDX_DHtml_CheckBox(pDX, _T("UseWaveOutDeviceByDefault"), m_sgi_UseWaveOutDeviceByDefault);

	DDX_DHtml_ElementInnerHtml (pDX, _T("startupcheckexts"), m_sgi_startupcheckexts);
	DDX_DHtml_ElementInnerHtml (pDX, _T("gpulist"), m_sgs_gpulist);
	DDX_DHtml_ElementInnerHtml (pDX, _T("decodervalue"), m_sgs_decoderinitvalue);

	DDX_DHtml_ElementValue (pDX, _T("subfont1"), m_sgs_subfont1);
	DDX_DHtml_SelectValue( pDX, _T("subalign1"), m_sgs_subalign1);
	DDX_DHtml_CheckBox(pDX, _T("suboveride1"), m_sgi_suboveride1);
	DDX_DHtml_ElementValue (pDX, _T("subhpos1"), m_sgs_subhpos1);
	DDX_DHtml_ElementValue (pDX, _T("subvpos1"), m_sgs_subvpos1);
	DDX_DHtml_ElementValue (pDX, _T("engsizeratio1"), m_sgs_engsubradio1);

	DDX_DHtml_ElementValue (pDX, _T("subfont2"), m_sgs_subfont2);
	DDX_DHtml_SelectValue( pDX, _T("subalign2"), m_sgs_subalign2);
	DDX_DHtml_CheckBox(pDX, _T("suboveride2"), m_sgi_suboveride2);
	DDX_DHtml_ElementValue (pDX, _T("subhpos2"), m_sgs_subhpos2);
	DDX_DHtml_ElementValue (pDX, _T("subvpos2"), m_sgs_subvpos2);
	DDX_DHtml_ElementValue (pDX, _T("engsizeratio2"), m_sgs_engsubradio2);

	DDX_DHtml_SelectValue( pDX, _T("videorender"), m_sgs_videorender);
	DDX_DHtml_SelectValue( pDX, _T("decoder"), m_sgs_decoder);
	DDX_DHtml_SelectIndex( pDX, _T("videorender"), m_sgi_videorender);
	DDX_DHtml_CheckBox(pDX, _T("lockbackbuff"), m_sgi_lockbackbuff);
	DDX_DHtml_CheckBox(pDX, _T("gpuacelbase"), m_sgi_gpuacel);
	DDX_DHtml_CheckBox(pDX, _T("internaltspliteronly"), m_sgi_chkinternaltspliteronly);

	DDX_DHtml_CheckBox(pDX, _T("vmr9mixer"), m_sgi_uservmrmixer);
	
	DDX_DHtml_CheckBox(pDX, _T("usespdif"), m_sgi_usespdif);


	DDX_DHtml_CheckBox(pDX, _T("normalize"), m_sgi_normalize);
	DDX_DHtml_CheckBox(pDX, _T("downsample44k"), m_sgi_downsample44k);
	DDX_DHtml_SelectIndex( pDX, _T("channelsetting"), m_sgi_channelsetting);

	DDX_DHtml_CheckBox(pDX, _T("chkautoupdate"), m_sgi_autoupdate );
	DDX_DHtml_SelectValue(pDX, _T("updateversion"), m_sgs_updateversion);

}
HRESULT CUESettingPanel::OnColorSub(IHTMLElement *pElement){

	CString szId = this->GetIDfromElement(pElement);
	if(m_sgs_updateversion.IsEmpty()){
		m_sgs_updateversion = _T("stable");
	}

	int iSub, iCol;
	swscanf(szId, _T("sub%dc%d") , &iSub, &iCol);
	if ( iCol > 0 && iCol < 5)
		iCol--;
	else
		return S_FALSE;

	STSStyle sts;
	if(iSub == 2){sts = m_stss2;}else{sts = m_stss;}

	int i = 0;
	CColorDialog dlg(sts.colors[iCol], CC_ANYCOLOR|CC_FULLOPEN, this);
	//dlg.m_cc.Flags |= CC_FULLOPEN;
	if(dlg.DoModal() == IDOK)
	{
		if(iSub == 2){
			m_stss2.colors[iCol] = dlg.m_cc.rgbResult;
		}else{
			m_stss.colors[iCol] = dlg.m_cc.rgbResult;
		}
		this->setBackgdColorByID( szId , dlg.m_cc.rgbResult);
	}
	return S_OK;
}
CString CUESettingPanel::GetIDfromElement(IHTMLElement *pElement){
	CComBSTR sId;
	pElement->get_id( &sId );
	return CString(sId);
}
HRESULT CUESettingPanel::OnFontSetting(IHTMLElement *pElement){


	CString szId = this->GetIDfromElement(pElement);

	LOGFONT lf;
	if (szId == _T("subfont2") ){lf <<= m_stss2;}else{lf <<= m_stss;}
	STSStyle sts ;

	CFontDialog dlg(&lf, CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT|CF_FORCEFONTEXIST|CF_SCALABLEONLY|CF_EFFECTS);
	if(dlg.DoModal() == IDOK)
	{
		CString str;
		sts = lf;
		str.Format( _T("%s(%d)"), lf.lfFaceName ,sts.fontSize );
		
		if (szId == _T("subfont2") ){m_stss2 = lf; m_sgs_subfont2 = str;}else{	m_stss = lf; m_sgs_subfont1 = str;}
		UpdateData(FALSE);
		
	}
	return S_OK;
}


BOOL CUESettingPanel::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	switch(this->idPage){
		case IDD_PPAGEAUDIOSWITCHER:
			m_sgs_initblock = _T("audiosetting");
			break;
		case IDD_PPAGESUBTITLES:
			m_sgs_initblock = _T("subsetting");
			break;
	}

	AppSettings& s = AfxGetAppSettings();

	if(s.bHasCUDAforCoreAVC){
		m_sgs_CUDAVC = _T("true");
	}
	if(s.bSupportFFGPU){
		m_sgs_FFGPU = _T("true");
	}

	//if(!s.szaGPUStrings.GetCount()){
	CSVPToolBox svpTool;
	int bGPUPerfer = svpTool.GetGPUString(&s.szaGPUStrings);
	//}
	m_sgs_gpulist = L"您拥有的显卡有：<br/>";
	for(int i = 0; i < s.szaGPUStrings.GetCount();i++){
		m_sgs_gpulist.Append(s.szaGPUStrings[i] + _T("<br/>"));
	}
	if(bGPUPerfer){
		m_sgs_gpulist.Append(L"适宜使用GPU加速");
	}else{
		m_sgs_gpulist.Append(L"可能不适宜使用GPU加速 <a class='small' href='https://bbs.shooter.cn/forumdisplay.php?fid=6'>报告错误</a>");
	}

	//Genral Setting
	m_sgs_chkremhistory = s.fKeepHistory;
	m_sgi_chkexitfullscreen = s.fExitFullScreenAtTheEnd;
	m_sgi_chkremwinpos = s.fRememberWindowPos || s.fRememberWindowSize;
	m_sgi_chkcdromenu = s.fHideCDROMsSubMenu;
	m_sgi_chkplayrepeat = s.fLoopForever;
	m_sgi_chkfileass = s.fCheckFileAsscOnStartup ;
	m_sgi_chkabnormal = s.priority != NORMAL_PRIORITY_CLASS;
 	m_sgi_chkuseini = ((CMPlayerCApp*)AfxGetApp())->IsIniValid();
	m_sgi_startupcheckexts = s.szStartUPCheckExts;
	m_sgi_chkautoresumeplay = s.autoResumePlay;
	m_sgi_chkuseSmartDrag = s.useSmartDrag;
	m_sgi_chktrayicon = s.fTrayIcon;
	
	m_sgs_stepsmall.Format(_T("%d"), s.nJumpDistS/1000) ;
	m_sgs_stepmed.Format(_T("%d"), s.nJumpDistM/1000) ;
	m_sgs_stepbig.Format(_T("%d"), s.nJumpDistL/1000) ;

	//Video Setting
	m_sgi_uservmrmixer = s.fVMR9MixerMode;
	if(s.iDSVideoRendererType == 6 && s.iRMVideoRendererType == 2 && s.iQTVideoRendererType == 2){
		m_sgi_videorender = 0; //DX9
	}else if(s.iDSVideoRendererType == 5 && s.iRMVideoRendererType == 1 && s.iQTVideoRendererType == 1){
		m_sgi_videorender = 1; //DX7
	}else{
		m_sgi_videorender = 2; //自定义
	}
	m_sgi_lockbackbuff = s.fVMRSyncFix;
	m_sgi_gpuacel = s.useGPUAcel;
	m_sgs_decoder = s.optionDecoder;
	if(m_sgs_decoder.IsEmpty()){
		if(m_sgi_gpuacel){
			m_sgs_decoder = _T("internalGPUdec");
		}else{
			m_sgs_decoder = _T("internaldec");
		}
	}
	m_sgs_decoderinitvalue = m_sgs_decoder;
	m_sgi_chkinternaltspliteronly = s.fUseInternalTSSpliter;
	

	//Audio Setting
	m_sgi_normalize = s.fAudioNormalize;
	if( s.AudioBoost > 1 ){
		m_sgi_noaudioboost = 0;
	}
	m_sgi_UseWaveOutDeviceByDefault = s.bUseWaveOutDeviceByDefault;
	m_sgs_speaker.Format(_T("%d") ,  s.iDecSpeakers % 1000 );
	m_sgi_usespdif = !!( s.iDecSpeakers >= 1000 );
	//m_sgi_downsample44k = s.fDownSampleTo441;

	m_sgi_chkautozoom = s.fRememberZoomLevel;
	if (s.fCustomChannelMapping == FALSE){
		m_sgi_channelsetting = 0;
	}

	 //Sub Setting
	m_sgi_chkautodownloadsvpsub = s.autoDownloadSVPSub;
	m_sgs_subfont1.Format( _T("%s(%d)"), s.subdefstyle.fontName , (INT)s.subdefstyle.fontSize);
	m_sgs_subalign1.Format( _T("%d") , s.subdefstyle.scrAlignment ); 
	m_sgi_suboveride1 = s.fOverridePlacement;
	m_sgs_subhpos1.Format( _T("%d") , s.nHorPos );
	m_sgs_subvpos1.Format( _T("%d") , s.nVerPos );
	m_sgs_engsubradio1.Format(_T("%.3f"), s.subdefstyle.engRatio);

	m_sgs_subfont2.Format( _T("%s(%d)"), s.subdefstyle2.fontName , (INT)s.subdefstyle2.fontSize);
	m_sgs_subalign2.Format( _T("%d") , s.subdefstyle2.scrAlignment );
	m_sgi_suboveride2 = s.fOverridePlacement2;
	m_sgs_subhpos2.Format( _T("%d") , s.nHorPos2 );
	m_sgs_subvpos2.Format( _T("%d") , s.nVerPos2 );
	m_sgs_engsubradio2.Format(_T("%.3f"), s.subdefstyle2.engRatio);

	m_sgi_autoupdate = (s.tLastCheckUpdater < 2000000000);//

	if(CMPlayerCApp::IsVista()){
		DisplayNodeByID(_T("disableevrline"), FALSE);
	}

	UpdateData(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUESettingPanel::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CDHtmlDialog::OnDocumentComplete(pDisp, szUrl);
	setBackgdColorByID(_T("sub1c1"), m_stss.colors[0]);
	setBackgdColorByID(_T("sub1c2"), m_stss.colors[1]);
	setBackgdColorByID(_T("sub1c3"), m_stss.colors[2]);
	setBackgdColorByID(_T("sub1c4"), m_stss.colors[3]);
	setBackgdColorByID(_T("sub2c1"), m_stss2.colors[0]);
	setBackgdColorByID(_T("sub2c2"), m_stss2.colors[1]);
	setBackgdColorByID(_T("sub2c3"), m_stss2.colors[2]);
	setBackgdColorByID(_T("sub2c4"), m_stss2.colors[3]);

}
COLORREF CUESettingPanel::getBackgdColorByID(CString szId){
	IHTMLElement *pElement;
	GetElement( szId, &pElement	);
	if (pElement){
		IHTMLStyle *phtmlStyle;
		pElement->get_style(&phtmlStyle);
		if (phtmlStyle)
		{
			VARIANT varColor;
			varColor.vt = VT_I4;
			phtmlStyle->get_backgroundColor(&varColor);
			phtmlStyle->Release();
			return   RGB( GetBValue(varColor.lVal) ,  GetGValue(varColor.lVal) ,  GetRValue(varColor.lVal));
			
		} 
	}
	return 0;
}
void CUESettingPanel::setBackgdColorByID(CString szId, COLORREF color){
	IHTMLElement *pElement;
	GetElement( szId, &pElement	);
	if (pElement){
		IHTMLStyle *phtmlStyle;
		pElement->get_style(&phtmlStyle);
		if (phtmlStyle)
		{
			VARIANT varColor;
			varColor.vt = VT_I4;
			varColor.lVal =  RGB( GetBValue(color) ,  GetGValue(color) ,  GetRValue(color));
			phtmlStyle->put_backgroundColor(varColor);
			phtmlStyle->Release();
		} 
	}
}
void CUESettingPanel::DisplayNodeByID(CString szId, BOOL bBlock){
	IHTMLElement *pElement;
	GetElement( szId, &pElement	);
	if (pElement){
		IHTMLStyle *phtmlStyle;
		pElement->get_style(&phtmlStyle);
		if (phtmlStyle)
		{
			if(bBlock)
				phtmlStyle->put_display(_T("block"));
			else
				phtmlStyle->put_display(_T("inline"));
			
		} 
	}
}
#include "..\..\filters\transform\mpadecfilter\a52dec-0.7.4\include\a52.h"
#include "..\..\filters\transform\mpadecfilter\dtsdec-0.0.1\include\dts.h"

void CUESettingPanel::ApplyAllSetting(){
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	//Genral Setting
	if(m_sgi_chkuseini) ((CMPlayerCApp*)AfxGetApp())->StoreSettingsToIni();
	else ((CMPlayerCApp*)AfxGetApp())->StoreSettingsToRegistry();

	s.fKeepHistory = !!m_sgs_chkremhistory ;
	if(!s.fKeepHistory){
		for(int i = 0; i < s.MRU.GetSize(); i++) s.MRU.Remove(i);
		for(int i = 0; i < s.MRUDub.GetSize(); i++) s.MRUDub.Remove(i);
		for(int i = 0; i < s.MRUUrl.GetSize(); i++) s.MRUUrl.Remove(i);
		s.MRU.WriteList();
		s.MRUDub.WriteList();
		s.MRUUrl.WriteList();
	}

	s.autoResumePlay = !!m_sgi_chkautoresumeplay;
	if(!s.autoResumePlay ){
		CAtlList<CString> sl;
		s.SetFav(FAV_FILE, sl, 1);
		s.SetFav(FAV_DVD, sl, 1);
	}
	s.fExitFullScreenAtTheEnd = !!m_sgi_chkexitfullscreen ;
	s.fRememberWindowPos = !!m_sgi_chkremwinpos;
	s.fRememberWindowSize = !!m_sgi_chkremwinpos;
	s.fHideCDROMsSubMenu = !!m_sgi_chkcdromenu;
	s.fLoopForever = !!m_sgi_chkplayrepeat;
	s.nLoops = 1;
	s.fCheckFileAsscOnStartup = !!m_sgi_chkfileass ;
	//s.priority = !m_sgi_chkabnormal ? NORMAL_PRIORITY_CLASS : GetVersion() < 0 ? HIGH_PRIORITY_CLASS : ABOVE_NORMAL_PRIORITY_CLASS;
	s.szStartUPCheckExts = m_sgi_startupcheckexts ;
	m_sgi_chkuseini = ((CMPlayerCApp*)AfxGetApp())->IsIniValid();
	s.fRememberZoomLevel = !!m_sgi_chkautozoom ;
	s.useSmartDrag = !!m_sgi_chkuseSmartDrag ;
	s.fUseInternalTSSpliter = m_sgi_chkinternaltspliteronly;
	
	s.nJumpDistS = _wtof(m_sgs_stepsmall) * 1000;
	s.nJumpDistM = _wtof(m_sgs_stepmed) * 1000;
	s.nJumpDistL = _wtof(m_sgs_stepbig) * 1000;
	
	//Video Setting
	s.bDisableEVR = !m_sgi_gpuacel;

	s.fVMR9MixerMode = m_sgi_uservmrmixer ;
	if(s.fVMR9MixerMode){
		m_sgs_videorender = _T("DX9");
	}
	if(m_sgs_videorender == _T("DX9")){
		s.iDSVideoRendererType = 6;
		s.iRMVideoRendererType = 2;
		s.iQTVideoRendererType = 2;
	}else if(m_sgs_videorender == _T("DX7")){
		s.iDSVideoRendererType = 5;
		s.iRMVideoRendererType = 1;
		s.iQTVideoRendererType = 1;
	}else{

	}
	s.fTrayIcon = m_sgi_chktrayicon;
	s.fVMRSyncFix = !!m_sgi_lockbackbuff;
	s.useGPUAcel = !!m_sgi_gpuacel;
	s.optionDecoder = m_sgs_decoder;
	s.useGPUCUDA = 0;
	s.onlyUseInternalDec = 0;		


	if(m_sgs_decoder == _T("internalGPUdec") ){
			s.onlyUseInternalDec = 1;
			s.DXVAFilters = ~0;
	}else if(m_sgs_decoder == _T("PDVDGPUdec") ){
	}else if(m_sgs_decoder == _T("CoreAVCGPUdec") ){
			s.useGPUCUDA = 1;
			s.DXVAFilters = ~0;
	}else if(m_sgs_decoder == _T("internaldec") ){
			s.DXVAFilters = 0;
			s.onlyUseInternalDec = 1;
		
	}else if(m_sgs_decoder == _T("CoreAVCdec") ){
		s.DXVAFilters = 0;
	}

	//Audio Setting
	s.fAudioNormalize = !!m_sgi_normalize  ;
	s.fAudioNormalizeRecover = TRUE;
// 	if(m_sgi_noaudioboost)
// 		s.AudioBoost = 1;
// 	else
// 		s.AudioBoost = 70;
	//s.fDownSampleTo441 = !!m_sgi_downsample44k ;

/*
	switch(m_sgi_channelsetting ){
		case 0:
			s.fCustomChannelMapping = FALSE;
			break;
		case 1:
			s.fCustomChannelMapping = TRUE;
			s.pSpeakerToChannelMap[1][0] = 1;
			s.pSpeakerToChannelMap[1][1] = 1;
			break;
		case 2:		
			s.fCustomChannelMapping = TRUE;
			s.pSpeakerToChannelMap[1][0] = 1<<1;
			s.pSpeakerToChannelMap[1][1] = 1<<1;

		default:
			break;
	}*/

	//s.fCustomChannelMapping = !!m_fCustomChannelMapping;
	//memcpy(s.pSpeakerToChannelMap, m_pSpeakerToChannelMap, sizeof(m_pSpeakerToChannelMap));
	if(m_pASF)
	{
		//m_pASF->SetSpeakerConfig(s.fCustomChannelMapping, s.pSpeakerToChannelMap);
		//m_pASF->EnableDownSamplingTo441(s.fDownSampleTo441);
		//m_pASF->SetAudioTimeShift(s.fAudioTimeShift ? 10000i64*s.tAudioTimeShift : 0);
		m_pASF->SetNormalizeBoost(s.fAudioNormalize, s.fAudioNormalizeRecover, s.AudioBoost);
	}

	s.bUseWaveOutDeviceByDefault = m_sgi_UseWaveOutDeviceByDefault ;
	
	{
		int iSS = _wtoi(m_sgs_speaker);
		int iFRS = iSS/10;
		BOOL iLFE = iSS % 10;
		m_aacdownmix = FALSE;

		switch(iFRS){
			case 20:
				m_ac3spkcfg = A52_STEREO ;
				m_dtsspkcfg = DTS_STEREO ;
				m_aacdownmix = TRUE;
				break;
			case 21:
				m_ac3spkcfg = A52_2F1R  ;
				m_dtsspkcfg = DTS_2F1R  ;
				m_aacdownmix = TRUE;
				break;
			case 22:
				m_ac3spkcfg = A52_2F2R  ;
				m_dtsspkcfg = DTS_2F2R  ;
				break;
			case 30:
				m_ac3spkcfg = A52_3F  ;
				m_dtsspkcfg = DTS_3F  ;
				m_aacdownmix = TRUE;
				break;
			case 31:
				m_ac3spkcfg = A52_3F1R  ;
				m_dtsspkcfg = DTS_3F1R  ;
				m_aacdownmix = TRUE;
				break;
			case 32:
				m_ac3spkcfg = A52_3F2R  ;
				m_dtsspkcfg = DTS_3F2R  ;
				break;
			default:
				m_ac3spkcfg = A52_STEREO ;
				m_dtsspkcfg = DTS_STEREO ;
				m_aacdownmix = TRUE;
				break;
			
		}

		if(iLFE){
			m_ac3spkcfg |= A52_LFE;
			m_dtsspkcfg |= DTS_LFE;
		}
		if(m_sgi_usespdif){
			iSS += 1000;
			m_ac3spkcfg = -m_ac3spkcfg;
			m_dtsspkcfg = -m_dtsspkcfg;
		}
		s.iDecSpeakers = iSS;
		if(m_pMDF){
			//m_pMDF->SetSampleFormat((MPCSampleFormat)m_outputformat);
			m_pMDF->SetSpeakerConfig(IMpaDecFilter::ac3, m_ac3spkcfg);
			//m_pMDF->SetDynamicRangeControl(IMpaDecFilter::ac3, m_ac3drc);
			m_pMDF->SetSpeakerConfig(IMpaDecFilter::dts, m_dtsspkcfg);
			//m_pMDF->SetDynamicRangeControl(IMpaDecFilter::dts, m_dtsdrc);
			m_pMDF->SetSpeakerConfig(IMpaDecFilter::aac, m_aacdownmix);
		}

		CRegKey key;
		if(ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, _T("Software\\SPlayer\\Filters\\MPEG Audio Decoder")))
		{
			key.SetDWORDValue(_T("Ac3SpeakerConfig"), m_ac3spkcfg);
			key.SetDWORDValue(_T("DtsSpeakerConfig"), m_dtsspkcfg);
			key.SetDWORDValue(_T("AacSpeakerConfig"), m_aacdownmix);
			
		}
		
	}

	//Sub Setting
	s.autoDownloadSVPSub = m_sgi_chkautodownloadsvpsub ;
	s.fOverridePlacement = !!m_sgi_suboveride1  ;
	s.fOverridePlacement2 = !!m_sgi_suboveride2  ;
	
	m_stss.engRatio = _wtof( m_sgs_engsubradio1 );
	m_stss2.engRatio = _wtof( m_sgs_engsubradio2 );

	m_stss.scrAlignment = _wtoi(m_sgs_subalign1);
	m_stss2.scrAlignment = _wtoi(m_sgs_subalign2);
	
	if ( ( (m_stss.fontName == _T("微软雅黑") || m_stss.fontName == _T("Microsoft YaHei") ) && s.subdefstyle.fontName != m_stss.fontName ) 
			|| ( (m_stss2.fontName == _T("微软雅黑") || m_stss2.fontName == _T("Microsoft YaHei") ) && s.subdefstyle2.fontName != m_stss.fontName ) )
	{
		s.bNotChangeFontToYH = TRUE;
	}
	s.subdefstyle = m_stss;
	s.subdefstyle2 = m_stss2;

	s.nHorPos = _wtoi(m_sgs_subhpos1);
	s.nVerPos = _wtoi(m_sgs_subvpos1);
	s.nHorPos2 = _wtoi(m_sgs_subhpos2);
	s.nVerPos2 = _wtoi(m_sgs_subvpos2);
	
	if(m_sgi_autoupdate){
	 s.tLastCheckUpdater = (UINT)time(NULL) - 100000;
	}else{
	 s.tLastCheckUpdater = 2000000000;
	}
	s.useGPUCUDA = SVP_SetCoreAvcCUDA(s.useGPUCUDA);
	s.UpdateData(true);
}

HRESULT STDMETHODCALLTYPE CUESettingPanel::GetHostInfo(DOCHOSTUIINFO *pInfo){
	pInfo->dwFlags |= DOCHOSTUIFLAG_THEME | DOCHOSTUIFLAG_SCROLL_NO ;
	return S_OK;
}
// CUESettingPanel message handlers
HRESULT STDMETHODCALLTYPE CUESettingPanel::ShowContextMenu(DWORD /*dwID*/, POINT *ppt, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/)
{
	
	return S_OK;
}
HRESULT CUESettingPanel::OnButtonAdvanceSetting(IHTMLElement* /*pElement*/)
{
	if(AfxMessageBox(_T("射手播放器并不推荐您使用旧的“高级设置”面板\r\n不当的操作可能会带来难以预知的问题\r\n确定要继续么？"),MB_YESNO|MB_DEFBUTTON2 ) == IDYES){
		this->bOpenAdvancePanel = TRUE;
	// 	if(AfxMessageBox(_T("是否保存当前的修改"),MB_YESNO) == IDYES){
	// 		ApplyAllSetting();
	// 	}
	// 	
		OnCancel();
	}
	return S_OK;
}
HRESULT CUESettingPanel::OnButtonReset(IHTMLElement* /*pElement*/)
{
	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	if(pFrame){
		pFrame->PostMessage(WM_COMMAND, ID_RESET_SETTING);
	}
	OnCancel();
	return S_OK;
}
HRESULT CUESettingPanel::OnButtonApply(IHTMLElement* /*pElement*/)
{
	this->ApplyAllSetting();
	return S_OK;
}
HRESULT CUESettingPanel::OnHotKey(IHTMLElement *pElement){
	CAutoPtr<CPPageAccelTbl> page(new CPPageAccelTbl());
	CPropertySheet dlg(_T("快捷键设置..."), this);
	dlg.AddPage(page);
	dlg.DoModal() ;

	return S_OK;
}
HRESULT CUESettingPanel::OnChangeBG(IHTMLElement* /*pElement*/){
	CAutoPtr<CPPageLogo> page(new CPPageLogo());
	CPropertySheet dlg(_T("界面背景设置..."), this);
	dlg.AddPage(page);
	dlg.DoModal() ;
	return S_OK;
}

HRESULT CUESettingPanel::OnFileAss(IHTMLElement* /*pElement*/){
	CAutoPtr<CPPageFormats> page(new CPPageFormats());
	CPropertySheet dlg(_T("文件关联设置..."), this);
	dlg.AddPage(page);
	dlg.DoModal() ;
	return S_OK;
}
HRESULT CUESettingPanel::OnButtonOK(IHTMLElement* /*pElement*/)
{
	this->ApplyAllSetting();
	OnOK();
	return S_OK;
}
HRESULT CUESettingPanel::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}
