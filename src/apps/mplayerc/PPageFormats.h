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

// CPPageFormats dialog

class CPPageFormats
{

private:

	BOOL m_bCheckDefaultPlayer;

	void SetListItemState(int nItem);
	static CString GetEnqueueCommand();
	static CString GetOpenCommand(bool open_in_new_windows = false);
	static CComPtr<IApplicationAssociationRegistration>	m_pAAR;
	static BOOL SetFileAssociation(CString strExt, CString strProgID, bool fRegister);
	static CString GetFileIcon(CString strExt);
  static CString GetFileTypeName(CString strExt);
public:
	typedef enum {AP_VIDEO=0,AP_MUSIC,AP_AUDIOCD,AP_DVDMOVIE, AP_SVCDMOVIE, AP_VCDMOVIE, AP_BDMOVIE, AP_DVDAUDIO, AP_CAPTURECAMERA , } autoplay_t;
	void AddAutoPlayToRegistry(autoplay_t ap, bool fRegister);
	bool IsAutoPlayRegistered(autoplay_t ap);


	CPPageFormats();
	virtual ~CPPageFormats();

	static bool IsRegistered(CString ext);
	static bool RegisterExt(CString ext, bool fRegister, CString PerceivedType);
	
	enum {COL_CATEGORY, COL_ENGINE};
	CString m_exts;
	CStatic m_autoplay;
	CButton m_apvideo;
	CButton m_apmusic;
	CButton m_apaudiocd;
	CButton m_apdvd;
	int m_iRtspHandler;
	BOOL m_fRtspFileExtFirst;
	BOOL m_bInsufficientPrivileges;
	

};
