// sinet_dyn.cpp : Defines the exported functions for the DLL application.
//

#include "pch.h"
#include "sinet_dyn.h"


// This is an example of an exported variable
SINET_DYN_API int nsinet_dyn=0;

// This is an example of an exported function.
SINET_DYN_API int fnsinet_dyn(void)
{
	return 42;
}