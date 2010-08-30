#include "stdafx.h"
#include "OptionDlg_Win.h"
#include "OptionBasicPage_Win.h"
#include "OptionSubtitlePage_Win.h"
#include "OptionAdvancedPage_Win.h"
#include "OptionAssociationPage_Win.h"

OptionDlg::OptionDlg(void):
  WTL::CPropertySheetImpl<OptionDlg>(IDS_OPTIONSTITLE)
{
  m_basicpage       = new OptionBasicPage();
  m_subtitlepage    = new OptionSubtitlePage();
  m_advancedpage    = new OptionAdvancedPage();
  m_associationpage = new OptionAssociationPage();

  m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;

  AddPage(*m_basicpage);
  AddPage(*m_subtitlepage);
  AddPage(*m_advancedpage);
  AddPage(*m_associationpage);
}

OptionDlg::~OptionDlg(void)
{
  delete m_basicpage;
  delete m_subtitlepage;
  delete m_advancedpage;
  delete m_associationpage;
}