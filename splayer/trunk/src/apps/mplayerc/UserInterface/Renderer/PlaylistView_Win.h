#ifndef PLAYLISTVIEW_WIN_H
#define PLAYLISTVIEW_WIN_H

class PlaylistView:
  public ATL::CWindowImpl<PlaylistView>,
  public WTL::COwnerDraw<PlaylistView>
{
public:
  DECLARE_WND_CLASS(L"SPlayer_Playlist")

  PlaylistView(void);

  void Refresh();

  BEGIN_MSG_MAP(PlaylistView)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_PAINT(OnPaint)
    MSG_WM_ERASEBKGND(OnEraseBkgnd)
    MSG_WM_SIZE(OnSize)
    MSG_WM_CTLCOLORLISTBOX(OnColorListBox)
    CHAIN_MSG_MAP(WTL::COwnerDraw<PlaylistView>)
  END_MSG_MAP()

  // message handlers
  int OnCreate(LPCREATESTRUCT lpcs);
  void OnPaint(WTL::CDCHandle dc);
  BOOL OnEraseBkgnd(WTL::CDCHandle dc);
  void OnSize(UINT nType, CSize size);
  HBRUSH OnColorListBox(WTL::CDCHandle dc, WTL::CListBox listBox);

  // owner-draw logic for playlist
  void DrawItem(LPDRAWITEMSTRUCT lpdis);
  void MeasureItem(LPMEASUREITEMSTRUCT lpmis);

private:
  void _PaintWorker(HDC hdc, RECT rc);

  std::vector<std::wstring> m_texts;

  WTL::CListBox m_list;
  WTL::CButton  m_btn_load;
  WTL::CButton  m_btn_save;
  WTL::CButton  m_btn_clear;

  WTL::CFont    m_font_bold;
  WTL::CFont    m_font_normal;
  WTL::CFont    m_font_symbol;
  WTL::CBrush   m_br_list;

  int m_textcolor;
  int m_textcolor_hilite;
  int m_basecolor;
  int m_basecolor2;
  int m_basecolor3;

  int m_caption_height;
  int m_bottom_height;
  int m_button_height;
  int m_entry_height;
  int m_entry_padding;
  int m_padding;
};

#endif // PLAYLISTVIEW_WIN_H