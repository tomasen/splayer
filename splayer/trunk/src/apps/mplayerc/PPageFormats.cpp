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

// PPageFormats.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "PPageFormats.h"
#define CINTERFACE
#include <shobjidl.h>
#include "../../svplib/SVPToolBox.h"
//CString			g_strRegisteredAppName = _T("SPlayer") ;//cant use ResStr(IDS_FILEASSOC_REG_APPNAME);
//CString			g_strRegisteredKey	= _T("Software\\Clients\\Media\\SPlayer\\Capabilities");

CString			g_strRegisteredAppName = _T("射手影音播放器") ;//cant use ResStr(IDS_FILEASSOC_REG_APPNAME);
CString			g_strOldAssoc		  = _T("PreviousRegistration");
CString			g_strRegisteredKey	= _T("Software\\Clients\\Media\\射手影音播放器\\Capabilities");
BOOL f_setContextFiles  = true;
BOOL f_setAssociatedWithIcon  = true;
// CPPageFormats dialog
CComPtr<IApplicationAssociationRegistration>	CPPageFormats::m_pAAR;

CPPageFormats::CPPageFormats()
:	m_exts(_T(""))
	, m_iRtspHandler(0)
	, m_fRtspFileExtFirst(FALSE)
	, m_bInsufficientPrivileges(FALSE)
{
	if (AfxGetMyApp()->IsVista() && !IsUserAnAdmin())
	{
		
		m_bInsufficientPrivileges = true;
	}

	//TODO: detetc language setting and change  g_strRegisteredAppName  g_strRegisteredKey
	AppSettings& s = AfxGetAppSettings();
	if(!s.bIsChineseUIUser()){
		g_strRegisteredAppName = _T("SPlayer") ;
		g_strRegisteredKey	= _T("Software\\Clients\\Media\\SPlayer\\Capabilities");
	
	}
	
}

CPPageFormats::~CPPageFormats()
{
}


static bool MakeRegParams(CString ext, CString& path, CString& fn, CString& extfile, CString& cmd)
{
	if(ext.GetLength() == 0)
		return(false);

	TCHAR buff[MAX_PATH];
	if(::GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH) == 0)
		return(false);

	path = buff;

	fn = path.Mid(path.ReverseFind('\\')+1).MakeLower();
	if(fn.IsEmpty())
		return(false);

	extfile = ext.TrimLeft('.')+_T("file");

	cmd = _T("\"") + path + _T("\" \"%1\"");

	return(true);
}

bool CPPageFormats::IsRegistered(CString ext)
{

	BOOL	bIsDefault = FALSE;
	CString strProgID = _T("SPlayer") + ext;
	CString FileIcon = GetFileIcon(ext);
	if ( m_pAAR == NULL)
	{
		// Default manager (requiered at least Vista)
		HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration,
			NULL,
			CLSCTX_INPROC ,
			__uuidof(IApplicationAssociationRegistration),
			(void**)&m_pAAR);
		
	}
	
	if (m_pAAR)
	{
		// The Vista way
		HRESULT	hr;
		hr = m_pAAR->QueryAppIsDefault(ext, AT_FILEEXTENSION, AL_EFFECTIVE, g_strRegisteredAppName, &bIsDefault);
	}
	else
	{
		// The 2000/XP way
		CRegKey		key;
		TCHAR		buff[256];
		ULONG		len = sizeof(buff);
		memset(buff, 0, len);

		if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext))
			return false;

		if(ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len) && !CString(buff).Trim().IsEmpty())
			return false;

		bIsDefault = (buff == strProgID);
	}
	if(!f_setContextFiles)
	{
		CRegKey		key;
		TCHAR		buff[MAX_PATH];
		ULONG		len = sizeof(buff);

		if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open"), KEY_READ))
		{
			CString		strCommand = ResStr(IDS_OPEN_WITH_MPC);
			if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len))
				f_setContextFiles = (strCommand.CompareNoCase(CString(buff)) == 0);
		}
	}

	// Check if association is for this instance of MPC
	if (bIsDefault)
	{
		CRegKey		key;
		TCHAR		buff[MAX_PATH];
		ULONG		len = sizeof(buff);

		bIsDefault = FALSE;
		if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open\\command"), KEY_READ))
		{
			CString		strCommand = GetOpenCommand();
			if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len))
				bIsDefault = (strCommand.CompareNoCase(CString(buff)) == 0);
		}

	}

	if(bIsDefault){
	//	SetFileAssociation(ext, strProgID, TRUE);
	}

	return !!bIsDefault;
}
CString CPPageFormats::GetFileIcon(CString strExt){
	TCHAR           buff[MAX_PATH];
	ULONG           len = sizeof(buff);
	memset(buff, 0, len);
	CString FileIcon;
	UINT iconId = 0;

	CString szExtIconfile(strExt);
	szExtIconfile.Trim('.');
	CSVPToolBox svpTool;
	szExtIconfile = svpTool.GetPlayerPath(CString(_T("skins\\icons\\"))+szExtIconfile + _T(".ico") );
	if(svpTool.ifFileExist(szExtIconfile)){
		return szExtIconfile;
	}
	//if(isSubtitleFile(strExt)){
	//	iconId = IDI_SUBTITLE_FILE;
	//}else
  {
		CMediaFormats& mf = AfxGetAppSettings().Formats;
		for(size_t i = 0; i < mf.GetCount(); i++)
		{
			if ( mf[i].FindExt(strExt) ){
				
					CString szType = mf[i].GetLabel();

					szExtIconfile = szType;
					szExtIconfile = svpTool.GetPlayerPath(CString(_T("skins\\icons\\"))+szExtIconfile.Trim() + _T(".ico") );
					if(svpTool.ifFileExist(szExtIconfile))
						return szExtIconfile;

          iconId = mf[i].GetIconType();

				break;
			}
			
		}
	}

    if (iconId > 0)
    {
      ::GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH);
      FileIcon.Format(_T("\"%s\",%d"), buff, iconId + 2 - IDI_AUDIOCD); 
      
      return FileIcon;
    }
    else
    {
      return _T("");
    }
}
CString CPPageFormats::GetFileTypeName(CString strExt)
{
  CMediaFormats& mf = AfxGetAppSettings().Formats;
  for(size_t i = 0; i < mf.GetCount(); i++)
  {
    if ( mf[i].FindExt(strExt) ){

      return mf[i].GetLabel();
      break;
    }
  }

  return _T("SPlayer") + strExt;
}
BOOL CPPageFormats::SetFileAssociation(CString strExt, CString strProgID, bool fRegister)
{
	CString         extoldreg, extOldIcon;
	CRegKey         key;
	HRESULT         hr = S_OK;
	TCHAR           buff[MAX_PATH];
	ULONG           len = sizeof(buff);
	memset(buff, 0, len);

	if (m_pAAR == NULL)
	{
		// Default manager (requiered at least Vista)
		HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration,
			NULL,
			CLSCTX_INPROC,
			__uuidof(IApplicationAssociationRegistration),
			(void**)&m_pAAR);
		if (FAILED(hr))
		{
			m_pAAR = NULL;
		}
	}
	CString FileIcon = GetFileIcon(strExt);

	if (m_pAAR)
	{
		    // Register MPC for the windows "Default application" manager
		
			if(ERROR_SUCCESS == key.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\RegisteredApplications")))
			{
				key.SetStringValue(g_strRegisteredAppName/*_T("Media Player Classic")*/, g_strRegisteredKey);

				
				if(ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE, g_strRegisteredKey))
					return(false);

				// ==>>  TODO icon !!!
				key.SetStringValue(_T("ApplicationDescription"), ResStr(IDS_APP_DESCRIPTION), REG_EXPAND_SZ);
				//key.SetStringValue(_T("ApplicationIcon"), AppIcon, REG_EXPAND_SZ);
				key.SetStringValue(_T("ApplicationName"), g_strRegisteredAppName, REG_EXPAND_SZ);
			}
		
		// The Vista way
		CString         strNewApp;
		if (fRegister)
		{
			// Create non existing file type
			if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strExt)) return(false);
           
            
			WCHAR*          pszCurrentAssociation;
			// Save current application associated
			if (SUCCEEDED (m_pAAR->QueryCurrentDefault (strExt, AT_FILEEXTENSION, AL_EFFECTIVE, &pszCurrentAssociation)))
			{
				if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID))
					return(false);

				key.SetStringValue( NULL , pszCurrentAssociation);
                //

				//key.SetStringValue(g_strOldAssoc, pszCurrentAssociation);

				// Get current icon for file type
				
				if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, CString(pszCurrentAssociation) + _T("\\DefaultIcon")))
				{
					
					if(ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && !FileIcon.Trim().IsEmpty())
					{
						
					}
				}
				
				if (!FileIcon.IsEmpty() && ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\DefaultIcon")))
					key.SetStringValue (NULL, FileIcon);

                
                    


				CoTaskMemFree (pszCurrentAssociation);
			}
			strNewApp = g_strRegisteredAppName;
		}
		else
		{
			if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, strProgID))
				return(false);

			if(ERROR_SUCCESS == key.QueryStringValue(g_strOldAssoc, buff, &len))
				strNewApp = buff;

			// TODO : retrieve registered app name from previous association (or find Bill function for that...)
		}

		hr = m_pAAR->SetAppAsDefault(strNewApp, strExt, AT_FILEEXTENSION);
        
		if(hr != S_OK){
			return false;
		}
	}
	else
	{
		// The 2000/XP way
		if (fRegister)
		{
			// Set new association
			if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strExt))
				return(false);

			len = sizeof(buff);
			memset(buff, 0, len);
			if(ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && !CString(buff).Trim().IsEmpty())
				extoldreg = buff;
			if(ERROR_SUCCESS != key.SetStringValue(NULL, strProgID)) return(false);

			// Get current icon for file type
			/*
			if (!extoldreg.IsEmpty())
			{
			if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, extoldreg + _T("\\DefaultIcon")))
			{
			len = sizeof(buff);
			memset(buff, 0, len);
			if(ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && !CString(buff).Trim().IsEmpty())
			extOldIcon = buff;
			}
			}
			*/

			
			// Save old association
			if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID))
				return(false);

			key.SetStringValue(g_strOldAssoc, extoldreg);

			if (!FileIcon.IsEmpty() && ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\DefaultIcon")))
				key.SetStringValue (NULL, FileIcon);
/*
			if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, strProgID + _T("\\DefaultIcon")))
			{
				key.SetStringValue(NULL, FileIcon);
			}
*/

			/*
			if (!extOldIcon.IsEmpty() && (ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\DefaultIcon"))))
			key.SetStringValue (NULL, extOldIcon);
			*/
		}
		else
		{
			// Get previous association
			len = sizeof(buff);
			memset(buff, 0, len);
			if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID))
				return(false);
			if(ERROR_SUCCESS == key.QueryStringValue(g_strOldAssoc, buff, &len) && !CString(buff).Trim().IsEmpty())
				extoldreg = buff;

			// Set previous association
			if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strExt))
				return(false);
			key.SetStringValue(NULL, extoldreg);
		}

	}

	return SUCCEEDED (hr);
}

bool CPPageFormats::RegisterExt(CString ext, bool fRegister, CString PerceivedType)
{
	CRegKey         key;
	bool            bSetValue;
	CString strProgID =  _T("SPlayer") + ext;
  
  CString strLabel = _T("");
	if(ext == _T(".rar")){
		return true;
	}
	if(fRegister){
		//为保证成功注册清理垃圾
		AfxGetMyApp()->DelRegTree(HKEY_CLASSES_ROOT,  ext);
		AfxGetMyApp()->DelRegTree(HKEY_CLASSES_ROOT,  _T("KMPlayer") + ext);
		AfxGetMyApp()->DelRegTree(HKEY_CLASSES_ROOT,  _T("QQPlayer") + ext);
		AfxGetMyApp()->DelRegTree(HKEY_CLASSES_ROOT,  _T("stormplayer") + ext);
		AfxGetMyApp()->DelRegTree(HKEY_CLASSES_ROOT,  _T("RealPlayer") + ext+L".6");
		AfxGetMyApp()->DelRegTree(HKEY_CLASSES_ROOT,  _T("RealPlayer") + ext+L".10");
		AfxGetMyApp()->DelRegTree(HKEY_CLASSES_ROOT,  _T("Xmp") + ext);
	}

	CString path, fn, cmd;
	if(!MakeRegParams(ext, path, fn, strLabel, cmd))
		return(false);

  
	if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, ext))
		return(false);
	TCHAR buff[256];
	ULONG len = sizeof(buff);

	len = sizeof(buff);
	memset(buff, 0, len);

	if(ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && !CString(buff).Trim().IsEmpty())
	{
		strLabel = buff;
	}
	key.SetStringValue(NULL, strProgID);
  
  strLabel = GetFileTypeName(ext);

  if(!PerceivedType.IsEmpty())
    key.SetStringValue (L"PerceivedType", PerceivedType);

	BOOL bIsRAR = ( ext.Right(3).MakeLower() == _T("rar") );
	if(bIsRAR) {

		return true;
		// Create ProgID for this file type
		/*
		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID)) return(false);
		if(ERROR_SUCCESS != key.SetStringValue(NULL, _T("SPlayer.RARFile"))) return(false);

		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\enqueue"))) return(false);
		if(ERROR_SUCCESS != key.SetStringValue(NULL, ResStr(IDS_ADD_TO_PLAYLIST))) return(false);
		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\enqueue\\command"))) return(false);
		if(bSetValue && (ERROR_SUCCESS != key.SetStringValue(NULL, GetEnqueueCommand()))) return(false);

		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\play"))) return(false);
		if(ERROR_SUCCESS != key.SetStringValue(NULL, ResStr(IDS_OPEN_WITH_MPC))) return(false);
		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\play\\command"))) return(false);
		if(bSetValue && (ERROR_SUCCESS != key.SetStringValue(NULL, GetOpenCommand()))) return(false);

		return true;
		*/
	}
	
	if(!fRegister && !bIsRAR)
	{
		if(fRegister != IsRegistered(ext))
			SetFileAssociation (ext, strProgID, fRegister);
		key.Attach(HKEY_CLASSES_ROOT);
		key.RecurseDeleteKey(strProgID);
		return(true);
	}

	bSetValue = fRegister || (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open\\command"), KEY_READ));

	// Create ProgID for this file type
	if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID)) return(false);
	
	if(ERROR_SUCCESS != key.SetStringValue(NULL, strLabel)) return(false);

	// Add to playlist option
	if(f_setContextFiles || bIsRAR)
	{
		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\enqueue"))) return(false);
		if(ERROR_SUCCESS != key.SetStringValue(NULL, ResStr(IDS_ADD_TO_PLAYLIST))) return(false);

		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\enqueue\\command"))) return(false);
		if(bSetValue && (ERROR_SUCCESS != key.SetStringValue(NULL, GetEnqueueCommand()))) return(false);
	}
	else
	{
		key.Attach(HKEY_CLASSES_ROOT);
		key.RecurseDeleteKey(strProgID + _T("\\shell\\enqueue"));
	}

	// Play option
	if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open"))) return(false);
	if(f_setContextFiles || bIsRAR)
	{
		if(ERROR_SUCCESS != key.SetStringValue(NULL, ResStr(IDS_OPEN_WITH_MPC))) return(false);
	}
	else
	{
		if(ERROR_SUCCESS != key.SetStringValue(NULL, _T(""))) return(false);
	}

	if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open\\command"))) return(false);
	if(bSetValue && (ERROR_SUCCESS != key.SetStringValue(NULL, GetOpenCommand()))) return(false);


	// Play option
	if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\openewnd"))) return(false);
	if(f_setContextFiles || bIsRAR)
	{
		if(ERROR_SUCCESS != key.SetStringValue(NULL, ResStr(IDS_OPEN_WITH_MPC_IN_NEW_WND))) return(false);
	}
	else
	{
		if(ERROR_SUCCESS != key.SetStringValue(NULL, _T(""))) return(false);
	}

	if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\openewnd\\command"))) return(false);
	if(bSetValue && (ERROR_SUCCESS != key.SetStringValue(NULL, GetOpenCommand(true)))) return(false);



	if(ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE, g_strRegisteredKey + _T("\\FileAssociations"))) return(false);
	if(ERROR_SUCCESS != key.SetStringValue(ext, strProgID)) return(false);

	if(f_setAssociatedWithIcon && !bIsRAR)
	{
 		CString AppIcon = GetFileIcon(ext);
 		TCHAR buff[MAX_PATH];
// 
// 		CString mpciconlib = GetProgramDir() + _T("\\mpciconlib.dll");
// 
// 		if(FileExists(mpciconlib))
// 		{
// 			int icon_index = GetIconIndex(ext);
// 			CString m_typeicon = mpciconlib;
// 
// 			/* icon_index value -1 means no icon was found in the iconlib for the file extension */
// 			if((icon_index >= 0) && ExtractIcon(AfxGetApp()->m_hInstance,(LPCWSTR)m_typeicon, icon_index))
// 			{
// 				m_typeicon = "\""+mpciconlib+"\"";
// 				AppIcon.Format(_T("%s,%d"), m_typeicon, icon_index);
// 			}
// 		}

		/* no icon was found for the file extension, so use MPC's icon */
		if((AppIcon.IsEmpty()) && (::GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH)))
		{
			AppIcon = buff;
			AppIcon = "\""+AppIcon+"\"";
			AppIcon += _T(",0");
		}

		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\DefaultIcon"))) return(false);
		if(bSetValue && (ERROR_SUCCESS != key.SetStringValue(NULL, AppIcon))) return(false);
	}
	else
	{
		key.Attach(HKEY_CLASSES_ROOT);
		key.RecurseDeleteKey(strProgID + _T("\\DefaultIcon"));
	}

	if(fRegister != IsRegistered(ext) && !bIsRAR)
		SetFileAssociation (ext, strProgID, fRegister);

	return(true);

}
CString CPPageFormats::GetEnqueueCommand()
{
	CString          path;

	TCHAR buff[MAX_PATH];
	if(::GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH) == 0)
		return _T("");

	path = buff;
	return _T("\"") + path + _T("\" /add \"%1\"");
}

CString CPPageFormats::GetOpenCommand(bool open_in_new_windows)
{
	CString          path;
	TCHAR buff[MAX_PATH];

	if(::GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH) == 0)
		return _T("");

	path = buff;
	if(open_in_new_windows)
		return _T("\"") + path + _T("\"  /new  \"%1\"");
	else
		return _T("\"") + path + _T("\" \"%1\"");
}

static struct {LPCSTR verb, cmd; UINT action;} handlers[] =
{
	{"VideoFiles", " %1", IDS_AUTOPLAY_PLAYVIDEO},
	{"MusicFiles", " %1", IDS_AUTOPLAY_PLAYMUSIC},
	{"CDAudio", " %1 /cd ", IDS_AUTOPLAY_PLAYAUDIOCD},
	{"DVDMovie", " %1 /dvd ", IDS_AUTOPLAY_PLAYDVDMOVIE},
	{"SuperVideoCDMovie", " %1 /cd ", IDS_AUTOPLAY_PLAYSVCDMOVIE},
	{"VideoCDMovie", " %1 /cd ", IDS_AUTOPLAY_PLAYVCDMOVIE},
	{"BluRay", " %1 /dvd ", IDS_AUTOPLAY_PLAYBDMOVIE},
	{"DVDAudio", " %1 /dvd ", IDS_AUTOPLAY_PLAYDVDAUDIO},
	{"Camera", " %1 /cap ", IDS_AUTOPLAY_CAPTURECAMERA},

};

void CPPageFormats::AddAutoPlayToRegistry(autoplay_t ap, bool fRegister)
{
	if(!AfxGetAppSettings().fXpOrBetter) return;

	TCHAR buff[MAX_PATH];
	if(::GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH) == 0) return;
	CString exe = buff;

	int i = (int)ap;
	if(i < 0 || i >= countof(handlers)) return;

	CRegKey key;

	if(fRegister)
	{
		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, _T("SPlayer.Autorun"))) return;//MediaPlayerClassic
		key.Close();

		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, 
			CString(CStringA("SPlayer.Autorun\\Shell\\Play") + handlers[i].verb + "\\Command"))) return;
		key.SetStringValue(NULL, _T("\"") + exe + _T("\"") + handlers[i].cmd);
		key.Close();

		if(ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE,
			CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\Handlers\\Play") + handlers[i].verb + "OnArrival"))) return;
		key.SetStringValue(_T("Action"), ResStr(handlers[i].action));
		key.SetStringValue(_T("Provider"), g_strRegisteredAppName/*_T("Media Player Classic")*/);
		key.SetStringValue(_T("InvokeProgID"), _T("SPlayer.Autorun"));
		key.SetStringValue(_T("InvokeVerb"), CString(CStringA("Play") + handlers[i].verb ));//
		key.SetStringValue(_T("DefaultIcon"), exe + _T(",0"));
		key.Close();

		if(ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE, 
			CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlers\\Play") + handlers[i].verb + "OnArrival"))) return;
		key.SetStringValue(CString(CStringA("Play") + handlers[i].verb + "OnArrival"), _T(""));
		key.Close();

		if(ERROR_SUCCESS != key.Create(HKEY_CURRENT_USER, 
			CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlersDefaultSelection\\Play") + handlers[i].verb + "OnArrival"))) return;
		key.SetStringValue(NULL, CString(CStringA("Play") + handlers[i].verb + "OnArrival") );
		key.Close();
/*
		if(ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE, 
			CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlersDefaultSelection\\Play") + handlers[i].verb + "OnArrival"))) return;
		key.SetStringValue(NULL, CString(CStringA("Play") + handlers[i].verb + "OnArrival") );
		key.Close();*/
		
	}
	else
	{
		if(ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE, 
			CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlers\\Play") + handlers[i].verb + "OnArrival"))) return;
		key.DeleteValue(CString(CStringA("Play") + handlers[i].verb + "OnArrival"));
		key.Close();
	}
}

bool CPPageFormats::IsAutoPlayRegistered(autoplay_t ap)
{
	ULONG len;
	TCHAR buff[MAX_PATH];
	if(::GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH) == 0) return(false);
	CString exe = buff;

	int i = (int)ap;
	if(i < 0 || i >= countof(handlers)) return(false);

	CRegKey key;

	if(ERROR_SUCCESS != key.Open(HKEY_LOCAL_MACHINE, 
		CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlers\\Play") + handlers[i].verb + "OnArrival"),
		KEY_READ)) return(false);
	len = countof(buff);
	if(ERROR_SUCCESS != key.QueryStringValue(
		CString(_T("Play")) + handlers[i].verb + _T("OnArrival"), 
		buff, &len)) return(false);
	key.Close();

	if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, 
		CString(CStringA("SPlayer.Autorun\\Shell\\Play") + handlers[i].verb + "\\Command"),
		KEY_READ)) return(false);
	len = countof(buff);
	if(ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len))
		return(false);
	if(_tcsnicmp(_T("\"") + exe, buff, exe.GetLength() + 1))
		return(false);
	key.Close();

	return(true);
}

