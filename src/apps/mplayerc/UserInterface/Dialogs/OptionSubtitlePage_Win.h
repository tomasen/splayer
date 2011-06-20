#ifndef OPTIONSUBTITLEPAGE_WIN_H
#define OPTIONSUBTITLEPAGE_WIN_H

#include "../../resource.h"
#include "../Support/FontParamsManage.h"
#include "../Support/SubtitleStyle.h"
#include "../Support/ListImpl.h"

class OptionSubtitlePage:
  public WTL::CPropertyPageImpl<OptionSubtitlePage>,
  public WTL::COwnerDraw<OptionSubtitlePage>,
  public WTL::CWinDataExchange<OptionSubtitlePage>
{
public:
  enum { IDD = IDD_OPTION_SUBTITLE};
  OptionSubtitlePage(void);

  BEGIN_MSG_MAP(OptionSubtitlePage)
    MSG_WM_INITDIALOG(OnInitDialog)
    MSG_WM_DESTROY(OnDestroy)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
    COMMAND_HANDLER_EX(IDC_LIST, LBN_SELCHANGE, OnSubtitleStyleChange)
    //COMMAND_HANDLER_EX(IDC_LIST, LBN_DBLCLK, OnListDoubleClick)
    COMMAND_HANDLER_EX(IDC_BUTTON_SAVESUBTITLE_CUSTOM_FOLDER, BN_CLICKED, OnBrowserForFolder)
    COMMAND_HANDLER_EX(IDC_RADIO_SAVESUBTITLE_SAME_FOLDER, BN_CLICKED, OnSelectSameFolder)
    COMMAND_HANDLER_EX(IDC_RADIO_SAVESUBTITLE_CUSTOM_FOLDER, BN_CLICKED, OnSelectCustomFolder)
    CHAIN_MSG_MAP(WTL::CPropertyPageImpl<OptionSubtitlePage>)
    CHAIN_MSG_MAP(WTL::COwnerDraw<OptionSubtitlePage>)
  END_MSG_MAP()

  BEGIN_DDX_MAP(OptionSubtitlePage)
    DDX_CHECK(IDC_CHECK_AUTOMATCHSUBTITLE, m_fetchsubtitlefromshooter)
    DDX_TEXT(IDC_EDIT_SAVESUBTITLE_CUSTOM_FOLDER, m_sCustomPath)
  END_DDX_MAP()

  // message handlers
  BOOL OnInitDialog(HWND hwnd, LPARAM lParam);
  void OnDestroy();
  void OnMouseMove(UINT wParma, CPoint pt);
  void OnLButtonDbClick(UINT wParma, CPoint pt);

  void OnSubtitleStyleChange(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnListDoubleClick(UINT uNotifyCode, int nID, CWindow wndCtl);

  void OnBrowserForFolder(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnSelectSameFolder(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnSelectCustomFolder(UINT uNotifyCode, int nID, CWindow wndCtl);

  // owner-draw logic for subtitle styles
  void DrawItem(LPDRAWITEMSTRUCT lpdis);
  void MeasureItem(LPMEASUREITEMSTRUCT lpmis);

  // activate/apply handler
  int OnSetActive();
  int OnApply();

  void RefreshStyles();

  // before draw list items.
  int PreserveItemDivideRect(WTL::CRect rc, int index, BOOL bdivide);
  WTL::CRect GetTextDivideRect(int index);
  WTL::CRect GetHittestDivideRect(int index);
  void PaintListItemBackground(HDC dc, int width, int height);
  void PaintHighLightListBckground(HDC dc, WTL::CRect rc, BOOL bmain = true);

private:
  WTL::CComboBox  m_secsubtitlestyle;
  CListImpl       m_subtitlestyle;

  WTL::CString    m_sCustomPath;

  int m_mainstyle;
  int m_secstyle;

  int m_styleentry_height;
  int m_styleentry_width;

  int m_fetchsubtitlefromshooter;

  FontParamsManage m_fontparams;
  DrawSubtitle m_subtitle;
  std::vector<WTL::CRect>   m_listtextrc;
  std::vector<WTL::CRect>   m_listhittestrc;
  std::vector<std::wstring> m_samplevec;
  WTL::CRect                m_highlightrc;
  int                       m_highlightstat;

  void ApplySubtitleStyle();
  int ApplySubtitleSavePath();
};

#endif // OPTIONSUBTITLEPAGE_WIN_H