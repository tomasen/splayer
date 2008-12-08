// UESettingPanel.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "UESettingPanel.h"


// CUESettingPanel dialog

IMPLEMENT_DYNCREATE(CUESettingPanel, CDHtmlDialog)

BEGIN_MESSAGE_MAP(CUESettingPanel, CDHtmlDialog)
END_MESSAGE_MAP()



BEGIN_DHTML_EVENT_MAP(CUESettingPanel)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
	DHTML_EVENT_ONCLICK(_T("ButtonApply"), OnButtonApply)
	DHTML_EVENT_ONCLICK(_T("ButtonAdvanceSetting"), OnButtonAdvanceSetting)
END_DHTML_EVENT_MAP()

CUESettingPanel::CUESettingPanel(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CUESettingPanel::IDD, CUESettingPanel::IDH, pParent)
{
	this->bOpenAdvancePanel = FALSE;
}

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
	
	
}

BOOL CUESettingPanel::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	
	//m_sgs_initblock = _T("subsetting");


 	AppSettings& s = AfxGetAppSettings();
	m_sgi_chkexitfullscreen = s.fExitFullScreenAtTheEnd;
	m_sgi_chkremwinpos = s.fRememberWindowPos || s.fRememberWindowSize;
	m_sgi_chkcdromenu = s.fHideCDROMsSubMenu;
	m_sgi_chkplayrepeat = s.fLoopForever;
	m_sgi_chkfileass = s.fCheckFileAsscOnStartup ;
	m_sgi_chkabnormal = s.priority != NORMAL_PRIORITY_CLASS;
 	m_sgi_chkuseini = ((CMPlayerCApp*)AfxGetApp())->IsIniValid();
	
	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CUESettingPanel::ApplyAllSetting(){
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	if(m_sgi_chkuseini) ((CMPlayerCApp*)AfxGetApp())->StoreSettingsToIni();
	else ((CMPlayerCApp*)AfxGetApp())->StoreSettingsToRegistry();

	s.fExitFullScreenAtTheEnd = !!m_sgi_chkexitfullscreen ;
	s.fRememberWindowPos = !!m_sgi_chkremwinpos;
	s.fRememberWindowSize = !!m_sgi_chkremwinpos;
	s.fHideCDROMsSubMenu = !!m_sgi_chkcdromenu;
	s.fLoopForever = !!m_sgi_chkplayrepeat;
	s.fCheckFileAsscOnStartup = !!m_sgi_chkfileass ;
	s.priority = !m_sgi_chkabnormal ? NORMAL_PRIORITY_CLASS : GetVersion() < 0 ? HIGH_PRIORITY_CLASS : ABOVE_NORMAL_PRIORITY_CLASS;
	
	m_sgi_chkuseini = ((CMPlayerCApp*)AfxGetApp())->IsIniValid();

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
