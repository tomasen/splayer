// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __AFX_WINFORMS_COMMAND_TARGET_INTERFACES_H__
#define __AFX_WINFORMS_COMMAND_TARGET_INTERFACES_H__
////////////////////////////////////////////////////////////////////////////////
// Implements the ICommandSource and ICommandUI interfaces for CWinFromsView
// WM_COMMAND Routing support.

/////////////////////////////////////////////////////////////////////////////
// CCommandSource - WinForms form hosted as MFC View uses a reference to an instance
// of CCommandSource to add and remove command handler, thus participating in MFC
// Command Routing mechanism
namespace Microsoft{
	namespace VisualC{
		namespace MFC{

public ref class CCommandSource : public ICommandSource
{
	ref struct Range
	{
		const unsigned int m_min;
		const unsigned int m_max;
		System::MulticastDelegate ^const m_pHandler;

		Range(unsigned int min, unsigned int max,
			System::MulticastDelegate ^ pHandler)
			: m_min(min), m_max(max), m_pHandler(pHandler) {}
	};

	ref struct ExactRange : public Range
	{
		ExactRange(unsigned int min, unsigned int max)
			: Range(min, max, nullptr) {}
		virtual bool Equals(Object ^o) override;	//used by IndexOf		
	};

	ref struct ContainedRange : public Range
	{
		ContainedRange(unsigned int id)
			: Range(id, id, nullptr) {}
		virtual bool Equals(Object ^o) override;	//used by IndexOf		
	};

	System::Collections::ArrayList ^m_alCmd;
	System::Collections::ArrayList ^m_alCmdUI;

	static System::MulticastDelegate^ FindHandler(System::Collections::ArrayList ^al, UINT nID);
	
public:
	CCommandSource();
	//Command Handlers
	virtual void AddCommandHandler(unsigned int cmdID, CommandHandler^ cmdHandler);	
	virtual void AddCommandRangeHandler(unsigned int cmdIDMin, unsigned int cmdIDMax, 
								CommandHandler^ cmdHandler);	
	virtual void RemoveCommandHandler(unsigned int cmdID);
	virtual void RemoveCommandRangeHandler(unsigned int cmdIDMin, unsigned int cmdIDMax);
	CommandHandler^ FindCommandHandler(UINT nID);
	//Update Command Handlers
	virtual void AddCommandUIHandler(unsigned int cmdID, CommandUIHandler^ cmdUIHandler);
	virtual void AddCommandRangeUIHandler(unsigned int cmdIDMin, unsigned int cmdIDMax, 
								  CommandUIHandler^ cmdUIHandler);
	virtual void RemoveCommandUIHandler(unsigned int cmdID);	
	virtual void RemoveCommandRangeUIHandler(unsigned int cmdIDMin, unsigned int cmdIDMax);	
	CommandUIHandler^ FindCommandUIHandler(UINT nID);	

	virtual void PostCommand(unsigned int command );	
	virtual void SendCommand(unsigned int command );	
};


/////////////////////////////////////////////////////////////////////////////
// CCmdUI wrapper
//
public ref class CCommandUI : public ICommandUI
{
	CCmdUI &m_CmdUI;
public:
	CCommandUI(CCmdUI &CmdUI) : m_CmdUI(CmdUI) {}

	virtual void ContinueRouting();	
	property bool  Enabled 
	{
		virtual void set(bool value);
	}
	property UICheckState Check
	{	
		virtual void set(UICheckState value);
	}
	property bool  Radio
	{
		virtual void set(bool value);
	}
	property System::String^  Text
	{
		virtual void set(System::String^ value);
	}
	property UINT  ID
	{
		virtual UINT get();
	}
	property UINT  Index
	{
		virtual UINT get();
	}				
	
};
		} //MFC
	} //VisualC
} //Microsoft

#endif //__AFX_WINFORMS_COMMAND_TARGET_INTERFACES_H__