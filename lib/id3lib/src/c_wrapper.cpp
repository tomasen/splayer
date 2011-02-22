// $Id: c_wrapper.cpp,v 1.22 2002/09/21 17:23:32 t1mpy Exp $

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

//#include <string.h>
#include "id3.h"
#include "tag.h"
#include "field.h"

#if defined HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

  // tag wrappers

#define ID3_CATCH(code) try { code; } catch (...) { }

  ID3_C_EXPORT ID3Tag* CCONV
  ID3Tag_New(void)
  {
    ID3_Tag* tag = NULL;
    ID3_CATCH(tag = new ID3_Tag);
    return reinterpret_cast<ID3Tag *>(tag);
  }


  ID3_C_EXPORT void CCONV
  ID3Tag_Delete(ID3Tag *tag)
  {
    if (tag)
    {
      ID3_CATCH(delete reinterpret_cast<ID3_Tag*>(tag));
    }
  }


  ID3_C_EXPORT void CCONV
  ID3Tag_Clear(ID3Tag *tag)
  {
    if (tag)
    {
      ID3_CATCH(reinterpret_cast<ID3_Tag*>(tag)->Clear());
    }
  }


  ID3_C_EXPORT bool CCONV
  ID3Tag_HasChanged(const ID3Tag *tag)
  {
    bool changed = false;

    if (tag)
    {
      ID3_CATCH(changed = reinterpret_cast<const ID3_Tag * >(tag)->HasChanged());
    }

    return changed;
  }


  ID3_C_EXPORT void CCONV
  ID3Tag_SetUnsync(ID3Tag *tag, bool unsync)
  {
    if (tag)
    {
      ID3_CATCH(reinterpret_cast<ID3_Tag *>(tag)->SetUnsync(unsync));
    }
  }


  ID3_C_EXPORT void CCONV
  ID3Tag_SetExtendedHeader(ID3Tag *tag, bool ext)
  {
    if (tag)
    {
      ID3_CATCH(reinterpret_cast<ID3_Tag *>(tag)->SetExtendedHeader(ext));
    }
  }

  ID3_C_EXPORT void CCONV
  ID3Tag_SetPadding(ID3Tag *tag, bool pad)
  {
    if (tag)
    {
      ID3_CATCH(reinterpret_cast<ID3_Tag *>(tag)->SetPadding(pad));
    }
  }


  ID3_C_EXPORT void CCONV
  ID3Tag_AddFrame(ID3Tag *tag, const ID3Frame *frame)
  {
    if (tag)
    {
      ID3_CATCH(reinterpret_cast<ID3_Tag *>(tag)->AddFrame(reinterpret_cast<const ID3_Frame *>(frame)));
    }
  }


  ID3_C_EXPORT bool CCONV
  ID3Tag_AttachFrame(ID3Tag *tag, ID3Frame *frame)
  {
    bool b = false;
    if (tag)
    {
      ID3_CATCH(b = reinterpret_cast<ID3_Tag *>(tag)->AttachFrame(reinterpret_cast<ID3_Frame *>(frame)));
    }
    return b;
  }


  ID3_C_EXPORT void CCONV
  ID3Tag_AddFrames(ID3Tag *tag, const ID3Frame *frames, size_t num)
  {
    if (tag)
    {
      ID3_CATCH(reinterpret_cast<ID3_Tag *>(tag)->AddFrames(reinterpret_cast<const ID3_Frame *>(frames), num));
    }
  }


  ID3_C_EXPORT ID3Frame* CCONV
  ID3Tag_RemoveFrame(ID3Tag *tag, const ID3Frame *frame)
  {
    ID3_Frame* rem_frame = NULL;
    if (tag)
    {
      ID3_CATCH(rem_frame = reinterpret_cast<ID3_Tag *>(tag)->RemoveFrame(reinterpret_cast<const ID3_Frame *>(frame)));
    }
    return reinterpret_cast<ID3Frame*>(rem_frame);
  }


  ID3_C_EXPORT ID3_Err CCONV
  ID3Tag_Parse(ID3Tag *tag, const uchar header[ ID3_TAGHEADERSIZE ],
               const uchar *buffer)
  {
    size_t size = 0;
    if (tag)
    {
      ID3_CATCH(size = reinterpret_cast<ID3_Tag *>(tag)->Parse(header, buffer));
    }
    return ID3E_NoError;
  }


  ID3_C_EXPORT size_t CCONV
  ID3Tag_Link(ID3Tag *tag, const char *fileName)
  {
    size_t offset = 0;
    if (tag)
    {
      ID3_CATCH(offset = reinterpret_cast<ID3_Tag *>(tag)->Link(fileName));
    }
    return offset;
  }

  ID3_C_EXPORT size_t CCONV
  ID3Tag_LinkWithFlags(ID3Tag *tag, const char *fileName, flags_t flags)
  {
    size_t offset = 0;
    if (tag)
    {
      ID3_CATCH(offset = reinterpret_cast<ID3_Tag *>(tag)->Link(fileName,flags));
    }
    return offset;
  }



  ID3_C_EXPORT ID3_Err CCONV
  ID3Tag_Update(ID3Tag *tag)
  {
    flags_t flags = 0;
    if (tag)
    {
      ID3_CATCH(flags = reinterpret_cast<ID3_Tag *>(tag)->Update());
    }
    return ID3E_NoError;
  }

  ID3_C_EXPORT ID3_Err CCONV
  ID3Tag_UpdateByTagType(ID3Tag *tag, flags_t tag_type)
  {
    flags_t flags = 0;
    if (tag)
    {
      ID3_CATCH(flags = reinterpret_cast<ID3_Tag *>(tag)->Update(tag_type));
    }
    return ID3E_NoError;
  }


  ID3_C_EXPORT ID3_Err CCONV
  ID3Tag_Strip(ID3Tag *tag, flags_t ulTagFlags)
  {
    if (tag)
    {
      ID3_CATCH(reinterpret_cast<ID3_Tag *>(tag)->Strip(ulTagFlags));
    }
    return ID3E_NoError;
  }


  ID3_C_EXPORT ID3Frame* CCONV
  ID3Tag_FindFrameWithID(const ID3Tag *tag, ID3_FrameID id)
  {
    ID3_Frame *frame = NULL;

    if (tag)
    {
      ID3_CATCH(frame = reinterpret_cast<const ID3_Tag *>(tag)->Find(id));
    }

    return reinterpret_cast<ID3Frame *>(frame);
  }


  ID3_C_EXPORT ID3Frame* CCONV
  ID3Tag_FindFrameWithINT(const ID3Tag *tag, ID3_FrameID id,
                          ID3_FieldID fld, uint32 data)
  {
    ID3_Frame *frame = NULL;

    if (tag)
    {
      ID3_CATCH(frame = reinterpret_cast<const ID3_Tag *>(tag)->Find(id, fld, data));
    }

    return reinterpret_cast<ID3Frame *>(frame);
  }


  ID3_C_EXPORT ID3Frame* CCONV
  ID3Tag_FindFrameWithASCII(const ID3Tag *tag, ID3_FrameID id,
                            ID3_FieldID fld, const char *data)
  {
    ID3_Frame *frame = NULL;

    if (tag)
    {
      ID3_CATCH(frame = reinterpret_cast<const ID3_Tag *>(tag)->Find(id, fld, data));
    }

    return reinterpret_cast<ID3Frame *>(frame);
  }


  ID3_C_EXPORT ID3Frame* CCONV
  ID3Tag_FindFrameWithUNICODE(const ID3Tag *tag, ID3_FrameID id,
                              ID3_FieldID fld, const unicode_t *data)
  {
    ID3_Frame *frame = NULL;

    if (tag)
    {
      ID3_CATCH(frame = reinterpret_cast<const ID3_Tag *>(tag)->Find(id, fld, data));
    }

    return reinterpret_cast<ID3Frame *>(frame);
  }


  ID3_C_EXPORT size_t CCONV
  ID3Tag_NumFrames(const ID3Tag *tag)
  {
    size_t num = 0;

    if (tag)
    {
      ID3_CATCH(num = reinterpret_cast<const ID3_Tag *>(tag)->NumFrames());
    }

    return num;
  }


  ID3_C_EXPORT bool CCONV
  ID3Tag_HasTagType(const ID3Tag *tag, ID3_TagType tt)
  {
    bool has_tt = false;

    if (tag)
    {
      ID3_CATCH(has_tt = reinterpret_cast<const ID3_Tag *>(tag)->HasTagType(tt));
    }

    return has_tt;
  }

  ID3_C_EXPORT ID3TagIterator* CCONV
  ID3Tag_CreateIterator(ID3Tag* tag)
  {
    ID3_Tag::Iterator* iter = NULL;

    if (tag)
    {
      ID3_CATCH(iter = reinterpret_cast<ID3_Tag*>(tag)->CreateIterator());
    }

    return reinterpret_cast<ID3TagIterator*>(iter);
  }

  ID3_C_EXPORT ID3TagConstIterator* CCONV
  ID3Tag_CreateConstIterator(const ID3Tag* tag)
  {
    ID3_Tag::ConstIterator* iter = NULL;

    if (tag)
    {
      ID3_CATCH(iter = reinterpret_cast<const ID3_Tag*>(tag)->CreateIterator());
    }

    return reinterpret_cast<ID3TagConstIterator*>(iter);
  }

  ID3_C_EXPORT void CCONV
  ID3TagIterator_Delete(ID3TagIterator *iter)
  {
    if (iter)
    {
      ID3_CATCH(delete reinterpret_cast<ID3_Tag::Iterator*>(iter));
    }
  }

  ID3_C_EXPORT ID3Frame* CCONV
  ID3TagIterator_GetNext(ID3TagIterator *iter)
  {
    ID3_Frame* frame = NULL;
    if (iter)
    {
      ID3_CATCH(frame = reinterpret_cast<ID3_Tag::Iterator*>(iter)->GetNext());
    }
    return reinterpret_cast<ID3Frame*>(frame);
  }

  ID3_C_EXPORT void CCONV
  ID3TagConstIterator_Delete(ID3TagConstIterator *iter)
  {
    if (iter)
    {
      ID3_CATCH(delete reinterpret_cast<ID3_Tag::ConstIterator*>(iter));
    }
  }

  ID3_C_EXPORT const ID3Frame* CCONV
  ID3TagConstIterator_GetNext(ID3TagConstIterator *iter)
  {
    const ID3_Frame* frame = NULL;
    if (iter)
    {
      ID3_CATCH(frame = reinterpret_cast<ID3_Tag::ConstIterator*>(iter)->GetNext());
    }
    return reinterpret_cast<const ID3Frame*>(frame);
  }

  // frame wrappers

  ID3_C_EXPORT ID3Frame* CCONV
  ID3Frame_New(void)
  {
    ID3_Frame* frame = NULL;
    ID3_CATCH(frame = new ID3_Frame);
    return reinterpret_cast<ID3Frame *>(frame);
  }

  ID3_C_EXPORT ID3Frame* CCONV
  ID3Frame_NewID(ID3_FrameID id)
  {
    ID3_Frame* frame = NULL;
    ID3_CATCH(frame = new ID3_Frame(id));
    return reinterpret_cast<ID3Frame *>(frame);
  }

  ID3_C_EXPORT void CCONV
  ID3Frame_Delete(ID3Frame *frame)
  {
    if (frame)
    {
      ID3_CATCH(delete reinterpret_cast<ID3_Frame *>(frame));
    }
  }


  ID3_C_EXPORT void CCONV
  ID3Frame_Clear(ID3Frame *frame)
  {
    if (frame)
    {
      ID3_CATCH(reinterpret_cast<ID3_Frame *>(frame)->Clear());
    }
  }


  ID3_C_EXPORT void CCONV
  ID3Frame_SetID(ID3Frame *frame, ID3_FrameID id)
  {
    if (frame)
    {
      ID3_CATCH(reinterpret_cast<ID3_Frame *>(frame)->SetID(id));
    }
  }


  ID3_C_EXPORT ID3_FrameID CCONV
  ID3Frame_GetID(const ID3Frame *frame)
  {
    ID3_FrameID id = ID3FID_NOFRAME;

    if (frame)
    {
      ID3_CATCH(id = reinterpret_cast<const ID3_Frame *>(frame)->GetID());
    }

    return id;
  }


  ID3_C_EXPORT ID3Field* CCONV
  ID3Frame_GetField(const ID3Frame *frame, ID3_FieldID name)
  {
    ID3_Field *field = NULL;

    if (frame)
    {
      ID3_CATCH(field = reinterpret_cast<const ID3_Frame *>(frame)->GetField(name));
    }

    return reinterpret_cast<ID3Field *>(field);
  }


  ID3_C_EXPORT void CCONV
  ID3Frame_SetCompression(ID3Frame *frame, bool comp)
  {
    if (frame)
    {
      ID3_CATCH(reinterpret_cast<ID3_Frame *>(frame)->SetCompression(comp));
    }
  }


  ID3_C_EXPORT bool CCONV
  ID3Frame_GetCompression(const ID3Frame *frame)
  {
    bool compressed = false;
    if (frame)
    {
      ID3_CATCH(compressed = reinterpret_cast<const ID3_Frame *>(frame)->GetCompression());
    }
    return compressed;
  }


  // field wrappers


  ID3_C_EXPORT void CCONV
  ID3Field_Clear(ID3Field *field)
  {
    if (field)
    {
      ID3_CATCH(reinterpret_cast<ID3_Field *>(field)->Clear());
    }
  }


  ID3_C_EXPORT size_t CCONV
  ID3Field_Size(const ID3Field *field)
  {
    size_t size = 0;

    if (field)
    {
      ID3_CATCH(size = reinterpret_cast<const ID3_Field *>(field)->Size());
    }

    return size;
  }


  ID3_C_EXPORT size_t CCONV
  ID3Field_GetNumTextItems(const ID3Field *field)
  {
    size_t items = 0;

    if (field)
    {
      ID3_CATCH(items = reinterpret_cast<const ID3_Field *>(field)->GetNumTextItems());
    }

    return items;
  }


  ID3_C_EXPORT void CCONV
  ID3Field_SetINT(ID3Field *field, uint32 data)
  {
    if (field)
    {
      ID3_CATCH(reinterpret_cast<ID3_Field *>(field)->Set(data));
    }
  }


  ID3_C_EXPORT uint32 CCONV
  ID3Field_GetINT(const ID3Field *field)
  {
    uint32 value = 0;

    if (field)
    {
      ID3_CATCH(value = reinterpret_cast<const ID3_Field *>(field)->Get());
    }

    return value;
  }


  ID3_C_EXPORT void CCONV
  ID3Field_SetUNICODE(ID3Field *field, const unicode_t *string)
  {
    if (field)
    {
      ID3_CATCH(reinterpret_cast<ID3_Field *>(field)->Set(string));
    }
  }


  ID3_C_EXPORT size_t CCONV
  ID3Field_GetUNICODE(const ID3Field *field, unicode_t *buffer, size_t maxChars)
  {
    size_t numChars = 0;

    if (field)
    {
      ID3_CATCH(numChars = reinterpret_cast<const ID3_Field *>(field)->Get(buffer, maxChars));
    }

    return numChars;
  }


  ID3_C_EXPORT size_t CCONV
  ID3Field_GetUNICODEItem(const ID3Field *field, unicode_t *buffer,
                          size_t maxChars, size_t itemNum)
  {
    size_t numChars = 0;

    if (field)
    {
      ID3_CATCH(numChars = reinterpret_cast<const ID3_Field *>(field)->Get(buffer, maxChars, itemNum));
    }

    return numChars;
  }


  ID3_C_EXPORT void CCONV
  ID3Field_AddUNICODE(ID3Field *field, const unicode_t *string)
  {
    if (field)
    {
      ID3_CATCH(reinterpret_cast<ID3_Field *>(field)->Add(string));
    }
  }


  ID3_C_EXPORT void CCONV
  ID3Field_SetASCII(ID3Field *field, const char *string)
  {
    if (field)
    {
      ID3_CATCH(reinterpret_cast<ID3_Field *>(field)->Set(string));
    }
  }


  ID3_C_EXPORT size_t CCONV
  ID3Field_GetASCII(const ID3Field *field, char *buffer, size_t maxChars)
  {
    size_t numChars = 0;

    if (field)
    {
      ID3_CATCH(numChars = reinterpret_cast<const ID3_Field *>(field)->Get(buffer, maxChars));
    }

    return numChars;
  }

  ID3_C_EXPORT size_t CCONV
  ID3Field_GetASCIIItem(const ID3Field *field, char *buffer,
                        size_t maxChars, size_t itemNum)
  {
    size_t numChars = 0;

    if (field)
    {
      ID3_CATCH(numChars = reinterpret_cast<const ID3_Field *>(field)->Get(buffer, maxChars, itemNum));
    }

    return numChars;
  }


  ID3_C_EXPORT void CCONV
  ID3Field_AddASCII(ID3Field *field, const char *string)
  {
    if (field)
    {
      ID3_CATCH(reinterpret_cast<ID3_Field *>(field)->Add(string));
    }
  }


  ID3_C_EXPORT void CCONV
  ID3Field_SetBINARY(ID3Field *field, const uchar *data, size_t size)
  {
    if (field)
    {
      ID3_CATCH(reinterpret_cast<ID3_Field *>(field)->Set(data, size));
    }
  }


  ID3_C_EXPORT void CCONV
  ID3Field_GetBINARY(const ID3Field *field, uchar *buffer, size_t buffLength)
  {
    if (field)
    {
      ID3_CATCH(reinterpret_cast<const ID3_Field *>(field)->Get(buffer, buffLength));
    }
  }


  ID3_C_EXPORT void CCONV
  ID3Field_FromFile(ID3Field *field, const char *fileName)
  {
    if (field)
    {
      ID3_CATCH(reinterpret_cast<ID3_Field *>(field)->FromFile(fileName));
    }
  }


  ID3_C_EXPORT void CCONV
  ID3Field_ToFile(const ID3Field *field, const char *fileName)
  {
    if (field)
    {
      ID3_CATCH(reinterpret_cast<const ID3_Field *>(field)->ToFile(fileName));
    }
  }

#ifdef __cplusplus
}
#endif /* __cplusplus */

