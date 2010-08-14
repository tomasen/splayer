/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include <atlcoll.h>
#include "BaseGraph.h"

class CMediaFormatCategory
{
protected:
	CString m_label, m_specreqnote;
  CString m_PerceivedType;
	CAtlList<CString> m_exts, m_backupexts;
	int m_fAudioOnly;
	engine_t m_engine;
  UINT m_IconType;

public:
	CMediaFormatCategory();
	CMediaFormatCategory(
		CString label, CAtlList<CString>& exts, CString PerceivedType, UINT IconType, int fAudioOnly = false,
		CString specreqnote =  _T(""), engine_t e = DirectShow);
	CMediaFormatCategory(
		CString label, CString exts, CString PerceivedType, UINT IconType, int fAudioOnly = false,
		CString specreqnote =  _T(""), engine_t e = DirectShow);
	virtual ~CMediaFormatCategory();

	void UpdateData(bool fSave);

	CMediaFormatCategory(const CMediaFormatCategory& mfc);
	CMediaFormatCategory& operator = (const CMediaFormatCategory& mfc);

	void RestoreDefaultExts();
	void SetExts(CAtlList<CString>& exts);
	void SetExts(CString exts);

	bool FindExt(CString ext) {return m_exts.Find(ext.TrimLeft(_T(".")).MakeLower()) != NULL;}

	CString GetLabel() {return m_label;}
	CString GetFilter();
	CString GetExts(bool fAppendEngine = false);
	void GetExtArray(CAtlArray<CString>& szaExts);
	CString GetExtsWithPeriod(bool fAppendEngine = false);
	CString GetBackupExtsWithPeriod(bool fAppendEngine = false);
	CString GetSpecReqNote() {return m_specreqnote;}
	int IsAudioOnly() {return m_fAudioOnly ;}
  CString getPerceivedType(){return m_PerceivedType;}
  UINT GetIconType() {return m_IconType;}
	engine_t GetEngineType() {return m_engine;}
	void SetEngineType(engine_t e) {m_engine = e;}
};

class CMediaFormats : public CAtlArray<CMediaFormatCategory>
{
protected:
	engine_t m_iRtspHandler;
	bool m_fRtspFileExtFirst;

public:
	CMediaFormats();
	virtual ~CMediaFormats();

	void UpdateData(bool fSave);

	engine_t GetRtspHandler(bool& fRtspFileExtFirst);
	void SetRtspHandler(engine_t e, bool fRtspFileExtFirst);

	bool IsUsingEngine(CString path, engine_t e);
	engine_t GetEngine(CString path);

	bool FindExt(CString ext, bool fAudioOnly = false);
	void GetExtsArray(CAtlArray<CString>& mask,  bool noAudio = TRUE);
	void GetFilter(CString& filter, CAtlArray<CString>& mask);
	void GetAudioFilter(CString& filter, CAtlArray<CString>& mask);
	BOOL IsAudioFile(CString szFilename);
	BOOL IsUnPlayableFile(CString szFilename, bool bRestrict = false);
	
};
