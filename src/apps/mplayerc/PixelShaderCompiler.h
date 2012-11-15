/*
* $Id: PixelShaderCompiler.h 193 2007-09-09 09:12:21Z alexwild $
*
* (C) 2003-2006 Gabest
* (C) 2006-2007 see AUTHORS
*
* This file is part of mplayerc.
*
* Mplayerc is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* Mplayerc is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#pragma once

#include <d3d9helper.h>
#include <d3d9.h>


class CPixelShaderCompiler
{ 
	typedef HRESULT (WINAPI * D3DXCompileShaderPtr) (
		LPCSTR		pSrcData,
		UINT		SrcDataLen,
		CONST D3D_SHADER_MACRO* pDefines,
		ID3DInclude *	pInclude,
		LPCSTR		pFunctionName,
		LPCSTR		pProfile,
		DWORD		Flags,
		ID3DBlob  *	ppShader,
		ID3DBlob *	ppErrorMsgs,
		ID3DBlob * ppConstantTable);

	typedef HRESULT (WINAPI * D3DXDisassembleShaderPtr) (
		CONST DWORD*	pShader, 
		bool		EnableColorCode, 
		LPCSTR		pComments, 
		ID3DBlob *	ppDisassembly);

	D3DXCompileShaderPtr m_pD3DXCompileShader;
	D3DXDisassembleShaderPtr m_pD3DXDisassembleShader;

	CComPtr<IDirect3DDevice9> m_pD3DDev;

public:
	CPixelShaderCompiler(IDirect3DDevice9* pD3DDev, bool fStaySilent = false);
	virtual ~CPixelShaderCompiler();

	HRESULT CompileShader(
		LPCSTR pSrcData,
		LPCSTR pFunctionName,
		LPCSTR pProfile,
		DWORD Flags,
		IDirect3DPixelShader9** ppPixelShader,
		CString* disasm = NULL,
		CString* errmsg = NULL);
};