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

#include "stdafx.h"
#include <atlbase.h>
#include "MediaFormats.h"
#include "mplayerc.h"
//
// CMediaFormatCategory
//

CMediaFormatCategory::CMediaFormatCategory()
	: m_fAudioOnly(false)
{
}

CMediaFormatCategory::CMediaFormatCategory(
	CString label, CAtlList<CString>& exts, CString PerceivedType, int fAudioOnly,
	CString specreqnote, engine_t engine)
{
	m_label = label;
	m_exts.AddTailList(&exts);
	m_backupexts.AddTailList(&m_exts);
	m_specreqnote = specreqnote;
	m_fAudioOnly = fAudioOnly;
	m_engine = engine;
    m_PerceivedType = PerceivedType;
}

CMediaFormatCategory::CMediaFormatCategory(
	CString label, CString exts, CString PerceivedType, int fAudioOnly,
	CString specreqnote, engine_t engine)
{
	m_label = label;
	ExplodeMin(exts, m_exts, ' ');
	POSITION pos = m_exts.GetHeadPosition();
	while(pos) m_exts.GetNext(pos).TrimLeft('.');

	m_backupexts.AddTailList(&m_exts);
	m_specreqnote = specreqnote;
	m_fAudioOnly = fAudioOnly;
	m_engine = engine;
    m_PerceivedType = PerceivedType;
}

CMediaFormatCategory::~CMediaFormatCategory()
{
}

void CMediaFormatCategory::UpdateData(bool fSave)
{
	if(fSave)
	{
		AfxGetMyApp()->WriteProfileString(_T("FileFormats"), m_label, GetExts(true));
	}
	else
	{
		SetExts(AfxGetMyApp()->GetProfileString(_T("FileFormats"), m_label, GetExts(true)));
	}
}

CMediaFormatCategory::CMediaFormatCategory(const CMediaFormatCategory& mfc)
{
	*this = mfc;
}

CMediaFormatCategory& CMediaFormatCategory::operator = (const CMediaFormatCategory& mfc)
{
	m_label = mfc.m_label;
	m_specreqnote = mfc.m_specreqnote;
	m_exts.RemoveAll();
	m_exts.AddTailList(&mfc.m_exts);
	m_backupexts.RemoveAll();
	m_backupexts.AddTailList(&mfc.m_backupexts);
	m_fAudioOnly = mfc.m_fAudioOnly;
	m_engine = mfc.m_engine;
	return *this;
}

void CMediaFormatCategory::RestoreDefaultExts()
{
	m_exts.RemoveAll();
	m_exts.AddTailList(&m_backupexts);
}

void CMediaFormatCategory::SetExts(CAtlList<CString>& exts)
{
	m_exts.RemoveAll();
	m_exts.AddTailList(&exts);
}

void CMediaFormatCategory::SetExts(CString exts)
{
	m_exts.RemoveAll();
	ExplodeMin(exts, m_exts, ' ');
	POSITION pos = m_exts.GetHeadPosition();
	while(pos)
	{
		POSITION cur = pos;
		CString& ext = m_exts.GetNext(pos);
		if(ext[0] == '\\') {m_engine = (engine_t)_tcstol(ext.TrimLeft('\\'), NULL, 10); m_exts.RemoveAt(cur);}
		else ext.TrimLeft('.');
	}
}
void CMediaFormatCategory::GetExtArray(CAtlArray<CString>& szaExts)
{
	CString filter;
	POSITION pos = m_exts.GetHeadPosition();
	while(pos){
		szaExts.Add(_T("*.") + m_exts.GetNext(pos) );
	}
}
CString CMediaFormatCategory::GetFilter()
{
	CString filter;
	POSITION pos = m_exts.GetHeadPosition();
	while(pos) filter += _T("*.") + m_exts.GetNext(pos) + _T(";");
	filter.TrimRight(_T(";")); // cheap... 
	return(filter);
}
CString CMediaFormatCategory::GetExts(bool fAppendEngine)
{
	CString exts = Implode(m_exts, ' ');
	if(fAppendEngine) exts += CString(_T(" \\")) + (TCHAR)(0x30 + (int)m_engine);
	return(exts);
}

CString CMediaFormatCategory::GetExtsWithPeriod(bool fAppendEngine)
{
	CString exts;
	POSITION pos = m_exts.GetHeadPosition();
	while(pos) exts += _T(".") + m_exts.GetNext(pos) + _T(" ");
	exts.TrimRight(_T(" ")); // cheap...
	if(fAppendEngine) exts += CString(_T(" \\")) + (TCHAR)(0x30 + (int)m_engine);
	return(exts);
}

CString CMediaFormatCategory::GetBackupExtsWithPeriod(bool fAppendEngine)
{
	CString exts;
	POSITION pos = m_backupexts.GetHeadPosition();
	while(pos) exts += _T(".") + m_backupexts.GetNext(pos) + _T(" ");
	exts.TrimRight(_T(" ")); // cheap...
	if(fAppendEngine) exts += CString(_T(" \\")) + (TCHAR)(0x30 + (int)m_engine);
	return(exts);
}

//
// 	CMediaFormats
//

CMediaFormats::CMediaFormats()
{
}

CMediaFormats::~CMediaFormats()
{
}

void CMediaFormats::UpdateData(bool fSave)
{
	if(fSave)
	{
		AfxGetMyApp()->WriteProfileString(_T("FileFormats"), NULL, NULL);

		AfxGetMyApp()->WriteProfileInt(_T("FileFormats"), _T("RtspHandler"), m_iRtspHandler);
		AfxGetMyApp()->WriteProfileInt(_T("FileFormats"), _T("RtspFileExtFirst"), m_fRtspFileExtFirst);
	}
	else
	{
		RemoveAll();
#define ADDFMT(f) Add(CMediaFormatCategory##f)
		ADDFMT((_T("Windows Media file"), _T("wmv wmp wm asf"), _T("video")));
		ADDFMT((_T("Windows Media Audio file"), _T("wma"), _T("audio"), true));
		ADDFMT((_T("Video file"), _T("avi"), _T("video")));
		ADDFMT((_T("Audio file"), _T("wav"), _T("audio"), true));
		ADDFMT((_T("MPEG Media file"), _T("mpg mpeg mpe m1v m2v mpv2 mp2v"), _T("video")));
		ADDFMT((_T("VCD MPEG Data file"), _T("dat"), _T("video"),-1));
		ADDFMT((_T("Collegesoft Media file"), _T("csf"), _T("video")));
		ADDFMT((_T("MPEG Transport-Stream file"), _T("ts tp tpr pva pss m2ts m2t mts evo m2p mpls"), _T("video")));
		ADDFMT((_T("MPEG Audio file"), _T("mpa mp2 m1a m2a"), _T("audio"), true));
        ADDFMT((_T("VP8 Webm file"), _T("webm"), _T("video")));
		ADDFMT((_T("DVD file"), _T("vob ifo"), _T("video")));
		ADDFMT((_T("DVD Audio file"), _T("ac3 dts"), _T("audio"), true));
		ADDFMT((_T("MP3 Format Sound"), _T("mp3"), _T("audio"), true));
		ADDFMT((_T("MIDI file"), _T("mid midi rmi"), _T("audio"), true));
		ADDFMT((_T("Indeo Video file"), _T("ivf"), _T("video")));
		ADDFMT((_T("AIFF Format Sound"), _T("aif aifc aiff"), _T("audio"), true));
		ADDFMT((_T("Monkey's Audio APE File"), _T("ape"), _T("audio"), true));
		ADDFMT((_T("AU Format Sound"), _T("au snd"), _T("audio"), true));
		ADDFMT((_T("Ogg Media file"), _T("ogm ogv"), _T("video")));
		ADDFMT((_T("Ogg Vorbis Audio file"), _T("ogg"), _T("audio"), true));
		ADDFMT((_T("CD Audio Track"), _T("cda"), _T("audio"), true, _T("Windows 2000/XP or better")));
		ADDFMT((_T("FLIC file"), _T("fli flc flic"), _T("audio")));
		ADDFMT((_T("DVD2AVI Project file"), _T("d2v"), _T("video")));
		ADDFMT((_T("MPEG4 file"), _T("mp4 m4v hdmov"), _T("video")));
		ADDFMT((_T("Mobile Phone file"), _T("3gp 3gpp"), _T("video")));
		ADDFMT((_T("MPEG4 Audio file"), _T("m4a m4b aac"), _T("video"), true));
		ADDFMT((_T("Matroska Media file"), _T("mkv"), _T("video")));
		ADDFMT((_T("Matroska Audio file"), _T("mka"), _T("audio"), true));
		ADDFMT((_T("PSP/PMP Media file"), _T("pmp"), _T("video"), false));
		ADDFMT((_T("Smacker/Bink Media file"), _T("smk bik"), _T("video"), false, _T("smackw32/binkw32.dll in dll path")));
		ADDFMT((_T("ratdvd file"), _T("ratdvd"), _T("video"), false, _T("ratdvd media file")));
		ADDFMT((_T("RoQ Media file"), _T("roq"), _T("video"), false));
		ADDFMT((_T("Real Media file"), _T("rm rmvb ram rpm rmm"), _T("video"), false, _T("RealPlayer or Real Alternative") )); //RealMedia
		ADDFMT((_T("Real Audio file"), _T("ra"), _T("audio"), true, _T("RealPlayer or Real Alternative"))); // RealMedia
		ADDFMT((_T("Real Script file"), _T("rt rp smi smil"), _T("video"), false, _T("RealPlayer or Real Alternative"))); // RealMedia
		ADDFMT((_T("Dirac Video file"), _T("drc"), _T("video"), false));
		ADDFMT((_T("DirectShow Media file"), _T("dsm dsv dsa dss"), _T("video")));
		ADDFMT((_T("Musepack file"), _T("mpc"), _T("video"), true));
		ADDFMT((_T("FLAC Audio file"), _T("flac"), _T("audio"), true));
		ADDFMT((_T("ALAC Audio file"), _T("alac"), _T("audio"), true));
		ADDFMT((_T("Flash Video file"), _T("flv iflv f4v"), _T("video")));
		ADDFMT((_T("Shockwave Flash file"), _T("swf"), _T("video"), false, _T("ShockWave ActiveX control"), ShockWave));
		ADDFMT((_T("Shockwave Audio file"), _T("swa"), _T("audio"), true, _T("ShockWave ActiveX control"), ShockWave));
		ADDFMT((_T("Quicktime file"), _T("mov qt amr 3g2 3gp2"), _T("video"), false, _T("QuickTime or codec pack")));//QuickTime
		ADDFMT((_T("IVM file"), _T("ivm"), _T("video"), false));
		ADDFMT((_T("Image file"), _T("jpeg jpg bmp gif pic dib tiff tif"), _T("image") , -2)); //png not supported
		ADDFMT((_T("Playlist file"), _T("asx m3u pls wvx wax wmx mpcpl cue"), _T("Application")));
		ADDFMT((_T("Rar Playlist file"), _T("Application"), _T("rar"), -1));
		ADDFMT((_T("Subtitle file"), _T("srt idx sub ssa ass xss usf"), _T("text"), -2));
		ADDFMT((_T("Other"), _T("divx vp6 rmvb amv"), _T("video")));
#undef ADDFMT

		m_iRtspHandler = (engine_t)AfxGetMyApp()->GetProfileInt(_T("FileFormats"), _T("RtspHandler"), (int)DirectShow); //RealMedia
		m_fRtspFileExtFirst = !!AfxGetMyApp()->GetProfileInt(_T("FileFormats"), _T("RtspFileExtFirst"), 1);
	}

	//for(int i = 0; i < GetCount(); i++)
	//	GetAt(i).UpdateData(fSave);
}

engine_t CMediaFormats::GetRtspHandler(bool& fRtspFileExtFirst)
{
	fRtspFileExtFirst = m_fRtspFileExtFirst;
	return m_iRtspHandler;
}

void CMediaFormats::SetRtspHandler(engine_t e, bool fRtspFileExtFirst)
{
	m_iRtspHandler = e;
	m_fRtspFileExtFirst = fRtspFileExtFirst;
}

bool CMediaFormats::IsUsingEngine(CString path, engine_t e)
{
	return(GetEngine(path) == e);
}

engine_t CMediaFormats::GetEngine(CString path)
{
	path.Trim().MakeLower();

	if(!m_fRtspFileExtFirst && path.Find(_T("rtsp://")) == 0)
		return m_iRtspHandler;

	CString ext = CPath(path).GetExtension();
	ext.MakeLower();
	if(!ext.IsEmpty())
	{
		if(path.Find(_T("rtsp://")) == 0)
		{
			if(ext == _T(".ram") || ext == _T(".rm") || ext == _T(".ra"))
				return RealMedia;
			if(ext == _T(".qt") || ext == _T(".mov"))
				return QuickTime;
        }else if(ext == _T(".ram")){
            return RealMedia;
        }

		for(size_t i = 0; i < GetCount(); i++)
		{
			CMediaFormatCategory& mfc = GetAt(i);
			if(mfc.FindExt(ext))
				return mfc.GetEngineType();
		}
	}

	if(m_fRtspFileExtFirst && path.Find(_T("rtsp://")) == 0)
		return m_iRtspHandler;

	return DirectShow;
}

bool CMediaFormats::FindExt(CString ext, bool fAudioOnly)
{
	ext.TrimLeft(_T("."));

	if(!ext.IsEmpty())
	{
		for(size_t i = 0; i < GetCount(); i++)
		{
			CMediaFormatCategory& mfc = GetAt(i);
			if((!fAudioOnly || mfc.IsAudioOnly() == 1) && mfc.FindExt(ext)) 
				return(true);
		}
	}

	return(false);
}
void CMediaFormats::GetExtsArray(CAtlArray<CString>& mask, bool noAudio){

	for(size_t i = 0; i < GetCount(); i++) 
	{
		CMediaFormatCategory& mfc = GetAt(i);
		if( noAudio && mfc.IsAudioOnly() == 1 ) continue;
		if( mfc.GetEngineType() != DirectShow) continue;
		CString szLabel = mfc.GetLabel();
		if( szLabel.Find(_T("Subtitle")) >= 0 || szLabel.Find(_T("字幕")) >= 0 )  continue;
		if( szLabel.Find(_T("Image File")) >= 0 || szLabel.Find(_T("图片")) >= 0  )  continue;
		if( mfc.IsAudioOnly() <= -2 ) continue;
		mfc.GetExtArray(mask);
	}

}
void CMediaFormats::GetFilter(CString& filter, CAtlArray<CString>& mask)
{
	CString		strTemp;

	filter += _T("Media files (all types)|");
	mask.Add(_T(""));

	for(size_t i = 0; i < GetCount(); i++) 
	{
		strTemp  = GetAt(i).GetFilter() + _T(";");;
		mask[0] += strTemp;
		filter  += strTemp;
	}
	mask[0].TrimRight(_T(";"));
	filter.TrimRight(_T(";"));
	filter += _T("|");

	for(size_t i = 0; i < GetCount(); i++)
	{
		CMediaFormatCategory& mfc = GetAt(i);
		filter += mfc.GetLabel() + _T("|" + GetAt(i).GetFilter() + _T("|"));
		mask.Add(mfc.GetFilter());
	}

	filter.Append( ResStr(IDS_FILEOPEN_DISLOG_ALL_MEDIA_TYPE) );
	mask.Add(_T("*.*"));

	filter += _T("|");
}
BOOL CMediaFormats::IsUnPlayableFile(CString szFilename, bool bRestrict){
	CPath fPath(szFilename);
	CString szThisExtention = fPath.GetExtension();
	BOOL bDefaultRet = false;
	if(bRestrict)
		bDefaultRet = true;

	for(size_t i = 0; i < GetCount(); i++)
	{
		CMediaFormatCategory& mfc = GetAt(i);
		if( mfc.FindExt(szThisExtention) ){
			CString szLabel = mfc.GetLabel();
			if ( szLabel.Find(_T("Subtitle")) >= 0 || szLabel.Find(_T("字幕")) >= 0){
				return TRUE;
			}
			if ( szLabel.Find(_T("Image file")) >= 0 || szLabel.Find(_T("图片")) >= 0){
				return TRUE;
			}
			if ( szLabel.Find(_T("Real Script file")) >= 0 || szLabel.Find(_T("脚本")) >= 0){
				return TRUE;
			}

			return FALSE;
		}
	}
	return bDefaultRet;
}
BOOL CMediaFormats::IsAudioFile(CString szFilename){
	CPath fPath(szFilename);
	CString szThisExtention = fPath.GetExtension();

	for(size_t i = 0; i < GetCount(); i++)
	{
		CMediaFormatCategory& mfc = GetAt(i);
		if(mfc.IsAudioOnly() != 1 || mfc.GetEngineType() != DirectShow) continue;
		if( mfc.FindExt(szThisExtention) ){
			return TRUE;
		}
	}
	return FALSE;
}
void CMediaFormats::GetAudioFilter(CString& filter, CAtlArray<CString>& mask)
{
	CString		strTemp;
	filter += _T("Audio files (all types)|");
	mask.Add(_T(""));

	for(size_t i = 0; i < GetCount(); i++)
	{
		CMediaFormatCategory& mfc = GetAt(i);
		if(mfc.IsAudioOnly() != 1 || mfc.GetEngineType() != DirectShow) continue;
		strTemp  = GetAt(i).GetFilter() + _T(";");
		mask[0] += strTemp;
		filter  += strTemp;
	}

	mask[0].TrimRight(_T(";"));
	filter.TrimRight(_T(";"));
	filter += _T("|");

	for(size_t i = 0; i < GetCount(); i++)
	{
		CMediaFormatCategory& mfc = GetAt(i);
		if(mfc.IsAudioOnly() != 1 || mfc.GetEngineType() != DirectShow) continue;
		filter += mfc.GetLabel() + _T("|") + GetAt(i).GetFilter() + _T("|");
		mask.Add(mfc.GetFilter());
	}

	filter += _T("All files (*.*)|(*.*)|");
	mask.Add(_T("*.*"));

	filter += _T("|");
}
