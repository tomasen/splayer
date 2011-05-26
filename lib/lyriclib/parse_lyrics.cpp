#include "stdafx.h"
#include "parse_lyrics.h"
//#include "string_conv.h"
#include "..\..\src\svplib\svplib.h"

const size_t ATTRIB_NAME_LENGTH = 2;

static const struct ST_ATTRIB { const tchar * name; int type; }

attrib_arr[] =
  {
    { _T("ti"), LT_TITLE },
    { _T("ar"), LT_ARTIST },
    { _T("al"), LT_ALBUM },
    { _T("by"), LT_AUTHOR },
    { _T("lg"), LT_LANGUAGE },
  };


//string [xx:xx.x] to double
uint parse_lyrics::ts2t(const tchar * const ts, double & out)
{
  ASSERT(ts);

  if ('[' != *ts)
    return 0;

  const tchar * p1 = ts + 1;

  const tchar * p2 = p1;

  uint tarr[] = { 0, 0, 0 };

  double ret = 0.0;

  uint pos = 0;

  bool b1 = false;

  // For lyric panel format.
  if (m_is_panel_format)
  {
    uint tarr[] = {0, 0, 0, 0};

    do
    {
      while (util::is_number(*p2))
        ++p2;

      if ((':' == *p2 || '.' == *p2) && pos < 4)
      {
        tarr[ pos ] = util::xtoi(p1, p2 - p1);
        p1 = ++p2;
        ++pos;
      }
      else if (']' == *p2)
      {
        if (pos < 4)
        {
          tarr[ pos ] = util::xtoi(p1, p2 - p1);
          ++p2;
          ++pos;
          break;
        }
        else
        {
          ret = 0.0;
          return 0;
        }
      }
      else
      {
        ret = 0.0;
        return 0;
      }
    }
    while (true);

    uint d = 1;

    ret += tarr[ pos - 2 ] * 0.1;

    for (int i = pos - 3; i >= 0; --i, d *= 60)
    {
      ret += tarr[ i ] * d;
      int a = 0;
    }

    out = ret;
    return p2 - ts;
  }

  // For simple LRC format
  do
  {
    while (util::is_number(*p2))
      ++p2;

    if (pos == 3)
    {
      m_is_panel_format = true;
      m_convert_format_start = false;
      return 0;
    }

    if ((':' == *p2 || '.' == *p2) && pos < 3)
    {
      if ('.' == *p2)
        if (b1)
        {
          ret = 0.0;
          return 0;
        }
        else
          b1 = true;

      tarr[ pos ] = util::xtoi(p1, p2 - p1);

      if (tarr[ pos ] > 59)
      {
        ret = 0.0;
        return 0;
      }

      p1 = ++p2;
      ++pos;
    }
    else if (']' == *p2)
    {
      if (b1 || pos == 2)
      {
        double tmp = 0.1;

        for (const tchar * p = p1; p < p2; ++p, tmp /= 10.0)
          ret += (double) (*p - '0') * tmp;

        ++p2;

        break;
      }
      else if (pos < 2)
      {
        tarr[ pos ] = util::xtoi(p1, p2 - p1);

        if (tarr[ pos ] > 59)
        {
          ret = 0.0;
          return 0;
        }

        ++p2;
        ++pos;
        break;
      }
      else
      {
        ret = 0.0;
        return 0;
      }
    }
    else
    {
      ret = 0.0;
      return 0;
    }
  }
  while (true);

  uint d = 1;

  for (int i = util::std_max<int>(0, pos - 1); i >=
        0;

        --i, d *= 60)

  {
    ret += tarr[ i ] * d;
    int a = 0;
  }

  out = ret;
  return p2 - ts;
}

//double to string [xx:xx.xx]
const tchar * parse_lyrics::t2ts(double t, tstring & out)
{
  out.reserve(16);
  out = '[';
  uint tmp;
  uint time = (uint) (t * 100.0);

  if (time >= 360000)
  {
    tmp = time / 360000;
    out << tmp;
    out += ':';
    time -= tmp * 360000;
  }

  tmp = time / 60000;
  out += '0' + tmp;
  time -= tmp * 60000;
  tmp = time / 6000;
  out += '0' + tmp;
  time -= tmp * 6000;
  out += ':';
  tmp = time / 1000;
  out += '0' + tmp;
  time -= tmp * 1000;
  tmp = time / 100;
  out += '0' + tmp;
  time -= tmp * 100;
  out += '.';
  tmp = time / 10;
  out += '0' + tmp;
  time -= tmp * 10;
  out += '0' + time;
  out += ']';

  return out.c_str();
}

int parse_lyrics::scan_attrib(const tstring & str, tstring * pout)
{
  int ret = LT_UNKNOWN;
  const tchar * p = str.c_str();

  while (' ' == *p || '\t' == *p)
    ++p;

  if ('[' == *p)
  {
    ++p;

    while (' ' == *p || '\t' == *p)
      ++p;
  }
  else
    return LT_ERROR;

  if (!_tcsnicmp(p, _T("offset"), _countof(_T("offset")) - 1))
  {
    ret = LT_OFFSET;
    p += _countof(_T("offset")) - 1;
  }

  //else if (!_tcsnicmp(p, _T("Encoding"), _countof(_T("Encoding")) - 1))
  //{
  // ret = LT_CHARSET;
  // p += _countof(_T("Encoding")) - 1;
  //}
  //else
  {

    for (uint i = 0; i < _countof(attrib_arr); ++i)
    {
      if (!_tcsnicmp(p, attrib_arr[ i ].name, ATTRIB_NAME_LENGTH))
      {
        ret = attrib_arr[ i ].type;
        p += ATTRIB_NAME_LENGTH;
        break;
      }
    }

  }

  if (LT_UNKNOWN == ret)
  {
    return LT_ERROR;
  }

  while (' ' == *p || '\t' == *p)
    ++p;

  if (':' != *p)
    return LT_ERROR;

  ++p;

  while (' ' == *p || '\t' == *p)
    ++p;

  const tchar * pend = p + _tcslen(p) - 1;

  if (p == pend)
    return LT_ERROR;

  while (' ' == *pend || '\t' == *pend)
    --pend;

  if (pend <= p || ']' != *pend)
    return LT_ERROR;

  if (pout)
  {
    while ((' ' == *pend || '\t' == *pend) && pend >= p)
      --pend;

    pout->assign(p, pend);
  }

  return ret;
}




void parse_lyrics::reset()
{
  m_currentline.clear();
  data.clear();
  buffer.erase();
  offset_d = 0.0;
  attrib.clear();
  modified = false;
  xm_pos = 0;
  //b_cvt_lyric = false;
  //m_music_time = 0.0;
  m_is_panel_format = false;
  m_convert_format_start = false;
  b_has_timestamp = false;
}

//====================================================
//read lyrics to this object
//======================================================
bool parse_lyrics::read_lyrics(const tchar * in)
{
  ASSERT(in);

  if ('\0' == *in)
  {
    return false;
  }

  //SVP_LogMsg5(L"lyric %s" , in);

  reset();
  tstring line;
  buffer = in;
  const tchar * pos = in;
  uint ts = 0, nts = 0;

  do
  {
    util::get_line(line, pos);
    util::trim_right(line, _T(" \t\r\n"));
    util::trim_left(line, _T(" \t"));

    if (!line.empty() && line.find_first_not_of(_T(" \t")) != tstring::npos)
    {
      attrib_item_type p_attrib;
      int ret = scan_attrib(line, p_attrib.get());

      //SVP_LogMsg5(L"lyric line %s" , line.c_str());
      if (LT_OFFSET == ret)
        offset_d = ((double) util::xtoi(p_attrib->c_str()) + 0.0001) / 1000;

      //else if (LT_CHARSET == ret)
      //{
      // m_encoding.clear();
      // m_encoding.append(p_attrib->c_str());
      //}
      else if (LT_UNKNOWN != ret && LT_ERROR != ret)
      {
        *p_attrib = line;
        attrib.push_back(p_attrib);
      }
      else
      {
        vector<double> v;
        double d;
        const tchar * p = line.c_str();
        uint len = 0;
        
        while ((len = ts2t(p, d)) > 0)
        {
          if (m_is_panel_format && m_convert_format_start)
            break;


          v.push_back(d);

          p += len;

          while (' ' == *p || '\t' == *p)
            ++p;
        }

        if (m_is_panel_format && m_convert_format_start)
        {
          data.clear();
          //buffer.erase();
          offset_d = 0.0;
          attrib.clear();
          //modified = false;
          //xm_pos = 0;
          //m_is_panel_format = false;
          m_convert_format_start = false;
          pos = in;

          continue;
        }

        if (v.size() > 0)
        {
          for (uint i = 0; i < v.size(); ++i)
          {
            item_type item;
            item->Time = v[ i ];

            if (0 == *p)
              item->Lyric = _T(" ");
            else{
              item->Lyric = p;
              //SVP_LogMsg5(L"lyric %f %s", double(item->Time), item->Lyric.c_str());
            }
            data.push_back(item);
           
          }

          ++ts;
        }
        else
          ++nts;
      }
    }

  }
  while (0 != *pos);

  b_has_timestamp = ts > 7 || ts > 2 * nts;

  if (!b_has_timestamp)
  {
    pos = in;
    data.clear();

    do
    {
      item_type item;
      util::get_line(item->Lyric, pos);
      util::trim_right(item->Lyric, _T("\r\n"));
      data.push_back(item);
    }
    while (0 != *pos);
  }
  else if (data.size() > 0)
  {
    std::sort(data.begin(), data.end());
  }
//SVP_LogMsg5(L"lyric xx %d", data.size());

  return true;
}

void parse_lyrics::convert_offset()
{
  iterator iter = data.begin();

  while (iter != data.end())
  {

    (double) iter->get
    () ->Time -= offset_d;

    if (iter->get
         () ->Time < 0)
      (double) iter->get
      () ->Time = 0.0;

    ++iter;
  }
}

const tchar * parse_lyrics::remake_lyric(bool b_include_tag)
{
  const tchar crlf_sig[] = _T("\r\n");

  if (!has_timestamp())
    return buffer.c_str();

  buffer.erase();

  if (b_include_tag)
  {
    for (uint i = 0; i < attrib.size(); ++i)
    {
      buffer += *attrib[ i ];
      buffer += crlf_sig;
    }

    if (offset_d > 0.01 || offset_d < -0.01)
    {
      buffer += _T("[offset:");
      //buffer += pfc::stringcvt::string_os_from_utf8_fast(pfc::format_int((int) (offset_d * 1000)));
      CString szBuf;
      szBuf.Format(L"%d", (int) (offset_d * 1000));
      buffer += szBuf ;
      buffer += _T("]");
      buffer += crlf_sig;
    }

    // add encoding
    //if (m_encoding == 0)
    //{
    // buffer += _T("[Encoding:");
    // buffer.append(m_encoding);
    // buffer += _T("]\r\n");
    //}

    buffer += crlf_sig;

    iterator iter = data.begin();

    typedef util::bind_ptr<lyric_type> str_item_type;

    vector<str_item_type> v;

    while (iter != data.end())
    {
      vector<str_item_type>::iterator iter2 = v.begin();

      while (iter2 != v.end())
      {
        if (0 == _tcscmp(iter->get
                           () ->Lyric.c_str(), iter2->get
                           () ->Lyric.c_str()))
          break;

        ++iter2;
      }

      if (iter2 == v.end())
      {
        //add new one
        str_item_type pnew((str_item_type::data_t *) str_item_type::create());

        t2ts(iter->get
              () ->Time, pnew->Time);

        pnew->Lyric = iter->get
                      () ->Lyric;

        v.push_back(pnew);
      }
      else
      {
        //append one
        tstring tmp;

        t2ts(iter->get
              () ->Time, tmp);

        iter2->get
        () ->Time += tmp;
      }

      ++iter;
    }

    for (vector<str_item_type>::iterator iter2 = v.begin(); iter2 != v.end(); ++iter2)
    {

      buffer += iter2->get
                () ->Time;

      buffer += iter2->get
                () ->Lyric;

      buffer += crlf_sig;
    }
  }
  else
  {
    // Without timestamp

    for (data_type::iterator iter = data.begin(); iter != data.end(); ++iter)
    {

      tstring line = iter->get
                     () ->Lyric;

      buffer += util::trim(line, _T(" "));

      buffer += crlf_sig;
    }

    util::trim(buffer, _T(" \r\n"));
  }

  return buffer.c_str();
}


parse_lyrics::iterator parse_lyrics::find_lyric(double position)
{
  ASSERT(has_timestamp());
  ASSERT(data.size());

  if (xm_pos >= data.size())
    xm_pos = 0;

  double pos = position + offset_d;

  if (pos > data[ xm_pos ] ->Time)
  {
    for (uint i = xm_pos; i < data.size(); ++i)
    {
      if (pos < data[ i ] ->Time)
      {
        xm_pos = i - !(0 == i);
        return data.begin() + xm_pos;
      }
    }
  }
  else
  {
    for (uint i = xm_pos; i != ~0U; --i)
    {
      if (pos >= data[ i ] ->Time)
      {
        xm_pos = i;
        break;
      }
    }

    return data.begin() + xm_pos;
  }

  return data.end();
}


CString parse_lyrics::find_lyric_line(double position)
{
    if(data.size() <= 0)
        return L"";

    if(!has_timestamp())
        return L"";

    xm_pos = 0;

    double pos = position + offset_d;

    if (pos > data[ xm_pos ] ->Time)
    {
        for (uint i = xm_pos; i < data.size(); ++i)
        {
            if (pos < data[ i ] ->Time)
            {
                xm_pos = i - !(0 == i);
                try{
                    return CString(data[xm_pos].get() ->Lyric.c_str());
                }catch(...){}
                break;
            }
        }
    }
    
   
    //item_type
    return L"";
}
