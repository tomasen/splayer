// $Id: helpers.cpp,v 1.12 2002/09/21 17:23:32 t1mpy Exp $

// id3lib: a C++ library for creating and manipulating id3v1/v2 tags
// Copyright 1999, 2000  Scott Thomas Haug

// Lots of hacking added to this file by Scott Wheeler (scott@slackorama.net)
// 11/02/2001

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



#include <ctype.h>

#include "helpers.h"
#include "tag_impl.h" //has <stdio.h> "tag.h" "header_tag.h" "frame.h" "field.h" "spec.h" "id3lib_strings.h" "utils.h"

using namespace dami;

String id3::v2::getString(const ID3_Frame* frame, ID3_FieldID fldName)
{
  if (!frame)
  {
    return "";
  }
  ID3_Field* fp = frame->GetField(fldName);
  if (!fp)
  {
    return "";
  }
  ID3_TextEnc enc = fp->GetEncoding();
  fp->SetEncoding(ID3TE_ASCII);

  String text(fp->GetRawText(), fp->Size());

  fp->SetEncoding(enc);
  return text;
}

String id3::v2::getStringAtIndex(const ID3_Frame* frame, ID3_FieldID fldName,
                                 size_t nIndex)
{
  if (!frame)
  {
    return "";
  }
  String text;
  ID3_Field* fp = frame->GetField(fldName);
  if (fp && fp->GetNumTextItems() < nIndex)
  {
    ID3_TextEnc enc = fp->GetEncoding();
    fp->SetEncoding(ID3TE_ASCII);

    text = fp->GetRawTextItem(nIndex);

    fp->SetEncoding(enc);
  }
  return text;
}

size_t id3::v2::removeFrames(ID3_TagImpl& tag, ID3_FrameID id)
{
  size_t numRemoved = 0;
  ID3_Frame* frame = NULL;

  while ((frame = tag.Find(id)) != NULL)
  {
    frame = tag.RemoveFrame(frame);
    delete frame;
    numRemoved++;
  }

  return numRemoved;
}

String id3::v2::getFrameText(const ID3_TagImpl& tag, ID3_FrameID id)
{
  ID3_Frame* frame = tag.Find(id);
  return getString(frame, ID3FN_TEXT);
}

ID3_Frame* id3::v2::setFrameText(ID3_TagImpl& tag, ID3_FrameID id, String text)
{
  ID3_Frame* frame = tag.Find(id);
  if (!frame)
  {
    frame = new ID3_Frame(id);
    if(!tag.AttachFrame(frame)) return NULL;
  }
  frame->GetField(ID3FN_TEXT)->Set(text.c_str());

  return frame;
}

////////////////////////////////////////////////////////////

ID3_Frame* id3::v2::hasArtist(const ID3_TagImpl& tag)
{
  ID3_Frame* fp = NULL;
  (fp = tag.Find(ID3FID_LEADARTIST)) ||
  (fp = tag.Find(ID3FID_BAND))       ||
  (fp = tag.Find(ID3FID_CONDUCTOR))  ||
  (fp = tag.Find(ID3FID_COMPOSER));
  return fp;
}

String id3::v2::getArtist(const ID3_TagImpl& tag)
{
  ID3_Frame* frame = hasArtist(tag);
  return getString(frame, ID3FN_TEXT);
}

ID3_Frame* id3::v2::setArtist(ID3_TagImpl& tag, String text)
{
  removeArtists(tag);
  return setFrameText(tag, ID3FID_LEADARTIST, text);
}

size_t id3::v2::removeArtists(ID3_TagImpl& tag)
{
  size_t numRemoved = 0;
  ID3_Frame* frame = NULL;

  while ((frame = hasArtist(tag)) != NULL)
  {
    frame = tag.RemoveFrame(frame);
    delete frame;
    numRemoved++;
  }

  return numRemoved;
}

////////////////////////////////////////////////////////////

ID3_Frame* id3::v2::hasAlbum(const ID3_TagImpl& tag)
{
  ID3_Frame* frame = tag.Find(ID3FID_ALBUM);
  return(frame);
}

String id3::v2::getAlbum(const ID3_TagImpl& tag)
{
  return getFrameText(tag, ID3FID_ALBUM);
}

ID3_Frame* id3::v2::setAlbum(ID3_TagImpl& tag, String text)
{
  return setFrameText(tag, ID3FID_ALBUM, text);
}

size_t id3::v2::removeAlbums(ID3_TagImpl& tag)
{
  return removeFrames(tag, ID3FID_ALBUM);
}

////////////////////////////////////////////////////////////

ID3_Frame* id3::v2::hasTitle(const ID3_TagImpl& tag)
{
  ID3_Frame* frame = tag.Find(ID3FID_TITLE);
  return(frame);
}

String id3::v2::getTitle(const ID3_TagImpl& tag)
{
  return getFrameText(tag, ID3FID_TITLE);
}

ID3_Frame* id3::v2::setTitle(ID3_TagImpl& tag, String text)
{
  return setFrameText(tag, ID3FID_TITLE, text);
}

size_t id3::v2::removeTitles(ID3_TagImpl& tag)
{
  return removeFrames(tag, ID3FID_TITLE);
}

////////////////////////////////////////////////////////////

ID3_Frame* id3::v2::hasYear(const ID3_TagImpl& tag)
{
  ID3_Frame* frame = tag.Find(ID3FID_YEAR);
  return(frame);
}

String id3::v2::getYear(const ID3_TagImpl& tag)
{
  return getFrameText(tag, ID3FID_YEAR);
}

ID3_Frame* id3::v2::setYear(ID3_TagImpl& tag, String text)
{
  return setFrameText(tag, ID3FID_YEAR, text);
}

size_t id3::v2::removeYears(ID3_TagImpl& tag)
{
  return removeFrames(tag, ID3FID_YEAR);
}

////////////////////////////////////////////////////////////

ID3_Frame* id3::v2::hasV1Comment(const ID3_TagImpl& tag)
{
  ID3_Frame* frame = NULL;
  (frame = tag.Find(ID3FID_COMMENT, ID3FN_DESCRIPTION, STR_V1_COMMENT_DESC)) ||
  (frame = tag.Find(ID3FID_COMMENT, ID3FN_DESCRIPTION, ""                 )) ||
  (frame = tag.Find(ID3FID_COMMENT));
  return(frame);
}

ID3_Frame* id3::v2::hasComment(const ID3_TagImpl& tag)
{
  ID3_Frame* frame = tag.Find(ID3FID_COMMENT);
  return(frame);
}

String id3::v2::getV1Comment(const ID3_TagImpl& tag)
{
  ID3_Frame* frame;
  (frame = tag.Find(ID3FID_COMMENT, ID3FN_DESCRIPTION, STR_V1_COMMENT_DESC)) ||
  (frame = tag.Find(ID3FID_COMMENT, ID3FN_DESCRIPTION, ""                 )) ||
  (frame = tag.Find(ID3FID_COMMENT));
  return getString(frame, ID3FN_TEXT);
}

String id3::v2::getComment(const ID3_TagImpl& tag, String desc)
{
  ID3_Frame* frame = tag.Find(ID3FID_COMMENT, ID3FN_DESCRIPTION, desc.c_str());
  return getString(frame, ID3FN_TEXT);
}

ID3_Frame* id3::v2::setComment(ID3_TagImpl& tag, String text, String desc,
                               String lang)
{
  ID3D_NOTICE( "id3::v2::setComment: trying to find frame with description = " << desc );
  ID3_Frame* frame = NULL;
  // See if there is already a comment with this description
  for (ID3_TagImpl::iterator iter = tag.begin(); iter != tag.end(); ++iter)
  {
    frame = *iter;
    if (frame == NULL)
    {
      continue;
    }
    if (frame->GetID() == ID3FID_COMMENT)
    {
      String tmpDesc = getString(frame, ID3FN_DESCRIPTION);
      if (tmpDesc == desc)
      {
        ID3D_NOTICE( "id3::v2::setComment: found frame with description = " << desc );
        break;
      }
    }
    frame = NULL;
  }
  if (frame == NULL)
  {
    ID3D_NOTICE( "id3::v2::setComment: creating new comment frame" );
    frame = new ID3_Frame(ID3FID_COMMENT);
    if(!tag.AttachFrame(frame)) return NULL;
  }
  if (!frame)
  {
    ID3D_WARNING( "id3::v2::setComment: ack! no frame" );
  }
  else
  {
    frame->GetField(ID3FN_LANGUAGE)->Set(lang.c_str());
    frame->GetField(ID3FN_DESCRIPTION)->Set(desc.c_str());
    frame->GetField(ID3FN_TEXT)->Set(text.c_str());
  }

  return frame;
}

// Remove all comments from the tag
size_t id3::v2::removeAllComments(ID3_TagImpl& tag)
{
  return removeFrames(tag, ID3FID_COMMENT);
}

// Remove all comments from the tag with the given description
size_t id3::v2::removeComments(ID3_TagImpl& tag, String desc)
{
  size_t numRemoved = 0;

  for (ID3_TagImpl::iterator iter = tag.begin(); iter != tag.end(); ++iter)
  {
    ID3_Frame* frame = *iter;
    if (frame == NULL)
    {
      continue;
    }
    if (frame->GetID() == ID3FID_COMMENT)
    {
      // See if the description we have matches the description of the
      // current comment.  If so, remove the comment
      String tmpDesc = getString(frame, ID3FN_DESCRIPTION);
      if (tmpDesc == desc)
      {
        frame = tag.RemoveFrame(frame);
        delete frame;
        numRemoved++;
      }
    }
  }

  return numRemoved;
}

////////////////////////////////////////////////////////////

ID3_Frame* id3::v2::hasTrack(const ID3_TagImpl& tag)
{
  ID3_Frame* frame = tag.Find(ID3FID_TRACKNUM);
  return(frame);
}

String id3::v2::getTrack(const ID3_TagImpl& tag)
{
  return getFrameText(tag, ID3FID_TRACKNUM);
}

size_t id3::v2::getTrackNum(const ID3_TagImpl& tag)
{
  String sTrack = getTrack(tag);
  return ::atoi(sTrack.c_str());
}

ID3_Frame* id3::v2::setTrack(ID3_TagImpl& tag, uchar trk, uchar ttl)
{
  ID3_Frame* frame = NULL;
  String track = toString((size_t)trk);
  if (ttl > 0)
  {
    track += "/";
    track += toString((size_t)ttl);
  }
  setFrameText(tag, ID3FID_TRACKNUM, track);

  return frame;
}

size_t id3::v2::removeTracks(ID3_TagImpl& tag)
{
  return removeFrames(tag, ID3FID_TRACKNUM);
}

////////////////////////////////////////////////////////////

ID3_Frame* id3::v2::hasGenre(const ID3_TagImpl& tag)
{
  ID3_Frame* frame = tag.Find(ID3FID_CONTENTTYPE);
  return(frame);
}

String id3::v2::getGenre(const ID3_TagImpl& tag)
{
  return getFrameText(tag, ID3FID_CONTENTTYPE);
}

size_t id3::v2::getGenreNum(const ID3_TagImpl& tag)
{
  String sGenre = getGenre(tag);
  size_t ulGenre = 0xFF;
  size_t size = sGenre.size();

  // If the genre string begins with "(ddd)", where "ddd" is a number, then
  // "ddd" is the genre number---get it
  size_t i = 0;
  if (i < size && size && sGenre[i] == '(')
  {
    ++i;
    while (i < size && isdigit(sGenre[i]))
    {
      ++i;
    }
    if (i < size && sGenre[i] == ')')
    {
      // if the genre number is greater than 255, its invalid.
      ulGenre = min(0xFF, atoi(&sGenre[1]));
    }
  }

  return ulGenre;
}

ID3_Frame* id3::v2::setGenre(ID3_TagImpl& tag, size_t genre)
{
  String sGenre = "(";
  sGenre += toString(genre) + ")";
  return setFrameText(tag, ID3FID_CONTENTTYPE, sGenre);
}

size_t id3::v2::removeGenres(ID3_TagImpl& tag)
{
  return removeFrames(tag, ID3FID_CONTENTTYPE);
}

////////////////////////////////////////////////////////////

ID3_Frame* id3::v2::hasLyrics(const ID3_TagImpl& tag)
{
  ID3_Frame* frame = tag.Find(ID3FID_UNSYNCEDLYRICS);
  return(frame);
}

String id3::v2::getLyrics(const ID3_TagImpl& tag)
{
  return getFrameText(tag, ID3FID_UNSYNCEDLYRICS);
}

ID3_Frame* id3::v2::setLyrics(ID3_TagImpl& tag, String text, String desc,
                              String lang)
{
  ID3_Frame* frame = NULL;
  // See if there is already a comment with this description
  for (ID3_TagImpl::iterator iter = tag.begin(); iter != tag.end(); ++iter)
  {
    frame = *iter;
    if (frame == NULL)
    {
      continue;
    }
    if (frame->GetID() == ID3FID_COMMENT)
    {
      String tmpDesc = getString(frame, ID3FN_DESCRIPTION);
      if (tmpDesc == desc)
      {
        break;
      }
    }
    frame = NULL;
  }
  if (frame == NULL)
  {
    frame = new ID3_Frame(ID3FID_UNSYNCEDLYRICS);
    if(!tag.AttachFrame(frame)) return NULL;
  }
  frame->GetField(ID3FN_LANGUAGE)->Set(lang.c_str());
  frame->GetField(ID3FN_DESCRIPTION)->Set(desc.c_str());
  frame->GetField(ID3FN_TEXT)->Set(text.c_str());

  return frame;
}

size_t id3::v2::removeLyrics(ID3_TagImpl& tag)
{
  return removeFrames(tag, ID3FID_UNSYNCEDLYRICS);
}

String id3::v2::getLyricist(const ID3_TagImpl& tag)
{
  return getFrameText(tag, ID3FID_LYRICIST);
}

ID3_Frame* id3::v2::setLyricist(ID3_TagImpl& tag, String text)
{
  return setFrameText(tag, ID3FID_LYRICIST, text);
}

size_t id3::v2::removeLyricists(ID3_TagImpl& tag)
{
  return removeFrames(tag, ID3FID_LYRICIST);
}

////////////////////////////////////////////////////////////

ID3_Frame* id3::v2::hasSyncLyrics(const ID3_TagImpl& tag, String lang, String desc)
{
  ID3_Frame* frame=NULL;
  (frame = tag.Find(ID3FID_SYNCEDLYRICS, ID3FN_LANGUAGE, lang)) ||
  (frame = tag.Find(ID3FID_SYNCEDLYRICS, ID3FN_DESCRIPTION, desc));
  return(frame);
}

ID3_Frame* id3::v2::setSyncLyrics(ID3_TagImpl& tag, BString data,
                                  ID3_TimeStampFormat format, String desc,
                                  String lang, ID3_ContentType type)
{
  ID3_Frame* frame = NULL;

  // check if a SYLT frame of this language or descriptor already exists
  (frame = tag.Find(ID3FID_SYNCEDLYRICS, ID3FN_LANGUAGE, lang)) ||
  (frame = tag.Find(ID3FID_SYNCEDLYRICS, ID3FN_DESCRIPTION, desc));

  if (!frame)
  {
    frame = new ID3_Frame(ID3FID_SYNCEDLYRICS);
    if(!tag.AttachFrame(frame)) return NULL;
  }
  frame->GetField(ID3FN_LANGUAGE)->Set(lang.c_str());
  frame->GetField(ID3FN_DESCRIPTION)->Set(desc.c_str());
  frame->GetField(ID3FN_TIMESTAMPFORMAT)->Set(format);
  frame->GetField(ID3FN_CONTENTTYPE)->Set(type);
  frame->GetField(ID3FN_DATA)->Set(data.data(), data.size());

  return frame;
}

BString id3::v2::getSyncLyrics(const ID3_TagImpl& tag, String lang, String desc)
{
  // check if a SYLT frame of this language or descriptor exists
  ID3_Frame* frame = NULL;
  (frame = tag.Find(ID3FID_SYNCEDLYRICS, ID3FN_LANGUAGE, lang)) ||
  (frame = tag.Find(ID3FID_SYNCEDLYRICS, ID3FN_DESCRIPTION, desc)) ||
  (frame = tag.Find(ID3FID_SYNCEDLYRICS));

  // get the lyrics size
  ID3_Field* fld = frame->GetField(ID3FN_DATA);
  return BString(reinterpret_cast<const BString::value_type *>(fld->GetRawBinary()), fld->Size());
}

