// $Id: frame.cpp,v 1.35 2002/08/10 10:42:42 t1mpy Exp $

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

#if defined HAVE_CONFIG_H
#include <config.h>
#endif

//#include "frame.h"
#include "readers.h"
#include "frame_impl.h"

/** \class ID3_Frame frame.h id3/frame.h
 ** \brief The representative class of an id3v2 frame.
 ** 
 ** id3lib defines frames in a funny way.  Using some nice c++ conventions,
 ** ID3_FrameImpl class objects appear to be quite polymorphic; that is, they
 ** can take on many forms.  The same ID3_FrameImpl class provides the
 ** facilities for the implementation of a complex APIC frame and for a simple
 ** text frame.
 ** 
 ** @author Dirk Mahoney
 ** @version $Id: frame.cpp,v 1.35 2002/08/10 10:42:42 t1mpy Exp $
 ** @see ID3_Tag
 ** @see ID3_Field
 ** @see ID3_Err
 **/

/** Default constructor; accepts as a default parameter the type of frame
 ** to create.
 ** 
 ** The parameter which will internally set the frame's structure.  See
 ** SetID() for more details.
 **     
 ** @param id The type of frame to create
 ** @see ID3_FrameID
 ** @see SetID
 **/
ID3_Frame::ID3_Frame(ID3_FrameID id)
  : _impl(new ID3_FrameImpl(id))
{
}

ID3_Frame::ID3_Frame(const ID3_Frame& frame)
  : _impl(new ID3_FrameImpl(frame))
{
}

ID3_Frame::~ID3_Frame()
{
  delete _impl;
}

/** Clears the frame of all data and resets the frame such that it can take
 ** on the form of any id3v2 frame that id3lib supports.
 ** 
 ** @see ID3_Tag::Clear
 **/
void ID3_Frame::Clear()
{
  _impl->Clear();
}

/** Returns the type of frame that the object represents.
 ** 
 ** Useful in conjunction with ID3_Tag::Find() method
 ** 
 ** @returns The type, or id, of the frame
 ** @see ID3_Tag::Find
 **/
ID3_FrameID ID3_Frame::GetID() const
{
  return _impl->GetID();
}

/** Establishes the internal structure of an ID3_FrameImpl object so
 ** that it represents the id3v2 frame indicated by the parameter
 ** 
 ** Given an ID3_FrameID (a list of which is found in &lt;id3/field.h&gt;),
 ** SetID() will structure the object according to the
 ** frame you wish to implement.
 ** 
 ** Either using this call or via the constructor, this must be the first
 ** command performed on an ID3_FrameImpl object.  
 ** 
 ** \code
 **   myFrame.SetID(ID3FID_TITLE);
 ** \endcode
 ** 
 ** @param id The type of frame this frame should be set to
 ** @see ID3_FrameID
 **/
bool ID3_Frame::SetID(ID3_FrameID id)
{
  return _impl->SetID(id);
}

bool ID3_Frame::SetSpec(ID3_V2Spec spec)
{
  return _impl->SetSpec(spec);
}

ID3_V2Spec ID3_Frame::GetSpec() const
{
  return _impl->GetSpec();
}

/** Returns a pointer to the frame's internal field indicated by the
 ** parameter.
 **
 ** \code
 **   ID3_TextEnc enc;
 **   enc = (ID3_TextEnc) myFrame.GetField(ID3FN_TEXTENC)->Get();
 ** \endcode
 ** 
 ** @param name The name of the field to be retrieved
 ** @returns A reference to the desired field
 **/
ID3_Field& ID3_Frame::Field(ID3_FieldID fieldName) const
{
  return *this->GetField(fieldName);
}

ID3_Field* ID3_Frame::GetField(ID3_FieldID fieldName) const
{
  return _impl->GetField(fieldName);
}

size_t ID3_Frame::NumFields() const
{
  return _impl->NumFields();
}

/*
ID3_Field* ID3_Frame::GetFieldNum(size_t index) const
{
  return _impl->GetFieldNum(index);
}
*/

size_t ID3_Frame::Size()
{
  return _impl->Size();
}


bool ID3_Frame::HasChanged() const
{
  return _impl->HasChanged();
}

ID3_Frame& ID3_Frame::operator=( const ID3_Frame &rFrame )
{
  if (this != &rFrame)
  {
    *_impl = rFrame;
  }
  return *this;
}

const char* ID3_Frame::GetDescription(ID3_FrameID id)
{
  return ID3_FrameImpl::GetDescription(id);
}

const char* ID3_Frame::GetDescription() const
{
  return _impl->GetDescription();
}

const char* ID3_Frame::GetTextID() const
{
  return _impl->GetTextID();
}

bool ID3_Frame::Parse(ID3_Reader& reader) 
{
  return _impl->Parse(reader);
}

void ID3_Frame::Render(ID3_Writer& writer) const
{
  _impl->Render(writer);
}

bool ID3_Frame::Contains(ID3_FieldID id) const
{
  return _impl->Contains(id);
}

/** Sets the compression flag within the frame.  When the compression flag is
 ** is set, compression will be attempted.  However, the frame might not
 ** actually be compressed after it is rendered if the "compressed" data is
 ** no smaller than the "uncompressed" data.
 **/
bool ID3_Frame::SetCompression(bool b)
{
  return _impl->SetCompression(b);
}

/** Returns whether or not the compression flag is set.  After parsing a tag,
 ** this will indicate whether or not the frame was compressed.  After
 ** rendering a tag, however, it does not actually indicate if the frame is
 ** compressed rendering.  It only indicates whether or not compression was
 ** attempted.  A frame will not be compressed, even whent the compression
 ** flag is set, if the "compressed" data is no smaller than the
 ** "uncompressed" data.
 **/
bool ID3_Frame::GetCompression() const
{
  return _impl->GetCompression();
}

size_t ID3_Frame::GetDataSize() const
{
  return _impl->GetDataSize();
}

bool ID3_Frame::SetEncryptionID(uchar id)
{
  return _impl->SetEncryptionID(id);
}

uchar ID3_Frame::GetEncryptionID() const
{
  return _impl->GetEncryptionID();
}

bool ID3_Frame::SetGroupingID(uchar id)
{
  return _impl->SetGroupingID(id);
}

uchar ID3_Frame::GetGroupingID() const
{
  return _impl->GetGroupingID();
}

namespace
{
  class IteratorImpl : public ID3_Frame::Iterator
  {
    ID3_FrameImpl::iterator _cur;
    ID3_FrameImpl::iterator _end;
  public:
    IteratorImpl(ID3_FrameImpl& frame)
      : _cur(frame.begin()), _end(frame.end())
    {
    }

    ID3_Field* GetNext() 
    { 
      ID3_Field* next = NULL;
      while (next == NULL && _cur != _end)
      {
        next = *_cur;
        ++_cur;
      }
      return next;
    }
  };

  
  class ConstIteratorImpl : public ID3_Frame::ConstIterator
  {
    ID3_FrameImpl::const_iterator _cur;
    ID3_FrameImpl::const_iterator _end;
  public:
    ConstIteratorImpl(ID3_FrameImpl& frame)
      : _cur(frame.begin()), _end(frame.end())
    {
    }
    const ID3_Field* GetNext() 
    { 
      ID3_Field* next = NULL;
      while (next == NULL && _cur != _end)
      {
        next = *_cur;
        ++_cur;
      }
      return next;
    }
  };
}

ID3_Frame::Iterator* 
ID3_Frame::CreateIterator()
{
  return new IteratorImpl(*_impl);
}

ID3_Frame::ConstIterator* 
ID3_Frame::CreateIterator() const
{
  return new ConstIteratorImpl(*_impl);
}

