#ifndef OPTIONDLG_WIN_H
#define OPTIONDLG_WIN_H

class OptionBasicPage;
class OptionSubtitlePage;
class OptionAdvancedPage;
class OptionAssociationPage;
class OptionHotkeyPage;

class OptionDlg:
  public WTL::CPropertySheetImpl<OptionDlg>
{
public:
  OptionDlg(void);
  ~OptionDlg(void);

private:
  OptionBasicPage*        m_basicpage;
  OptionSubtitlePage*     m_subtitlepage;
  OptionAdvancedPage*     m_advancedpage;
  OptionAssociationPage*  m_associationpage;
  OptionHotkeyPage*       m_hotkeypage;
};

#endif // OPTIONDLG_WIN_H