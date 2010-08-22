#include "stdafx.h"
#include "db_leos.h"


void db_leos::GetResults(window_config *wcfg, std::vector<lyric_data>* results, tstring title, tstring artist, tstring album)
{

  title = ReplaceString(_T("&"), _T("and"), title);
  artist = ReplaceString(_T("&"), _T("and"), artist);
  title = ReplaceString(_T("pt."), _T("part"), title);
  artist = ReplaceString(_T("pt."), _T("part"), artist);

  tstring out;

  out = _T("api_search.php?auth=foo_uie_lyrics2&artist=");
  out += CURLEncode::Encode(artist);
  out += _T("&songtitle=");
  out += CURLEncode::Encode(title);

  tstring remoteText;

  if (!ReadInternetText(remoteText, _T("FOO_UIE_LYRICS2"), _T("api.leoslyrics.com"), 80, out, _T(""), false, wcfg->cfg_timeout, wcfg))
    return ;

  CMarkup xml;

  if (!xml.SetDoc(remoteText.c_str()))
    return ;

  if (xml.FindChildElem(_T("searchResults")))
  {
    xml.IntoElem();

    while (xml.FindChildElem(_T("result")))
    {
      xml.IntoElem();

      if (!_wcsicmp(_T("true"), xml.GetAttrib(_T("exactMatch")).c_str()))
      {
        tstring hid = xml.GetAttrib(_T("hid"));

        if (!hid.empty())
        {

          lyric_data newData(wcfg);
          newData.SourceType = ST_INTERNET;
          newData.IsLoaded = false;
          newData.IsTimestamped = false;
          newData.matchArtist = _T("");
          newData.matchAlbum = _T("");
          newData.matchTitle = title;
          newData.db = this;
          newData.OnlineService = LD_LEOS;
          newData.Quality = Q_MEDIUM;
          newData.Source = _T("http://api.leoslyrics.com/api_lyrics.php?auth=foo_uie_lyrics2&hid=");
          newData.Source += hid;

          results->push_back(newData);
        }
      }

      xml.OutOfElem();
    }

    xml.OutOfElem();
  }

}

void db_leos::DownloadLyrics(window_config *wcfg, lyric_data* data)
{
  data->Text = _T("");
  tstring xmltext;

  if (!ReadInternetTextFromUrl(xmltext, _T("FOO_UIE_LYRICS2"), data->Source, wcfg->cfg_timeout, wcfg))
    return ;

  tstring info_text;

  CMarkup xml;

  if (!xml.SetDoc(xmltext.c_str()))
    return ;

  if (xml.FindChildElem(_T("lyric")))
  {
    xml.IntoElem();

    if (xml.FindChildElem(_T("text")))
    {
      xml.IntoElem();

      info_text = xml.GetData();

      xml.OutOfElem();
    }

    xml.OutOfElem();
  }

  data->Text = info_text;

  data->Text = ReplaceString(_T("\r"), _T(""), data->Text);
  data->Text = ReplaceString(_T("\n"), _T("\r\n"), data->Text);
  data->IsLoaded = true;

  if (data->Text.find(_T("[")) != tstring::npos)
    data->IsTimestamped = true;
  else
    data->IsTimestamped = false;

}

tchar* db_leos::GetName()
{
  return _T("LeosLyrics");
}

LYRICS_DATABASES db_leos::GetID()
{
  return LD_LEOS;
}
