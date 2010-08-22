#ifndef __PARSE_LYRICS_H
#define __PARSE_LYRICS_H

//this library is from foo-display-lyrics 
// http://code.google.com/p/foo-display-lyrics/

#include <stdlib.h>
#include <cstring>
#include <iostream>

#include "utils.h"

enum LYRIC_ATTRIB_TYPE { LT_ERROR, LT_UNKNOWN, LT_OFFSET,       /*LT_CHARSET,*/ LT_TITLE, LT_ARTIST, LT_ALBUM, LT_AUTHOR, LT_LANGUAGE };


class parse_lyrics
{

  public:

    struct LYRIC_STRUCT
    {
      double Time;
      tstring Lyric;
      inline bool operator<(const LYRIC_STRUCT & rhs) { return Time < rhs.Time; }
    };

    typedef util::bind_ptr<LYRIC_STRUCT> item_type;
    typedef std::vector<item_type> data_type;
    typedef data_type::const_iterator iterator;

    typedef util::bind_ptr<tstring> attrib_item_type;
    typedef std::vector<attrib_item_type> attrib_data_type;

  private:

    struct lyric_type
    {
      tstring Time;
      tstring Lyric;
    };

    double offset_d;
    data_type data;
    attrib_data_type attrib;
    tstring buffer;
    bool modified;
    bool b_has_timestamp;
    uint xm_pos;

    //uint m_encoding; //Encoding TAG

    bool m_is_panel_format;
    bool m_convert_format_start;

    //
    tstring m_currentline;

  public:
    parse_lyrics() { reset(); }

    void reset();

    inline iterator begin() { return data.begin(); }

    inline iterator end() { return data.end(); }

    inline bool is_modified() const { return modified; }

    inline bool has_timestamp() const { return b_has_timestamp; }

    inline data_type::size_type get_count() const { return data.size(); }

    bool read_lyrics(const tchar * in);

    inline double get_offset() const { return offset_d; }

    inline void set_offset(double offset) { offset_d = offset; modified = true; }

    const tchar * remake_lyric(bool b_include_tag);
    parse_lyrics::iterator find_lyric(double position);
    void convert_offset();

    uint ts2t(const tchar * const ts, double & out);
    const tchar * t2ts(double t, tstring & out);
    int scan_attrib(const tstring & str, tstring * pout);

    inline tstring & currentline() { return m_currentline; }
    CString find_lyric_line(double position);
};


#endif
