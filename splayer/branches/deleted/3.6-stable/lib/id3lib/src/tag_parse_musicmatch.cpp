// $Id: tag_parse_musicmatch.cpp,v 1.19 2002/07/02 22:15:18 t1mpy Exp $

// id3lib: a C++ library for creating and manipulating id3v1/v2 tags
// Copyright 1999, 2000  Scott Thomas Haug

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

#if defined HAVE_CONFIG_H
#include <config.h>
#endif


#include <ctype.h>
#include "tag_impl.h" //has <stdio.h> "tag.h" "header_tag.h" "frame.h" "field.h" "spec.h" "id3lib_strings.h" "utils.h"
#include "helpers.h"
#include "id3/io_decorators.h" //has "readers.h" "io_helpers.h" "utils.h"

using namespace dami;

namespace 
{
  uint32 readSeconds(ID3_Reader& reader, size_t len)
  {
    io::ExitTrigger et(reader);
    io::WindowedReader wr(reader, len);
    ID3_Reader::pos_type beg = wr.getCur();
    uint32 seconds = 0;
    uint32 cur = 0;
    while (!wr.atEnd())
    {
      ID3_Reader::char_type ch = wr.readChar();
      if (':' == ch)
      {
        seconds += 60 * cur;
        cur = 0;
      }
      else if (!isdigit(ch))
      {
        return 0;
      }
      else
      {
        cur = cur * 10 + (ch - '0');
      }
    }
    et.release();
    return seconds + cur;
  }
  
  ID3_Frame* readTextFrame(ID3_Reader& reader, ID3_FrameID id, const String desc = "")
  {
    uint32 size = io::readLENumber(reader, 2);
    ID3D_NOTICE( "readTextFrame: size = " << size );
    if (size == 0)
    {
      return NULL;
    }
    
    String text;
    if (ID3FID_SONGLEN != id)
    {
      io::LineFeedReader lfr(reader);
      text = io::readText(lfr, size);
      ID3D_NOTICE( "readTextFrame: text = " << text );
    }
    else
    {
      text = toString(readSeconds(reader, size) * 1000);
      ID3D_NOTICE( "readTextFrame: songlen = " << text );
    }
    
    ID3_Frame* frame = new ID3_Frame(id);
    if (frame)
    {
      if (frame->Contains(ID3FN_TEXT))
      {
        frame->GetField(ID3FN_TEXT)->Set(text.c_str());
      }
      else if (frame->Contains(ID3FN_URL))
      {
        frame->GetField(ID3FN_URL)->Set(text.c_str());
      }
      if (frame->Contains(ID3FN_LANGUAGE))
      {
        frame->GetField(ID3FN_LANGUAGE)->Set("XXX");
      }
      if (frame->Contains(ID3FN_DESCRIPTION))
      {
        frame->GetField(ID3FN_DESCRIPTION)->Set(desc.c_str());
      }
    }
    return frame;
  }
};
  
bool mm::parse(ID3_TagImpl& tag, ID3_Reader& rdr)
{
  io::ExitTrigger et(rdr);
  ID3_Reader::pos_type end = rdr.getCur();
  if (end < rdr.getBeg() + 48)
  {
    ID3D_NOTICE( "mm::parse: bailing, not enough bytes to parse, pos = " << end );
    return false;
  }
  
  rdr.setCur(end - 48);
  String version;
  
  {
    if (io::readText(rdr, 32) != "Brava Software Inc.             ")
    {
      ID3D_NOTICE( "mm::parse: bailing, couldn't find footer" );
      return false;
    }
    
    version = io::readText(rdr, 4);
    if (version.size() != 4 || 
        !isdigit(version[0]) || version[1] != '.' ||
        !isdigit(version[2]) || 
        !isdigit(version[3]))
    {
      ID3D_WARNING( "mm::parse: bailing, nonstandard version = " << version );
      return false;
    }
  }
    
  ID3_Reader::pos_type beg = rdr.setCur(end - 48);
  et.setExitPos(beg);
  if (end < 68)
  {
    ID3D_NOTICE( "mm::parse: bailing, not enough bytes to parse offsets, pos = " << end );
    return false;
  }
  rdr.setCur(end - 68);
    
  io::WindowedReader dataWindow(rdr);
  dataWindow.setEnd(rdr.getCur());

  uint32 offsets[5];
    
  io::WindowedReader offsetWindow(rdr, 20);
  for (size_t i = 0; i < 5; ++i)
  {
    offsets[i] = io::readLENumber(rdr, sizeof(uint32));
  }

  size_t metadataSize = 0;
  if (version <= "3.00")
  {
    // All MusicMatch tags up to and including version 3.0 had metadata 
    // sections exactly 7868 bytes in length.
    metadataSize = 7868;
  }
  else
  {
    // MusicMatch tags after version 3.0 had three possible lengths for their
    // metadata sections.  We can determine which it was by searching for
    // the version section signature that should precede the metadata section
    // by exactly 256 bytes.
    size_t possibleSizes[] = { 8132, 8004, 7936 };
      
    for (size_t i = 0; i < sizeof(possibleSizes)/sizeof(size_t); ++i)
    {
      dataWindow.setCur(dataWindow.getEnd());
        
      // Our offset will be exactly 256 bytes prior to our potential metadata
      // section
      size_t offset = possibleSizes[i] + 256;
      if (dataWindow.getCur() < offset)
      {
        // if our filesize is less than the offset, then it can't possibly
        // be the correct offset, so try again.
        continue;
      }
      dataWindow.setCur(dataWindow.getCur() - offset);
        
      // now read in the signature to see if it's a match
      if (io::readText(dataWindow, 8) == "18273645")
      {
        metadataSize = possibleSizes[i];
        break;
      }
    }
  }
  if (0 == metadataSize)
  {
    // if we didn't establish a size for the metadata, then something is
    // wrong.  probably should log this.
    ID3D_WARNING( "mm::parse: bailing, couldn't find meta data signature, end = " << end );
    return false;
  }
    
  // parse the offset pointers to determine the actual sizes of all the 
  // sections
  size_t sectionSizes[5];
  size_t tagSize = metadataSize;
    
  // we already know the size of the last section
  sectionSizes[4] = metadataSize;
    
  size_t lastOffset = 0;
  for (int i = 0; i < 5; i++)
  {
    size_t thisOffset = offsets[i];
    //ASSERT(thisOffset > lastOffset);
    if (i > 0)
    {
      size_t sectionSize = thisOffset - lastOffset;
      sectionSizes[i-1] = sectionSize;
      tagSize += sectionSize;
    }
    lastOffset = thisOffset;
  }
    
  // now check to see that our tag size is reasonable
  if (dataWindow.getEnd() < tagSize)
  {
    // Ack!  The tag size doesn't jive with the tag's ending position in
    // the file.  Bail!
    ID3D_WARNING( "mm::parse: bailing, tag size is too big, tag size = " << tagSize << ", end = " << end );
    return false;
  }

  dataWindow.setBeg(dataWindow.getEnd() - tagSize);
  dataWindow.setCur(dataWindow.getBeg());
    
  // Now calculate the adjusted offsets
  offsets[0] = dataWindow.getBeg();
  for (size_t i = 0; i < 4; ++i)
  {
    offsets[i+1] = offsets[i] + sectionSizes[i];
  }
    
  // now check for a tag header and adjust the tag_beg pointer appropriately
  if (dataWindow.getBeg() >= 256)
  {
    rdr.setCur(dataWindow.getBeg() - 256);
    if (io::readText(rdr, 8) == "18273645")
    {
      et.setExitPos(rdr.getCur() - 8);
    }
    else
    {
      et.setExitPos(dataWindow.getBeg());
    }
    dataWindow.setCur(dataWindow.getBeg());
  }
    
  // Now parse the various sections...
    
  // Parse the image extension at offset 0
  dataWindow.setCur(offsets[0]);
  String imgExt = io::readTrailingSpaces(dataWindow, 4);
    
  // Parse the image binary at offset 1
  dataWindow.setCur(offsets[1]);
  uint32 imgSize = io::readLENumber(dataWindow, 4);
  if (imgSize == 0)
  {
    // no image binary.  don't do anything.
  }
  else
  {
    io::WindowedReader imgWindow(dataWindow, imgSize);
    if (imgWindow.getEnd() < imgWindow.getBeg() + imgSize)
    {
      // Ack!  The image size given extends beyond the next offset!  This is 
      // not good...  log?
    }
    else
    {
      BString imgData = io::readAllBinary(imgWindow);
      ID3_Frame* frame = new ID3_Frame(ID3FID_PICTURE);
      if (frame)
      {
        String mimetype("image/");
        mimetype += imgExt;
        frame->GetField(ID3FN_MIMETYPE)->Set(mimetype.c_str());
        frame->GetField(ID3FN_IMAGEFORMAT)->Set("");
        frame->GetField(ID3FN_PICTURETYPE)->Set(static_cast<unsigned int>(0));
        frame->GetField(ID3FN_DESCRIPTION)->Set("");
        frame->GetField(ID3FN_DATA)->Set(reinterpret_cast<const uchar*>(imgData.data()), imgData.size());
        tag.AttachFrame(frame);
      }
    }
  }
    
  //file.seekg(offsets[2]);
  //file.seekg(offsets[3]);
  dataWindow.setCur(offsets[4]);
    
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_TITLE));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_ALBUM));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_LEADARTIST));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_CONTENTTYPE));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_COMMENT, "MusicMatch_Tempo"));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_COMMENT, "MusicMatch_Mood"));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_COMMENT, "MusicMatch_Situation"));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_COMMENT, "MusicMatch_Preference"));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_SONGLEN));
    
  // The next 12 bytes can be ignored.  The first 8 represent the 
  // creation date as a 64 bit floating point number.  The last 4 are
  // for a play counter.
  dataWindow.skipChars(12);
    
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_COMMENT, "MusicMatch_Path"));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_COMMENT, "MusicMatch_Serial"));
    
  // 2 bytes for track
  uint32 trkNum = io::readLENumber(dataWindow, 2);
  if (trkNum > 0)
  {
    String trkStr = toString(trkNum);
    ID3_Frame* frame = new ID3_Frame(ID3FID_TRACKNUM);
    if (frame)
    {
      frame->GetField(ID3FN_TEXT)->Set(trkStr.c_str());
      tag.AttachFrame(frame);
    }
  }
    
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_COMMENT, "MusicMatch_Notes"));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_COMMENT, "MusicMatch_Bio"));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_UNSYNCEDLYRICS));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_WWWARTIST));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_WWWCOMMERCIALINFO));
  tag.AttachFrame(readTextFrame(dataWindow, ID3FID_COMMENT, "MusicMatch_ArtistEmail"));
    
  // email?

  return true;
}

