#pragma once

#include "MediaScrollbar.h"
#include "MediaListView.h"
#include "../../Model/MediaModel.h"

class MediaCenterView :
  public ATL::CWindowImpl<MediaCenterView>,
  public MediaScrollbar,
  public MediaListView
{
public:
  DECLARE_WND_CLASS(L"SPlayer_MediaCenter")

  BEGIN_MSG_MAP(MediaCenterView)
    CHAIN_MSG_MAP(MediaScrollbar)
    CHAIN_MSG_MAP(MediaListView)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_LBUTTONUP, OnLBUp)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBKGnd)
  END_MSG_MAP()

public:
  MediaCenterView();
  ~MediaCenterView();

public:
  HRESULT OnLBUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  HRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
  HRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  HRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  HRESULT OnEraseBKGnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  HRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

  BOOL ShowMediaCenter(HWND hwnd, int boxwidth, int boxheight, RECT& margin);
  void HideMediaCenter();
  void CalcWnd();

private:
  RECT m_clientrc;
  HWND m_parentwnd;
  int m_width;
  int m_height;
  MediaModel m_model;
};