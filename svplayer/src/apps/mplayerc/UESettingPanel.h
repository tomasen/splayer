#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

// CUESettingPanel dialog

class CUESettingPanel : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CUESettingPanel)
	virtual HRESULT STDMETHODCALLTYPE  GetHostInfo(DOCHOSTUIINFO *pInfo);
	virtual HRESULT STDMETHODCALLTYPE  ShowContextMenu(DWORD /*dwID*/, POINT *ppt, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/);
	void ApplyAllSetting();
	void ApplyGeneralSetting();
	void ApplyVideoSetting();
	void ApplyAudioSetting();
	void ApplySubSetting();
	void ApplyFileAsscSetting();
	void ApplyHotkeySetting();
	int m_sgi_chkremwinpos;
	int m_sgi_chkcdromenu;
	int m_sgi_chkuseini;
	int m_sgi_chkfileass;
	int m_sgi_chkplayrepeat;
	int m_sgi_chkexitfullscreen;
	int m_sgi_chkabnormal;
	int m_sgi_chkautoplay;
	CString m_sgs_initblock;
	
public:
	CUESettingPanel(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUESettingPanel();
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);
	HRESULT OnButtonApply(IHTMLElement *pElement);
	HRESULT OnButtonAdvanceSetting(IHTMLElement *pElement);
	bool bOpenAdvancePanel;
	int idPage;
// Dialog Data
	enum { IDD = IDD_DHTML_SETTING, IDH = IDR_HTML_UESETTINGPANEL };
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
