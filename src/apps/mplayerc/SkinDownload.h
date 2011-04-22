#pragma once
#include "afxdhtml.h"
#include "resource.h"
#include <threadhelper.h>

class SkinDownload : public CDHtmlDialog, public ThreadHelperImpl<SkinDownload>
{
public:

  DECLARE_DYNAMIC(SkinDownload)
  
  virtual HRESULT STDMETHODCALLTYPE ShowContextMenu(DWORD /*dwID*/, POINT *ppt, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/);

  SkinDownload(void);
  ~SkinDownload(void);

  void ShowFrame();

  void HideFrame();

  void OnClose();

  void DownloadSkin(std::wstring url);
  
  void _Thread();

  std::wstring GetModuleFolder();

protected:
  virtual BOOL OnInitDialog();

  void OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags,
    VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel);

  DECLARE_MESSAGE_MAP()
  DECLARE_EVENTSINK_MAP()

private:
  std::wstring m_url;
  std::wstring m_filename;
};
