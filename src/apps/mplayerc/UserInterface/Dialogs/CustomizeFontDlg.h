#pragma once

#include "../../resource.h"
#include "../Support/FontParamsManage.h"
#include "../Support/SubtitleStyle.h"

class CustomizeFontDlg:
  public CDialogImpl<CustomizeFontDlg>,
  public WTL::COwnerDraw<CustomizeFontDlg>,
  public WTL::CWinDataExchange<CustomizeFontDlg>
{
public:

  enum { IDD = IDD_FONTCUSTOMIZE};

  CustomizeFontDlg();
  ~CustomizeFontDlg();

  BEGIN_MSG_MAP(CustomizeFontDlg)
    MSG_WM_INITDIALOG(OnInitDialog)
    MSG_WM_CLOSE(OnClose)
    MSG_WM_DESTROY(OnDestroy)
    COMMAND_HANDLER_EX(IDC_FONTNAME_LIST, LBN_SELCHANGE, OnFontNameChange)
    COMMAND_HANDLER_EX(IDC_FONTSIZE_LIST, LBN_SELCHANGE, OnFontSizeChange)
    COMMAND_HANDLER_EX(IDC_FONTCOLOR_BUTTON1, BN_CLICKED, OnColorSelect)
    COMMAND_HANDLER_EX(IDC_FONTCOLOR_BUTTON2, BN_CLICKED, OnColorSelect)
    COMMAND_HANDLER_EX(IDC_FONTCOLOR_BUTTON3, BN_CLICKED, OnColorSelect)
    COMMAND_HANDLER_EX(IDC_STROKECOLOR, BN_CLICKED, OnColorSelect)
    COMMAND_HANDLER_EX(IDC_SHADOWCOLOR, BN_CLICKED, OnColorSelect)
    COMMAND_HANDLER_EX(IDC_FONTCOLOR_BUTTON1, BN_DOUBLECLICKED, OnColorDoubleClick)
    COMMAND_HANDLER_EX(IDC_FONTCOLOR_BUTTON2, BN_DOUBLECLICKED, OnColorDoubleClick)
    COMMAND_HANDLER_EX(IDC_FONTCOLOR_BUTTON3, BN_DOUBLECLICKED, OnColorDoubleClick)
    COMMAND_HANDLER_EX(IDC_STROKECOLOR, BN_DOUBLECLICKED, OnColorDoubleClick)
    COMMAND_HANDLER_EX(IDC_SHADOWCOLOR, BN_DOUBLECLICKED, OnColorDoubleClick)
    COMMAND_HANDLER_EX(IDC_FONTOK_BUTTON, BN_CLICKED, OnOK)
    COMMAND_HANDLER_EX(IDC_FONTCANCEL_BUTTON, BN_CLICKED, OnCancel)
    COMMAND_HANDLER_EX(IDC_STROKESIZE_COMBO, CBN_SELCHANGE, OnStrokeSizeChange)
    COMMAND_HANDLER_EX(IDC_SHADOWSIZE_COMBO, CBN_SELCHANGE, OnShadowSizeChange)
    COMMAND_HANDLER_EX(IDC_FONTRESTORE_BUTTON, BN_CLICKED, OnRestore)
    CHAIN_MSG_MAP(WTL::COwnerDraw<CustomizeFontDlg>)
  END_MSG_MAP()

  BOOL OnInitDialog(HWND hwnd, LPARAM lParam);
  BOOL OnClose();
  void OnDestroy();

  void OnFontNameChange(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnFontSizeChange(UINT uNotifyCode, int nID, CWindow wndCtl);
  //owner draw
  void DrawItem(LPDRAWITEMSTRUCT lpdis);
  //void MeasureItem(LPMEASUREITEMSTRUCT lpmis);
  void OnColorSelect(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnColorDoubleClick(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnStrokeSizeChange(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnShadowSizeChange(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnOK(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnRestore(UINT uNotifyCode, int nID, CWindow wndCtl);

  void InitFontNameList();
  void InitFontSizeList();
  void InitStrokeSizeComboBox();
  void InitShadowSizeComboBox();
  void Refresh();
  void FlushPreview();
  void Preview(HDC hdc, WTL::CRect rc, std::wstring sample, StyleParam* style);
  
  void SetSampleText(std::wstring samplemain, std::wstring samplesecon);
  void SetFontParam(StyleParam* parammain, StyleParam* paramsecon, BOOL bmain);
  //StyleParam* GetFontParam();
  void SetTitleText(std::wstring title);


private:

  WTL::CListBox  m_fontnamelist;
  WTL::CListBox  m_fontsizelist;
  WTL::CComboBox m_strokesizebox;
  WTL::CComboBox m_shadowsizebox;
  WTL::CButton   m_preview;
  
  std::set<std::wstring> m_fontset;
  std::map<int, int> m_colormap;
  std::wstring       m_sampletextmain;
  std::wstring       m_sampletextsecon;

  StyleParam         m_fontpamorg;
  StyleParam*        m_fontpammain;
  StyleParam*        m_fontpamsecon;
  StyleParam*        m_fontpamcurr;

  std::wstring       m_titletext;
  
};