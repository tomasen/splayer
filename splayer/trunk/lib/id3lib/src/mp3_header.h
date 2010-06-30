// -*- C++ -*-
// $Id: mp3_header.h,v 1.4 2002/11/02 17:48:51 t1mpy Exp $

// id3lib: a C++ library for creating and manipulating id3v1/v2 tags
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

#ifndef _MP3_HEADER_H_
#define _MP3_HEADER_H_

#include "io_decorators.h" //has "readers.h" "io_helpers.h" "utils.h"

class Mp3Info
{
public:
  Mp3Info() { _mp3_header_output = new Mp3_Headerinfo; };
  ~Mp3Info() { this->Clean(); };
  void Clean();

  const Mp3_Headerinfo* GetMp3HeaderInfo() const { return _mp3_header_output; };
  bool Parse(ID3_Reader&, size_t mp3size);

  Mpeg_Layers Layer() const { return _mp3_header_output->layer; };
  Mpeg_Version Version() const { return _mp3_header_output->version; };
  MP3_BitRates Bitrate() const { return _mp3_header_output->bitrate; };
  Mp3_ChannelMode ChannelMode() const { return _mp3_header_output->channelmode; };
  Mp3_ModeExt ModeExt() const { return _mp3_header_output->modeext; };
  Mp3_Emphasis Emphasis() const { return _mp3_header_output->emphasis; };
  Mp3_Crc Crc() const { return _mp3_header_output->crc; };
  uint32 VbrBitrate() const { return _mp3_header_output->vbr_bitrate; };
  uint32 Frequency() const { return _mp3_header_output->frequency; };
  uint32 Framesize() const { return _mp3_header_output->framesize; };
  uint32 Frames() const { return _mp3_header_output->frames; };
  bool Private() const { return _mp3_header_output->privatebit; };
  bool Copyrighted() const { return _mp3_header_output->copyrighted; };
  bool Original() const { return _mp3_header_output->original; };
  uint32 Seconds() const { return _mp3_header_output->time; };

private:

  struct _mp3_header_internal //http://www.mp3-tech.org/programmer/frame_header.html
  {
//byte 1
    unsigned char frame_sync_a : 8; /* all bits should be set */
//byte 2
    unsigned char protection_bit : 1;
    unsigned char layer : 2;
    unsigned char id : 2;
    unsigned char frame_sync_b : 3; /* all bits should be set */
//byte 3
    unsigned char private_bit : 1;
    unsigned char padding_bit : 1;
    unsigned char frequency : 2;
    unsigned char bitrate_index : 4;
//byte 4
    unsigned char emphasis : 2;
    unsigned char original : 1;
    unsigned char copyright : 1;
    unsigned char mode_ext : 2;//only used in joint stereo
    unsigned char mode : 2;
  };

  Mp3_Headerinfo* _mp3_header_output;
}; //Info

#endif /* _MP3_HEADER_H_ */

