// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "afxwfrmcmd.h"

namespace Microsoft {
	namespace VisualC {
			namespace MFC {

//////////////////////////////////////////////////////////////////////////
//CCommandSource
//

System::MulticastDelegate ^ CCommandSource::FindHandler(System::Collections::ArrayList ^al, UINT nID)
{
	int i = 0;
	System::Collections::IEnumerator^ cmdEnum = al->GetEnumerator();
	while ( cmdEnum->MoveNext() )
	{
		Range ^r = safe_cast<Range^>(cmdEnum->Current);
		if ( r->m_min <= nID && r->m_max >= nID )
		{
			return safe_cast<Range^>(al[i])->m_pHandler;
		}
		i++;
	}
	return nullptr;
}

bool CCommandSource::ExactRange::Equals(Object ^o)	//used by IndexOf
{
	Range ^r = safe_cast <Range^>(o);
	return m_min == r->m_min && m_max == r->m_max;
}

bool CCommandSource::ContainedRange::Equals(Object ^o)	//used by IndexOf
{
	Range ^r = safe_cast<Range^>(o);
	return m_min >= r->m_min && m_min <= r->m_max;
}

CCommandSource::CCommandSource()
{
	m_alCmd = gcnew System::Collections::ArrayList;
	m_alCmdUI = gcnew System::Collections::ArrayList;
}

void CCommandSource::AddCommandHandler(unsigned int cmdID, CommandHandler^ cmdHandler)
{
	ENSURE_ARG(cmdHandler!=nullptr);
	m_alCmd->Add(gcnew Range(cmdID, cmdID, cmdHandler));
}

void CCommandSource::AddCommandRangeHandler(unsigned int cmdIDMin, unsigned int cmdIDMax, 
								CommandHandler^ cmdHandler)
{
	ENSURE_ARG(cmdHandler!=nullptr);
	m_alCmd->Add(gcnew Range(cmdIDMin, cmdIDMax, cmdHandler));
}

void CCommandSource::RemoveCommandHandler(unsigned int cmdID)
{
	int i = 0;
	System::Collections::IEnumerator^ cmdEnum = m_alCmd->GetEnumerator();
	while ( cmdEnum->MoveNext() )
	{
		Range ^r = safe_cast<Range^>(cmdEnum->Current);
		if ( r->m_min == cmdID )
		{
			break;
		}
		i++;
	}
	m_alCmd->RemoveAt(i);
}

void CCommandSource::RemoveCommandRangeHandler(unsigned int cmdIDMin, unsigned int cmdIDMax)
{
	int i = 0;
	System::Collections::IEnumerator^ cmdEnum = m_alCmd->GetEnumerator();
	while ( cmdEnum->MoveNext() )
	{
		Range ^r = safe_cast<Range^>(cmdEnum->Current);
		if ( r->m_min == cmdIDMin && r->m_max == cmdIDMax )
		{
			break;
		}
		i++;
	}
	m_alCmd->RemoveAt(i);
}

void CCommandSource::AddCommandUIHandler(unsigned int cmdID, CommandUIHandler^ cmdUIHandler)
{
	ENSURE_ARG(cmdUIHandler!=nullptr);
	m_alCmdUI->Add(gcnew Range(cmdID, cmdID, cmdUIHandler));
}
void CCommandSource::AddCommandRangeUIHandler(unsigned int cmdIDMin, unsigned int cmdIDMax, 
							  CommandUIHandler^ cmdUIHandler)
{
	ENSURE_ARG(cmdUIHandler!=nullptr);
	m_alCmdUI->Add(gcnew Range(cmdIDMin, cmdIDMax, cmdUIHandler));
}
void CCommandSource::RemoveCommandUIHandler(unsigned int cmdID)
{
	int i = 0;
	System::Collections::IEnumerator^ cmdEnum = m_alCmdUI->GetEnumerator();
	while ( cmdEnum->MoveNext() )
	{
		Range ^r = safe_cast<Range^>(cmdEnum->Current);
		if ( r->m_min == cmdID )
		{
			break;
		}
		i++;
	}
	m_alCmdUI->RemoveAt(i);
}

void CCommandSource::RemoveCommandRangeUIHandler(unsigned int cmdIDMin, unsigned int cmdIDMax)
{
	int i = 0;
	System::Collections::IEnumerator^ cmdEnum = m_alCmdUI->GetEnumerator();
	while ( cmdEnum->MoveNext() )
	{
		Range ^r = safe_cast<Range^>(cmdEnum->Current);
		if ( r->m_min == cmdIDMin && r->m_max == cmdIDMax )
		{
			break;
		}
		i++;
	}
	m_alCmdUI->RemoveAt(i);
}

void CCommandSource::PostCommand(unsigned int command )
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND, command);
}
void CCommandSource::SendCommand(unsigned int command )
{
	AfxGetMainWnd()->SendMessage(WM_COMMAND, command);
}

CommandHandler^ CCommandSource::FindCommandHandler(UINT nID)
{
	return safe_cast<CommandHandler^>(FindHandler(m_alCmd, nID));
}

CommandUIHandler^ CCommandSource::FindCommandUIHandler(UINT nID)
{
	return safe_cast<CommandUIHandler^>(FindHandler(m_alCmdUI, nID));
}

/////////////////////////////////////////////////////////////////////////////
// CCommandUI
//
void CCommandUI::ContinueRouting()
{
	m_CmdUI.ContinueRouting();
}

void CCommandUI::Enabled::set(bool bOn)
{
	m_CmdUI.Enable(bOn);
}
void CCommandUI::Check::set(UICheckState Check)
{
	m_CmdUI.SetCheck((System::Int32)Check);
}
void CCommandUI::Radio::set(bool bOn)
{
	m_CmdUI.SetRadio(bOn);
}
void CCommandUI::Text::set(System::String ^text)
{
	m_CmdUI.SetText(CString(text));
}
unsigned int CCommandUI::ID::get()
{
	return m_CmdUI.m_nID;
}
unsigned int CCommandUI::Index::get()
{
	return m_CmdUI.m_nIndex;
}
		} //MFC
	} //VisualC
} //Microsoft

// Calling this function ensures that managed code is executed in this module
// This in turn ensures that the module constructor (.cctor) is executed and global
// objects are correctly initialized.
extern "C" void AfxmEnsureManagedInitialization()
{
    // Just need some cheap managed code that won't be optimized away
    System::GC::KeepAlive(System::Int32::MaxValue);
}

// Calling this function ensures that all managed references to the specified
// object are released.
extern "C" void AfxmReleaseManagedReferences(IUnknown* pIUnknown)
{
	System::IntPtr pUnknAsInt = static_cast<System::IntPtr>(pIUnknown);
	System::Object^ oManagedWrapper = System::Runtime::InteropServices::Marshal::GetObjectForIUnknown( pUnknAsInt );
	while( System::Runtime::InteropServices::Marshal::ReleaseComObject( oManagedWrapper ) );
}

/////////////////////////////////////////////////////////////////////////////
// Assembly Information

#include <atlbuild.h>

using namespace System::Reflection;

#ifdef BUILD_PREBUILT

// TEMP (alecont)
#define _MFC_FILENAME_VER_DOT "9.0"

[assembly: AssemblyTitle("mfcm" _MFC_FILENAME_VER)];
[assembly: AssemblyDescription("MFC " _MFC_FILENAME_VER_DOT " Managed Interoperability")];
[assembly: AssemblyConfiguration("")];
[assembly: AssemblyCompany("Microsoft")];
[assembly: AssemblyProduct("Visual Studio 9.0")];
[assembly: AssemblyCopyright("Microsoft Corporation")];
[assembly: AssemblyTrademark("")];
[assembly: AssemblyCulture("")];

[assembly: AssemblyVersion(_LIBS_USER_FULL_VER)];

[assembly: AssemblyDelaySign(true)];
[assembly: AssemblyKeyFile("..\\finalpublickey.snk")];
[assembly: AssemblyKeyName("")];

#else

#error Following information required to build private version


[assembly: AssemblyTitle("")];
[assembly: AssemblyDescription("MFC " "9.0" " Managed Interoperability")];
[assembly: AssemblyConfiguration("")];
[assembly: AssemblyCompany("")];
[assembly: AssemblyProduct("")];
[assembly: AssemblyCopyright("")];
[assembly: AssemblyTrademark("")];
[assembly: AssemblyCulture("")];

[assembly: AssemblyVersion("1.0.0.0")];

[assembly: AssemblyDelaySign(true)];
[assembly: AssemblyKeyFile("")];
[assembly: AssemblyKeyName("")];

#endif
