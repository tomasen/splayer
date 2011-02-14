#ifndef BTNEDITCTRL_H
#define BTNEDITCTRL_H

// CBtnEditCtrl is an edit control that has a button inserted at the end.
// Original credit: http://www.catch22.net/tuts/editbutton
class CBtnEditCtrl : 
  public CWindowImpl<CBtnEditCtrl, CEdit>
{
public:
  CBtnEditCtrl(void);
  ~CBtnEditCtrl(void);

  typedef struct
  {
    UINT uCmdId;       // sent in a WM_COMMAND message
    UINT fButtonDown;  // is the button up/down?
    BOOL fMouseDown;   // is the mouse activating the button?
    WNDPROC oldproc;   // need to remember the old window procedure

    // size of the current window borders.
    // given these, we know where to insert our button
    int cxLeftEdge, cxRightEdge; 
    int cyTopEdge,  cyBottomEdge;

  } InsBut;

  // empty message map to avoid errors
  BEGIN_MSG_MAP(CBtnEditCtrl)
  END_MSG_MAP()

public:
  HRESULT Init(UINT uiButtonID);

private:
  static LRESULT CALLBACK InsButProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  static void GetButtonRect(InsBut *pbut, RECT *rect);
  BOOL InsertButton(HWND hwnd, UINT uCmdId);
  void RedrawNC(HWND hwnd);
  static void DrawInsertedButton(HWND hwnd, InsBut *pbut, RECT *prect);
};

#endif // BTNEDITCTRL_H