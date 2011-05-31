// $Id: spec.cpp,v 1.2 2002/07/31 13:20:49 t1mpy Exp $

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

#if defined HAVE_CONFIG_H
#include <config.h>
#endif

#include "spec.h"

ID3_V2Spec ID3_VerRevToV2Spec(uchar ver, uchar rev)
{
  ID3_V2Spec spec = ID3V2_UNKNOWN;
  if (2 == ver)
  {
    if (0 == rev)
    {
      spec = ID3V2_2_0;
    }
    else if (1 == rev)
    {
      spec = ID3V2_2_1;
    }
  } 
  else if (3 == ver)
  {
    if (0 == rev)
    {
      spec = ID3V2_3_0;
    }
  }
  else if (4 == ver)
  {
    if (0 == rev)
    {
      spec = ID3V2_4_0;
    }
  }

  return spec;
}

uchar ID3_V2SpecToVer(ID3_V2Spec spec)
{
  uchar ver = 0;
  switch (spec)
  {
    case ID3V2_2_0:
    case ID3V2_2_1:
      ver = 2;
      break;
    case ID3V2_3_0:
      ver = 3;
      break;
    case ID3V2_4_0:
      ver = 4;
      break;
    default:
      break;
  }
  return ver;
}

uchar ID3_V2SpecToRev(ID3_V2Spec spec)
{
  uchar rev = 0;
  switch (spec)
  {
    case ID3V2_4_0:
      rev = 0;
      break;
    case ID3V2_3_0:
      rev = 0;
      break;
    case ID3V2_2_1:
      rev = 1;
      break;
    case ID3V2_2_0:
      rev = 0;
      break;
    default:
      break;
  }
  return rev;
}

