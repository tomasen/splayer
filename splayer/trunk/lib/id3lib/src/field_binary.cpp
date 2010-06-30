// $Id: field_binary.cpp,v 1.27 2003/03/02 14:23:59 t1mpy Exp $

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

#include <stdio.h>
//#include <string.h>
#include <memory.h>

#include "field_impl.h"
#include "reader.h"
#include "writer.h"
#include "io_helpers.h"
#include "id3/utils.h" // has <config.h> "id3/id3lib_streams.h" "id3/globals.h" "id3/id3lib_strings.h"

using namespace dami;

size_t ID3_FieldImpl::Set(const uchar* data, size_t len)
{
  size_t size = 0;
  if ((this->GetType() == ID3FTY_BINARY) && data && len)
  {
    BString str(data, len);
    size = dami::min(len, this->SetBinary(str));
  }
  return size;
}

/** Copies the supplied unicode string to the field.
 **
 ** Again, like the string types, the binary Set() function copies the data
 ** so you may dispose of the source data after a call to this method.
 **/
size_t ID3_FieldImpl::SetBinary(BString data) //< data to assign to this field.
{
  size_t size = 0;
  if (this->GetType() == ID3FTY_BINARY)
  {
    this->Clear();
    size_t fixed = _fixed_size;
    size = data.size();
    if (fixed == 0)
    {
      _binary = data;
    }
    else
    {
      _binary.assign(data, 0, dami::min(size, fixed));
      if (size < fixed)
      {
        _binary.append(fixed - size, '\0');
      }
    }
    size = _binary.size();
    _changed = true;
  }
  return size;
}

BString ID3_FieldImpl::GetBinary() const
{
  BString data;
  if (this->GetType() == ID3FTY_BINARY)
  {
    data = _binary;
  }
  return data;
}


const uchar* ID3_FieldImpl::GetRawBinary() const
{
  const uchar* data = NULL;
  if (this->GetType() == ID3FTY_BINARY)
  {
    data = _binary.data();
  }
  return data;
}


/** Copies the field's internal string to the buffer.
 **
 ** It copies the data in the field into the buffer, for as many bytes as the
 ** field contains, or the size of buffer, whichever is smaller.
 **
 ** \code
 **   uchar buffer[1024];
 **   myFrame.GetField(ID3FN_DATA)->Get(buffer, sizeof(buffer));
 ** \endcode
 **/
size_t ID3_FieldImpl::Get(uchar *buffer,    //< Destination of retrieved string
                          size_t max_bytes //< Max number of bytes to copy
                          ) const
{
  size_t bytes = 0;
  if (this->GetType() == ID3FTY_BINARY)
  {
    bytes = dami::min(max_bytes, this->Size());
    if (NULL != buffer && bytes > 0)
    {
      ::memcpy(buffer, _binary.data(), bytes);
    }
  }
  return bytes;
}


/** Copies binary data from the file specified to the field.
 **
 ** \code
 **   myFrame.GetField(ID3FN_DATA)->FromFile("mypic.jpg");
 ** \endcode
 **/
void ID3_FieldImpl::FromFile(const char *info //< Source filename
                             )
{
  if (this->GetType() != ID3FTY_BINARY || NULL == info)
  {
    return;
  }

  FILE* temp_file = ::fopen(info, "rb");
  if (temp_file != NULL)
  {
    ::fseek(temp_file, 0, SEEK_END);
    size_t fileSize = ::ftell(temp_file);
    ::fseek(temp_file, 0, SEEK_SET);

    uchar* buffer = new uchar[fileSize];
    if (buffer != NULL)
    {
      ::fread(buffer, 1, fileSize, temp_file);

      this->Set(buffer, fileSize);

      delete [] buffer;
    }

    ::fclose(temp_file);
  }
}


/** Copies binary data from the field to the specified file.
 **
 ** \code
 **   myFrame.GetField(ID3FN_DATA)->ToFile("output.bin");
 ** \endcode
 **/
void ID3_FieldImpl::ToFile(const char *info //< Destination filename
                           ) const
{
  if (this->GetType() != ID3FTY_BINARY || NULL == info)
  {
    return;
  }

  size_t size = this->Size();
  if (size > 0)
  {
    FILE* temp_file = ::fopen(info, "wb");
    if (temp_file != NULL)
    {
      ::fwrite(_binary.data(), 1, size, temp_file);
      ::fclose(temp_file);
    }
  }

  return ;
}


bool ID3_FieldImpl::ParseBinary(ID3_Reader& reader)
{
  // copy the remaining bytes, unless we're fixed length, in which case copy
  // the minimum of the remaining bytes vs. the fixed length
  _binary = io::readAllBinary(reader);
  return true;
}

void ID3_FieldImpl::RenderBinary(ID3_Writer& writer) const
{
  writer.writeChars(this->GetRawBinary(), this->Size());
}

