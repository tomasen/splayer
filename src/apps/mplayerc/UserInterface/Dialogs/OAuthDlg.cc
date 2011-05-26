
// mfchostDoc.cpp : CmfchostDoc 类的实现
//

#include "stdafx.h"
#include <exdispid.h>
#include "OAuthDlg.h"
#include "logging.h"
#include "../../resource.h"
#include <ResLoader.h>

IMPLEMENT_DYNAMIC(CircleBtn, CBitmapButton)

BEGIN_MESSAGE_MAP(CircleBtn, CBitmapButton)
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE(WM_MOUSEMOVE, OnMouseMove)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_PAINT()
END_MESSAGE_MAP()
CircleBtn::CircleBtn() :
	m_trackleave(FALSE)
{
	m_out.LoadBitmap(IDB_OAUTHCLOSEOUT);
	m_over.LoadBitmap(IDB_OAUTHCLOSEOVER);
}
CircleBtn::~CircleBtn()
{

}
void CircleBtn::SetCircleWnd()
{
	CRgn rgn;
	rgn.CreateEllipticRgn(1, 1, 40, 40);
	SetWindowRgn(rgn, TRUE);
}

void CircleBtn::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);
	//SetCircleWnd();
}

void CircleBtn::OnLButtonDown(UINT nFlags, CPoint point)
{
}

void CircleBtn::OnLButtonUp(UINT nFlags, CPoint point)
{
	GetParent()->ShowWindow(SW_HIDE);
}

LRESULT CircleBtn::OnMouseLeave(WPARAM, LPARAM)
{
	m_trackleave = FALSE;
	Invalidate(TRUE);
	return S_FALSE;
}

LRESULT CircleBtn::OnMouseMove(WPARAM, LPARAM)
{
	if (!m_trackleave)
	{
		TRACKMOUSEEVENT tl;
		tl.cbSize = sizeof(tl);
		tl.hwndTrack = m_hWnd;
		tl.dwFlags = TME_LEAVE;
		tl.dwHoverTime = 1;
		m_trackleave = TrackMouseEvent(&tl);
		Invalidate(TRUE);
	}
	return S_FALSE;
}

void CircleBtn::OnPaint()
{
	CPaintDC dc(this);

	CDC mdc;
	mdc.CreateCompatibleDC(&dc);

	HBITMAP hold = (HBITMAP)mdc.SelectObject(&(m_trackleave?m_over:m_out));
	BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
	dc.AlphaBlend(0, 0, 34, 29, &mdc, 0, 0, 34, 29, bf);
	mdc.SelectObject(hold);
}

IMPLEMENT_DYNAMIC(OAuthDlg, CDHtmlDialog)

BEGIN_MESSAGE_MAP(OAuthDlg, CDHtmlDialog)
  ON_WM_SIZE()
  ON_WM_CREATE()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(OAuthDlg)

END_DHTML_EVENT_MAP()

BEGIN_DISPATCH_MAP(OAuthDlg, CDHtmlDialog)
	DISP_FUNCTION(OAuthDlg, "CallSPlayer", CallSPlayer, VT_BSTR, VTS_BSTR VTS_BSTR)
END_DISPATCH_MAP()

OAuthDlg::OAuthDlg()
{
  SupportContextMenu(FALSE);
  HostFlags(DOCHOSTUIFLAG_THEME | DOCHOSTUIFLAG_SCROLL_NO | DOCHOSTUIFLAG_NO3DBORDER
    | DOCHOSTUIFLAG_DISABLE_HELP_MENU | DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_ENABLE_ACTIVEX_INACTIVATE_MODE
    | DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE | DOCHOSTUIFLAG_OVERRIDEBEHAVIORFACTORY
    );
}

OAuthDlg::~OAuthDlg()
{
}

BSTR OAuthDlg::CallSPlayer(LPCTSTR p, LPCTSTR param)
{
	CString ret = L"0";
	std::wstring cmd(p);
	if (cmd.empty())
		ret = L"-1";
	else if (cmd == L"close")
		ShowWindow(SW_HIDE);
	else
		ret = L"-1";
	return ret.AllocSysString();
}

int OAuthDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_btnclose.Create(NULL, L"oauthclose", WS_CHILD|WS_VISIBLE , CRect(450, 5, 484, 34), this, 1);

	return __super::OnCreate(lpCreateStruct);
}

BOOL OAuthDlg::OnInitDialog()
{
  DhtmlDlgBase::OnInitDialog();

  SupportJSCallBack();
  SetUserAgent("Mozilla/5.0 (Linux; U; Android 0.5; en-us) AppleWebKit/522+ (KHTML, like Gecko) Safari/419.3");

  CString strResourceURL;
  LPTSTR lpszModule = new TCHAR[_MAX_PATH];

  if (lpszModule && GetModuleFileName(NULL, lpszModule, _MAX_PATH))
  {
    // load resource html regardless by language
    strResourceURL.Format(_T("res://%s/%d"), lpszModule, IDR_HTML_BUSY);
    Navigate(strResourceURL, 0, 0, 0);
  }
  else
    Navigate(L"about:blank");


  return TRUE;
}

void OAuthDlg::CalcOauthPos()
{
  if (!IsWindowVisible())
    return;

  RECT rc;
  GetParent()->GetWindowRect(&rc);

  rc.top += (rc.bottom-rc.top-400)/2-10;
  rc.left += (rc.right-rc.left-500)/2;
  rc.right = 500;
  rc.bottom = 400;
  SetFramePos(rc);
}

void OAuthDlg::OnSize(UINT nType, int cx, int cy)
{
  __super::OnSize(nType, cx, cy);

  CRect rc;
  WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
  GetWindowPlacement(&wp);
  GetWindowRect(&rc);
  rc-=rc.TopLeft();

  // destroy old region
  if((HRGN)m_rgn)
    m_rgn.DeleteObject();

  // create rounded rect region based on new window size
  if (wp.showCmd != SW_MAXIMIZE )
    m_rgn.CreateRoundRectRgn(0, 0, rc.Width(), rc.Height(), 7, 7);
  else
    m_rgn.CreateRectRgn( 0,0, rc.Width(), rc.Height() );

  SetWindowRgn(m_rgn,TRUE);
}

void OAuthDlg::SetUrl(std::wstring url)
{
  if (url.empty())
    return;

  Navigate(url.c_str());
}

STDMETHODIMP OAuthDlg::TranslateAccelerator(LPMSG lpMsg, const GUID* /*pguidCmdGroup*/, DWORD /*nCmdID*/)
{
  switch (lpMsg->message)
  {
  case WM_CHAR:
    switch (lpMsg->wParam)
    {
    case ' ':			// SPACE - Activate a link
      return S_FALSE;	// S_FALSE = Let the control process the key stroke.
    }
    break;
  case WM_KEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
    switch (lpMsg->wParam)
    {
    case VK_TAB:		// Cycling through controls which can get the focus
    case VK_SPACE:		// Activate a link
      return S_FALSE; // S_FALSE = Let the control process the key stroke.
    case VK_ESCAPE:
      HideFrame();
      break;
    }
    break;
  }

  return S_FALSE;
}