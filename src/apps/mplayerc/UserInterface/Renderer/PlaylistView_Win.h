#ifndef PLAYLISTVIEW_WIN_H
#define PLAYLISTVIEW_WIN_H

class PlaylistView:
  public ATL::CWindowImpl<PlaylistView>,
  public WTL::COwnerDraw<PlaylistView>,
  public WTL::CCustomDraw<PlaylistView>
{
public:
  DECLARE_WND_CLASS(L"SPlayer_Playlist")

  PlaylistView(void);
  void Refresh();

  enum {IDC_LISTBOX, IDC_BTNLOAD, IDC_BTNSAVE, IDC_BTNCLEAR};

  BEGIN_MSG_MAP(PlaylistView)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_PAINT(OnPaint)
    MSG_WM_ERASEBKGND(OnEraseBkgnd)
    MSG_WM_SIZE(OnSize)
    MSG_WM_CTLCOLORLISTBOX(OnColorListBox)
    COMMAND_HANDLER(IDC_LISTBOX, LBN_DBLCLK, OnLbnDblClk)
    COMMAND_HANDLER(IDC_BTNLOAD, BN_CLICKED, OnBtnLoad)
    COMMAND_HANDLER(IDC_BTNSAVE, BN_CLICKED, OnBtnSave)
    COMMAND_HANDLER(IDC_BTNCLEAR, BN_CLICKED, OnBtnClear)
    CHAIN_MSG_MAP(WTL::COwnerDraw<PlaylistView>)
    CHAIN_MSG_MAP(WTL::CCustomDraw<PlaylistView>)
  END_MSG_MAP()

  // message handlers
  int OnCreate(LPCREATESTRUCT lpcs);
  void OnPaint(WTL::CDCHandle dc);
  BOOL OnEraseBkgnd(WTL::CDCHandle dc);
  void OnSize(UINT nType, CSize size);
  HBRUSH OnColorListBox(WTL::CDCHandle dc, WTL::CListBox listBox);
  LRESULT OnLbnDblClk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled); 
  LRESULT OnBtnLoad(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled); 
  LRESULT OnBtnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled); 
  LRESULT OnBtnClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled); 

  // owner-draw logic for playlist
  void DrawItem(LPDRAWITEMSTRUCT lpdis);
  void MeasureItem(LPMEASUREITEMSTRUCT lpmis);

  // custom-draw logic for buttons
  DWORD OnPrePaint(int idCtrl, LPNMCUSTOMDRAW lpnmcd);

  // generic gradient filler, |color_tl| = color_topleft, |color_br| = color_bottomright
  // |vert| true = vertical gradient, false = horizontal gradient
  static void _FillGradient(HDC hdc, RECT rc, int color_tl, int color_br, bool vert = true);
  static void _DrawRectNoCorner(WTL::CDCHandle& dc, RECT& rc, int offset);

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
  int m_basecolor4;

  int m_caption_height;
  int m_bottom_height;
  int m_button_height;
  int m_entry_height;
  int m_entry_padding;
  int m_padding;
};

#endif // PLAYLISTVIEW_WIN_H