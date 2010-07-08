#include "stdafx.h"
#include "OptionDlg_Win.h"
#include "OptionBasicPage_Win.h"
#include "OptionSubtitlePage_Win.h"
#include "OptionAdvancedPage_Win.h"
#include "OptionAssociationPage_Win.h"
#include "OptionHotkeyPage_Win.h"

OptionDlg::OptionDlg(void)
{
  m_basicpage       = new OptionBasicPage();
  m_subtitlepage    = new OptionSubtitlePage();
  m_advancedpage    = new OptionAdvancedPage();
  m_associationpage = new OptionAssociationPage();
  m_hotkeypage      = new OptionHotkeyPage();

  AddPage(*m_basicpage);
  AddPage(*m_subtitlepage);
  AddPage(*m_advancedpage);
  AddPage(*m_associationpage);
  AddPage(*m_hotkeypage);
}

OptionDlg::~OptionDlg(void)
{
  delete m_basicpage;
  delete m_subtitlepage;
  delete m_advancedpage;
  delete m_associationpage;
  delete m_hotkeypage;
}