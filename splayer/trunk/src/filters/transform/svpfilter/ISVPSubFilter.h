
#pragma once
#include "..\..\..\subpic\ISubPic.h"


interface __declspec(uuid("E8D381DD-8C7D-4a6f-96ED-92BBB64064CF")) ISVPSubFilter : public IUnknown
{
	STDMETHOD (Put_OSD) (LPVOID* lpInput ) PURE;
	STDMETHOD (Set_AutoEnlargeARForSub) (BOOL bEnlarge ) PURE;
};
