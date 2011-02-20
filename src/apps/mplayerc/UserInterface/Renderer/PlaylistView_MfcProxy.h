#ifndef PLAYLISTVIEW_MFCPROXY_H
#define PLAYLISTVIEW_MFCPROXY_H

class PlaylistView;

//////////////////////////////////////////////////////////////////////////
//
//  For PlaylistView development (PlaylistView_Win.h), the current
//  frame window used MFC-based layout logic. We choose to comply with
//  this logic rather than process our own layout (sizing, positioning)
//  logic. This proxy class is created to contain the PlaylistView, and
//  provide an MFC window to the frame.
//
class PlaylistViewMfcProxy:
  public CSizingControlBarG
{
public:
  PlaylistView* GetView();
  BOOL Create(CWnd* pParentWnd);

protected:
  DECLARE_MESSAGE_MAP()

  int OnCreate(LPCREATESTRUCT lpcs);
  void OnSize(UINT nType, int cx, int cy);

private:
  std::tr1::shared_ptr<PlaylistView> m_view;
};

#endif // PLAYLISTVIEW_MFCPROXY_H