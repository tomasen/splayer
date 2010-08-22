#include "stdafx.h"
#include "db_viewlyrics.h"
#include "main.h"
//#include "vars.h"

void db_viewlyrics::GetResults(window_config *wcfg, std::vector<lyric_data>* results, tstring title, tstring artist, tstring album)
{

  title = ReplaceString(_T("&"), _T("and"), title);
  artist = ReplaceString(_T("&"), _T("and"), artist);
  title = ReplaceString(_T("pt."), _T("part"), title);
  artist = ReplaceString(_T("pt."), _T("part"), artist);

  tstring out;

  out = _T("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
  out += _T("<search filetype=\"lyrics\" artist=\"");
  out += ReplaceString(_T("\""), _T(""), artist);
  out += _T("\" title=\"");
  out += ReplaceString(_T("\""), _T(""), title);
  out += _T("\" />");

  tstring remoteText = _T("");

  if (!ReadInternetText(remoteText, _T("MiniLyrics"), _T("www.viewlyrics.com"), 1212, _T("searchlyrics.htm"), out, true, wcfg->cfg_timeout, wcfg))
    return ;

  CMarkup xml;

  if (!xml.SetDoc(remoteText.c_str()))
    return ;

  if (!_wcsicmp(xml.GetAttrib(_T("result")).c_str(), _T("NOT_FOUND")))
    return ;


  while (xml.FindChildElem(_T("fileinfo")))
  {
    xml.IntoElem();

    if (artist.empty() || !_wcsicmp(artist.c_str(), xml.GetAttrib(_T("artist")).c_str()))
    {
      if (!xml.GetAttrib(_T("filename")).empty())
      {
        bool found_lrc = false;
        tstring info_link = xml.GetAttrib(_T("link"));
        tstring info_ext = util::rget_after(info_link, '.');

        if (_wcsicmp(info_ext.c_str(), _T("lrc")) == 0)
          found_lrc = true;

        double info_rate = _wtof(xml.GetAttrib(_T("rate")).c_str());

        lyric_data newData(wcfg);

        newData.SourceType = ST_INTERNET;

        newData.IsLoaded = false;

        newData.IsTimestamped = found_lrc;

        newData.matchArtist = xml.GetAttrib(_T("artist")).c_str();

        newData.matchAlbum = xml.GetAttrib(_T("album")).c_str();

        newData.matchTitle = title;

        newData.db = this;

        newData.OnlineService = LD_VIEWLYRICS;

        if (info_rate > 4.0f)
          newData.Quality = Q_GOOD;

        if (info_rate < 3.0f)
          newData.Quality = Q_BAD;
        else
          newData.Quality = Q_MEDIUM;

        newData.Source = info_link.c_str();

        results->push_back(newData);
      }
    }

    xml.OutOfElem();
  }

  return ;
}


void db_viewlyrics::DownloadLyrics(window_config *wcfg, lyric_data* data)
{
  data->Text = _T("");
  ReadInternetTextFromUrl(data->Text, _T("MiniLyrics"), data->Source, wcfg->cfg_timeout, wcfg);
  data->Text = ReplaceString(_T("\r"), _T(""), data->Text);
  data->Text = ReplaceString(_T("\n"), _T("\r\n"), data->Text);
  data->IsLoaded = true;
}

tchar* db_viewlyrics::GetName()
{
  return _T("Viewlyrics");
}

LYRICS_DATABASES db_viewlyrics::GetID()
{
  return LD_VIEWLYRICS;
}
