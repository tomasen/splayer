// $Id: field_string_ascii.cpp,v 1.29 2003/03/02 14:23:58 t1mpy Exp $

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

#include "field_impl.h"
#include "id3/utils.h" // has <config.h> "id3/id3lib_streams.h" "id3/globals.h" "id3/id3lib_strings.h"
#include "io_helpers.h"

using namespace dami;

/** \fn ID3_Field& ID3_Field::operator=(const char* data)
 ** \brief Shortcut for the Set operator.
 ** \param data The string to assign to this field
 ** \sa Set(const char*)
 **/

/** \brief Copies the supplied string to the field.
 ** You may dispose of the source string after a call to this method.
 ** \code
 **   myFrame.GetField(ID3FN_TEXT)->Set("ID3Lib is very cool!");
 ** \endcode
 **/
size_t ID3_FieldImpl::Set(const char* data)
{
  size_t len = 0;
  if ((this->GetType() == ID3FTY_TEXTSTRING) && data)
  {
    String str(data);
    len = this->SetText_i(str);
  }
  return len;
}

// the ::Get() function for ASCII

/** Copies the contents of the field into the supplied buffer, up to the
 ** number of characters specified; for fields with multiple entries, the
 ** optional third parameter indicates which of the fields to retrieve.
 **
 ** The third parameter is useful when using text lists (see Add(const char*)
 ** for more details).  The default value for this third parameter is 1,
 ** which returns the entire string if the field contains only one item.
 **
 ** It returns the number of characters (not bytes necessarily, and not
 ** including any NULL terminator) of the supplied buffer that are now used.
 **
 ** \code
 **   char myBuffer[1024];
 **   size_t charsUsed = myFrame.GetField(ID3FN_TEXT)->Get(buffer, 1024);
 ** \endcode
 **
 ** It fills the buffer with as much data from the field as is present in the
 ** field, or as large as the buffer, whichever is smaller.
 **
 ** \code
 **   char myBuffer[1024];
 **   size_t charsUsed = myFrame.GetField(ID3FN_TEXT)->Get(buffer, 1024, 3);
 ** \endcode
 **
 ** This fills the buffer with up to the first 1024 characters from the third
 ** element of the text list.
 **
 ** \sa Add(const char*)
 **/
size_t ID3_FieldImpl::Get(char* buffer, size_t maxLength) const
{
  size_t size = 0;
  if (this->GetType() == ID3FTY_TEXTSTRING &&
      this->GetEncoding() == ID3TE_ASCII &&
      buffer != NULL && maxLength > 0)
  {
    String data = this->GetText();
    size = dami::min(maxLength, data.size());
    ::memcpy(buffer, data.data(), size);
    if (size < maxLength)
    {
      buffer[size] = '\0';
    }
  }

  return size;
}

size_t ID3_FieldImpl::Get(char* buf, size_t maxLen, size_t index) const
{
  size_t size = 0;
  if (this->GetType() == ID3FTY_TEXTSTRING &&
      this->GetEncoding() == ID3TE_ASCII &&
      buf != NULL && maxLen > 0)
  {
    String data = this->GetTextItem(index);
    size = dami::min(maxLen, data.size());
    ::memcpy(buf, data.data(), size);
    if (size < maxLen)
    {
      buf[size] = '\0';
    }
  }
  return size;
}

String ID3_FieldImpl::GetText() const
{
  String data;
  if (this->GetType() == ID3FTY_TEXTSTRING)
  {
    data = _text;
  }
  return data;
}

String ID3_FieldImpl::GetTextItem(size_t index) const
{
  String data;
  if (this->GetType() == ID3FTY_TEXTSTRING &&
      this->GetEncoding() == ID3TE_ASCII)
  {
    const char* raw = this->GetRawTextItem(index);
    if (raw != NULL)
    {
      data = raw;
    }
  }
  return data;
}

namespace
{
  String getFixed(String data, size_t size)
  {
    String text(data, 0, size);
    if (text.size() < size)
    {
      text.append(size - text.size(), '\0');
    }
    return text;
  }
}


size_t ID3_FieldImpl::SetText_i(String data)
{
  this->Clear();
  if (_fixed_size > 0)
  {
    _text = getFixed(data, _fixed_size);
  }
  else
  {
    _text = data;
  }
  ID3D_NOTICE( "SetText_i: text = \"" << _text << "\"" );
  _changed = true;

  if (_text.size() == 0)
  {
    _num_items = 0;
  }
  else
  {
    _num_items = 1;
  }

  return _text.size();
}

size_t ID3_FieldImpl::SetText(String data)
{
  size_t len = 0;
  if (this->GetType() == ID3FTY_TEXTSTRING)
  {
    len = this->SetText_i(data);
  }
  return len;
}


/** For fields which support this feature, adds a string to the list of
 ** strings currently in the field.
 **
 ** This is useful for using id3v2 frames such as the involved people list,
 ** composer, and part of setp.  You can use the GetNumTextItems() method to
 ** find out how many such items are in a list.
 **
 ** \code
 **   myFrame.GetField(ID3FN_TEXT)->Add("this is a test");
 ** \endcode
 **
 ** \param string The string to add to the field
 **/
size_t ID3_FieldImpl::AddText_i(String data)
{
  size_t len = 0;  // how much of str we copied into this field (max is strLen)
  ID3D_NOTICE ("ID3_FieldImpl::AddText_i: Adding \"" << data << "\"" );
  if (this->GetNumTextItems() == 0)
  {
    // there aren't any text items in the field so just assign the string to
    // the field
    len = this->SetText_i(data);
  }
  else
  {

    // ASSERT(_fixed_size == 0)
    _text += '\0';
    if (this->GetEncoding() == ID3TE_UNICODE)
    {
      _text += '\0';
    }
    _text.append(data);
    len = data.size();
    _num_items++;
  }

  return len;
}

size_t ID3_FieldImpl::AddText(String data)
{
  size_t len = 0;
  if (this->GetType() == ID3FTY_TEXTSTRING)
  {
    len = this->AddText_i(data);
  }
  return len;
}

size_t ID3_FieldImpl::Add(const char* data)
{
  size_t len = 0;
  if (this->GetType() == ID3FTY_TEXTSTRING)
  {
    String str(data);
    len = this->AddText_i(str);
  }
  return len;
}

const char* ID3_FieldImpl::GetRawText() const
{
  const char* text = NULL;
  if (this->GetType() == ID3FTY_TEXTSTRING &&
      this->GetEncoding() == ID3TE_ASCII)
  {
    text = _text.c_str();
  }
  return text;
}

const char* ID3_FieldImpl::GetRawTextItem(size_t index) const
{
  const char* text = NULL;
  if (this->GetType() == ID3FTY_TEXTSTRING &&
      this->GetEncoding() == ID3TE_ASCII &&
      index < this->GetNumTextItems())
  {
    text = _text.c_str();
    for (size_t i = 0; i < index; ++i)
    {
      text += strlen(text) + 1;
    }
  }
  return text;
}

namespace
{
  String readEncodedText(ID3_Reader& reader, size_t len, ID3_TextEnc enc)
  {
    if (enc == ID3TE_ASCII)
    {
      return io::readText(reader, len);
    }
    return io::readUnicodeText(reader, len);
  }

  String readEncodedString(ID3_Reader& reader, ID3_TextEnc enc)
  {
    if (enc == ID3TE_ASCII)
    {
      return io::readString(reader);
    }
    return io::readUnicodeString(reader);
  }

  size_t writeEncodedText(ID3_Writer& writer, String data, ID3_TextEnc enc)
  {
    if (enc == ID3TE_ASCII)
    {
      return io::writeText(writer, data);
    }
    return io::writeUnicodeText(writer, data);
  }

  size_t writeEncodedString(ID3_Writer& writer, String data, ID3_TextEnc enc)
  {
    if (enc == ID3TE_ASCII)
    {
      return io::writeString(writer, data);
    }
    return io::writeUnicodeString(writer, data);
  }
}

bool ID3_FieldImpl::ParseText(ID3_Reader& reader)
{
  ID3D_NOTICE( "ID3_Field::ParseText(): reader.getBeg() = " << reader.getBeg() );
  ID3D_NOTICE( "ID3_Field::ParseText(): reader.getCur() = " << reader.getCur() );
  ID3D_NOTICE( "ID3_Field::ParseText(): reader.getEnd() = " << reader.getEnd() );
  this->Clear();

  ID3_TextEnc enc = this->GetEncoding();
  size_t fixed_size = this->Size();
  if (fixed_size)
  {
    ID3D_NOTICE( "ID3_Field::ParseText(): fixed size string" );
    // The string is of fixed length
    String text = readEncodedText(reader, fixed_size, enc);
    this->SetText(text);
    ID3D_NOTICE( "ID3_Field::ParseText(): fixed size string = " << text );
  }
  else if (_flags & ID3FF_LIST)
  {
    ID3D_NOTICE( "ID3_Field::ParseText(): text list" );
    // lists are always the last field in a frame.  parse all remaining
    // characters in the reader
    while (!reader.atEnd())
    {
      String text = readEncodedString(reader, enc);
      this->AddText(text);
      ID3D_NOTICE( "ID3_Field::ParseText(): adding string = " << text );
    }
  }
  else if (_flags & ID3FF_CSTR)
  {
    ID3D_NOTICE( "ID3_Field::ParseText(): null terminated string" );
    String text = readEncodedString(reader, enc);
    this->SetText(text);
    ID3D_NOTICE( "ID3_Field::ParseText(): null terminated string = " << text );
  }
  else
  {
    ID3D_NOTICE( "ID3_Field::ParseText(): last field string" );
    String text = readEncodedText(reader, reader.remainingBytes(), enc);
    // not null terminated.
    this->AddText(text);
    ID3D_NOTICE( "ID3_Field::ParseText(): last field string = " << text );
  }

  _changed = false;
  return true;
}

void ID3_FieldImpl::RenderText(ID3_Writer& writer) const
{
  ID3_TextEnc enc = this->GetEncoding();

  if (_flags & ID3FF_CSTR)
  {
    writeEncodedString(writer, _text, enc);
  }
  else
  {
    writeEncodedText(writer, _text, enc);
  }
  _changed = false;
};

/** Returns the number of items in a text list.
 **
 ** \code
 **   size_t numItems = myFrame.GetField(ID3FN_UNICODE)->GetNumItems();
 ** \endcode
 **
 ** \return The number of items in a text list.
 **/
size_t ID3_FieldImpl::GetNumTextItems() const
{
  return _num_items;
}

