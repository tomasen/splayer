#ifndef PLAYLISTVIEW_WIN_H
#define PLAYLISTVIEW_WIN_H

class PlaylistView:
  public ATL::CWindowImpl<PlaylistView>,
  public WTL::COwnerDraw<PlaylistView>
{
public:
  DECLARE_WND_CLASS(L"SPlayer_Playlist")

  PlaylistView(void);

  BEGIN_MSG_MAP(PlaylistView)
    MSG_WM_PAINT(OnPaint)
    MSG_WM_ERASEBKGND(OnEraseBkgnd)
    CHAIN_MSG_MAP(WTL::COwnerDraw<PlaylistView>)
  END_MSG_MAP()

  // message handlers
  void OnPaint(WTL::CDCHandle dc);
  BOOL OnEraseBkgnd(WTL::CDCHandle dc);

  // owner-draw logic for playlist
  void DrawItem(LPDRAWITEMSTRUCT lpdis);
  void MeasureItem(LPMEASUREITEMSTRUCT lpmis);

private:
};

#endif // PLAYLISTVIEW_WIN_H