// $Id: header_frame.cpp,v 1.22 2002/07/02 22:13:10 t1mpy Exp $

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


#include <memory.h>
#include "header_frame.h"
#include "id3/utils.h" // has <config.h> "id3/id3lib_streams.h" "id3/globals.h" "id3/id3lib_strings.h"
#include "frame_def.h"
#include "field_def.h"
#include "field_impl.h"
#include "io_helpers.h"

using namespace dami;

void ID3_FrameHeader::SetUnknownFrame(const char* id)
{
  Clear();
  _frame_def = new ID3_FrameDef;
  if (NULL == _frame_def)
  {
    // log this;
    return;
  }
  _frame_def->eID = ID3FID_NOFRAME;
  _frame_def->bTagDiscard = false;
  _frame_def->bFileDiscard = false;
  _frame_def->aeFieldDefs = ID3_FieldDef::DEFAULT;
  _frame_def->sDescription = NULL;
  if (strlen(id) <= 3)
  {
    strcpy(_frame_def->sShortTextID, id);
    strcpy(_frame_def->sLongTextID, "");
  }
  else
  {
    strcpy(_frame_def->sLongTextID, id);
    strcpy(_frame_def->sShortTextID, "");
  }
  _dyn_frame_def = true;
}

bool ID3_FrameHeader::SetFrameID(ID3_FrameID id)
{
  if (id == ID3FID_NOFRAME || id == this->GetFrameID())
  {
    return false;
  }
  _frame_def = ID3_FindFrameDef(id);
  _flags.set(TAGALTER, _frame_def->bTagDiscard);
  _flags.set(FILEALTER, _frame_def->bFileDiscard);

  _changed = true;
  return true;
}

size_t ID3_FrameHeader::Size() const
{
  if (!_info)
  {
    return 0;
  }
  return 
    _info->frame_bytes_id   + 
    _info->frame_bytes_size + 
    _info->frame_bytes_flags;
}

bool ID3_FrameHeader::Parse(ID3_Reader& reader)
{
  ID3D_NOTICE( "ID3_FrameHeader::Parse(): getCur() = " << reader.getCur() );
  io::ExitTrigger et(reader);
  if (!_info)
  {
    return false;
  }
  if (reader.getEnd() < reader.getCur() + 10)
  {
    return false;
  }

  String textID = io::readText(reader, _info->frame_bytes_id);

  ID3D_NOTICE( "ID3_FrameHeader::Parse: textID = " << textID );
  ID3D_NOTICE( "ID3_FrameHeader::Parse: getCur() = " << reader.getCur() );

  ID3_FrameID fid = ID3_FindFrameID(textID.c_str());
  if (ID3FID_NOFRAME == fid)
  {
    this->SetUnknownFrame(textID.c_str());
    ID3D_NOTICE( "ID3_FrameHeader::Parse: unknown frame id" );
  }
  else
  {
    this->SetFrameID(fid);
  }

  uint32 dataSize = io::readBENumber(reader, _info->frame_bytes_size);
  ID3D_NOTICE( "ID3_FrameHeader::Parse: dataSize = " << dataSize );
  ID3D_NOTICE( "ID3_FrameHeader::Parse: getCur() = " << reader.getCur() );
  this->SetDataSize(dataSize);

  uint32 flags = io::readBENumber(reader, _info->frame_bytes_flags);
  _flags.add(flags);

  ID3D_NOTICE( "ID3_FrameHeader::Parse: flags = " << flags );
  ID3D_NOTICE( "ID3_FrameHeader::Parse: getCur() = " << reader.getCur() );
  et.setExitPos(reader.getCur());

  return true;
}

void ID3_FrameHeader::Render(ID3_Writer& writer) const
{
  size_t size = 0;

  if (NULL == _frame_def)
  {
    // TODO: log this
    ID3D_WARNING( "ID3_FrameHeader::Render(): _frame_def is NULL!" );
    return;
    //ID3_THROW(ID3E_InvalidFrameID);
  }
  char *textID;
  if (_info->frame_bytes_id == strlen(_frame_def->sShortTextID))
  {
    textID = _frame_def->sShortTextID;
  }
  else
  {
    textID = _frame_def->sLongTextID;
  }

  ID3D_NOTICE( "ID3_FrameHeader::Render(): writing " << textID << ", " << (int) _info->frame_bytes_size << " bytes");
  writer.writeChars((uchar *) textID, _info->frame_bytes_id);

  io::writeBENumber(writer, _data_size, _info->frame_bytes_size);
  io::writeBENumber(writer, _flags.get(), _info->frame_bytes_flags);
}

const char* ID3_FrameHeader::GetTextID() const
{
  char *textID = "";
  if (_info && _frame_def)
  {
    if (_info->frame_bytes_id == strlen(_frame_def->sShortTextID))
    {
      textID = _frame_def->sShortTextID;
    }
    else
    {
      textID = _frame_def->sLongTextID;
    }
  }
  return textID;
}

ID3_FrameHeader& ID3_FrameHeader::operator=(const ID3_FrameHeader& hdr)
{
  if (this != &hdr)
  {
    this->Clear();
    this->ID3_Header::operator=(hdr);
    if (!hdr._dyn_frame_def)
    {
      _frame_def = hdr._frame_def;
    }
    else
    {
      _frame_def = new ID3_FrameDef;
      if (NULL == _frame_def)
      {
        // TODO: throw something here...
      }
      _frame_def->eID = hdr._frame_def->eID;
      _frame_def->bTagDiscard = hdr._frame_def->bTagDiscard;
      _frame_def->bFileDiscard = hdr._frame_def->bFileDiscard;
      _frame_def->aeFieldDefs = hdr._frame_def->aeFieldDefs;
      strcpy(_frame_def->sShortTextID, hdr._frame_def->sShortTextID);
      strcpy(_frame_def->sLongTextID, hdr._frame_def->sLongTextID);
      _dyn_frame_def = true;
    }
  }
  return *this;
}

ID3_FrameID ID3_FrameHeader::GetFrameID() const
{
  ID3_FrameID eID = ID3FID_NOFRAME;
  if (NULL != _frame_def)
  {
    eID = _frame_def->eID;
  }

  return eID;
}

const ID3_FrameDef *ID3_FrameHeader::GetFrameDef() const
{
  return _frame_def;
}

bool ID3_FrameHeader::Clear()
{
  bool changed = this->ID3_Header::Clear();
  if (_dyn_frame_def)
  {
    delete _frame_def;
    _dyn_frame_def = false;
    changed = true;
  }
  if (_frame_def)
  {
    _frame_def = NULL;
    changed = true;
  }
  return changed;
}

