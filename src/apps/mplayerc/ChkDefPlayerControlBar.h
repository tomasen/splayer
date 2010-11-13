#ifndef CHKDEFPLAYERCONTROLBAR_H
#define CHKDEFPLAYERCONTROLBAR_H

#include "SVPButton.h"
#include "SVPStatic.h"
#include "SVPDialog.h"
//#include "PPageFormats.h"
#include "../../svplib/SVPToolBox.h"

class ChkDefPlayerControlBar :
  public CSVPDialog
{
public:
  ChkDefPlayerControlBar();
  ~ChkDefPlayerControlBar();
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  void Relayout();
  afx_msg void OnNoMoreQuesCheck();
  afx_msg void OnButtonOK();
  afx_msg void OnButtonManual();
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
  bool IsDefaultPlayer();
private:
  DECLARE_DYNAMIC(ChkDefPlayerControlBar)
  CFont      m_font;
  CSVPStatic m_csllabel;
  CSVPButton m_cbnomoreques;
  CSVPButton m_cbok;
  CSVPButton m_cbmanual;
  bool       m_bnomorequestion;
  CSVPToolBox  svpTool;

  std::vector<std::wstring> szaExt;
  std::vector<std::wstring> szaNotExt;
  void SetDefaultPlayer(int ilimitime = 0);
  void SetKeyboardNativeMediaPlayers();
  void SetKeyboardNativeMediaPlayers2();
protected:
  DECLARE_MESSAGE_MAP()
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
};








#endif // CHKDEFPLAYERCONTROLBAR_H