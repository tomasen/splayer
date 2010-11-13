// $Id: tag_parse.cpp,v 1.47 2002/11/24 17:33:30 t1mpy Exp $

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

//#if defined HAVE_CONFIG_H
//#include <config.h> // Must include before zlib.h to compile on WinCE
//#endif

//#include <zlib.h>
//#include <string.h>
//#include <memory.h>

#include "tag_impl.h" //has <stdio.h> "tag.h" "header_tag.h" "frame.h" "field.h" "spec.h" "id3lib_strings.h" "utils.h"
//#include "id3/io_decorators.h" //has "readers.h" "io_helpers.h" "utils.h"
#include "io_strings.h"

using namespace dami;

namespace
{
  bool parseFrames(ID3_TagImpl& tag, ID3_Reader& rdr)
  {
    ID3_Reader::pos_type beg = rdr.getCur();
    io::ExitTrigger et(rdr, beg);
    ID3_Reader::pos_type last_pos = beg;
    size_t totalSize = 0;
    size_t frameSize = 0;
    while (!rdr.atEnd() && rdr.peekChar() != '\0')
    {
      ID3D_NOTICE( "id3::v2::parseFrames(): rdr.getBeg() = " << rdr.getBeg() );
      ID3D_NOTICE( "id3::v2::parseFrames(): rdr.getCur() = " << rdr.getCur() );
      ID3D_NOTICE( "id3::v2::parseFrames(): rdr.getEnd() = " << rdr.getEnd() );
      last_pos = rdr.getCur();
      ID3_Frame* f = new ID3_Frame;
      f->SetSpec(tag.GetSpec());
      bool goodParse = f->Parse(rdr);
      frameSize = rdr.getCur() - last_pos;
      ID3D_NOTICE( "id3::v2::parseFrames(): frameSize = " << frameSize );
      totalSize += frameSize;

      if (frameSize == 0)
      {
        // There is a problem.
        // If the frame size is 0, then we can't progress.
        ID3D_WARNING( "id3::v2::parseFrames(): frame size is 0, can't " <<
                      "continue parsing frames");
        delete f;
        // Break for now.
        break;
      }
      else if (!goodParse)
      {
        // bad parse!  we can't attach this frame.
        ID3D_WARNING( "id3::v2::parseFrames(): bad parse, deleting frame");
        delete f;
      }
      else if (f->GetID() != ID3FID_METACOMPRESSION)
      {
        ID3D_NOTICE( "id3::v2::parseFrames(): attaching non-compressed " <<
                     "frame");
        // a good, uncompressed frame.  attach away!
        tag.AttachFrame(f);
      }
      else
      {
        ID3D_NOTICE( "id3::v2::parseFrames(): parsing ID3v2.2.1 " <<
                     "compressed frame");
        // hmm.  an ID3v2.2.1 compressed frame.  It contains 1 or more
        // compressed frames.  Uncompress and call parseFrames recursively.
        ID3_Field* fld = f->GetField(ID3FN_DATA);
        if (fld)
        {
          ID3_MemoryReader mr(fld->GetRawBinary(), fld->BinSize());
          ID3_Reader::char_type ch = mr.readChar();
          if (ch != 'z')
          {
            // unknown compression method
            ID3D_WARNING( "id3::v2::parseFrames(): unknown compression id " <<
                          " = '" << ch << "'" );
          }
          else
          {
            uint32 newSize = io::readBENumber(mr, sizeof(uint32));
            size_t oldSize = f->GetDataSize() - sizeof(uint32) - 1;
            io::CompressedReader cr(mr, newSize);
            parseFrames(tag, cr);
            if (!cr.atEnd())
            {
              // hmm.  it didn't parse the entire uncompressed data.  wonder
              // why.
              ID3D_WARNING( "id3::v2::parseFrames(): didn't parse entire " <<
                            "id3v2.2.1 compressed memory stream");
            }
          }
        }
        delete f;
      }
      et.setExitPos(rdr.getCur());
    }
    if (rdr.peekChar() == '\0')
    {
      ID3D_NOTICE( "id3::v2::parseFrames: done parsing, padding at postion " <<
                   rdr.getCur() );
    }
    else
    {
      ID3D_NOTICE( "id3::v2::parseFrames: done parsing, [cur, end] = [" <<
                   rdr.getCur() << ", " << rdr.getEnd() << "]" );
    }
    return true;
  }
};

bool id3::v2::parse(ID3_TagImpl& tag, ID3_Reader& reader)
{
  ID3_Reader::pos_type beg = reader.getCur();
  io::ExitTrigger et(reader);

  ID3_TagHeader hdr;

  io::WindowedReader wr(reader, ID3_TagHeader::SIZE);

  if (!hdr.Parse(wr) || wr.getCur() == beg)
  {
    ID3D_NOTICE( "id3::v2::parse(): parsing header failes" );
    return false;
  }
  if (hdr.GetExtended())
  {
    hdr.ParseExtended(reader);
  }
  tag.SetSpec(hdr.GetSpec());

  size_t dataSize = hdr.GetDataSize();
  ID3D_NOTICE( "ID3_TagImpl::Parse(ID3_Reader&): dataSize = " << dataSize);

  wr.setWindow(wr.getCur(), dataSize);
  et.setExitPos(wr.getEnd());

  ID3D_NOTICE( "ID3_TagImpl::Parse(ID3_Reader&): data window beg = " << wr.getBeg() );
  ID3D_NOTICE( "ID3_TagImpl::Parse(ID3_Reader&): data window cur = " << wr.getCur() );
  ID3D_NOTICE( "ID3_TagImpl::Parse(ID3_Reader&): data window end = " << wr.getEnd() );
  tag.SetExtended(hdr.GetExtended());
  if (!hdr.GetUnsync())
  {
    tag.SetUnsync(false);
    parseFrames(tag, wr);
  }
  else
  {
    // The buffer has been unsynced.  It will have to be resynced to be
    // readable.  This has to be done a character at a time.
    //
    // The original reader may be reading in characters from a file.  Doing
    // this a character at a time is quite slow.  To improve performance, read
    // in the entire buffer into a string, then create an UnsyncedReader from
    // the string.
    //
    // It might be better to implement a BufferedReader so that the details
    // of this can be abstracted away behind a class
    tag.SetUnsync(true);
    BString raw = io::readAllBinary(wr);
    io::BStringReader bsr(raw);
    io::UnsyncedReader ur(bsr);
    ID3D_NOTICE( "ID3_TagImpl::Parse(ID3_Reader&): unsync beg = " << ur.getBeg() );
    ID3D_NOTICE( "ID3_TagImpl::Parse(ID3_Reader&): unsync cur = " << ur.getCur() );
    ID3D_NOTICE( "ID3_TagImpl::Parse(ID3_Reader&): unsync end = " << ur.getEnd() );

    // Now read the UnsyncedReader into another string, and parse the frames
    // from the string.  This is done so that 1. the unsynced reader is
    // unsynced exactly once, removing the possibility of multiple unsyncings
    // of the same string, and 2) so that calls to readChars aren't done a
    // character at a time for every call
    BString synced = io::readAllBinary(ur);
    io::BStringReader sr(synced);
    parseFrames(tag, sr);
  }

  return true;
}

void ID3_TagImpl::ParseFile()
{
  ifstream file;
  if (ID3E_NoError != openReadableFile(this->GetFileName(), file))
  {
    // log this...
    return;
  }
  ID3_IFStreamReader ifsr(file);
  ParseReader(ifsr);
  file.close();
}

//used for streaming media
void ID3_TagImpl::ParseReader(ID3_Reader &reader)
{
  size_t mp3_core_size;
  size_t bytes_till_sync;

  io::WindowedReader wr(reader);
  wr.setBeg(wr.getCur());

  _file_tags.clear();
  _file_size = reader.getEnd();

  ID3_Reader::pos_type beg  = wr.getBeg();
  ID3_Reader::pos_type cur  = wr.getCur();
  ID3_Reader::pos_type end  = wr.getEnd();

  ID3_Reader::pos_type last = cur;

  if (_tags_to_parse.test(ID3TT_ID3V2))
  {
    do
    {
      last = cur;
      // Parse tags at the beginning of the file first...
      if (id3::v2::parse(*this, wr))
      {
        _file_tags.add(ID3TT_ID3V2);
      }
      cur  = wr.getCur();
      wr.setBeg(cur);
    } while (!wr.atEnd() && cur > last);
  }
  // add silly padding outside the tag to _prepended_bytes
  if (!wr.atEnd() && wr.peekChar() == '\0')
  {
    ID3D_NOTICE( "ID3_TagImpl::ParseReader(): found padding outside tag" );
    do
    {
      last = cur;
      cur = wr.getCur() + 1;
      wr.setBeg(cur);
      wr.setCur(cur);
    } while (!wr.atEnd() &&  cur > last && wr.peekChar() == '\0');
  }
  if (!wr.atEnd() && _file_size - (cur - beg) > 4 && wr.peekChar() == 255)
  { //unfortunatly, this is necessary for finding an invalid padding
    wr.setCur(cur + 1); //cur is known by peekChar
    if (wr.readChar() == '\0' && wr.readChar() == '\0' && wr.peekChar() == '\0')
    { //three empty bytes found, enough for me, this is stupid padding
      cur += 3; //those are now allready read in (excluding the peekChar, since it will be added by do{})
      do
      {
        last = cur;
        cur = wr.getCur() + 1;
        wr.setBeg(cur);
        wr.setCur(cur);
      } while (!wr.atEnd() &&  cur > last && wr.peekChar() == '\0');
    }
    else
      wr.setCur(cur);
  }
  _prepended_bytes = cur - beg;
  // go looking for the first sync byte to add to bytes_till_sync
  // by not adding it to _prepended_bytes, we preserve this 'unknown' data
  // The routine's only effect is helping the lib to find things as bitrate etc.
  beg  = wr.getBeg();
  if (!wr.atEnd() && wr.peekChar() != 0xFF) //no sync byte, so, either this is not followed by a mp3 file or it's a fLaC file, or an encapsulating format, better check it
  {
    ID3D_NOTICE( "ID3_TagImpl::ParseReader(): Didn't find mp3 sync byte" );
    if ((_file_size - (cur - beg)) >= 4)
    { //there is room to search for some kind of ID
      unsigned char buf[5];
      wr.readChars(buf, 4);
      buf[4] = '\0';
      // check for RIFF (an encapsulating format) ID
      if (strncmp((char*)buf, "RIFF", 4) == 0 || strncmp((char*)buf, "RIFX", 4) == 0)
      {
        // next 4 bytes are RIFF size, skip them
        cur = wr.getCur() + 4;
        wr.setCur(cur);
        // loop until first possible sync byte
        if (!wr.atEnd() && wr.peekChar() != 0xFF)
        {
          do
          {
            last = cur;
            cur = wr.getCur() + 1;
            wr.setCur(cur);
          } while (!wr.atEnd() &&  cur > last && wr.peekChar() != 0xFF);
        }
      }
      else if (strncmp((char*)buf, "fLaC", 4) == 0)
      { //a FLAC file, no need looking for a sync byte
        beg = cur;
      }
      else
      { //since we set the cursor 4 bytes ahead for looking for RIFF, RIFX or fLaC, better set it back
        // but peekChar allready checked the first one, so we add one
        cur = cur + 1;
        wr.setCur(cur);
        //go looking for a sync byte
        if (!wr.atEnd() && wr.peekChar() != 0xFF) //no sync byte, we have an unknown byte
        {
          do
          {
            last = cur;
            cur = wr.getCur() + 1;
            wr.setCur(cur);
          } while (!wr.atEnd() &&  cur > last && wr.peekChar() != 0xFF);
        }
      }
    } //if ((_file_size - (cur - beg)) >= 4)
    else
    { //remaining size is smaller than 4 bytes, can't be useful, but leave it for now
      beg = cur;
      //file.close();
      //return;
    }
  }
  bytes_till_sync = cur - beg;

  cur = wr.setCur(end);
  if (_file_size > _prepended_bytes)
  {
    do
    {
      last = cur;
      ID3D_NOTICE( "ID3_TagImpl::ParseReader(): beg = " << wr.getBeg() );
      ID3D_NOTICE( "ID3_TagImpl::ParseReader(): cur = " << wr.getCur() );
      ID3D_NOTICE( "ID3_TagImpl::ParseReader(): end = " << wr.getEnd() );
      // ...then the tags at the end
      ID3D_NOTICE( "ID3_TagImpl::ParseReader(): musicmatch? cur = " << wr.getCur() );
      if (_tags_to_parse.test(ID3TT_MUSICMATCH) && mm::parse(*this, wr))
      {
        ID3D_NOTICE( "ID3_TagImpl::ParseReader(): musicmatch! cur = " << wr.getCur() );
        _file_tags.add(ID3TT_MUSICMATCH);
        wr.setEnd(wr.getCur());
      }
      ID3D_NOTICE( "ID3_TagImpl::ParseReader(): lyr3v1? cur = " << wr.getCur() );
      if (_tags_to_parse.test(ID3TT_LYRICS3) && lyr3::v1::parse(*this, wr))
      {
        ID3D_NOTICE( "ID3_TagImpl::ParseReader(): lyr3v1! cur = " << wr.getCur() );
        _file_tags.add(ID3TT_LYRICS3);
        wr.setEnd(wr.getCur());
      }
      ID3D_NOTICE( "ID3_TagImpl::ParseReader(): lyr3v2? cur = " << wr.getCur() );
      if (_tags_to_parse.test(ID3TT_LYRICS3V2) && lyr3::v2::parse(*this, wr))
      {
        ID3D_NOTICE( "ID3_TagImpl::ParseReader(): lyr3v2! cur = " << wr.getCur() );
        _file_tags.add(ID3TT_LYRICS3V2);
        cur = wr.getCur();
        wr.setCur(wr.getEnd());//set to end to seek id3v1 tag
        //check for id3v1 tag and set End accordingly
        ID3D_NOTICE( "ID3_TagImpl::ParseReader(): id3v1? cur = " << wr.getCur() );
        if (_tags_to_parse.test(ID3TT_ID3V1) && id3::v1::parse(*this, wr))
        {
          ID3D_NOTICE( "ID3_TagImpl::ParseReader(): id3v1! cur = " << wr.getCur() );
          _file_tags.add(ID3TT_ID3V1);
        }
        wr.setCur(cur);
        wr.setEnd(cur);
      }
      ID3D_NOTICE( "ID3_TagImpl::ParseReader(): id3v1? cur = " << wr.getCur() );
      if (_tags_to_parse.test(ID3TT_ID3V1) && id3::v1::parse(*this, wr))
      {
        ID3D_NOTICE( "ID3_TagImpl::ParseReader(): id3v1! cur = " << wr.getCur() );
        wr.setEnd(wr.getCur());
        _file_tags.add(ID3TT_ID3V1);
      }
      cur = wr.getCur();
    } while (cur != last);
    _appended_bytes = end - cur;

    // Now get the mp3 header
    mp3_core_size = (_file_size - _appended_bytes) - (_prepended_bytes + bytes_till_sync);
    if (mp3_core_size >= 4)
    { //it has at least the size for a mp3 header (a mp3 header is 4 bytes)
      wr.setBeg(_prepended_bytes + bytes_till_sync);
      wr.setCur(_prepended_bytes + bytes_till_sync);
      wr.setEnd(_file_size - _appended_bytes);

      _mp3_info = new Mp3Info;
      ID3D_NOTICE( "ID3_TagImpl::ParseReader(): mp3header? cur = " << wr.getCur() );

      if (_mp3_info->Parse(wr, mp3_core_size))
      {
        ID3D_NOTICE( "ID3_TagImpl::ParseReader(): mp3header! cur = " << wr.getCur() );
      }
      else
      {
        delete _mp3_info;
        _mp3_info = NULL;
      }
    }
  }
  else
    this->SetPadding(false); //no need to pad an empty file
}

