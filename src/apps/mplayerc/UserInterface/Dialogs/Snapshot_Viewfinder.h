#pragma once

class CSnapshot_Win;

class CSnapshot_Viewfinder
{
  friend class CSnapshot_Win;

public:
  CSnapshot_Viewfinder(HWND hParent);
  ~CSnapshot_Viewfinder();

protected:
  void OnMouseMove(UINT nFlags, CPoint point, bool &bNeedUpdate);
  void OnLButtonDown(UINT nFlags, CPoint point);
  void OnLButtonUp(UINT nFlags, CPoint point);

  void HandleNormalArea(UINT nFlags, CPoint point, bool &bNeedUpdate);
  void HandleMoveArea(UINT nFlags, CPoint point, bool &bNeedUpdate);
  void HandleNWPointArea(UINT nFlags, CPoint point, bool &bNeedUpdate);
  void HandleNEPointArea(UINT nFlags, CPoint point, bool &bNeedUpdate);
  void HandleSEPointArea(UINT nFlags, CPoint point, bool &bNeedUpdate);
  void HandleSWPointArea(UINT nFlags, CPoint point, bool &bNeedUpdate);
  void HandleTopLineArea(UINT nFlags, CPoint point, bool &bNeedUpdate);
  void HandleRightLineArea(UINT nFlags, CPoint point, bool &bNeedUpdate);
  void HandleBottomLineArea(UINT nFlags, CPoint point, bool &bNeedUpdate);
  void HandleLeftLineArea(UINT nFlags, CPoint point, bool &bNeedUpdate);

  void DetermineCurrentState(CPoint point);
  void DetermineRectEachState();
  void SetCursorByCurState(CPoint point);
  void ExchangeStateAndRect();

  HWND GetParent();

private:
  typedef void (CSnapshot_Viewfinder::*pfnAreaHandler)(UINT nFlags, CPoint point, bool &bNeedUpdate);
  std::map<std::wstring, pfnAreaHandler>  m_mpAreaHandler;

  std::map<std::wstring, HCURSOR>         m_mpCursors;

  CPoint                                  m_piLastCursorPos;
  CRect                                   m_rcViewfinder;

  std::wstring                            m_sCurState;
  std::map<std::wstring, CRect>           m_mpRectEachState;

  HWND                                    m_hParent;
  CRect                                   m_rcParent;
};