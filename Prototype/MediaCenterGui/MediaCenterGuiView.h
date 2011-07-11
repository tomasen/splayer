// MediaCenterGuiView.h : interface of the CMediaCenterGuiView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CMediaCenterGuiView : public CWindowImpl<CMediaCenterGuiView>
{
public:
	DECLARE_WND_CLASS(NULL)
  
  ListBlocks m_listblocks;

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	BEGIN_MSG_MAP(CMediaCenterGuiView)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnClick)
    MESSAGE_HANDLER(WM_LBUTTONUP, OnUnClick)
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnErase)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

  RECT m_scrollbarrect;
  void CreateMediaGui()
  {
    RECT clientrc, block;
    RECT margin = {5, 20, 64, 64};
    block.left = 0;block.top = 0;
    block.right = 138;block.bottom = 138;
    m_listblocks.SetOption(block, margin, 30);

    GetClientRect(&clientrc);
    m_listblocks.SetClientrc(clientrc);
  }

  void AddBlock()
  {
    RECT rect;
    m_listblocks.AddBlock(rect);
    InvalidateRect(&rect);
    //InvalidateRect(&m_scrollbarrect);
  }

  LRESULT OnErase(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
  {
    bHandled = TRUE;
    return 0;
  }

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
    RECT rcClient;
    GetClientRect(&rcClient);
    WTL::CPaintDC wtldc(m_hWnd);
    WTL::CMemoryDC mdc(wtldc, rcClient);
    HBRUSH hbrush = ::CreateSolidBrush(RGB(231, 231, 231));
    mdc.FillRect(&rcClient, hbrush);
    m_listblocks.DoPaint(mdc);

		//TODO: Add your drawing code here

		return 0;
	}

  HRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
  {
    POINT currpt;
    RECT rect;

    currpt.x = GET_X_LPARAM(lParam);
    currpt.y = GET_Y_LPARAM(lParam);

    m_listblocks.SelectBlockEffect(currpt, rect);
    m_listblocks.DragScrollBar(currpt);
    Invalidate();
    return 0;
  }

  LRESULT OnClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  {
    POINT curr;
    ::GetCursorPos(&curr);
    ScreenToClient(&curr);
    SetCapture();
    m_listblocks.SelectScrollBar(curr, m_scrollbarrect);
    m_listblocks.SelectBlockClick(m_hWnd);
    return 0;
  }

  LRESULT OnUnClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  {
    m_listblocks.UnDragScrollBar();
    ReleaseCapture();
    InvalidateRect(&m_scrollbarrect);
    return 0;
  }

  LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
  {
    int w = LOWORD(lParam);
    int h = HIWORD(lParam);
    RECT rc = {0, 0, w, h};
    m_listblocks.SetClientrc(rc);
    m_listblocks.AutoBreakline();
    Invalidate();
    return 0;
  }
};
