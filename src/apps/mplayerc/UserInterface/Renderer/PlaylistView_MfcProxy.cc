#include "StdAfx.h"
#include "PlaylistView_MfcProxy.h"
#include "PlaylistView_Win.h"

BEGIN_MESSAGE_MAP(PlaylistViewMfcProxy, CWnd)
  ON_WM_CREATE()
  ON_WM_SIZE()
END_MESSAGE_MAP()

PlaylistView* PlaylistViewMfcProxy::GetView()
{
  return m_view.get();
}

BOOL PlaylistViewMfcProxy::Create(CWnd* pParentWnd)
{
  if(!CSizingControlBarG::Create(_T("Playlist"), pParentWnd, 0))
    return FALSE;

  return TRUE;
}

int PlaylistViewMfcProxy::OnCreate(LPCREATESTRUCT lpcs)
{
  m_view.reset(new PlaylistView());
  m_view->Create(m_hWnd);
  return 0;
}

void PlaylistViewMfcProxy::OnSize(UINT nType, int cx, int cy)
{
  CSizingControlBarG::OnSize(nType, cx, cy);
  // some kind of weird CSizingControlBarG mechanism that needs
  // 2x2 offset deflate
  m_view->SetWindowPos(NULL, 2, 2, cx-4, cy-4, SWP_NOZORDER);
}