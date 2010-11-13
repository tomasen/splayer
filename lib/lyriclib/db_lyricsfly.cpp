#include "stdafx.h"
#include "db_lyricsfly.h"
#include "main.h"
//#include "vars.h"


void db_lyricsfly::GetResults(window_config *wcfg, std::vector<lyric_data>* results, tstring title, tstring artist, tstring album)
{
  title = ReplaceString(_T("&"), _T("and"), title);
  artist = ReplaceString(_T("&"), _T("and"), artist);
  title = ReplaceString(_T("pt."), _T("part"), title);
  artist = ReplaceString(_T("pt."), _T("part"), artist);

  tstring out;
  out = _T("api/api.php?i=");
  out += _T("55562032656-reeloo.net/wordpress/lyrics-for-lastfm-radio");
  out += _T("&a=");
  out += CURLEncode::Encode(artist);
  out += _T("&t=");
  out += CURLEncode::Encode(title);

  tstring remoteText;

  if (!ReadInternetText(remoteText, _T("Mozilla/4.0 (compatible) Greasemonkey"), _T("api.lyricsfly.com"), 80, out, _T(""), false, wcfg->cfg_timeout, wcfg))
    return ;

  CMarkup xml;

  if (!xml.SetDoc(remoteText.c_str()))
    return ;

  while (xml.FindChildElem(_T("sg")))
  {
    xml.IntoElem();

    tstring info_artist, info_title, info_album, info_lyrics;

    if (xml.FindChildElem(_T("tx")))
    {
      info_lyrics = xml.GetChildData();
    }
    else
    {
      xml.OutOfElem();
      continue;
    }

    xml.ResetChildPos();

    if (xml.FindChildElem(_T("ar")))
    {
      info_artist = xml.GetChildData();
    }

    xml.ResetChildPos();

    if (xml.FindChildElem(_T("al")))
    {
      info_album = xml.GetChildData();
    }

    xml.ResetChildPos();

    if (xml.FindChildElem(_T("tt")))
    {
      info_title = xml.GetChildData();
    }

    lyric_data newData(wcfg);
    newData.SourceType = ST_INTERNET;
    newData.IsLoaded = true;
    newData.matchArtist = info_artist;
    newData.matchAlbum = info_album;
    newData.matchTitle = info_title;
    newData.db = this;
    newData.OnlineService = LD_LYRICSFLY;
    newData.Quality = Q_MEDIUM;
    newData.Source = _T("lyricsfly");

    newData.Text = info_lyrics;
    newData.Text = ReplaceString(_T("[br]"), _T(""), newData.Text);
    newData.Text = ReplaceString(_T("\r"), _T(""), newData.Text);
    newData.Text = ReplaceString(_T("\n"), _T("\r\n"), newData.Text);

    if (newData.Text.find(_T("[")) != tstring::npos)
      newData.IsTimestamped = true;
    else
      newData.IsTimestamped = false;

    results->push_back(newData);

    xml.OutOfElem();
  }

}

void db_lyricsfly::DownloadLyrics(window_config *wcfg, lyric_data* data)
{
  data->IsLoaded = true;
}

tchar* db_lyricsfly::GetName()
{
  return _T("Lyricsfly");
}

LYRICS_DATABASES db_lyricsfly::GetID()
{
  return LD_LYRICSFLY;
}

