// -*- C++ -*-
// $Id: flags.h,v 1.1 2000/10/24 16:22:47 eldamitri Exp $

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

#ifndef _ID3LIB_FLAGS_H_
#define _ID3LIB_FLAGS_H_

#include "flags.h"

class ID3_Flags
{
public:
  typedef flags_t TYPE;

  ID3_Flags() : _f(0) { ; }
  virtual ~ID3_Flags() { ; }

  TYPE   get() const         { return _f; }
  bool   test(TYPE f) const { return (this->get() & f) == f; }
  bool   set(TYPE f)        { bool r = (_f != f); _f = f; return r; }
  bool   add(TYPE f)        { return this->set(this->get() | f); }
  bool   remove(TYPE f)     { return this->set(this->get() & ~f); }
  bool   clear()             { return this->set(0); }
  bool   set(TYPE f, bool b){ if (b) return this->add(f); return this->remove(f); }

  ID3_Flags& operator=(const ID3_Flags& f)
  { if (this != &f) { this->set(f.get()); } return *this; }

private:
  TYPE _f;
};

#endif /* _ID3LIB_FLAGS_H_ */
