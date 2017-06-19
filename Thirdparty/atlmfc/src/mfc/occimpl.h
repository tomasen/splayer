// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef _AFX_NO_OCC_SUPPORT

// MFC doesn't use the OLEDB 1.5 features of ATL and it causes
// compile problems, so just make ATL think it's not version 1.5
#if (OLEDBVER >= 0x0150)
#undef OLEDBVER
#define OLEDBVER 0x0100
#endif

#include "atldbcli.h"

class CDataSourceControl;
class CDataBoundProperty;

// CCmdTarget
	class COleControlContainer;
	class COleControlSite;

class COccManager;
struct _AFX_OCC_DIALOG_INFO;

#define DISPID_DATASOURCE   0x80010001
#define DISPID_DATAFIELD    0x80010002

interface AFX_NOVTABLE IDataSourceListener : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE OnDataMemberChanged(BSTR bstrDM) = 0;
	virtual HRESULT STDMETHODCALLTYPE OnDataMemberAdded(BSTR bstrDM) = 0;
	virtual HRESULT STDMETHODCALLTYPE OnDataMemberRemoved(BSTR bstrDM) = 0;
};

interface AFX_NOVTABLE IDataSource : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetDataMember(BSTR bstrDM, const GUID __RPC_FAR* riid, IUnknown __RPC_FAR* __RPC_FAR* ppunk) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDataMemberName(long lIndex, BSTR __RPC_FAR *pbstrDM) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDataMemberCount(long __RPC_FAR *plCount) = 0;
	virtual HRESULT STDMETHODCALLTYPE AddDataSourceListener(IDataSourceListener __RPC_FAR *pDSL) = 0;
	virtual HRESULT STDMETHODCALLTYPE RemoveDataSourceListener(IDataSourceListener __RPC_FAR *pDSL) = 0;
};

/////////////////////////////////////////////////////////////////////////////
// OLE Databinding support class for data sources

interface IRowPosition;

class CDataSourceControl
{
private:
	CDataSourceControl() {};
public:
	struct METAROWTYPE
	{
		DBCOLUMNID idColumnID;
		DWORD dwColumnID;
		LPSTR lpstrName;
		DWORD dwName;
		CPtrList* m_pClientList;
	};

	CDataSourceControl(COleControlSite *pClientSite);
	~CDataSourceControl();
	HRESULT Initialize();
	virtual IUnknown* GetCursor();
	HRESULT GetMetaData();
	virtual void BindProp(COleControlSite* pClientSite, BOOL bBind = TRUE);
	virtual void BindProp(CDataBoundProperty* pProperty, BOOL bBind = TRUE);
	virtual void BindColumns();
	BOOL CopyColumnID(DBCOLUMNID* pcidDst, DBCOLUMNID const *pcidSrc);
	HRESULT GetBoundClientRow();
	virtual HRESULT UpdateControls();
	virtual HRESULT UpdateCursor();
	COleVariant ToVariant(int nCol);

	COleControlSite *m_pClientSite;  // Back ptr to containing site
	ICursorMove* m_pCursorMove;
	ICursorUpdateARow* m_pCursorUpdateARow;
	INT_PTR m_nColumns;
	METAROWTYPE* m_pMetaRowData;
	CPtrList m_CursorBoundProps;
	void* m_pVarData;
	INT_PTR m_nBindings;
	DBCOLUMNBINDING *m_pColumnBindings;
	VARIANT* m_pValues;
	BOOL m_bUpdateInProgress;

// OLE/DB stuff
	IDataSource*            m_pDataSource;
	IRowPosition*           m_pRowPosition;
	ATL::CRowset<>*         m_pRowset;
	ATL::CDynamicAccessor*  m_pDynamicAccessor;

	DWORD m_dwRowsetNotify; // IRowsetNotify cookie
};

/////////////////////////////////////////////////////////////////////////////
// OLE Databinding support class for bound controls

class CDataBoundProperty
{
protected:
	CDataBoundProperty() {};
public:
	CDataBoundProperty(CDataBoundProperty* pLast, DISPID dispid, WORD ctlid);
	~CDataBoundProperty() {};
	void SetClientSite(COleControlSite *pClientSite);
	void SetDSCSite(COleControlSite *pDSCSite);
	void RemoveSource();
	void Notify();
	IUnknown* GetCursor();
	CDataBoundProperty* GetNext();

	COleControlSite *m_pClientSite;  // Back ptr to containing site
	WORD m_ctlid;
	DISPID m_dispid;
	COleControlSite *m_pDSCSite;
	BOOL m_bOwnXferOut;
	BOOL m_bIsDirty;
	CDataBoundProperty* m_pNext;
};

#endif // !_AFX_NO_OCC_SUPPORT
