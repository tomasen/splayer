#ifndef OPTIONDLG_WIN_H
#define OPTIONDLG_WIN_H

#define OPTIONDLG_BASIC 0
#define OPTIONDLG_SUBTITLE 1
#define OPTIONDLG_ADVANCED 2
#define OPTIONDLG_ASSOCIATION 3

class OptionBasicPage;
class OptionSubtitlePage;
class OptionAdvancedPage;
class OptionAssociationPage;

class OptionDlg:
  public WTL::CPropertySheetImpl<OptionDlg>
{
public:
  OptionDlg(int pageindex = 0);
  ~OptionDlg(void);

  BEGIN_MSG_MAP(OptionDlg)
    MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
    CHAIN_MSG_MAP(WTL::CPropertySheetImpl<OptionDlg>)
  END_MSG_MAP()

  LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

private:
  OptionBasicPage*        m_basicpage;
  OptionSubtitlePage*     m_subtitlepage;
  OptionAdvancedPage*     m_advancedpage;
  OptionAssociationPage*  m_associationpage;
};

#endif // OPTIONDLG_WIN_H