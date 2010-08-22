// $Id: field_string_unicode.cpp,v 1.33 2003/03/02 14:23:58 t1mpy Exp $

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

/** \fn ID3_Field& ID3_Field::operator=(const unicode_t*)
 ** \brief Shortcut for the Set operator.
 ** Performs similarly as operator=(const char*), taking a unicode_t
 ** string as a parameter rather than an ascii string.
 ** \sa Set(const unicode_t*)
 ** \param string The string to assign to the field
 **/

/** \brief Copies the supplied unicode string to the field.
 **
 ** Performs similarly as the ASCII Set() method, taking a unicode_t string
 ** as a parameter rather than an ascii string.
 **
 ** \param string The unicode string to set this field to.
 ** \sa Add(const unicode_t*)
 **/
size_t ID3_FieldImpl::Set(const unicode_t* data)
{
  size_t size = 0;
  if (this->GetType() == ID3FTY_TEXTSTRING &&
      this->GetEncoding() == ID3TE_UNICODE && data)
  {
    String text((const char*) data, ucslen(data) * 2);
    size = this->SetText_i(text);
  }
  return size;
}

size_t ID3_FieldImpl::Add(const unicode_t* data)
{
  size_t size = 0;
  if (this->GetType() == ID3FTY_TEXTSTRING &&
      this->GetEncoding() == ID3TE_UNICODE)
  {
    String text((const char*) data, ucslen(data) * 2);
    size = this->AddText_i(text);
  }
  return size;
}

/** Copies the contents of the field into the supplied buffer, up to the
 ** number of characters specified; for fields with multiple entries, the
 ** optional third parameter indicates which of the fields to retrieve.
 **
 ** Performs similarly as the ASCII Get(char *, size_t, size_t) method, taking
 ** a unicode_t string as a parameter rather than an ascii string.  The
 ** maxChars parameter still represents the maximum number of characters, not
 ** bytes.
 **
 ** \code
 **   unicode_t myBuffer[1024];
 **   size_t charsUsed = myFrame.GetField(ID3FN_UNICODE)->Get(buffer, 1024);
 ** \endcode
 **
 ** \param buffer   Where the field's data is copied to
 ** \param maxChars The maximum number of characters to copy to the buffer.
 ** \param itemNum  For fields with multiple items (such as the involved
 **                 people frame, the item number to retrieve.
 ** \sa Get(char *, size_t, size_t)
 **/
size_t ID3_FieldImpl::Get(unicode_t *buffer, size_t maxLength) const
{
  size_t length = 0;
  if (this->GetType() == ID3FTY_TEXTSTRING &&
      this->GetEncoding() == ID3TE_UNICODE &&
      buffer != NULL && maxLength > 0)
  {
    size_t size = this->Size();
    length = dami::min(maxLength, size);
    ::memcpy((void *)buffer, (void *)_text.data(), length * 2);
    if (length < maxLength)
    {
      buffer[length] = NULL_UNICODE;
    }
  }
  return length;
}

const unicode_t* ID3_FieldImpl::GetRawUnicodeText() const
{
  const unicode_t* text = NULL;
  if (this->GetType() == ID3FTY_TEXTSTRING &&
      this->GetEncoding() == ID3TE_UNICODE)
  {
    text = (unicode_t *)_text.data();
  }
  return text;
}

const unicode_t* ID3_FieldImpl::GetRawUnicodeTextItem(size_t index) const
{
  const unicode_t* text = NULL;
  if (this->GetType() == ID3FTY_TEXTSTRING &&
      this->GetEncoding() == ID3TE_UNICODE &&
      index < this->GetNumTextItems())
  {
    String unicode = _text + '\0' + '\0';
    text = (unicode_t *) unicode.data();
    for (size_t i = 0; i < index; ++i)
    {
      text += ucslen(text) + 1;
    }
  }
  return text;
}

size_t ID3_FieldImpl::Get(unicode_t *buffer, size_t maxLength, size_t itemNum) const
{
  size_t length = 0;
  size_t total_items = this->GetNumTextItems();
  if (this->GetType() == ID3FTY_TEXTSTRING &&
      this->GetEncoding() == ID3TE_UNICODE &&
      buffer != NULL && maxLength > 0 && itemNum < total_items)
  {
    const unicode_t* text = this->GetRawUnicodeTextItem(itemNum);
    if (NULL != text)
    {
      size_t length = dami::min(maxLength, ucslen(text));
      ::memcpy(buffer, text, length * 2);
      if (length < maxLength)
      {
        buffer[length] = NULL_UNICODE;
      }
    }
  }

  return length;
}


