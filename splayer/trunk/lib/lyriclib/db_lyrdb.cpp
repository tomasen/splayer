#include "stdafx.h"
#include "db_lyrdb.h"
#include "main.h"
//#include "vars.h"


void db_lyrdb::GetResults(window_config *wcfg, std::vector<lyric_data>* results, tstring title, tstring artist, tstring album)
{

  title = ReplaceString(_T("&"), _T("and"), title);
  artist = ReplaceString(_T("&"), _T("and"), artist);
  title = ReplaceString(_T("pt."), _T("part"), title);
  artist = ReplaceString(_T("pt."), _T("part"), artist);

  bool useFulltext = false;
  bool found = false;
  tstring out;

  if (!useFulltext)
  {
    out = _T("lookup.php?q=");
    out += CURLEncode::Encode(artist);
    out += _T("|");
    out += CURLEncode::Encode(title);
    out += _T("&for=match&agent=foo_uie_lyrics2");
  }
  else
  {
    out = _T("lookup.php?q=");
    out += CURLEncode::Encode(artist);
    out += _T(" - ");
    out += CURLEncode::Encode(title);
    out += _T("&for=fullt&agent=foo_uie_lyrics2");
  }

  out += _T("&query=plugin&type=plugin");

  tstring remoteText;

  if (!ReadInternetText(remoteText, _T("FOO_UIE_LYRICS2"), _T("webservices.lyrdb.com"), 80, out, _T(""), false, wcfg->cfg_timeout, wcfg) && remoteText != _T(""))
    return ;

  tstring::size_type ptr = 0;

  tstring::size_type end = 0;

  while (true)
  {
    if (ptr >= remoteText.size() || ptr == tstring::npos)
      break;

    end = remoteText.find(_T("\r\n"), ptr);

    if (end == tstring::npos && ptr == 0)
      end = remoteText.size() - 1;

    if (end >= remoteText.size() || end == tstring::npos)
      break;

    tstring line = remoteText.substr(ptr, end + 1 - ptr);

    // parse
    tstring::size_type pos = line.find(_T("\\"));

    if (pos != tstring::npos)
    {
      tstring line_id = line.substr(0, pos);
      tstring line_title = _T("");
      tstring line_artist = _T("");

      if (pos + 1 < line.size())
      {
        tstring::size_type pos2 = line.find(_T("\\"), pos + 1);

        if (pos2 != tstring::npos)
        {
          line_title = line.substr(pos + 1, pos2 - pos + 1);

          if (pos2 + 2 < line.size())
          {
            tstring::size_type pos3 = line.find(_T("\\"), pos2 + 1);

            if (pos3 != tstring::npos)
              line_artist = line.substr(pos2 + 1, pos3 - pos2 + 1);
          }
        }
      }

      lyric_data newData(wcfg);
      newData.SourceType = ST_INTERNET;
      newData.IsLoaded = false;
      newData.IsTimestamped = false;
      newData.matchAlbum = _T("");
      newData.matchArtist = line_artist;
      newData.matchTitle = line_title;
      newData.db = this;
      newData.OnlineService = LD_LYRDB;
      newData.Quality = Q_MEDIUM;
      newData.Source = _T("http://www.lyrdb.com/getlyr.php?q=");
      newData.Source += line_id;

      results->push_back(newData);

    }

    ptr = end + 2;
  }

  return ;
}


void db_lyrdb::DownloadLyrics(window_config *wcfg, lyric_data* data)
{
  data->Text = _T("");

  if (!ReadInternetTextFromUrl(data->Text, _T("FOO_UIE_LYRICS2"), data->Source, wcfg->cfg_timeout, wcfg))
    return ;

  data->Text = ReplaceString(_T("\r"), _T(""), data->Text);

  data->Text = ReplaceString(_T("\n"), _T("\r\n"), data->Text);

  data->Text = ReplaceString(_T("<!--f0c25b539901624b460e129d15264305-->"), _T(""), data->Text);

  data->IsLoaded = true;

  if (data->Text.find(_T("[")) != tstring::npos)
    data->IsTimestamped = true;
  else
    data->IsTimestamped = false;
}

tchar* db_lyrdb::GetName()
{
  return _T("LyrDB");
}

LYRICS_DATABASES db_lyrdb::GetID()
{
  return LD_LYRDB;
}
