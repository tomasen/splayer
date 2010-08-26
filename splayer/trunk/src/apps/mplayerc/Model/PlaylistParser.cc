#include "stdafx.h"
#include "./PlaylistParser.h"
#include "../Utils/ContentType.h"
//#include "../Playlist.h"
//#include "../MediaFormats.h"
// #include "../../../svplib/SVPRarLib.h"
#include "../../../subtitles/TextFile.h"
#include "../../../svplib/SVPToolBox.h"
#include <algorithm>
#include "../mplayerc.h"
// #include "../../../dsutil/HdmvClipInfo.h"

static CString CombinePath(CPath p, CString fn)
{
  if(fn.Find(':') >= 0 || fn.Find(_T("\\")) == 0) return fn;
  p.Append(CPath(fn));
  return (LPCTSTR)p;
}


void PlaylistParser::MergeList(std::vector<CPlaylistItem>& list,
                     std::vector<CPlaylistItem> listToAdd)
{
  for (std::vector<CPlaylistItem>::iterator iter = listToAdd.begin();
    iter != listToAdd.end(); iter++)
    list.push_back(*iter);
}

// std::wstring PlaylistParser::CombinePath(CPath p, std::wstring fn)
// {
//   if (fn.find(':') >= 0 && fn.find(':') < fn.size() || fn.find(_T("\\")) == 0)
//     return fn;
//   p.Append(CPath(fn.c_str()));
//   return (LPCTSTR)p;
// }

static int s_int_comp(const void* i1, const void* i2)
{
  return (int)i1 - (int)i2;
}

CPlaylistItem* PlaylistParser::GetCPlaylistItem(std::wstring fn,
                                               std::vector<std::wstring>* subs)
{
  std::vector<std::wstring> fns;
  fns.push_back(fn);
  return GetCPlaylistItem(fns, subs);
}

CPlaylistItem* PlaylistParser::GetCPlaylistItem(std::vector<std::wstring>& fns,
                                               std::vector<std::wstring>* subs)
{
  CPlaylistItem pli;
//   AppSettings&   s  = AfxGetAppSettings();
//   CMediaFormats& mf = s.Formats;
//   for (std::vector<std::wstring>::iterator iter = fns.begin();
//     iter != fns.end(); iter++)
//   {
//     if (mf.IsUnPlayableFile((*iter).c_str()))
//     {
//       fns.erase(iter);
//       continue;
//     }
//     (*iter).erase((*iter).find_last_not_of(L' ') + 1);
//     (*iter).erase(0, (*iter).find_first_not_of(L' '));
//     if (!(*iter).empty())
//       pli.m_fns.AddTail((*iter).c_str());
//   }
// 
//   if(subs)
//   {
//     for (std::vector<std::wstring>::iterator iter = fns.begin();
//       iter != fns.end(); iter++)
//     {
//       (*iter).erase((*iter).find_last_not_of(L' ') + 1);
//       (*iter).erase(0, (*iter).find_first_not_of(L' '));
//       if (!(*iter).empty())
//         pli.m_subs.AddTail((*iter).c_str());
//     }
//   }
// 
//   if(pli.m_fns.IsEmpty())
//     return NULL;
// 
//   std::wstring fn = (LPCTSTR)pli.m_fns.GetHead();
// 
//   if(s.fAutoloadAudio && fn.find(L"://") < 0)
//   {
//     int i = fn.find_last_of('.');
//     if (i > 0)
//     {
//       CMediaFormats& mf = AfxGetAppSettings().Formats;
//       std::wstring ext = fn.substr(i + 1, fn.size() - i - 1);
//       std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
//       if(!mf.FindExt(ext.c_str(), true))
//       {
//         std::wstring path = fn;
//         for (int i = 0; i < path.length(); i++)
//           if (path[i] == '/')
//             path[i] = '\\';
//         path = path.substr(0, path.find_last_of('\\') + 1);
//         WIN32_FIND_DATA fd = {0};
//         HANDLE hFind = FindFirstFile((fn.substr(0, i) + L"*.*").c_str(), &fd);
//         if(hFind != INVALID_HANDLE_VALUE)
//         {
//           do
//           {
//             if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//               continue;
//             std::wstring fullpath = path + (LPCTSTR)fd.cFileName;
//             int dotPos = fullpath.find_last_of('.');
//             std::wstring ext2 = fullpath.substr(dotPos + 1,
//               fullpath.size() - dotPos - 1);
//             std::transform(ext2.begin(), ext2.end(), ext2.begin(), tolower);
//             if(!FindFileInList(pli.m_fns, fullpath)
//               && ext != ext2 && mf.FindExt(ext2.c_str(), true)
//               && mf.IsUsingEngine(fullpath.c_str(), DirectShow))
//               pli.m_fns.AddTail(fullpath.c_str());
//           }
//           while(FindNextFile(hFind, &fd));
//           FindClose(hFind);
//         }
//       }
//     }
//   }
//   return &pli;
  return NULL;
}

std::vector<CPlaylistItem> PlaylistParser::GetPlaylistFromRar(std::wstring fn,
                                              std::vector<std::wstring>* subs)
{
  std::vector<CPlaylistItem> playlistsInRar;
  return playlistsInRar;
//   AppSettings& s = AfxGetAppSettings();
//   CMediaFormats& mf = s.Formats;
//   CSVPRarLib svpRar;
//   std::vector<std::wstring> szFnsInRar;
//   CStringArray szFnsInRar_CSA;
//   svpRar.ListRar(fn.c_str(), &szFnsInRar_CSA);
//   for (int i = 0; i < szFnsInRar_CSA.GetCount(); i++)
//     szFnsInRar.push_back((LPCTSTR)szFnsInRar_CSA.GetAt(i));
//   //AfxMessageBox(fn);
//   for (std::vector<std::wstring>::iterator iter = szFnsInRar.begin();
//     iter != szFnsInRar.end(); iter++)
//   {
//     std::wstring szThisFn = *iter;
//     std::wstring ext = (LPCTSTR)CPath(szThisFn.c_str()).GetExtension();
//     std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
//     if (!mf.IsUnPlayableFile(szThisFn.c_str(), true) && ext != L".rar")
//     {
//       CPlaylistItem* newPli;
//       if ((newPli = GetCPlaylistItem(L"rar://" + fn + L"?" + szThisFn, subs)) != NULL)
//         playlistsInRar.push_back(*newPli);
//     }
//   }
//   return playlistsInRar;
}

std::vector<CPlaylistItem> PlaylistParser::Parse(std::wstring fn, std::vector<std::wstring>* subs)
{
  std::vector<std::wstring> sl;
  sl.push_back(fn);
  return Parse(sl, subs);

}

std::vector<CPlaylistItem> PlaylistParser::Parse(std::vector<std::wstring>& fns,
                                 std::vector<std::wstring>* subs)
{
  std::vector<CPlaylistItem> playlists;
  return playlists;
  if (fns.empty())
    return playlists;

  // resolve rar file
  bool fFindRar = false;
  for (std::vector<std::wstring>::iterator iter = fns.begin();
    iter != fns.end(); iter++)
  {
    if (fFindRar)
    {
      fns.erase(iter - 1);
      fFindRar = false;
    }
    if ((*iter).substr(0, 6) == L"rar://")
      continue;
    std::wstring ext = ::PathFindExtension((*iter).c_str());
    std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
    if (ext == L".rar")
    {
      MergeList(playlists, GetPlaylistFromRar(*iter, subs));
      fFindRar = true;
    }
  }
  if (fFindRar)
    fns.erase(fns.end());

  if(fns.size() <= 0)
    return playlists;

  // resolve .lnk files
  CComPtr<IShellLink> pSL;
  pSL.CoCreateInstance(CLSID_ShellLink);
  CComQIPtr<IPersistFile> pPF = pSL;
  for (std::vector<std::wstring>::iterator iter = fns.begin();
    iter != fns.end() && pSL && pPF; iter++)
  {
    std::wstring ext = ::PathFindExtension((*iter).c_str());
    std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
    TCHAR buff[MAX_PATH];
    if(ext != _T(".lnk")
      || FAILED(pPF->Load((*iter).c_str(), STGM_READ))
      || FAILED(pSL->Resolve(NULL, SLR_ANY_MATCH|SLR_NO_UI))
      || FAILED(pSL->GetPath(buff, MAX_PATH, NULL, 0)))
      continue;

    *iter = (LPCTSTR)buff;
  }

  //	
  std::vector<std::wstring> sl;
  if(SearchFiles(fns.at(0), sl))
  {
    if(sl.size() > 1)
      subs = NULL;
    for (std::vector<std::wstring>::iterator iter = sl.begin();
      iter != sl.end(); iter++)
      MergeList(playlists, Parse(*iter, subs));
    return playlists;
  }

  std::vector<std::wstring> redir;
  std::wstring ct = ContentType::Get(fns.at(0).c_str(), &redir);
  if(!redir.empty())
  {
//     POSITION pos = redir.GetHeadPosition();
//     while(pos)
//       MergeList(playlists, Parse((LPCTSTR)redir.GetNext(pos), subs));
    return playlists;
  }

  if(ct == L"application/x-cue-playlist")
  {
    MergeList(playlists, ParseCUEPlayList(fns.at(0)));
    return playlists;
  }
  if(ct == L"application/x-mpc-playlist")
  {
    MergeList(playlists, ParseMPCPlayList(fns.at(0)));
    return playlists;
  }
  else if(ct == L"application/x-bdmv-playlist")
  {
    MergeList(playlists, ParseBDMVPlayList(fns.at(0)));
    return playlists;
  }

  CPlaylistItem* newPli;
  if ((newPli = GetCPlaylistItem(fns, subs)) != NULL)
    playlists.push_back(*newPli);
  return playlists;
}

std::vector<CPlaylistItem> PlaylistParser::ParseCUEPlayList(std::wstring fn)
{
  std::vector<CPlaylistItem> playlist;
  return playlist;
//   CString str;//Use CString to match the param type of CWebTextFile::ReadString
//   std::map<int, CPlaylistItem> pli;//didn.t use???
//   std::vector<int> idx;//didn't use???
// 
//   CWebTextFile f;
//   if (!f.Open(fn.c_str()) || !f.ReadString(str))
//     return playlist;
//   if (f.GetEncoding() == CTextFile::ASCII)
//     f.SetEncoding(CTextFile::ANSI);
// 
//   CPath base(fn.c_str());
//   base.RemoveFileSpec();
// 
//   int idxCurrent = -1;
// 
//   CSVPToolBox svpToolBox;
// 
//   while(f.ReadString(str))
//   {
//     str.Trim();
//     int pos = str.Find(_T("FILE"));
//     if (pos >= 0)
//     {
//       int pos2 = str.ReverseFind(' ');
//       int pos3 = str.ReverseFind('\t');
//       pos2 = max(pos2, pos3);
//       std::wstring fn = str.Mid(pos+5, pos2-pos-5);
//       fn.erase(fn.find_last_not_of(L' ') + 1);
//       fn.erase(0, fn.find_first_not_of(L' '));
//       fn.erase(fn.find_last_not_of(L'"') + 1);
//       fn.erase(0, fn.find_first_not_of(L'"'));
//       if (!svpToolBox.ifFileExist(fn.c_str()))
//       {
//         std::wstring fnPath = PathFindFileName(fn.c_str());
//         CPath myBase(base);
//         myBase.RemoveBackslash();
//         myBase.Append(fnPath.c_str());
//         fn = (LPCTSTR)myBase;
//         if (!svpToolBox.ifFileExist(fn.c_str()))
//           continue;
//       }
//       CPlaylistItem pli;
//       pli.m_fns.AddTail(fn.c_str());
//       playlist.push_back(pli);
//     }
//   }
//   //for(size_t i = 0; i < idx.size(); i++)
//   //{
//   //  m_pl.AddTail(pli[idx[i]]);
//   //  if(idxCurrent > 0 && idxCurrent == idx[i]){
//   //    m_pl.SetPos( m_pl.GetTailPosition());
//   //  }
//   //}
//   return playlist;
}

std::vector<CPlaylistItem> PlaylistParser::ParseMPCPlayList(std::wstring fn)
{
  std::vector<CPlaylistItem> playlist;
  return playlist;

  CString str;
  std::map<int, CPlaylistItem> pli;
  std::vector<int> idx;

  CWebTextFile f;
  if(!f.Open(fn.c_str()) || !f.ReadString(str) || str != _T("MPCPLAYLIST"))
    return playlist;

  if(f.GetEncoding() == CTextFile::ASCII) 
    f.SetEncoding(CTextFile::ANSI);

  CPath base(fn.c_str());
  base.RemoveFileSpec();

  int idxCurrent = -1;

  while(f.ReadString(str))
  {
    //Use CAtlList<CString> to match the param type of Explode
    CAtlList<CString> sl;
    Explode(str, sl, ',', 3);
    if(sl.GetCount() != 3) continue;

    if(int i = _ttoi(sl.RemoveHead()))
    {
      std::wstring key   = (LPCTSTR)sl.RemoveHead();
      std::wstring value = (LPCTSTR)sl.RemoveHead();

      if(key == L"type")
      {
        pli[i].m_type = (CPlaylistItem::type_t)_ttol(value.c_str());
        idx.push_back(i);
      }
      else if (key == L"label")
        pli[i].m_label = value.c_str();
      else if (key == L"iscurrent")
        idxCurrent = i;
      else if (key == L"filename")
      {
        value = CombinePath(base, value.c_str());
        pli[i].m_fns.AddTail(value.c_str());
      }
      else if (key == L"subtitle")
      {
        value = CombinePath(base, value.c_str());
        pli[i].m_subs.AddTail(value.c_str());
      }
      else if (key == L"video")
      {
        while(pli[i].m_fns.GetCount() < 2)
          pli[i].m_fns.AddTail(_T(""));
        pli[i].m_fns.GetHead() = value.c_str();
      }
      else if (key == L"audio")
      {
        while(pli[i].m_fns.GetCount() < 2)
          pli[i].m_fns.AddTail(_T(""));
        pli[i].m_fns.GetTail() = value.c_str();
      }
      else if (key == L"vinput")
        pli[i].m_vinput = _ttol(value.c_str());
      else if (key == L"vchannel")
        pli[i].m_vchannel = _ttol(value.c_str());
      else if (key == L"ainput")
        pli[i].m_ainput = _ttol(value.c_str());
      else if (key == L"country")
        pli[i].m_country = _ttol(value.c_str());
    }
  }

  qsort(&idx[0], idx.size(), sizeof(int), s_int_comp);
  for(size_t i = 0; i < idx.size(); i++)
  {
    playlist.push_back(pli[idx[i]]);
    //if(idxCurrent > 0 && idxCurrent == idx[i])
    //  m_pl.SetPos(m_pl.GetTailPosition());
    //???????????
  }
  return playlist;
}

std::vector<CPlaylistItem> PlaylistParser::ParseBDMVPlayList(std::wstring fn)
{
  //didn't change types to match the param type of CHdmvClipInfo::FindMainMovie
  std::vector<CPlaylistItem> playlist;
//   CHdmvClipInfo ClipInfo;
//   CString       strPlaylistFile;
//   CAtlList<CHdmvClipInfo::CPlaylistItem> MainPlaylist;
// 
//   CPath Path(fn.c_str());
//   Path.RemoveFileSpec();
//   Path.RemoveFileSpec();
// 
//   if (SUCCEEDED(ClipInfo.FindMainMovie(Path + L"\\", strPlaylistFile, MainPlaylist)))
//   {
//     std::vector<std::wstring> strFiles;
//     strFiles.push_back((LPCTSTR)strPlaylistFile);
//     //Append(strFiles, MainPlaylist.GetCount()>1, NULL);
//     if (MainPlaylist.GetCount() > 1)
//     {
//       for (std::vector<std::wstring>::iterator iter = strFiles.begin();
//         iter != strFiles.end(); iter++)
//         MergeList(playlist, Parse(*iter, NULL));
//       return playlist;
//     }
//     else
//       return Parse(strFiles, NULL);
//   }
  return playlist;
}

bool PlaylistParser::FindFileInList(std::vector<std::wstring>& sl, std::wstring fn)
{
  bool fFound = false;
//   POSITION pos = sl.GetHeadPosition();
//   while(pos && !fFound)
//     if(!sl.GetNext(pos).CompareNoCase(fn.c_str()))
//       fFound = true;
  return(fFound);
}

bool PlaylistParser::SearchFiles(std::wstring mask, std::vector<std::wstring>& sl)
{
//   if(mask.find(_T("://")) >= 0)
//     return false;
//   mask.erase(mask.find_last_not_of(L' ') + 1);
//   mask.erase(0, mask.find_first_not_of(L' '));
//   sl.clear();
//   CMediaFormats& mf = AfxGetAppSettings().Formats;
// 
//   bool fFilterKnownExts;
//   WIN32_FILE_ATTRIBUTE_DATA fad;
//   mask =
//     (fFilterKnownExts = (GetFileAttributesEx(mask.c_str(), GetFileExInfoStandard, &fad)
//     && (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
//     ? mask.erase(mask.find_first_not_of(L"\\/") + 1) +L"\\*.*"
//     : mask;
// 
//   {
//     std::wstring dir = mask.substr(0,
//       max(mask.find_last_of('\\'), mask.find_last_of('/')) + 1);
//     WIN32_FIND_DATA fd;
//     HANDLE h = FindFirstFile(mask.c_str(), &fd);
//     if(h != INVALID_HANDLE_VALUE)
//     {
//       do
//       {
//         if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//           continue;
//         std::wstring fn   = (LPCTSTR)fd.cFileName;
//         std::wstring ext  = fn.substr(fn.find_last_of('.') + 1);
//         std::wstring path = dir + (LPCTSTR)fd.cFileName;
//         std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
// 
//         if(!fFilterKnownExts || mf.FindExt(ext.c_str()))
//           sl.push_back(path);
//       }
//       while(FindNextFile(h, &fd));
//       FindClose(h);
//       CAtlList<CString> sl_ATL;
//       for (std::vector<std::wstring>::iterator iter = sl.begin();
//         iter != sl.end(); iter++)
//         sl_ATL.AddTail((*iter).c_str());
//       if(sl.size() == 0 && mask.find(L":\\") == 1)
//         GetCDROMType(mask[0], sl_ATL);
//       sl.clear();
//       POSITION pos = sl_ATL.GetHeadPosition();
//       while (pos)
//         sl.push_back((LPCTSTR)sl_ATL.GetNext(pos));
//     }
//   }
//   bool fIfHasOne = mask.find(L'?') >= 0 && mask.find(L'?') < mask.size()
//     || mask.find(L'*') >= 0 && mask.find(L'*') < mask.size();
//   return (sl.size() > 1
//     || sl.size() == 1 && sl.at(0).compare(mask)
//     || sl.size() == 0 && fIfHasOne);
  return false;
}

