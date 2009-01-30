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
	DDX_DHtml_CheckBox(pDX, _T("chkautozoom"), m_sgi_chkautozoom);
	
	DDX_DHtml_ElementInnerHtml (pDX, _T("startupcheckexts"), m_sgi_startupcheckexts);
	
	DDX_DHtml_ElementValue (pDX, _T("subfont1"), m_sgs_subfont1);
	DDX_DHtml_SelectValue( pDX, _T("subalign1"), m_sgs_subalign1);
	DDX_DHtml_CheckBox(pDX, _T("suboveride1"), m_sgi_suboveride1);
	DDX_DHtml_ElementValue (pDX, _T("subhpos1"), m_sgs_subhpos1);
	DDX_DHtml_ElementValue (pDX, _T("subvpos1"), m_sgs_subvpos1);

	DDX_DHtml_ElementValue (pDX, _T("subfont2"), m_sgs_subfont2);
	DDX_DHtml_SelectValue( pDX, _T("subalign2"), m_sgs_subalign2);
	DDX_DHtml_CheckBox(pDX, _T("suboveride2"), m_sgi_suboveride2);
	DDX_DHtml_ElementValue (pDX, _T("subhpos2"), m_sgs_subhpos2);
	DDX_DHtml_ElementValue (pDX, _T("subvpos2"), m_sgs_subvpos2);

	DDX_DHtml_SelectValue( pDX, _T("videorender"), m_sgs_videorender);
	DDX_DHtml_SelectIndex( pDX, _T("videorender"), m_sgi_videorender);
	DDX_DHtml_CheckBox(pDX, _T("lockbackbuff"), m_sgi_lockbackbuff);
	DDX_DHtml_CheckBox(pDX, _T("gpuacel"), m_sgi_gpuacel);


	DDX_DHtml_CheckBox(pDX, _T("normalize"), m_sgi_normalize);
	DDX_DHtml_CheckBox(pDX, _T("downsample44k"), m_sgi_downsample44k);
	DDX_DHtml_SelectIndex( pDX, _T("channelsetting"), m_sgi_channelsetting);

}
HRESULT CUESettingPanel::OnColorSub(IHTMLElement *pElement){

	CString szId = this->GetIDfromElement(pElement);

	int iSub, iCol;
	swscanf(szId, _T("sub%dc%d") , &iSub, &iCol);
	if ( iCol > 0 && iCol < 5)
		iCol--;
	else
		return S_FALSE;

	STSStyle sts;
	if(iSub == 2){sts = m_stss2;}else{sts = m_stss;}

	int i = 0;
	CColorDialog dlg(sts.colors[iCol]);
	dlg.m_cc.Flags |= CC_FULLOPEN;
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
	
	if (this->idPage == IDD_PPAGEAUDIOSWITCHER){
		m_sgs_initblock = _T("audiosetting");
	}


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
	m_sgi_gpuacel = s.useGPUAcel;
	//Audio Setting
	m_sgi_normalize = s.fAudioNormalize;
	m_sgi_downsample44k = s.fDownSampleTo441;

	m_sgi_chkautozoom = s.fRememberZoomLevel;
	if (s.fCustomChannelMapping == FALSE){
		m_sgi_channelsetting = 0;
	}
	 //Sub Setting
	m_sgs_subfont1.Format( _T("%s(%d)"), s.subdefstyle.fontName , (INT)s.subdefstyle.fontSize);
	m_sgs_subalign1.Format( _T("%d") , s.subdefstyle.scrAlignment ); 
	m_sgi_suboveride1 = s.fOverridePlacement;
	m_sgs_subhpos1.Format( _T("%d") , s.nHorPos );
	m_sgs_subvpos1.Format( _T("%d") , s.nVerPos );

	m_sgs_subfont2.Format( _T("%s(%d)"), s.subdefstyle2.fontName , (INT)s.subdefstyle2.fontSize);
	m_sgs_subalign2.Format( _T("%d") , s.subdefstyle2.scrAlignment );
	m_sgi_suboveride2 = s.fOverridePlacement2;
	m_sgs_subhpos2.Format( _T("%d") , s.nHorPos2 );
	m_sgs_subvpos2.Format( _T("%d") , s.nVerPos2 );

	UpdateData(FALSE);



	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUESettingPanel::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CDHtmlDialog::OnDocumentComplete(pDisp, szUrl);
	m_stss = AfxGetAppSettings().subdefstyle;
	m_stss2 = AfxGetAppSettings().subdefstyle2;
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
	s.fRememberZoomLevel = !!m_sgi_chkautozoom ;
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
	 s.useGPUAcel = !!m_sgi_gpuacel;
	//Audio Setting
	s.fAudioNormalize = !!m_sgi_normalize  ;
	s.fDownSampleTo441 = !!m_sgi_downsample44k ;

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

	//Sub Setting
	s.fOverridePlacement = !!m_sgi_suboveride1  ;
	s.fOverridePlacement2 = !!m_sgi_suboveride2  ;
	
	s.subdefstyle = m_stss;
	s.subdefstyle2 = m_stss2;

	s.nHorPos = _wtoi(m_sgs_subhpos1);
	s.nVerPos = _wtoi(m_sgs_subvpos1);
	s.nHorPos2 = _wtoi(m_sgs_subhpos2);
	s.nVerPos2 = _wtoi(m_sgs_subvpos2);
	
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
