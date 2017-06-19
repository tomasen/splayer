// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the	
// Active Template Library product.

// RegObj.h : Declaration of the CRegObject

/////////////////////////////////////////////////////////////////////////////
// register

class ATL_NO_VTABLE CDLLRegObject : public CRegObject, public CComObjectRoot,
					  public CComCoClass<CDLLRegObject, &CLSID_Registrar>
{
public:
	CDLLRegObject() {}
	~CDLLRegObject(){CRegObject::ClearReplacements();}

BEGIN_COM_MAP(CDLLRegObject)
	COM_INTERFACE_ENTRY(IRegistrar)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CDLLRegObject)
	HRESULT FinalConstruct()
	{
		HRESULT hr = CComObjectRoot::FinalConstruct();
		if (SUCCEEDED(hr))
		{
			hr = CRegObject::FinalConstruct();
		}
		return hr;
	}
	void FinalRelease()
	{
		CComObjectRoot::FinalRelease();
	}
//we can't use the component because that's what we're registering
//we don't want to do the static registry because we'd have extra code
#pragma warning( push )
#pragma warning( disable: 4996 )  // disable "deprecated symbol" warning
	static HRESULT WINAPI UpdateRegistry(BOOL bRegister)
	{
		CComObject<CDLLRegObject>* p;
		CComObject<CDLLRegObject>::CreateInstance(&p);
		CComPtr<IRegistrar> pR;
		HRESULT hr = p->QueryInterface(IID_IRegistrar, (void**)&pR);
		if (FAILED(hr))
		{
			delete p;
			return hr;
		}
		return AtlModuleUpdateRegistryFromResourceD(&_Module,
			(LPCOLESTR)MAKEINTRESOURCE(IDR_Registrar), bRegister, NULL, pR);
	}
#pragma warning( pop )
};
