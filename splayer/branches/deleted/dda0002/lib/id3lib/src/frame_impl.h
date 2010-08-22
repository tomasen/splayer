// -*- C++ -*-
// $Id: frame_impl.h,v 1.6 2002/08/10 10:50:31 t1mpy Exp $

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

#ifndef _ID3LIB_FRAME_IMPL_H_
#define _ID3LIB_FRAME_IMPL_H_

#include <vector>
#ifndef HAVE_BITSET
#include "id3/id3lib_bitset"
#else
#include <bitset>
#endif
#include "id3/id3lib_frame.h"
#include "header_frame.h"

class ID3_FrameImpl
{
  typedef std::bitset<ID3FN_LASTFIELDID> Bitset;
  typedef std::vector<ID3_Field *> Fields;
public:
  typedef Fields::iterator iterator;
  typedef Fields::const_iterator const_iterator;
public:
  ID3_FrameImpl(ID3_FrameID id = ID3FID_NOFRAME);
  ID3_FrameImpl(const ID3_FrameHeader&);
  ID3_FrameImpl(const ID3_Frame&);

  /// Destructor.
  virtual ~ID3_FrameImpl();
  
  void        Clear();

  bool        SetID(ID3_FrameID id);
  ID3_FrameID GetID() const { return _hdr.GetFrameID(); }
  
  ID3_Field*  GetField(ID3_FieldID name) const;

  size_t      NumFields() const;
  
  const char* GetDescription() const;
  static const char* GetDescription(ID3_FrameID);

  const char* GetTextID() const { return _hdr.GetTextID(); }

  ID3_FrameImpl&  operator=(const ID3_Frame &);
  bool        HasChanged() const;
  bool        Parse(ID3_Reader&);
  void        Render(ID3_Writer&) const;
  size_t      Size();
  bool        Contains(ID3_FieldID fld) const
  { return _bitset.test(fld); }
  bool        SetSpec(ID3_V2Spec);
  ID3_V2Spec  GetSpec() const;

  /** Sets the compression flag within the frame.  When the compression flag is
   ** is set, compression will be attempted.  However, the frame might not
   ** actually be compressed after it is rendered if the "compressed" data is
   ** no smaller than the "uncompressed" data.
   **/
  bool        SetCompression(bool b)  { return _hdr.SetCompression(b); }
  /** Returns whether or not the compression flag is set.  After parsing a tag,
   ** this will indicate whether or not the frame was compressed.  After
   ** rendering a tag, however, it does not actually indicate if the frame is
   ** compressed rendering.  It only indicates whether or not compression was
   ** attempted.  A frame will not be compressed, even whent the compression
   ** flag is set, if the "compressed" data is no smaller than the
   ** "uncompressed" data.
   **/
  bool        GetCompression() const  { return _hdr.GetCompression(); }
  size_t      GetDataSize() const { return _hdr.GetDataSize(); }

  bool SetEncryptionID(uchar id)
  {
    bool changed = id != _encryption_id;
    _encryption_id = id;
    _changed = _changed || changed;
    _hdr.SetEncryption(true);
    return changed;
  }
  uchar GetEncryptionID() const { return _encryption_id; }
  bool SetGroupingID(uchar id)
  {
    bool changed = id != _grouping_id;
    _grouping_id = id;
    _changed = _changed || changed;
    _hdr.SetGrouping(true);
    return changed;
  }
  uchar GetGroupingID() const { return _grouping_id; }

  iterator         begin()       { return _fields.begin(); }
  iterator         end()         { return _fields.end(); }
  const_iterator   begin() const { return _fields.begin(); }
  const_iterator   end()   const { return _fields.end(); }
  
protected:
  bool        _SetID(ID3_FrameID);
  bool        _ClearFields();
  void        _InitFields();
  void        _InitFieldBits();
  void        _UpdateFieldDeps();

private:
  mutable bool        _changed;    // frame changed since last parse/render?
  Bitset      _bitset;             // which fields are present?
  Fields      _fields;
  ID3_FrameHeader _hdr;            // 
  uchar       _encryption_id;      // encryption id
  uchar       _grouping_id;        // grouping id
}
;

#endif /* _ID3LIB_FRAME_IMPL_H_ */
