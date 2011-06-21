
#pragma once

#include "LazyInstance.h"
#include <threadhelper.h>
#include "NetworkControlerImpl.h"
#include "../UserInterface/Dialogs/MovieComment_Win.h"
#include <comutil.h>

class UserShareController:
    public NetworkControlerImpl,
    public ThreadHelperImpl<UserShareController>,
    public LazyInstanceImpl<UserShareController>
{
public:

  UserShareController();
  ~UserShareController();

  void _Thread();
  void ShareMovie(std::wstring uuid, std::wstring sphash, std::wstring film);

  void SetCommentPlaneParent(HWND hwnd);
  void CalcCommentPlanePos();
  BOOL ShowCommentPlane();
  void HideCommentPlane();
  BOOL ToggleCommentPlane();

  BOOL OpenShooterMedia();
  BOOL CloseShooterMedia();
  std::wstring GenerateKey();
  std::wstring GetResponseData();
  BOOL CloseShareWnd();

private:
  std::wstring EncodeString(std::wstring str);

  std::wstring m_uuid;
  std::wstring m_sphash;
  std::wstring m_film;
  std::wstring m_retdata;

  MovieComment m_commentplane;
  HWND m_parentwnd;
};
