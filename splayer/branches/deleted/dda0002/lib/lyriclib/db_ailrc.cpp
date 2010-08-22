#include "stdafx.h"
#include "db_ailrc.h"
#include "main.h"
//#include "vars.h"


void db_ailrc::GetResults(window_config *wcfg, std::vector<lyric_data>* results, tstring title, tstring artist, tstring album)
{

  title = ReplaceString(_T("&"), _T("and"), title);
  artist = ReplaceString(_T("&"), _T("and"), artist);
  title = ReplaceString(_T("pt."), _T("part"), title);
  artist = ReplaceString(_T("pt."), _T("part"), artist);


  bool useAlbum = false;
  tstring out;

  out = _T("AiLrc_Search.aspx?N=");
  out += CURLEncode::Encode(title);
  out += _T("&S=");
  out += CURLEncode::Encode(artist);
  out += _T("&A=");

  if (useAlbum)
  {
    out += CURLEncode::Encode(album);
  }

  tstring remoteText;

  if (!ReadInternetText(remoteText, _T("Mozilla Firefox"), _T("ailrc.com"), 80, out, _T(""), false, wcfg->cfg_timeout * 4, wcfg))
    return ;

  tstring start = _T("style='CURSOR: hand' href='html");

  tstring stop = _T("'target='_blank'");

  tstring::size_type pos1 = 0;

  tstring::size_type pos2 = 0;

  tstring::size_type ptr = 0;

  while (ptr != tstring::npos && ptr < remoteText.size())
  {
    pos1 = remoteText.find(start, ptr);

    if (pos1 == tstring::npos)
      break;

    if (pos1 >= remoteText.size())
      break;

    pos2 = remoteText.find(stop, pos1);

    if (pos2 == tstring::npos)
      break;

    if (pos2 >= remoteText.size())
      break;

    tstring link_id = remoteText.substr(pos1 + start.size(), pos2 - start.size() - pos1);

    lyric_data newData(wcfg);

    newData.SourceType = ST_INTERNET;

    newData.IsLoaded = false;

    newData.IsTimestamped = false;

    newData.matchAlbum = album;

    newData.matchArtist = artist;

    newData.matchTitle = title;

    newData.db = this;

    newData.OnlineService = LD_AILRC;

    newData.Quality = Q_MEDIUM;

    newData.Source = _T("http://ailrc.com/html");

    newData.Source += link_id;

    results->push_back(newData);


    ptr = pos2 + stop.size() + 1;
  }


}

void db_ailrc::DownloadLyrics(window_config *wcfg, lyric_data* data)
{
  tstring rText;

  if (!ReadInternetTextFromUrl(rText, _T("Mozilla Firefox"), data->Source, wcfg->cfg_timeout * 4, wcfg))
    return ;

  tstring start = _T("<li class=\"unblock\">");

  tstring stop = _T("</li>");

  tstring::size_type pos1 = 0;

  tstring::size_type pos2 = 0;

  pos1 = rText.find(start);

  if (pos1 == tstring::npos)
    return ;

  if (pos1 >= rText.size())
    return ;

  pos2 = rText.find(stop, pos1);

  if (pos2 == tstring::npos)
    return ;

  if (pos2 >= rText.size())
    return ;

  data->Text = rText.substr(pos1 + start.size(), pos2 - start.size() - pos1);

  data->Text = ReplaceString(_T("<p>"), _T(""), data->Text);

  data->Text = ReplaceString(_T("</p>"), _T(""), data->Text);

  data->Text = ReplaceString(_T("\r"), _T(""), data->Text);

  data->Text = ReplaceString(_T("\n"), _T("\r\n"), data->Text);

  data->IsLoaded = true;

  if (data->Text.find(_T("[")) != tstring::npos)
    data->IsTimestamped = true;
  else
    data->IsTimestamped = false;
}

tchar* db_ailrc::GetName()
{
  return _T("ailrc");
}

LYRICS_DATABASES db_ailrc::GetID()
{
  return LD_AILRC;
}
