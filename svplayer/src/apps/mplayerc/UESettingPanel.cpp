// UESettingPanel.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "UESettingPanel.h"


// CUESettingPanel dialog

IMPLEMENT_DYNAMIC(CUESettingPanel, CDHtmlDialog)
CUESettingPanel::CUESettingPanel(IFilterGraph* pFG, CWnd* pParentWnd, UINT idPagein)
: CDHtmlDialog(CUESettingPanel::IDD, CUESettingPanel::IDH, pParentWnd)
{
	this->bOpenAdvancePanel = FALSE;
	m_pASF = FindFilter(__uuidof(CAudioSwitcherFilter), pFG);
	this->idPage = idPagein;
}

BEGIN_MESSAGE_MAP(CUESettingPanel, CDHtmlDialog)
END_MESSAGE_MAP()



BEGIN_DHTML_EVENT_MAP(CUESettingPanel)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
	DHTML_EVENT_ONCLICK(_T("ButtonApply"), OnButtonApply)
	DHTML_EVENT_ONCLICK(_T("ButtonAdvanceSetting"), OnButtonAdvanceSetting)
END_DHTML_EVENT_MAP()



CUESettingPanel::~CUESettingPanel()
{
}

void CUESettingPanel::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
	DDX_DHtml_ElementInnerText(pDX, _T("initvarblock") , m_sgs_initblock);
	DDX_DHtml_CheckBox(pDX, _T("chkremwinpos"), m_sgi_chkremwinpos);
	DDX_DHtml_CheckBox(pDX, _T("chkcdromenu"), m_sgi_chkcdromenu);
	DDX_DHtml_CheckBox(pDX, _T("chkuseini"), m_sgi_chkuseini);
	DDX_DHtml_CheckBox(pDX, _T("chkfileass"), m_sgi_chkfileass);
	DDX_DHtml_CheckBox(pDX, _T("chkplayrepeat"), m_sgi_chkplayrepeat);
	DDX_DHtml_CheckBox(pDX, _T("chkexitfullscreen"), m_sgi_chkexitfullscreen);
	DDX_DHtml_CheckBox(pDX, _T("chkabnormal"), m_sgi_chkabnormal);
	DDX_DHtml_ElementInnerHtml (pDX, _T("startupcheckexts"), m_sgi_startupcheckexts);
	
	DDX_DHtml_SelectValue( pDX, _T("videorender"), m_sgs_videorender);
	DDX_DHtml_SelectIndex( pDX, _T("videorender"), m_sgi_videorender);
	DDX_DHtml_CheckBox(pDX, _T("lockbackbuff"), m_sgi_lockbackbuff);


	DDX_DHtml_CheckBox(pDX, _T("normalize"), m_sgi_normalize);
	DDX_DHtml_CheckBox(pDX, _T("downsample44k"), m_sgi_downsample44k);
	DDX_DHtml_SelectIndex( pDX, _T("channelsetting"), m_sgi_channelsetting);

}

BOOL CUESettingPanel::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	
	//m_sgs_initblock = _T("subsetting");


 	AppSettings& s = AfxGetAppSettings();
	//Genral Setting
	m_sgi_chkexitfullscreen = s.fExitFullScreenAtTheEnd;
	m_sgi_chkremwinpos = s.fRememberWindowPos || s.fRememberWindowSize;
	m_sgi_chkcdromenu = s.fHideCDROMsSubMenu;
	m_sgi_chkplayrepeat = s.fLoopForever;
	m_sgi_chkfileass = s.fCheckFileAsscOnStartup ;
	m_sgi_chkabnormal = s.priority != NORMAL_PRIORITY_CLASS;
 	m_sgi_chkuseini = ((CMPlayerCApp*)AfxGetApp())->IsIniValid();
	m_sgi_startupcheckexts = s.szStartUPCheckExts;
	//Video Setting
	if(s.iDSVideoRendererType == 6 && s.iRMVideoRendererType == 2 && s.iQTVideoRendererType == 2){
		m_sgi_videorender = 0; //DX9
	}else if(s.iDSVideoRendererType == 5 && s.iRMVideoRendererType == 1 && s.iQTVideoRendererType == 1){
		m_sgi_videorender = 1; //DX7
	}else{
		m_sgi_videorender = 2; //自定义
	}
	m_sgi_lockbackbuff = s.fVMRSyncFix;

	//Audio Setting
	m_sgi_normalize = s.fAudioNormalize;
	m_sgi_downsample44k = s.fDownSampleTo441;

	if (s.fCustomChannelMapping == FALSE){
		m_sgi_channelsetting = 0;
	}
	 
	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CUESettingPanel::ApplyAllSetting(){
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	//Genral Setting
	if(m_sgi_chkuseini) ((CMPlayerCApp*)AfxGetApp())->StoreSettingsToIni();
	else ((CMPlayerCApp*)AfxGetApp())->StoreSettingsToRegistry();

	s.fExitFullScreenAtTheEnd = !!m_sgi_chkexitfullscreen ;
	s.fRememberWindowPos = !!m_sgi_chkremwinpos;
	s.fRememberWindowSize = !!m_sgi_chkremwinpos;
	s.fHideCDROMsSubMenu = !!m_sgi_chkcdromenu;
	s.fLoopForever = !!m_sgi_chkplayrepeat;
	s.fCheckFileAsscOnStartup = !!m_sgi_chkfileass ;
	s.priority = !m_sgi_chkabnormal ? NORMAL_PRIORITY_CLASS : GetVersion() < 0 ? HIGH_PRIORITY_CLASS : ABOVE_NORMAL_PRIORITY_CLASS;
	s.szStartUPCheckExts = m_sgi_startupcheckexts ;
	m_sgi_chkuseini = ((CMPlayerCApp*)AfxGetApp())->IsIniValid();

	//Video Setting
	
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
	s.fVMRSyncFix = !!m_sgi_lockbackbuff;

	//Audio Setting
	s.fAudioNormalize = m_sgi_normalize  ;
	s.fDownSampleTo441 = m_sgi_downsample44k ;

	switch(m_sgi_channelsetting ){
		case 0:
			s.fCustomChannelMapping = FALSE;
			break;
		case 1:
			s.fCustomChannelMapping = TRUE;
			s.pSpeakerToChannelMap[0][0] = 1;
			s.pSpeakerToChannelMap[1][0] = 1;
			s.pSpeakerToChannelMap[0][1] = 0;
			s.pSpeakerToChannelMap[1][1] = 0;
			break;
		case 2:		
			s.pSpeakerToChannelMap[0][0] = 0;
			s.pSpeakerToChannelMap[1][0] = 0;
			s.pSpeakerToChannelMap[0][1] = 1;
			s.pSpeakerToChannelMap[1][1] = 1;

		default:
			break;
	}
	//s.fCustomChannelMapping = !!m_fCustomChannelMapping;
	//memcpy(s.pSpeakerToChannelMap, m_pSpeakerToChannelMap, sizeof(m_pSpeakerToChannelMap));
	if(m_pASF)
	{
		m_pASF->SetSpeakerConfig(s.fCustomChannelMapping, s.pSpeakerToChannelMap);
		m_pASF->EnableDownSamplingTo441(s.fDownSampleTo441);
		m_pASF->SetAudioTimeShift(s.fAudioTimeShift ? 10000i64*s.tAudioTimeShift : 0);
		m_pASF->SetNormalizeBoost(s.fAudioNormalize, s.fAudioNormalizeRecover, s.AudioBoost);
	}
	s.UpdateData(true);
}

HRESULT STDMETHODCALLTYPE CUESettingPanel::GetHostInfo(DOCHOSTUIINFO *pInfo){
	pInfo->dwFlags |= DOCHOSTUIFLAG_THEME ;
	return S_OK;
}
// CUESettingPanel message handlers
HRESULT STDMETHODCALLTYPE CUESettingPanel::ShowContextMenu(DWORD /*dwID*/, POINT *ppt, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/)
{
	
	return S_OK;
}
HRESULT CUESettingPanel::OnButtonAdvanceSetting(IHTMLElement* /*pElement*/)
{
	this->bOpenAdvancePanel = TRUE;
// 	if(AfxMessageBox(_T("是否保存当前的修改"),MB_YESNO) == IDYES){
// 		ApplyAllSetting();
// 	}
// 	
	OnCancel();
	return S_OK;
}

HRESULT CUESettingPanel::OnButtonApply(IHTMLElement* /*pElement*/)
{
	this->ApplyAllSetting();
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
