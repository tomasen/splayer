// $Id: field.cpp,v 1.47 2002/11/03 00:41:27 t1mpy Exp $

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


#include "field_impl.h"
#include "id3/utils.h" // has <config.h> "id3/id3lib_streams.h" "id3/globals.h" "id3/id3lib_strings.h"
#include "field_def.h"
#include "frame_def.h"
#include "readers.h"
#include <assert.h>

using namespace dami;

// This is used for unimplemented frames so that their data is preserved when
// parsing and rendering
static ID3_FieldDef ID3FD_Unimplemented[] =
{
  {
    ID3FN_DATA,                         // FIELD NAME
    ID3FTY_BINARY,                      // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};

const ID3_FieldDef* ID3_FieldDef::DEFAULT = ID3FD_Unimplemented;

static ID3_FieldDef ID3FD_URL[] =
{
  {
    ID3FN_URL,                          // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};

static ID3_FieldDef ID3FD_UserURL[] =
{
  {
    ID3FN_TEXTENC,                      // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_DESCRIPTION,                  // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR | ID3FF_ENCODABLE,       // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_URL,                          // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};

static ID3_FieldDef ID3FD_Text[] =
{
  {
    ID3FN_TEXTENC,                      // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_TEXT,                         // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_ENCODABLE,                    // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};


static ID3_FieldDef ID3FD_UserText[] =
{
  {
    ID3FN_TEXTENC,                      // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_DESCRIPTION,                  // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR | ID3FF_ENCODABLE,       // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_TEXT,                         // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_ENCODABLE,                    // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};


static ID3_FieldDef ID3FD_GeneralText[] =
{
  {
    ID3FN_TEXTENC,                      // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_LANGUAGE,                     // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    3,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_DESCRIPTION,                  // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR | ID3FF_ENCODABLE,       // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_TEXT,                         // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_ENCODABLE,                    // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};

static ID3_FieldDef ID3FD_TermsOfUse[] =
{
  {
    ID3FN_TEXTENC,                      // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_3_0,                          // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_LANGUAGE,                     // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    3,                                  // FIXED LEN
    ID3V2_3_0,                          // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_TEXT,                         // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_3_0,                          // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_ENCODABLE,                    // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};

static ID3_FieldDef ID3FD_LinkedInfo[] =
{
  {
    ID3FN_ID,                           // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    3,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_2_1,                          // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_ID,                           // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    4,                                  // FIXED LEN
    ID3V2_3_0,                          // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_URL,                          // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_TEXT,                         // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};

static ID3_FieldDef ID3FD_Picture[] =
{
  {
    ID3FN_TEXTENC,                      // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_IMAGEFORMAT,                  // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    3,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_2_1,                          // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_MIMETYPE,                     // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_3_0,                          // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_PICTURETYPE,                  // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_DESCRIPTION,                  // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR | ID3FF_ENCODABLE,       // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_DATA,                         // FIELD NAME
    ID3FTY_BINARY,                      // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};

static ID3_FieldDef ID3FD_GEO[] =
{
  {
    ID3FN_TEXTENC,                      // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_MIMETYPE,                     // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_FILENAME,                     // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR | ID3FF_ENCODABLE,       // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_DESCRIPTION,                  // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR | ID3FF_ENCODABLE,       // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_DATA,                         // FIELD NAME
    ID3FTY_BINARY,                      // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};

static ID3_FieldDef ID3FD_UFI[] =
{
  {
    ID3FN_OWNER,                        // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_DATA,                         // FIELD NAME
    ID3FTY_BINARY,                      // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};

static ID3_FieldDef ID3FD_PlayCounter[] =
{
  {
    ID3FN_COUNTER,                      // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    4,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};

static ID3_FieldDef ID3FD_Popularimeter[] =
{
  {
    ID3FN_EMAIL,                        // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_RATING,                       // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_COUNTER,                      // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    4,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};

static ID3_FieldDef ID3FD_Private[] =
{
  {
    ID3FN_OWNER,                        // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_3_0,                          // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_DATA,                         // FIELD NAME
    ID3FTY_BINARY,                      // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_3_0,                          // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};


static ID3_FieldDef ID3FD_Registration[] =
{
  {
    ID3FN_OWNER,                        // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_3_0,                          // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_ID,                           // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_3_0,                          // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_DATA,                         // FIELD NAME
    ID3FTY_BINARY,                      // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_3_0,                          // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};

static ID3_FieldDef ID3FD_InvolvedPeople[] =
{
  {
    ID3FN_TEXTENC,                      // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_TEXT,                         // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_TEXTLIST,                     // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};

static ID3_FieldDef ID3FD_CDM[] =
{
  {
    ID3FN_DATA,                         // FIELD NAME
    ID3FTY_BINARY,                      // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_2_1,                          // INITIAL SPEC
    ID3V2_2_1,                          // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  }
};

static ID3_FieldDef ID3FD_SyncLyrics[] =
{
  {
    ID3FN_TEXTENC,                      // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_LANGUAGE,                     // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    3,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_TIMESTAMPFORMAT,              // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_CONTENTTYPE,                  // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_DESCRIPTION,                  // FIELD NAME
    ID3FTY_TEXTSTRING,                  // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_CSTR | ID3FF_ENCODABLE,       // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_DATA,                         // FIELD NAME
    ID3FTY_BINARY,                      // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};


/*
 * Currently unused
 */
#if defined _UNDEFINED_
static ID3_FieldDef ID3FD_Volume[] =
{
  {
    ID3FN_VOLUMEADJ,                    // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_NUMBITS,                      // FIELD NAME
    ID3FTY_INTEGER,                     // FIELD TYPE
    1,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_NONE,                         // FLAGS
    ID3FN_NOFIELD                       // LINKED FIELD
  },
  {
    ID3FN_VOLCHGRIGHT,                  // FIELD NAME
    ID3FTY_BITFIELD,                    // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_ADJUSTEDBY,                   // FLAGS
    ID3FN_NUMBITS                       // LINKED FIELD
  },
  {
    ID3FN_VOLCHGLEFT,                   // FIELD NAME
    ID3FTY_BITFIELD,                    // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_ADJUSTEDBY,                   // FLAGS
    ID3FN_NUMBITS                       // LINKED FIELD
  },
  {
    ID3FN_PEAKVOLRIGHT,                 // FIELD NAME
    ID3FTY_BITFIELD,                    // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_ADJUSTEDBY,                   // FLAGS
    ID3FN_NUMBITS                       // LINKED FIELD
  },
  {
    ID3FN_PEAKVOLLEFT,                  // FIELD NAME
    ID3FTY_BITFIELD,                    // FIELD TYPE
    0,                                  // FIXED LEN
    ID3V2_EARLIEST,                     // INITIAL SPEC
    ID3V2_LATEST,                       // ENDING SPEC
    ID3FF_ADJUSTEDBY,                   // FLAGS
    ID3FN_NUMBITS                       // LINKED FIELD
  },
  { ID3FN_NOFIELD }
};
#endif /* _UNDEFINED_ */

// **** Currently Implemented Frames
// APIC  PIC  ID3FID_PICTURE           Attached picture
// COMM  COM  ID3FID_COMMENT           Comments
// ENCR       ID3FID_CRYPTOREG         Encryption method registration
// GEOB  GEO  ID3FID_GENERALOBJECT     General encapsulated object
// GRID       ID3FID_GROUPINGREG       Group identification registration
// IPLS  IPL  ID3FID_INVOLVEDPEOPLE    Involved people list
// LINK  LNK  ID3FID_LINKEDINFO        Linked information
// PCNT  CNT  ID3FID_PLAYCOUNTER       Play counter
// POPM  POP  ID3FID_POPULARIMETER     Popularimeter
// PRIV       ID3FID_PRIVATE           Private frame
// SYLT  SLT  ID3FID_SYNCEDLYRICS      Synchronized lyric/text
// TALB  TAL  ID3FID_ALBUM             Album/Movie/Show title
// TBPM  TBP  ID3FID_BPM               BPM (beats per minute)
// TCOM  TCM  ID3FID_COMPOSER          Composer
// TCON  TCO  ID3FID_CONTENTTYPE       Content type
// TCOP  TCR  ID3FID_COPYRIGHT         Copyright message
// TDAT  TDA  ID3FID_DATE              Date
// TDLY  TDY  ID3FID_PLAYLISTDELAY     Playlist delay
// TENC  TEN  ID3FID_ENCODEDBY         Encoded by
// TEXT  TXT  ID3FID_LYRICIST          Lyricist/Text writer
// TFLT  TFT  ID3FID_FILETYPE          File type
// TIME  TKE  ID3FID_TIME              Time
// TIT1  TIM  ID3FID_CONTENTGROUP      Content group description
// TIT2  TT1  ID3FID_TITLE             Title/songname/content description
// TIT3  TT2  ID3FID_SUBTITLE          Subtitle/Description refinement
// TKEY  TT3  ID3FID_INITIALKEY        Initial key
// TLAN  TLA  ID3FID_LANGUAGE          Language(s)
// TLEN  TLE  ID3FID_SONGLEN           Length
// TMED  TMT  ID3FID_MEDIATYPE         Media type
// TOAL  TOT  ID3FID_ORIGALBUM         Original album/movie/show title
// TOFN  TOF  ID3FID_ORIGFILENAME      Original filename
// TOLY  TOL  ID3FID_ORIGLYRICIST      Original lyricist(s)/text writer(s)
// TOPE  TOA  ID3FID_ORIGARTIST        Original artist(s)/performer(s)
// TORY  TOR  ID3FID_ORIGYEAR          Original release year
// TOWN       ID3FID_FILEOWNER         File owner/licensee
// TPE1  TP1  ID3FID_LEADARTIST        Lead performer(s)/Soloist(s)
// TPE2  TP2  ID3FID_BAND              Band/orchestra/accompaniment
// TPE3  TP3  ID3FID_CONDUCTOR         Conductor/performer refinement
// TPE4  TP4  ID3FID_MIXARTIST         Interpreted, remixed, or otherwise modified
// TPOS  TPA  ID3FID_PARTINSET         Part of a set
// TPUB  TPB  ID3FID_PUBLISHER         Publisher
// TRCK  TRK  ID3FID_TRACKNUM          Track number/Position in set
// TRDA  TRD  ID3FID_RECORDINGDATES    Recording dates
// TRSN  TRN  ID3FID_NETRADIOSTATION   Internet radio station name
// TRSO  TRO  ID3FID_NETRADIOOWNER     Internet radio station owner
// TSIZ  TSI  ID3FID_SIZE              Size
// TSRC  TRC  ID3FID_ISRC              ISRC (international standard recording code)
// TSSE  TSS  ID3FID_ENCODERSETTINGS   Software/Hardware and encoding settings
// TXXX  TXX  ID3FID_USERTEXT          User defined text information
// TYER  TYE  ID3FID_YEAR              Year
// UFID  UFI  ID3FID_UNIQUEFILEID      Unique file identifier
// USER       ID3FID_TERMSOFUSE        Terms of use
// USLT  ULT  ID3FID_UNSYNCEDLYRICS    Unsynchronized lyric/text transcription
// WCOM  WCM  ID3FID_WWWCOMMERCIALINFO Commercial information
// WCOP  WCM  ID3FID_WWWCOPYRIGHT      Copyright/Legal infromation
// WOAF  WCP  ID3FID_WWWAUDIOFILE      Official audio file webpage
// WOAR  WAF  ID3FID_WWWARTIST         Official artist/performer webpage
// WOAS  WAR  ID3FID_WWWAUDIOSOURCE    Official audio source webpage
// WORS  WAS  ID3FID_WWWRADIOPAGE      Official internet radio station homepage
// WPAY  WRA  ID3FID_WWWPAYMENT        Payment
// WPUB  WPY  ID3FID_WWWPUBLISHER      Official publisher webpage
// WXXX  WXX  ID3FID_WWWUSER           User defined URL link
//       CDM  ID3FID_METACOMPRESSION   Compressed data meta frame

// **** Currently unimplemented frames
// AENC  CRA  ID3FID_AUDIOCRYPTO       Audio encryption
// COMR       ID3FID_COMMERCIAL        Commercial frame
// EQUA  EQU  ID3FID_EQUALIZATION      Equalization
// ETCO  ETC  ID3FID_EVENTTIMING       Event timing codes
// MCDI  MCI  ID3FID_CDID              Music CD identifier
// MLLT  MLL  ID3FID_MPEGLOOKUP        MPEG location lookup table
// OWNE       ID3FID_OWNERSHIP         Ownership frame
// POSS       ID3FID_POSITIONSYNC      Position synchronisation frame
// RBUF  BUF  ID3FID_BUFFERSIZE        Recommended buffer size
// RVAD  RVA  ID3FID_VOLUMEADJ         Relative volume adjustment
// RVRB  REV  ID3FID_REVERB            Reverb
// SYTC  STC  ID3FID_SYNCEDTEMPO       Synchronized tempo codes
//       CRM  ID3FID_METACRYPTO        Encrypted meta frame
static  ID3_FrameDef ID3_FrameDefs[] =
{
  //                          short  long   tag    file
  // frame id                 id     id     discrd discrd field defs           description
  {ID3FID_AUDIOCRYPTO,       "CRA", "AENC", false, false, ID3FD_Unimplemented, "Audio encryption"},
  {ID3FID_PICTURE,           "PIC", "APIC", false, false, ID3FD_Picture,       "Attached picture"},
  {ID3FID_COMMENT,           "COM", "COMM", false, false, ID3FD_GeneralText,   "Comments"},
  {ID3FID_COMMERCIAL,        ""   , "COMR", false, false, ID3FD_Unimplemented, "Commercial"},
  {ID3FID_CRYPTOREG,         ""   , "ENCR", false, false, ID3FD_Registration,  "Encryption method registration"},
  {ID3FID_EQUALIZATION,      "EQU", "EQUA", false, true,  ID3FD_Unimplemented, "Equalization"},
  {ID3FID_EVENTTIMING,       "ETC", "ETCO", false, true,  ID3FD_Unimplemented, "Event timing codes"},
  {ID3FID_GENERALOBJECT,     "GEO", "GEOB", false, false, ID3FD_GEO,           "General encapsulated object"},
  {ID3FID_GROUPINGREG,       ""   , "GRID", false, false, ID3FD_Registration,  "Group identification registration"},
  {ID3FID_INVOLVEDPEOPLE,    "IPL", "IPLS", false, false, ID3FD_InvolvedPeople,"Involved people list"},
  {ID3FID_LINKEDINFO,        "LNK", "LINK", false, false, ID3FD_LinkedInfo,    "Linked information"},
  {ID3FID_CDID,              "MCI", "MCDI", false, false, ID3FD_Unimplemented, "Music CD identifier"},
  {ID3FID_MPEGLOOKUP,        "MLL", "MLLT", false, true,  ID3FD_Unimplemented, "MPEG location lookup table"},
  {ID3FID_OWNERSHIP,         ""   , "OWNE", false, false, ID3FD_Unimplemented, "Ownership frame"},
  {ID3FID_PRIVATE,           ""   , "PRIV", false, false, ID3FD_Private,       "Private frame"},
  {ID3FID_PLAYCOUNTER,       "CNT", "PCNT", false, false, ID3FD_PlayCounter,   "Play counter"},
  {ID3FID_POPULARIMETER,     "POP", "POPM", false, false, ID3FD_Popularimeter, "Popularimeter"},
  {ID3FID_POSITIONSYNC,      ""   , "POSS", false, true,  ID3FD_Unimplemented, "Position synchronisation frame"},
  {ID3FID_BUFFERSIZE,        "BUF", "RBUF", false, false, ID3FD_Unimplemented, "Recommended buffer size"},
  {ID3FID_VOLUMEADJ,         "RVA", "RVAD", false, true,  ID3FD_Unimplemented, "Relative volume adjustment"},
  {ID3FID_REVERB,            "REV", "RVRB", false, false, ID3FD_Unimplemented, "Reverb"},
  {ID3FID_SYNCEDLYRICS,      "SLT", "SYLT", false, false, ID3FD_SyncLyrics,    "Synchronized lyric/text"},
  {ID3FID_SYNCEDTEMPO,       "STC", "SYTC", false, true,  ID3FD_Unimplemented, "Synchronized tempo codes"},
  {ID3FID_ALBUM,             "TAL", "TALB", false, false, ID3FD_Text,          "Album/Movie/Show title"},
  {ID3FID_BPM,               "TBP", "TBPM", false, false, ID3FD_Text,          "BPM (beats per minute)"},
  {ID3FID_COMPOSER,          "TCM", "TCOM", false, false, ID3FD_Text,          "Composer"},
  {ID3FID_CONTENTTYPE,       "TCO", "TCON", false, false, ID3FD_Text,          "Content type"},
  {ID3FID_COPYRIGHT,         "TCR", "TCOP", false, false, ID3FD_Text,          "Copyright message"},
  {ID3FID_DATE,              "TDA", "TDAT", false, false, ID3FD_Text,          "Date"},
  {ID3FID_PLAYLISTDELAY,     "TDY", "TDLY", false, false, ID3FD_Text,          "Playlist delay"},
  {ID3FID_ENCODEDBY,         "TEN", "TENC", false, true,  ID3FD_Text,          "Encoded by"},
  {ID3FID_LYRICIST,          "TXT", "TEXT", false, false, ID3FD_Text,          "Lyricist/Text writer"},
  {ID3FID_FILETYPE,          "TFT", "TFLT", false, false, ID3FD_Text,          "File type"},
  {ID3FID_TIME,              "TIM", "TIME", false, false, ID3FD_Text,          "Time"},
  {ID3FID_CONTENTGROUP,      "TT1", "TIT1", false, false, ID3FD_Text,          "Content group description"},
  {ID3FID_TITLE,             "TT2", "TIT2", false, false, ID3FD_Text,          "Title/songname/content description"},
  {ID3FID_SUBTITLE,          "TT3", "TIT3", false, false, ID3FD_Text,          "Subtitle/Description refinement"},
  {ID3FID_INITIALKEY,        "TKE", "TKEY", false, false, ID3FD_Text,          "Initial key"},
  {ID3FID_LANGUAGE,          "TLA", "TLAN", false, false, ID3FD_Text,          "Language(s)"},
  {ID3FID_SONGLEN,           "TLE", "TLEN", false, true,  ID3FD_Text,          "Length"},
  {ID3FID_MEDIATYPE,         "TMT", "TMED", false, false, ID3FD_Text,          "Media type"},
  {ID3FID_ORIGALBUM,         "TOT", "TOAL", false, false, ID3FD_Text,          "Original album/movie/show title"},
  {ID3FID_ORIGFILENAME,      "TOF", "TOFN", false, false, ID3FD_Text,          "Original filename"},
  {ID3FID_ORIGLYRICIST,      "TOL", "TOLY", false, false, ID3FD_Text,          "Original lyricist(s)/text writer(s)"},
  {ID3FID_ORIGARTIST,        "TOA", "TOPE", false, false, ID3FD_Text,          "Original artist(s)/performer(s)"},
  {ID3FID_ORIGYEAR,          "TOR", "TORY", false, false, ID3FD_Text,          "Original release year"},
  {ID3FID_FILEOWNER,         ""   , "TOWN", false, false, ID3FD_Text,          "File owner/licensee"},
  {ID3FID_LEADARTIST,        "TP1", "TPE1", false, false, ID3FD_Text,          "Lead performer(s)/Soloist(s)"},
  {ID3FID_BAND,              "TP2", "TPE2", false, false, ID3FD_Text,          "Band/orchestra/accompaniment"},
  {ID3FID_CONDUCTOR,         "TP3", "TPE3", false, false, ID3FD_Text,          "Conductor/performer refinement"},
  {ID3FID_MIXARTIST,         "TP4", "TPE4", false, false, ID3FD_Text,          "Interpreted, remixed, or otherwise modified by"},
  {ID3FID_PARTINSET,         "TPA", "TPOS", false, false, ID3FD_Text,          "Part of a set"},
  {ID3FID_PUBLISHER,         "TPB", "TPUB", false, false, ID3FD_Text,          "Publisher"},
  {ID3FID_TRACKNUM,          "TRK", "TRCK", false, false, ID3FD_Text,          "Track number/Position in set"},
  {ID3FID_RECORDINGDATES,    "TRD", "TRDA", false, false, ID3FD_Text,          "Recording dates"},
  {ID3FID_NETRADIOSTATION,   "TRN", "TRSN", false, false, ID3FD_Text,          "Internet radio station name"},
  {ID3FID_NETRADIOOWNER,     "TRO", "TRSO", false, false, ID3FD_Text,          "Internet radio station owner"},
  {ID3FID_SIZE,              "TSI", "TSIZ", false, true,  ID3FD_Text,          "Size"},
  {ID3FID_ISRC,              "TRC", "TSRC", false, false, ID3FD_Text,          "ISRC (international standard recording code)"},
  {ID3FID_ENCODERSETTINGS,   "TSS", "TSSE", false, false, ID3FD_Text,          "Software/Hardware and settings used for encoding"},
  {ID3FID_USERTEXT,          "TXX", "TXXX", false, false, ID3FD_UserText,      "User defined text information"},
  {ID3FID_YEAR,              "TYE", "TYER", false, false, ID3FD_Text,          "Year"},
  {ID3FID_UNIQUEFILEID,      "UFI", "UFID", false, false, ID3FD_UFI,           "Unique file identifier"},
  {ID3FID_TERMSOFUSE,        ""   , "USER", false, false, ID3FD_TermsOfUse,    "Terms of use"},
  {ID3FID_UNSYNCEDLYRICS,    "ULT", "USLT", false, false, ID3FD_GeneralText,   "Unsynchronized lyric/text transcription"},
  {ID3FID_WWWCOMMERCIALINFO, "WCM", "WCOM", false, false, ID3FD_URL,           "Commercial information"},
  {ID3FID_WWWCOPYRIGHT,      "WCP", "WCOP", false, false, ID3FD_URL,           "Copyright/Legal infromation"},
  {ID3FID_WWWAUDIOFILE,      "WAF", "WOAF", false, false, ID3FD_URL,           "Official audio file webpage"},
  {ID3FID_WWWARTIST,         "WAR", "WOAR", false, false, ID3FD_URL,           "Official artist/performer webpage"},
  {ID3FID_WWWAUDIOSOURCE,    "WAS", "WOAS", false, false, ID3FD_URL,           "Official audio source webpage"},
  {ID3FID_WWWRADIOPAGE,      "WRA", "WORS", false, false, ID3FD_URL,           "Official internet radio station homepage"},
  {ID3FID_WWWPAYMENT,        "WPY", "WPAY", false, false, ID3FD_URL,           "Payment"},
  {ID3FID_WWWPUBLISHER,      "WPB", "WPUB", false, false, ID3FD_URL,           "Official publisher webpage"},
  {ID3FID_WWWUSER,           "WXX", "WXXX", false, false, ID3FD_UserURL,       "User defined URL link"},
  {ID3FID_METACRYPTO,        "CRM", ""    , false, false, ID3FD_Unimplemented, "Encrypted meta frame"},
  {ID3FID_METACOMPRESSION,   "CDM", ""    , false, false, ID3FD_CDM,           "Compressed data meta frame"},
  {ID3FID_NOFRAME}
};

/** \class ID3_Field field.h id3/field.h
 ** \brief The representative class of an ID3v2 field.
 **
 ** As a general rule, you need never create an object of this type.  id3lib
 ** uses them internally as part of the id3_frame class.  You must know how to
 ** interact with these objects, though, and that's what this section is about.
 **
 ** The ID3_Field contains many overloaded methods to provide these facilities
 ** for four different data types: integers, ASCII strings, Unicode strings,
 ** and binary data.
 **
 ** An integer field supports the Get(), Set(uint32), and operator=(uint32)
 ** methods.
 **
 ** Both types of strings support the GetNumTextItems() method.
 **
 ** An ASCII string field supports the Get(char*, size_t, size_t)),
 ** Set(const char*), Add(const char*), and operator=(const char*) methods.
 **
 ** A Unicode field also supports Get(unicode_t*, size_t, size_t),
 ** Set(const unicode_t*), Add(const unicode_t*), and
 ** operator=(const unicode_t*).  Without elaborating, the Unicode
 ** methods behave exactly the same as their ASCII counterparts, taking
 ** \c unicode_t pointers in place of \c char pointers.
 **
 ** All strings in id3lib are handled internally as Unicode.  This means that
 ** when you set a field with an ASCII source type, it will be converted and
 ** stored internally as a Unicode string.  id3lib will handle all necessary
 ** conversions when parsing, rendering, and retrieving.  If you set a field as
 ** an ASCII string, then try to read the string into a \c unicode_t buffer,
 ** id3lib will automatically convert the string into Unicode so this will
 ** function as expected.  The same holds true in reverse.  Of course, when
 ** converting from Unicode to ASCII, you will experience problems when the
 ** Unicode string contains characters that don't map to ISO-8859-1.
 **
 ** A binary field supports the Get(uchar*, size_t), Set(const uchar*, size_t),
 ** FromFile(const char*), and ToFile(const char*) methods.  The binary field
 ** holds miscellaneous data that can't easily be described any other way, such
 ** as a JPEG image.
 **
 ** As a general implementation note, you should be prepared to support all
 ** fields in an id3lib frame, even if all fields in the id3lib version of the
 ** frame aren't present in the id3v2 version.  This is because of frames like
 ** the picture frame, which changed slightly from one version of the id3v2
 ** standard to the next (the IMAGEFORMAT format in 2.0 changed to a MIMETYPE
 ** in 3.0).  If you support all id3lib fields in a given frame, id3lib can
 ** generate the correct id3v2 frame for the id3v2 version you wish to support.
 ** Alternatively, just support the fields you know will be used in, say, 3.0
 ** if you only plan to generate 3.0 tags.
 **
 ** @author Dirk Mahoney
 ** @version $Id: field.cpp,v 1.47 2002/11/03 00:41:27 t1mpy Exp $
 ** \sa ID3_Tag
 ** \sa ID3_Frame
 ** \sa ID3_Err
 **/

ID3_FieldImpl::ID3_FieldImpl()
  : _id(ID3FN_NOFIELD),
    _type(ID3FTY_INTEGER),
    _spec_begin(ID3V2_EARLIEST),
    _spec_end(ID3V2_LATEST),
    _flags(0),
    _changed(false),
    _fixed_size(0),
    _num_items(0),
    _enc(ID3TE_NONE)
{
  this->Clear();
}

ID3_FieldImpl::ID3_FieldImpl(const ID3_FieldDef& def)
  : _id(def._id),
    _type(def._type),
    _spec_begin(def._spec_begin),
    _spec_end(def._spec_end),
    _flags(def._flags),
    _changed(false),
    _fixed_size(def._fixed_size),
    _num_items(0),
    _enc((_type == ID3FTY_TEXTSTRING) ? ID3TE_ASCII : ID3TE_NONE)
{
  this->Clear();
}

ID3_FieldImpl::~ID3_FieldImpl()
{
}

/** Clears any data and frees any memory associated with the field
 **
 ** \sa ID3_Tag::Clear()
 ** \sa ID3_Frame::Clear()
 **/
void ID3_FieldImpl::Clear()
{
  switch (_type)
  {
    case ID3FTY_INTEGER:
    {
      _integer = 0;
      break;
    }
    case ID3FTY_BINARY:
    {
      _binary.erase();
      if (_fixed_size > 0)
      {
        _binary.assign(_fixed_size, '\0');
      }
      break;
    }
    case ID3FTY_TEXTSTRING:
    {
      _text.erase();
      if (_fixed_size > 0)
      {
        if (this->GetEncoding() == ID3TE_UNICODE)
        {
          _text.assign(_fixed_size * 2, '\0');
        }
        else if (this->GetEncoding() == ID3TE_ASCII)
        {
          _text.assign(_fixed_size, '\0');
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }
  _changed    = true;

  return ;
}

bool
ID3_FieldImpl::HasChanged() const
{
  return _changed;
}

/** \fn size_t ID3_Field::Size() const
 ** \brief Returns the size of a field.
 **
 ** The value returned is dependent on the type of the field.  For ASCII
 ** strings, this returns the number of characters in the field, not including
 ** any NULL-terminator.  The same holds true for Unicode---it returns the
 ** number of characters in the field, not bytes, and this does not include
 ** the Unicode BOM, which isn't put in a Unicode string obtained by the
 ** Get(unicode_t*, size_t, size_t) method anyway.  For binary and
 ** integer fields, this returns the number of bytes in the field.
 **
 ** \code
 **   size_t howBig = myFrame.GetField(ID3FN_DATA)->Size();
 ** \endcode
 **
 ** \return The size of the field, either in bytes (for binary or integer
 **         fields) or characters (for strings).
 **/

size_t ID3_FieldImpl::BinSize() const
{
  if (_fixed_size > 0)
  {
    return _fixed_size;
  }
  size_t size = this->Size();
  if (_type == ID3FTY_TEXTSTRING)
  {
    ID3_TextEnc enc = this->GetEncoding();
    if (enc == ID3TE_UNICODE && size > 0)
    {
      size++;
    }
    if (_flags & ID3FF_CSTR)
    {
      size++;
    }
    if (enc == ID3TE_UNICODE)
    {
      size *= 2;
    }
  }
  return size;
}

size_t ID3_FieldImpl::Size() const
{
  size_t size = 0;
  // check to see if we are within the legal limit for this field 0 means
  // arbitrary length field
  if (_fixed_size > 0)
  {
    size = _fixed_size;
  }
  else if (_type == ID3FTY_INTEGER)
  {
    size = sizeof(uint32);
  }
  else if (_type == ID3FTY_TEXTSTRING)
  {
    size = _text.size();
  }
  else
  {
    size = _binary.size();
  }

  return size;
}

bool ID3_FieldImpl::Parse(ID3_Reader& reader)
{
  bool success = false;
  switch (this->GetType())
  {
    case ID3FTY_INTEGER:
    {
      success = this->ParseInteger(reader);
      break;
    }

    case ID3FTY_BINARY:
    {
      success = this->ParseBinary(reader);
      break;
    }

    case ID3FTY_TEXTSTRING:
    {
      success = this->ParseText(reader);
      break;
    }

    default:
    {
      ID3D_WARNING( "ID3_FieldImpl::Parse(): unknown field type" );
      break;
    }
  }
  return success;
}

ID3_FrameDef* ID3_FindFrameDef(ID3_FrameID id)
{
  ID3_FrameDef  *info   = NULL;

  for (size_t cur = 0; ID3_FrameDefs[cur].eID != ID3FID_NOFRAME; ++cur)
  {
    if (ID3_FrameDefs[cur].eID == id)
    {
      info = &ID3_FrameDefs[cur];
      break;
    }
  }

  return info;
}

ID3_FrameID
ID3_FindFrameID(const char *id)
{
  ID3_FrameID fid = ID3FID_NOFRAME;
  const int slen = strlen(id);

  for (size_t cur = 0; ID3_FrameDefs[cur].eID != ID3FID_NOFRAME; ++cur)
  {
    if (((strcmp(ID3_FrameDefs[cur].sShortTextID, id) == 0) &&
         slen == 3) ||
        ((strcmp(ID3_FrameDefs[cur].sLongTextID,  id) == 0) &&
         slen == 4))
    {
      fid = ID3_FrameDefs[cur].eID;
      break;
    }
  }

  return fid;
}

void ID3_FieldImpl::Render(ID3_Writer& writer) const
{
  switch (this->GetType())
  {
    case ID3FTY_INTEGER:
    {
      RenderInteger(writer);
      break;
    }

    case ID3FTY_BINARY:
    {
      RenderBinary(writer);
      break;
    }

    case ID3FTY_TEXTSTRING:
    {
      RenderText(writer);
      break;
    }

    default:
    {
      ID3D_WARNING ( "ID3D_FieldImpl::Render(): unknown field type" );
      break;
    }
  }
}

ID3_Field &
ID3_FieldImpl::operator=( const ID3_Field &rhs )
{
  const ID3_FieldImpl* fld = (const ID3_FieldImpl*) &rhs;
  if (this != &rhs && this->GetType() == fld->GetType())
  {
    switch (fld->GetType())
    {
      case ID3FTY_INTEGER:
      {
        this->SetInteger(fld->GetInteger());
        break;
      }
      case ID3FTY_TEXTSTRING:
      {
        this->SetEncoding(fld->GetEncoding());
        this->SetText(fld->GetText());
        break;
      }
      case ID3FTY_BINARY:
      {
        this->SetBinary(fld->GetBinary());
        break;
      }
      default:
      {
        break;
      }
    }
  }
  return *this;
}

bool ID3_FieldImpl::SetEncoding(ID3_TextEnc enc)
{
  bool changed = this->IsEncodable() && (enc != this->GetEncoding()) &&
    (ID3TE_NONE < enc && enc < ID3TE_NUMENCODINGS);
  if (changed)
  {
    _text = convert(_text, _enc, enc);
    _enc = enc;
    _changed = true;
  }
  return changed;
}

/** \class ID3_FrameInfo field.h id3/field.h
 ** \brief Provides information about the frame and field types supported by id3lib
 **
 ** You normally only need (at most) one instance of the ID3_FrameInfo.  It
 ** has no member data -- only methods which provide information about the
 ** frame types (and their component fields) supported by id3lib as defined
 ** in field.cpp .
 **
 ** Usage is straightforward.  The following function uses ID3_FrameInfo
 ** to display a summary of all the frames known to id3lib:
 ** \code
 **
 ** void ShowKnownFrameInfo {
 **   ID3_FrameInfo myFrameInfo;
 **   for (int cur = ID3FID_NOFRAME+1; cur <= myFrameInfo.MaxFrameID(); cur ++)
 **   {
 **     cout << "Short ID: " << myFrameInfo.ShortName(ID3_FrameID(cur)) <<
 **     " Long ID: " << myFrameInfo.LongName(ID3_FrameID(cur)) <<
 **     " Desription: " << myFrameInfo.Description(ID3_FrameID(cur)) << endl;
 **   }
 ** }
 ** \endcode
 **
 ** Functions are also provided to glean more information about the individual
 ** fields which make up any given frame type.  The following for() loop,
 ** embedded into the previous for() loop would provide a raw look at such
 ** information.  Realize, of course, that the field type is meaningless
 ** when printed.  Only when it is taken in the context of the ID3_FieldType enum
 ** does it take on any meaningful significance.
 **
 ** \code
 **  for (int cur = ID3FID_NOFRAME+1; cur <= fi.MaxFrameID(); cur ++)
 **  {
 **    int numfields = fi.NumFields(ID3_FrameID(cur));
 **
 **    cout << "ID: " << fi.LongName(ID3_FrameID(cur)) <<
 **    " FIELDS: " << numfields << endl;
 **    for(int i=0;i<numfields;i++) {
 **      cout << "TYPE: " << fi.FieldType(ID3_FrameID(cur),i) <<
 **      " SIZE: " << fi.FieldSize(ID3_FrameID(cur),i) <<
 **      " FLAGS: " << fi.FieldFlags(ID3_FrameID(cur),i) << endl;
 **
 **    }
 **
 **    cout << endl;
 **
 **  }
 ** \endcode
 **
 ** @author Cedric Tefft
 ** @version $Id: field.cpp,v 1.47 2002/11/03 00:41:27 t1mpy Exp $
 **/


char *ID3_FrameInfo::ShortName(ID3_FrameID frameID)
{
  ID3_FrameDef *pFD = ID3_FindFrameDef(frameID);
  if (pFD!=NULL)
    return pFD->sShortTextID;
  else
    return NULL;
}

char *ID3_FrameInfo::LongName(ID3_FrameID frameID)
{
  ID3_FrameDef *pFD = ID3_FindFrameDef(frameID);
  if (pFD!=NULL)
    return pFD->sLongTextID;
  else
    return NULL;
}

const char *ID3_FrameInfo::Description(ID3_FrameID frameID)
{
  ID3_FrameDef *pFD = ID3_FindFrameDef(frameID);
  if (pFD!=NULL)
    return pFD->sDescription;
  else
    return NULL;
}

int ID3_FrameInfo::MaxFrameID()
{
  return ID3FID_LASTFRAMEID-1;
}

int ID3_FrameInfo::NumFields(ID3_FrameID frameID)
{
  int fieldnum=0;
  ID3_FrameDef *pFD = ID3_FindFrameDef(frameID);
  if (pFD!=NULL)
  {
    while (pFD->aeFieldDefs[fieldnum]._id != ID3FN_NOFIELD)
    {
      ++fieldnum;
    }
  }
  return fieldnum;
}

ID3_FieldType ID3_FrameInfo::FieldType(ID3_FrameID frameID, int fieldnum)
{
  ID3_FrameDef *pFD = ID3_FindFrameDef(frameID);
  if (pFD!=NULL)
    return (pFD->aeFieldDefs[fieldnum]._type);
  else
    return ID3FTY_NONE;
}

size_t ID3_FrameInfo::FieldSize(ID3_FrameID frameID, int fieldnum)
{
  ID3_FrameDef *pFD = ID3_FindFrameDef(frameID);
  if (pFD!=NULL)
    return (pFD->aeFieldDefs[fieldnum]._fixed_size);
  else
    return 0;
}

flags_t ID3_FrameInfo::FieldFlags(ID3_FrameID frameID, int fieldnum)
{
  ID3_FrameDef *pFD = ID3_FindFrameDef(frameID);
  if (pFD!=NULL)
    return (pFD->aeFieldDefs[fieldnum]._flags);
  else
    return 0;
}

