// $Id: frame_parse.cpp,v 1.34 2002/07/06 13:53:18 t1mpy Exp $

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

#include "frame_impl.h"
#include "id3/io_decorators.h" //has "readers.h" "io_helpers.h" "utils.h"

using namespace dami;

namespace
{
  bool parseFields(ID3_Reader& rdr, ID3_FrameImpl& frame)
  {
    int iLoop;
    int iFields;
    io::ExitTrigger et(rdr);
    ID3_TextEnc enc = ID3TE_ASCII;  // set the default encoding 
    ID3_V2Spec spec = frame.GetSpec(); 
    // parse the frame's fields  
    iFields = frame.NumFields();
    ID3D_NOTICE( "ID3_FrameImpl::Parse(): num_fields = " << iFields );
    iLoop = 0;
    for (ID3_FrameImpl::iterator fi = frame.begin(); fi != frame.end(); ++fi)
    {
      ID3_Field* fp = *fi;
      ++iLoop;

      if (rdr.atEnd())
      { 
        // there's no remaining data to parse! 
        ID3D_WARNING( "ID3_FrameImpl::Parse(): out of data at postion " <<
                      rdr.getCur() );
        if (iLoop == iFields)
        {
          //if we are at the last field, (the 'data' field) it's apparently
          //an empty tag used for filling up padding, it's no problem
          //break will set the current 'cursor' to the right spot outside the for loop
          break;
        }
        return false;
      } 
      
      if (NULL == fp)
      {
        // Ack!  Why is the field NULL?  Log this...
        ID3D_WARNING( "ID3_FrameImpl::Parse(): field is null" );
        continue;
      }
      
      if (!fp->InScope(spec)) 
      {
        ID3D_NOTICE( "ID3_FrameImpl::Parse(): field is not in scope" );
        // continue with the rest of the fields
        continue; 
      }
      
      ID3D_NOTICE( "ID3_FrameImpl::Parse(): setting enc to " << enc );
      fp->SetEncoding(enc);
      ID3_Reader::pos_type beg = rdr.getCur();
      et.setExitPos(beg);
      ID3D_NOTICE( "ID3_FrameImpl::Parse(): parsing field, cur = " << beg );
      ID3D_NOTICE( "ID3_FrameImpl::Parse(): parsing field, end = " << 
                   rdr.getEnd() );
      if (!fp->Parse(rdr) || rdr.getCur() == beg) 
      { 
        // nothing to parse!  ack!  parse error... 
        ID3D_WARNING( "ID3_FrameImpl::Parse(): no data parsed, bad parse" );
        return false;
      }
      
      if (fp->GetID() == ID3FN_TEXTENC)  
      {
        enc = static_cast<ID3_TextEnc>(fp->Get());  
        ID3D_NOTICE( "ID3_FrameImpl::Parse(): found encoding = " << enc );
      }
    }
    et.setExitPos(rdr.getCur());
    
    return true;
  }
};

bool ID3_FrameImpl::Parse(ID3_Reader& reader) 
{ 
  io::ExitTrigger et(reader);
  ID3D_NOTICE( "ID3_FrameImpl::Parse(): reader.getBeg() = " << reader.getBeg() );
  ID3D_NOTICE( "ID3_FrameImpl::Parse(): reader.getCur() = " << reader.getCur() );
  ID3D_NOTICE( "ID3_FrameImpl::Parse(): reader.getEnd() = " << reader.getEnd() );
  ID3_Reader::pos_type beg = reader.getCur();

  if (!_hdr.Parse(reader) || reader.getCur() == beg)  
  { 
    ID3D_WARNING( "ID3_FrameImpl::Parse(): no header to parse" );
    return false; 
  }
  ID3D_NOTICE( "ID3_FrameImpl::Parse(): after hdr, getCur() = " << reader.getCur() );
  ID3D_NOTICE( "ID3_FrameImpl::Parse(): found frame! id = " << _hdr.GetTextID() );

  // data is the part of the frame buffer that appears after the header  
  const size_t dataSize = _hdr.GetDataSize();
  ID3D_NOTICE( "ID3_FrameImpl::Parse(): dataSize = " << dataSize );
  if (reader.getEnd() < beg + dataSize)
  {
    ID3D_WARNING( "ID3_FrameImpl::Parse(): not enough data to parse frame" );
    return false;
  }
  io::WindowedReader wr(reader, dataSize);
  ID3D_NOTICE( "ID3_FrameImpl::Parse(): window getBeg() = " << wr.getBeg() );
  ID3D_NOTICE( "ID3_FrameImpl::Parse(): window getCur() = " << wr.getCur() );
  ID3D_NOTICE( "ID3_FrameImpl::Parse(): window getEnd() = " << wr.getEnd() );
  
  unsigned long origSize = 0;
  if (_hdr.GetCompression())
  {
    origSize = io::readBENumber(reader, sizeof(uint32));
    ID3D_NOTICE( "ID3_FrameImpl::Parse(): frame is compressed, origSize = " << origSize );
  }

  if (_hdr.GetEncryption())
  {
    char ch = wr.readChar();
    this->SetEncryptionID(ch);
    ID3D_NOTICE( "ID3_FrameImpl::Parse(): frame is encrypted, encryption_id = " << (int) ch );
  }

  if (_hdr.GetGrouping())
  {
    char ch = wr.readChar();
    this->SetGroupingID(ch);
    ID3D_NOTICE( "ID3_FrameImpl::Parse(): frame is encrypted, grouping_id = " << (int) ch );
  }

  // set the type of frame based on the parsed header  
  this->_ClearFields(); 
  this->_InitFields(); 

  bool success = false;
  // expand out the data if it's compressed 
  if (!_hdr.GetCompression())
  {
    success = parseFields(wr, *this);
  }
  else
  {
    io::CompressedReader csr(wr, origSize);
    success = parseFields(csr, *this);
  }
  et.setExitPos(wr.getCur());

  _changed = false;
  return true;
} 

