#include "stdafx.h"
#include "db_lrcdb.h"
#include "main.h"
//#include "vars.h"


void db_lrcdb::GetResults(window_config *wcfg, std::vector<lyric_data>* results, tstring title, tstring artist, tstring album)
{

  title = ReplaceString(_T("&"), _T("and"), title);
  artist = ReplaceString(_T("&"), _T("and"), artist);
  title = ReplaceString(_T("pt."), _T("part"), title);
  artist = ReplaceString(_T("pt."), _T("part"), artist);


  bool found = false;
  bool useAlbum = false;
  tstring out;

  out = _T("search.php?artist=");
  out += CURLEncode::Encode(artist);
  out += _T("&title=");
  out += CURLEncode::Encode(title);

  if (useAlbum)
  {
    out += _T("&album=");
    out += CURLEncode::Encode(album);
  }

  out += _T("&query=plugin&type=plugin");

  tstring remoteText;

  if (ReadInternetText(remoteText, _T("FOO_UIE_LYRICS2"), _T("www.lrcdb.org"), 80, out, _T(""), false, wcfg->cfg_timeout, wcfg))
  {
    if (remoteText != _T("no match"))
    {
      tstring::size_type found1 = remoteText.find(_T(": "));
      tstring::size_type found2 = remoteText.find(_T("\t"));

      if (found1 != wstring::npos && found2 != wstring::npos)
      {
        found1 += 2;

        if (found1 < remoteText.length())
        {
          remoteText = remoteText.substr(found1, found2 - found1);

          if (remoteText != _T(""))
            found = true;
        }
      }
    }
  }

  if (found)
  {
    lyric_data newData(wcfg);
    newData.SourceType = ST_INTERNET;
    newData.IsLoaded = false;
    newData.IsTimestamped = false;
    newData.matchAlbum = album;
    newData.matchArtist = artist;
    newData.matchTitle = title;
    newData.db = this;
    newData.OnlineService = LD_LRCDB;
    newData.Quality = Q_GOOD;
    newData.Source = _T("http://www.lrcdb.org/lyric.php?lid=");
    newData.Source += remoteText;
    newData.Source += _T("&astext=yes");

    results->push_back(newData);
  }

}

void db_lrcdb::DownloadLyrics(window_config *wcfg, lyric_data* data)
{
  data->Text = _T("");

  if (!ReadInternetTextFromUrl(data->Text, _T("FOO_UIE_LYRICS2"), data->Source, wcfg->cfg_timeout, wcfg))
    return ;

  data->Text = ReplaceString(_T("\r"), _T(""), data->Text);

  data->Text = ReplaceString(_T("\n"), _T("\r\n"), data->Text);

  data->IsLoaded = true;

  if (data->Text.find(_T("[")) != tstring::npos)
    data->IsTimestamped = true;
  else
    data->IsTimestamped = false;

}

tchar* db_lrcdb::GetName()
{
  return _T("LrcDB");
}

LYRICS_DATABASES db_lrcdb::GetID()
{
  return LD_LRCDB;
}
