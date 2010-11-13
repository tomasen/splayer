#ifndef OSDVIEW_H
#define OSDVIEW_H

#include "LayeredWindowUtils_Win.h"

class OSDView:
  public ATL::CWindowImpl<OSDView>,
  public LayeredWindowUtils<OSDView>
{
public:
  OSDView(void);

  void Open(HWND hwnd_parent);

  // called by LayeredWindowUtils
  void DoLayeredPaint(WTL::CDCHandle dc, RECT rcclient);

  BEGIN_MSG_MAP(OSDView)
  END_MSG_MAP()
};

#endif // OSDVIEW_H