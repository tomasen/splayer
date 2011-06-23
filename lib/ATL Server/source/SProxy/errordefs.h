//
// errordefs.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "resource.h"

enum EMIT_TYPE 
{
	EMIT_ERROR = 0, 
	EMIT_WARNING
};

struct EMITDESC
{
	EMIT_TYPE emitType;
	UINT nEmitID;
	UINT nMsgID;
	bool bForce;
};

#define EMITERR_BASE 1000
#define EMITWARN_BASE 4000

#define DECLARE_EMITERR(__nEmitID, __nMsgID) \
	{ EMIT_ERROR, EMITERR_BASE + __nEmitID, __nMsgID, false },

#define DECLARE_EMITERR_FORCE(__nEmitID, __nMsgID) \
	{ EMIT_ERROR, EMITERR_BASE + __nEmitID, __nMsgID, true },

#define DECLARE_EMITWARN(__nEmitID, __nMsgID) \
	{ EMIT_WARNING, EMITWARN_BASE + __nEmitID, __nMsgID, true },

#ifndef _PREFIX_
extern __declspec(selectany) const EMITDESC g_Emits[] =
#else
extern const EMITDESC g_Emits[] =
#endif // _PREFIX_
{
// errors

	DECLARE_EMITERR_FORCE(1, IDS_SDL_PROCESS_FAILURE)
	DECLARE_EMITERR_FORCE(2, IDS_SDL_GENERATE_FAILURE)
	DECLARE_EMITERR(3, IDS_SDL_INTERNAL)
	DECLARE_EMITERR(4, IDS_SDL_PARSE_ERROR)
	DECLARE_EMITERR(5, IDS_SDL_UNRECOGNIZED_TAG)
	DECLARE_EMITERR(6, IDS_SDL_MISSING_ATTRIBUTE)
	DECLARE_EMITERR(7, IDS_SDL_MSXML)
	DECLARE_EMITERR(8, IDS_SDL_UNRECOGNIZED_DOC)
	DECLARE_EMITERR(9, IDS_SDL_UNRESOLVED_ELEM)
	DECLARE_EMITERR(10, IDS_SDL_UNRESOLVED_NAMESPACE)
	DECLARE_EMITERR(11, IDS_SDL_LITERAL_ONLY)
	DECLARE_EMITERR(12, IDS_SDL_INVALID_VALUE)
	DECLARE_EMITERR(13, IDS_SDL_UNRESOLVED_ELEM2)
	DECLARE_EMITERR_FORCE(14, IDS_SDL_RECURSIVE_TYPE)
	DECLARE_EMITERR_FORCE(15, IDS_SDL_MISSING_OPTION)
	DECLARE_EMITERR(16, IDS_SDL_FAILED_WSDL_OPEN)
	DECLARE_EMITERR(17, IDS_SDL_UNRESOLVED_MSGPART)
	DECLARE_EMITERR(18, IDS_SDL_SOAPHEADER_DUPNAME)
	DECLARE_EMITERR(19, IDS_SDL_SCHEMALEVEL_NAME)
	DECLARE_EMITERR(20, IDS_SDL_PAD_TYPE)
	DECLARE_EMITERR(21, IDS_SDL_PAD_INVALID_SOAP)
	DECLARE_EMITERR(22, IDS_SDL_RPC_ENCODED_TYPE)
	DECLARE_EMITERR(23, IDS_SDL_DOC_ENCODED)
	DECLARE_EMITERR(24, IDS_SDL_ENCODINGSTYLE)
	DECLARE_EMITERR(25, IDS_SDL_IO_DIFF_NAMESPACES)
	DECLARE_EMITERR(26, IDS_SDL_RPC_LITERAL)
	DECLARE_EMITERR(27, IDS_SDL_HEADER_DIFF_NAMESPACES)
	DECLARE_EMITERR(28, IDS_SDL_INVALID_ARRAY_DESC_ERR)
	DECLARE_EMITERR_FORCE(29, IDS_SDL_NO_GENERATE)
	DECLARE_EMITERR(30, IDS_SDL_BASE_EXTENSION)
	DECLARE_EMITERR_FORCE(31, IDS_SDL_PROCESS_DM_FAILURE)
	DECLARE_EMITERR(32, IDS_SDL_FAILED_DM_OPEN)
// warnings

	DECLARE_EMITWARN(0, IDS_SDL_PARSE_WARNING)
	DECLARE_EMITWARN(1, IDS_SDL_ONE_PORT)
	DECLARE_EMITWARN(2, IDS_SDL_SOAP_PORT_ONLY)
	DECLARE_EMITWARN(3, IDS_SDL_SOAP_BINDING_ONLY)
	DECLARE_EMITWARN(4, IDS_SDL_UNSUPPORTED_TAG)
	DECLARE_EMITWARN(5, IDS_SDL_UNSUPPORTED_STRING)
	DECLARE_EMITWARN(6, IDS_SDL_IGNORE_CMDITEM)
	DECLARE_EMITWARN(7, IDS_SDL_INVALID_ARRAY_DESC)
	DECLARE_EMITWARN(8, IDS_SDL_SKIP_EXTENSIBILITY)
	DECLARE_EMITWARN(9, IDS_SDL_CUSTOM_TYPE)
	DECLARE_EMITWARN(10, IDS_SDL_NO_ATTRIBUTES)
	DECLARE_EMITWARN(11, IDS_SDL_DEFAULT_TYPE)
};

class CEmitError
{
private:

	bool m_bEmitted;
	bool m_bEmitWarnings;

	int GetEmitIndex(UINT uID)
	{
		for (int i=0; i<(sizeof(g_Emits)/sizeof(g_Emits[0])); i++)
		{
			if (uID == g_Emits[i].nMsgID)
			{
				return i;
			}
		}

		return -1;
	}

	void EmitCmdLineMsg(UINT uID, va_list args, bool bError = true)
	{
		char *szMsg = (bError != false) ? "error" : "warning";

		CStringA m_strMsg;
		if (m_strMsg.LoadString(IDS_SDL_CMDLINE))
		{
			printf((LPCSTR) m_strMsg, szMsg);
		}

		int nIndex = GetEmitIndex(uID);
		if (nIndex != -1)
		{
			printf("D%d : ", g_Emits[nIndex].nEmitID);
			Emit(uID, args);
		}
	}

	void EmitFileMsg(UINT uID, LPCWSTR wszFile, int nLine, int nCol, UINT uIDExtra, va_list args, bool bError)
	{
		char *szMsg = (bError != false) ? "error" : "warning";
		int nIndex = GetEmitIndex(uID);
		if ((nIndex != -1) &&
			((bError == false) ||
			 (m_bEmitted == false) || (g_Emits[nIndex].bForce != false)))
		{
			CStringA strExtra;
			if (uIDExtra != 0)
			{
				if (!strExtra.LoadString(uIDExtra))
				{
					return;
				}
			}
			printf("\r\n%ws(%d,%d) : %s%s SDL%u : ", wszFile, nLine, nCol, 
				(LPCSTR)strExtra, szMsg, g_Emits[nIndex].nEmitID);	
			Emit(uID, args);
			
			if (bError != false)
			{
				m_bEmitted = true;
			}
		}
	}
	
public:

	CEmitError()
		:m_bEmitted(false), m_bEmitWarnings(true)
	{
	}

	void EmitErrorHr(HRESULT hr)
	{
		if (m_bEmitted == false)
		{
			char *szMsg = NULL;
			DWORD dwLen = ::FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM, 
				NULL, hr, 0, (LPSTR)&szMsg, 0, NULL);
			
			if (dwLen != 0)
			{
				printf("\r\nsproxy : error SDL%u : %s", EMITERR_BASE, szMsg);
				::LocalFree(szMsg);

				m_bEmitted = true;
			}
		}
	}

	void EmitError(UINT uID, va_list args)
	{
		int nIndex = GetEmitIndex(uID);
		if ((nIndex != -1) && ((m_bEmitted == false) || (g_Emits[nIndex].bForce != false)))
		{
			printf("\r\nsproxy : error SDL%u : ", g_Emits[nIndex].nEmitID);
			Emit(uID, args);

			m_bEmitted = true;
		}
	}

	void EmitWarning(UINT uID, va_list args)
	{
		if (m_bEmitWarnings != false)
		{
			int nIndex = GetEmitIndex(uID);
			if (nIndex != -1)
			{
				printf("\r\nsproxy : warning SDL%u : ", g_Emits[nIndex].nEmitID);
				Emit(uID, args);
			}
		}
	}

	void Emit(UINT uID, va_list args)
	{
		CStringA m_strMsg;
		if (m_strMsg.LoadString(uID))
		{
			vprintf((LPCSTR) m_strMsg, args);
		}
	}

	bool SetEmitWarnings(bool bWarn)
	{
		bool bPrev = m_bEmitWarnings;
		m_bEmitWarnings = bWarn;
		return bPrev;
	}

	void EmitCmdLineError(UINT uID, va_list args)
	{
		EmitCmdLineMsg(uID, args, true);
	}

	void EmitCmdLineWarning(UINT uID, va_list args)
	{
		EmitCmdLineMsg(uID, args, false);
	}

	void EmitFileWarning(UINT uID, LPCWSTR wszFile, int nLine, int nCol, UINT uIDExtra, va_list args)
	{
		if (m_bEmitWarnings != false)
		{
			EmitFileMsg(uID, wszFile, nLine, nCol, uIDExtra, args,  false);
		}
	}

	void EmitFileError(UINT uID, LPCWSTR wszFile, int nLine, int nCol, UINT uIDExtra, va_list args)
	{
		EmitFileMsg(uID, wszFile, nLine, nCol, uIDExtra, args, true);
	}
};

#ifndef _PREFIX_
__declspec(selectany) CEmitError g_Emit;
#else
CEmitError g_Emit;
#endif