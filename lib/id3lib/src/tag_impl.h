// -*- C++ -*-
// $Id: tag_impl.h,v 1.10 2002/11/02 17:35:56 t1mpy Exp $

// id3lib: a software library for creating and manipulating id3v1/v2 tags
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

#ifndef _ID3LIB_TAG_IMPL_H_
#define _ID3LIB_TAG_IMPL_H_

#include <list>
#include <stdio.h>
#include "tag.h" // has frame.h, field.h
#include "header_tag.h"
#include "mp3_header.h" //has io_decorators.h

class ID3_Reader;
class ID3_Writer;

namespace dami
{
  namespace id3
  {
    namespace v1
    {
      bool parse(ID3_TagImpl&, ID3_Reader&);
      void render(ID3_Writer&, const ID3_TagImpl&);
    };
    namespace v2
    {
      bool parse(ID3_TagImpl& tag, ID3_Reader& rdr);
      void render(ID3_Writer& writer, const ID3_TagImpl& tag);
    };
  };
  namespace lyr3
  {
    namespace v1
    {
      bool parse(ID3_TagImpl&, ID3_Reader&);
    };
    namespace v2
    {
      bool parse(ID3_TagImpl&, ID3_Reader&);
    };
  };
  namespace mm
  {
    bool parse(ID3_TagImpl&, ID3_Reader&);
  };
};

class ID3_TagImpl
{
  typedef std::list<ID3_Frame *> Frames;
public:
  typedef Frames::iterator       iterator;
  typedef Frames::const_iterator const_iterator;
public:
  ID3_TagImpl(const char *name = NULL);
  ID3_TagImpl(const ID3_Tag &tag);
  virtual ~ID3_TagImpl();

  void       Clear();
  bool       HasChanged() const;
  void       SetChanged(bool b) { _changed = b; }
  size_t     Size() const;

  bool       SetUnsync(bool);
  bool       SetExtended(bool);
  bool       SetExperimental(bool);
  bool       SetPadding(bool);

  bool       GetUnsync() const;
  bool       GetExtended() const;
  bool       GetExperimental() const;
  bool       GetFooter() const;

  size_t     GetExtendedBytes() const;

  void       AddFrame(const ID3_Frame&);
  void       AddFrame(const ID3_Frame*);
  bool       AttachFrame(ID3_Frame*);
  ID3_Frame* RemoveFrame(const ID3_Frame *);

  size_t     Link(const char *fileInfo, flags_t = (flags_t) ID3TT_ALL);
  size_t     Link(ID3_Reader &reader, flags_t = (flags_t) ID3TT_ALL);
  flags_t    Update(flags_t = (flags_t) ID3TT_ALL);
  flags_t    Strip(flags_t = (flags_t) ID3TT_ALL);

  size_t     GetPrependedBytes() const { return _prepended_bytes; }
  size_t     GetAppendedBytes() const { return _appended_bytes; }
  size_t     GetFileSize() const { return _file_size; }
  dami::String GetFileName() const { return _file_name; }

  ID3_Frame* Find(ID3_FrameID id) const;
  ID3_Frame* Find(ID3_FrameID id, ID3_FieldID fld, uint32 data) const;
  ID3_Frame* Find(ID3_FrameID id, ID3_FieldID fld, dami::String) const;
  ID3_Frame* Find(ID3_FrameID id, ID3_FieldID fld, dami::WString) const;

  size_t     NumFrames() const { return _frames.size(); }
  ID3_TagImpl&   operator=( const ID3_Tag & );

  bool       HasTagType(ID3_TagType tt) const { return _file_tags.test(tt); }
  ID3_V2Spec GetSpec() const;
  bool       SetSpec(ID3_V2Spec);

  static size_t IsV2Tag(ID3_Reader&);

  const Mp3_Headerinfo* GetMp3HeaderInfo() const { if (_mp3_info) return _mp3_info->GetMp3HeaderInfo(); else return NULL; }

  iterator         begin()       { return _frames.begin(); }
  iterator         end()         { return _frames.end(); }
  const_iterator   begin() const { return _frames.begin(); }
  const_iterator   end()   const { return _frames.end(); }

  /* Deprecated! */
  void       AddNewFrame(ID3_Frame* f) { this->AttachFrame(f); }
  size_t     Link(const char *fileInfo, bool parseID3v1, bool parseLyrics3);
  void       SetCompression(bool) { ; }
  void       AddFrames(const ID3_Frame *, size_t);
  bool       HasLyrics() const { return this->HasTagType(ID3TT_LYRICS); }
  bool       HasV2Tag()  const { return this->HasTagType(ID3TT_ID3V2); }
  bool       HasV1Tag()  const { return this->HasTagType(ID3TT_ID3V1); }
  size_t     PaddingSize(size_t) const;

protected:
  const_iterator Find(const ID3_Frame *) const;
  iterator Find(const ID3_Frame *);

  void       RenderExtHeader(uchar *);

  void       ParseFile();
  void       ParseReader(ID3_Reader &reader);

private:
  ID3_TagHeader _hdr;          // information relevant to the tag header
  bool       _is_padded;       // add padding to tags?

  Frames     _frames;

  mutable const_iterator   _cursor;  // which frame in list are we at
  mutable bool       _changed; // has tag changed since last parse or render?

  // file-related member variables
  dami::String _file_name;       // name of the file we are linked to
  size_t     _file_size;       // the size of the file (without any tag(s))
  size_t     _prepended_bytes; // number of tag bytes at start of file
  size_t     _appended_bytes;  // number of tag bytes at end of file
  bool       _is_file_writable;// is the associated file (via Link) writable?
  ID3_Flags  _tags_to_parse;   // which tag types should attempt to be parsed
  ID3_Flags  _file_tags;       // which tag types does the file contain
  Mp3Info    *_mp3_info;   // class used to retrieve _mp3_header
};

size_t     ID3_GetDataSize(const ID3_TagImpl&);

#endif /* _ID3LIB_TAG_IMPL_H_ */

