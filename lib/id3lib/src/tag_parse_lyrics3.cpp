// $Id: tag_parse_lyrics3.cpp,v 1.35 2002/10/04 08:52:23 t1mpy Exp $

// id3lib: a C++ library for creating and manipulating id3v1/v2 tags
// Copyright 1999, 2000  Scott Thomas Haug
// Copyright 2002 Thijmen Klok (thijmen@id3lib.org)

// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
// License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

// The id3lib authors encourage improvements and optimisations to be sent to
// the id3lib coordinator.  Please see the README file for details on where to
// send such submissions.  See the AUTHORS file for a list of people who have
// contributed to id3lib.  See the ChangeLog file for a list of changes to
// id3lib.  These files are distributed with id3lib at
// http://download.sourceforge.net/id3lib/

#include <ctype.h>
#include <memory.h>
#include "tag_impl.h" //has <stdio.h> "tag.h" "header_tag.h" "frame.h" "field.h" "spec.h" "id3lib_strings.h" "utils.h"
#include "helpers.h"
#include "id3/io_decorators.h" //has "readers.h" "io_helpers.h" "utils.h"
#include "io_strings.h"

using namespace dami;

namespace
{
  uint32 readIntegerString(ID3_Reader& reader, size_t numBytes)
  {
    uint32 val = 0;
    for (size_t i = 0; i < numBytes && isdigit(reader.peekChar()); ++i)
    {
      val = (val * 10) + (reader.readChar() - '0');
    }
    ID3D_NOTICE( "readIntegerString: val = " << val );
    return val;
  }

  uint32 readIntegerString(ID3_Reader& reader)
  {
    return readIntegerString(reader, reader.remainingBytes());
  }

  bool isTimeStamp(ID3_Reader& reader)
  {
    ID3_Reader::pos_type cur = reader.getCur();
    if (reader.getEnd() < cur + 7)
    {
      return false;
    }
    bool its = ('[' == reader.readChar() &&
                isdigit(reader.readChar()) && isdigit(reader.readChar()) &&
                ':' == reader.readChar() &&
                isdigit(reader.readChar()) && isdigit(reader.readChar()) &&
                ']' == reader.readChar());
    reader.setCur(cur);
    if (its)
    {
      ID3D_NOTICE( "isTimeStamp(): found timestamp, cur = " << reader.getCur() );
    }
    return its;
  }

  uint32 readTimeStamp(ID3_Reader& reader)
  {
    reader.skipChars(1);
    size_t sec = readIntegerString(reader, 2) * 60;
    reader.skipChars(1);
    sec += readIntegerString(reader, 2);
    reader.skipChars(1);
    ID3D_NOTICE( "readTimeStamp(): timestamp = " << sec );
    return sec * 1000;
  }

  bool findText(ID3_Reader& reader, String text)
  {
    if (text.empty())
    {
      return true;
    }

    size_t index = 0;
    while (!reader.atEnd())
    {
      ID3_Reader::char_type ch = reader.readChar();
      if (ch == text[index])
      {
        index++;
      }
      else if (ch == text[0])
      {
        index = 1;
      }
      else
      {
        index = 0;
      }
      if (index == text.size())
      {
        reader.setCur(reader.getCur() - index);
        ID3D_NOTICE( "findText: found \"" << text << "\" at " <<
                     reader.getCur() );
        break;
      }
    }
    return !reader.atEnd();
  };

  void lyrics3ToSylt(ID3_Reader& reader, ID3_Writer& writer)
  {
    while (!reader.atEnd())
    {
      bool lf = false;
      size_t ms = 0;
      size_t count = 0;
      while (isTimeStamp(reader))
      {
        // For now, just skip over multiple time stamps
        if (count++ > 0)
        {
          readTimeStamp(reader);
        }
        else
        {
          ms = readTimeStamp(reader);
        }
      }
      while (!reader.atEnd() && !isTimeStamp(reader))
      {
        ID3_Reader::char_type ch = reader.readChar();
        if (0x0A == ch && (reader.atEnd() || isTimeStamp(reader)))
        {
          lf = true;
          break;
        }
        else
        {
          writer.writeChar(ch);
        }
      }

      // put synch identifier
      writer.writeChar('\0');

      // put timestamp
      ID3D_NOTICE( "lyrics3toSylt: ms = " << ms );

      io::writeBENumber(writer, ms, sizeof(uint32));
      if (lf)
      {
        ID3D_NOTICE( "lyrics3toSylt: adding lf" );

        // put the LF
        writer.writeChar(0x0A);
      }
    }
  }
};

bool lyr3::v1::parse(ID3_TagImpl& tag, ID3_Reader& reader)
{
  io::ExitTrigger et(reader);
  ID3_Reader::pos_type end = reader.getCur();
  if (end < reader.getBeg() + 9 + 128)
  {
    ID3D_NOTICE( "id3::v1::parse: bailing, not enough bytes to parse, pos = " << end );
    return false;
  }
  reader.setCur(end - (9 + 128));

  {
    if (io::readText(reader, 9) != "LYRICSEND" ||
        io::readText(reader, 3) != "TAG")
    {
      return false;
    }
  }

  // we have a Lyrics3 v1.00 tag
  if (end < reader.getBeg() + 11 + 9 + 128)
  {
    // the file size isn't large enough to actually hold lyrics
    ID3D_WARNING( "id3::v1::parse: not enough data to parse lyrics3" );
    return false;
  }

  // reserve enough space for lyrics3 + id3v1 tag
  size_t window = end - reader.getBeg();
  size_t lyrDataSize = min<size_t>(window, 11 + 5100 + 9 + 128);
  reader.setCur(end - lyrDataSize);
  io::WindowedReader wr(reader, lyrDataSize - (9 + 128));

  if (!findText(wr, "LYRICSBEGIN"))
  {
    ID3D_WARNING( "id3::v1::parse: couldn't find LYRICSBEGIN, bailing" );
    return false;
  }

  et.setExitPos(wr.getCur());
  wr.skipChars(11);
  wr.setBeg(wr.getCur());

  io::LineFeedReader lfr(wr);
  String lyrics = io::readText(lfr, wr.remainingBytes());
  id3::v2::setLyrics(tag, lyrics, "Converted from Lyrics3 v1.00", "XXX");

  return true;
}

//bool parse(TagImpl& tag, ID3_Reader& reader)
bool lyr3::v2::parse(ID3_TagImpl& tag, ID3_Reader& reader)
{
  io::ExitTrigger et(reader);
  ID3_Reader::pos_type end = reader.getCur();
  if (end < reader.getBeg() + 6 + 9 + 128)
  {
    ID3D_NOTICE( "lyr3::v2::parse: bailing, not enough bytes to parse, pos = " << reader.getCur() );
    return false;
  }

  reader.setCur(end - (6 + 9 + 128));
  uint32 lyrSize = 0;

  ID3_Reader::pos_type beg = reader.getCur();
  lyrSize = readIntegerString(reader, 6);
  if (reader.getCur() < beg + 6)
  {
    ID3D_NOTICE( "lyr3::v2::parse: couldn't find numeric string, lyrSize = " <<
                 lyrSize );
    return false;
  }

  if (io::readText(reader, 9) != "LYRICS200" ||
      io::readText(reader, 3) != "TAG")
  {
    return false;
  }

  if (end < reader.getBeg() + lyrSize + 6 + 9 + 128)
  {
    ID3D_WARNING( "lyr3::v2::parse: not enough data to parse tag, lyrSize = " << lyrSize );
    return false;
  }
  reader.setCur(end - (lyrSize + 6 + 9 + 128));

  io::WindowedReader wr(reader);
  wr.setWindow(wr.getCur(), lyrSize);

  beg = wr.getCur();

  if (io::readText(wr, 11) != "LYRICSBEGIN")
  {
    // not a lyrics v2.00 tag
    ID3D_WARNING( "lyr3::v2::parse: couldn't find LYRICSBEGIN, bailing" );
    return false;
  }

  bool has_time_stamps = false;

  ID3_Frame* lyr_frame = NULL;

  while (!wr.atEnd())
  {
    uint32 fldSize;

    String fldName = io::readText(wr, 3);
    ID3D_NOTICE( "lyr3::v2::parse: fldName = " << fldName );
    fldSize = readIntegerString(wr, 5);
    ID3D_NOTICE( "lyr3::v2::parse: fldSize = " << fldSize );

    String fldData;

    io::WindowedReader wr2(wr, fldSize);
    io::LineFeedReader lfr(wr2);

    fldData = io::readText(lfr, fldSize);
    ID3D_NOTICE( "lyr3::v2::parse: fldData = \"" << fldData << "\"" );

    // the IND field
    if (fldName == "IND")
    {
      has_time_stamps = (fldData.size() > 1 && fldData[1] == '1');
    }

    // the TITLE field
    else if (fldName == "ETT" && !id3::v2::hasTitle(tag))
    {
      //tag.setTitle(fldData);
      id3::v2::setTitle(tag, fldData);
    }

    // the ARTIST field
    else if (fldName == "EAR" && !id3::v2::hasArtist(tag))
    {
      //tag.setArtist(fldData);
      id3::v2::setArtist(tag, fldData);
    }

    // the ALBUM field
    else if (fldName == "EAL" && !id3::v2::hasAlbum(tag))
    {
      //tag.setAlbum(fldData);
      id3::v2::setAlbum(tag, fldData);
    }

    // the Lyrics/Music AUTHOR field
    else if (fldName == "AUT")
    {
      //tag.setAuthor(fldData);
      id3::v2::setLyricist(tag, fldData);
    }

    // the INFORMATION field
    else if (fldName == "INF")
    {
      //tag.setInfo(fldData);
      id3::v2::setComment(tag, fldData, "Lyrics3 v2.00 INF", "XXX");
    }

    // the LYRICS field
    else if (fldName == "LYR")
    {
      // if already found an INF field, use it as description
      String desc =  "Converted from Lyrics3 v2.00";
      //tag.setLyrics(fldData);
      if (!has_time_stamps)
      {
        lyr_frame = id3::v2::setLyrics(tag, fldData, desc, "XXX");
      }
      else
      {
        // converts from lyrics3 to SYLT in-place
        io::StringReader sr(fldData);
        ID3D_NOTICE( "lyr3::v2::parse: determining synced lyrics" );
        BString sylt;
        io::BStringWriter sw(sylt);
        lyrics3ToSylt(sr, sw);

        lyr_frame = id3::v2::setSyncLyrics(tag, sylt, ID3TSF_MS, desc,
                                           "XXX", ID3CT_LYRICS);
        ID3D_NOTICE( "lyr3::v2::parse: determined synced lyrics" );
      }
    }
    else if (fldName == "IMG")
    {
      // currently unsupported
      ID3D_WARNING( "lyr3::v2::parse: IMG field unsupported" );
    }
    else
    {
      ID3D_WARNING( "lyr3::v2::parse: undefined field id: " <<
                    fldName );
    }
  }

  et.setExitPos(beg);
  return true;
}

