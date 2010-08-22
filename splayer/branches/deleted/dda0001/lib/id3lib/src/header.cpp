// $Id: header.cpp,v 1.11 2002/07/31 13:20:49 t1mpy Exp $

// id3lib: a C++ library for creating and manipulating id3v1/v2 tags
// Copyright 1999, 2000  Scott Thomas Haug
// Copyright 2002  Thijmen Klok (thijmen@id3lib.org)

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

#include "header.h"

#if defined HAVE_CONFIG_H
#include <config.h>
#endif

bool ID3_Header::SetSpec(ID3_V2Spec spec)
{
  static ID3_Header::Info _spec_info[] =
  {
  // Warning, EXT SIZE are minimum sizes, they can be bigger
  // SIZEOF SIZEOF SIZEOF IS EXT EXT  EXPERIM
  // FRID   FRSZ   FRFL   HEADER SIZE BIT
    {  3,     3,     0,     false, 0,   false }, // ID3V2_2_0
    {  3,     3,     0,     true,  8,   true  }, // ID3V2_2_1
    {  4,     4,     2,     false, 10,  false }, // ID3V2_3_0
    {  4,     4,     2,     false, 6,   false }  // ID3V2_4_0
  };
  
  bool changed = false;
  if (spec < ID3V2_EARLIEST || spec > ID3V2_LATEST)
  {
    changed = _spec != ID3V2_UNKNOWN;
    _spec = ID3V2_UNKNOWN;
    _info = NULL;
  }
  else
  {
    changed = _spec != spec;
    _spec = spec;
    _info = &_spec_info[_spec - ID3V2_EARLIEST];
  }
  _changed = _changed || changed;
  return changed;
}

