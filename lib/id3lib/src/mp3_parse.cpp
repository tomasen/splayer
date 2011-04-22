// -*- C++ -*-
// $Id: mp3_parse.cpp,v 1.6 2002/11/02 17:48:51 t1mpy Exp $

// id3lib: a C++ library for creating and manipulating id3v1/v2 tags
// Copyright 2002, Thijmen Klok (thijmen@id3lib.org)

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

#include "mp3_header.h"

#define FRAMES_FLAG     0x0001
#define BYTES_FLAG      0x0002
#define TOC_FLAG        0x0004
#define SCALE_FLAG      0x0008

static int ExtractI4(unsigned char *buf)
{
  int x;
  // big endian extract

  x = buf[0];
  x <<= 8;
  x |= buf[1];
  x <<= 8;
  x |= buf[2];
  x <<= 8;
  x |= buf[3];

  return x;
}

uint32 fto_nearest_i(float f)
{
  uint32 i;

  i = (uint32)f;
  if (i < f)
  {
    f -= i;
      if (f >= 0.5)
      return i+1;
    else
      return i;
  }
  else
    return i;
}

uint16 calcCRC(char *pFrame, size_t audiodatasize)
{
  size_t icounter;
  int tmpchar, crcmask, tmpi;
  uint16 crc = 0xffff;

  for (icounter = 2;  icounter < audiodatasize;  ++icounter)
  {
    if (icounter != 4  &&  icounter != 5) //skip the 2 chars of the crc itself
    {
      crcmask = 1 << 8;
      tmpchar = pFrame[icounter];
      while (crcmask >>= 1)
      {
        tmpi = crc & 0x8000;
        crc <<= 1;
        if (!tmpi ^ !(tmpchar & crcmask))
          crc ^= 0x8005;
      }
    }
  }
  crc &= 0xffff;
  return crc;
}

void Mp3Info::Clean()
{
  if (_mp3_header_output != NULL)
    delete _mp3_header_output;
  _mp3_header_output = NULL;
}

using namespace dami;

bool Mp3Info::Parse(ID3_Reader& reader, size_t mp3size)
{
  MP3_BitRates _mp3_bitrates[2][3][16] =
  {
    {
      { //MPEG 1, LAYER I
        MP3BITRATE_NONE,
        MP3BITRATE_32K,
        MP3BITRATE_64K,
        MP3BITRATE_96K,
        MP3BITRATE_128K,
        MP3BITRATE_160K,
        MP3BITRATE_192K,
        MP3BITRATE_224K,
        MP3BITRATE_256K,
        MP3BITRATE_288K,
        MP3BITRATE_320K,
        MP3BITRATE_352K,
        MP3BITRATE_384K,
        MP3BITRATE_416K,
        MP3BITRATE_448K,
        MP3BITRATE_FALSE
      },
      { //MPEG 1, LAYER II
        MP3BITRATE_NONE,
        MP3BITRATE_32K,
        MP3BITRATE_48K,
        MP3BITRATE_56K,
        MP3BITRATE_64K,
        MP3BITRATE_80K,
        MP3BITRATE_96K,
        MP3BITRATE_112K,
        MP3BITRATE_128K,
        MP3BITRATE_160K,
        MP3BITRATE_192K,
        MP3BITRATE_224K,
        MP3BITRATE_256K,
        MP3BITRATE_320K,
        MP3BITRATE_384K,
        MP3BITRATE_FALSE
      },
      { //MPEG 1, LAYER III
        MP3BITRATE_NONE,
        MP3BITRATE_32K,
        MP3BITRATE_40K,
        MP3BITRATE_48K,
        MP3BITRATE_56K,
        MP3BITRATE_64K,
        MP3BITRATE_80K,
        MP3BITRATE_96K,
        MP3BITRATE_112K,
        MP3BITRATE_128K,
        MP3BITRATE_160K,
        MP3BITRATE_192K,
        MP3BITRATE_224K,
        MP3BITRATE_256K,
        MP3BITRATE_320K,
        MP3BITRATE_FALSE
      }
    },
    {
      { //MPEG 2 or 2.5, LAYER I
        MP3BITRATE_NONE,
        MP3BITRATE_32K,
        MP3BITRATE_48K,
        MP3BITRATE_56K,
        MP3BITRATE_64K,
        MP3BITRATE_80K,
        MP3BITRATE_96K,
        MP3BITRATE_112K,
        MP3BITRATE_128K,
        MP3BITRATE_144K,
        MP3BITRATE_160K,
        MP3BITRATE_176K,
        MP3BITRATE_192K,
        MP3BITRATE_224K,
        MP3BITRATE_256K,
        MP3BITRATE_FALSE
      },
      { //MPEG 2 or 2.5, LAYER II
        MP3BITRATE_NONE,
        MP3BITRATE_8K,
        MP3BITRATE_16K,
        MP3BITRATE_24K,
        MP3BITRATE_32K,
        MP3BITRATE_40K,
        MP3BITRATE_48K,
        MP3BITRATE_56K,
        MP3BITRATE_64K,
        MP3BITRATE_80K,
        MP3BITRATE_96K,
        MP3BITRATE_112K,
        MP3BITRATE_128K,
        MP3BITRATE_144K,
        MP3BITRATE_160K,
        MP3BITRATE_FALSE
      },
      { //MPEG 2 or 2.5, LAYER III
        MP3BITRATE_NONE,
        MP3BITRATE_8K,
        MP3BITRATE_16K,
        MP3BITRATE_24K,
        MP3BITRATE_32K,
        MP3BITRATE_40K,
        MP3BITRATE_48K,
        MP3BITRATE_56K,
        MP3BITRATE_64K,
        MP3BITRATE_80K,
        MP3BITRATE_96K,
        MP3BITRATE_112K,
        MP3BITRATE_128K,
        MP3BITRATE_144K,
        MP3BITRATE_160K,
        MP3BITRATE_FALSE
      }
    }
  };

  Mp3_Frequencies _mp3_frequencies[4][4] =
  {
    { MP3FREQUENCIES_11025HZ, MP3FREQUENCIES_12000HZ, MP3FREQUENCIES_8000HZ,MP3FREQUENCIES_Reserved },  //MPEGVERSION_2_5
    { MP3FREQUENCIES_Reserved, MP3FREQUENCIES_Reserved, MP3FREQUENCIES_Reserved, MP3FREQUENCIES_Reserved},          //MPEGVERSION_Reserved
    { MP3FREQUENCIES_22050HZ, MP3FREQUENCIES_24000HZ, MP3FREQUENCIES_16000HZ, MP3FREQUENCIES_Reserved }, //MPEGVERSION_2
    { MP3FREQUENCIES_44100HZ, MP3FREQUENCIES_48000HZ, MP3FREQUENCIES_32000HZ, MP3FREQUENCIES_Reserved }  //MPEGVERSION_1
  };

  _mp3_header_internal *_tmpheader;

  const size_t HEADERSIZE = 4;//
  char buf[HEADERSIZE+1]; //+1 to hold the \0 char
  ID3_Reader::pos_type beg = reader.getCur() ;
  ID3_Reader::pos_type end = beg + HEADERSIZE ;
  reader.setCur(beg);
  int bitrate_index;

  _mp3_header_output->layer = MPEGLAYER_FALSE;
  _mp3_header_output->version = MPEGVERSION_FALSE;
  _mp3_header_output->bitrate = MP3BITRATE_FALSE;
  _mp3_header_output->channelmode = MP3CHANNELMODE_FALSE;
  _mp3_header_output->modeext = MP3MODEEXT_FALSE;
  _mp3_header_output->emphasis = MP3EMPHASIS_FALSE;
  _mp3_header_output->crc = MP3CRC_MISMATCH;
  _mp3_header_output->frequency = 0;
  _mp3_header_output->framesize = 0;
  _mp3_header_output->frames = 0;
  _mp3_header_output->time = 0;
  _mp3_header_output->vbr_bitrate = 0;

  reader.readChars(buf, HEADERSIZE);
  buf[HEADERSIZE]='\0';
  // copy the pointer to the struct

  if (((buf[0] & 0xFF) != 0xFF) || ((buf[1] & 0xE0) != 0xE0)) //first 11 bits should be 1
  {
    this->Clean();
    return false;
  }

  _tmpheader = reinterpret_cast<_mp3_header_internal *>(buf);

  bitrate_index = 0;
  switch (_tmpheader->id)
  {
    case 3:
      _mp3_header_output->version = MPEGVERSION_1;
      bitrate_index = 0;
      break;
    case 2:
      _mp3_header_output->version = MPEGVERSION_2;
      bitrate_index = 1;
      break;
    case 1:
      this->Clean();
      return false; //wouldn't know how to handle it
      break;
    case 0:
      _mp3_header_output->version = MPEGVERSION_2_5;
      bitrate_index = 1;
      break;
    default:
      this->Clean();
      return false;
      break;
  };

  switch (_tmpheader->layer)
  {
    case 3:
      _mp3_header_output->layer = MPEGLAYER_I;
      break;
    case 2:
      _mp3_header_output->layer = MPEGLAYER_II;
      break;
    case 1:
      _mp3_header_output->layer = MPEGLAYER_III;
      break;
    case 0:
      this->Clean();
      return false; //wouldn't know how to handle it
      break;
    default:
      this->Clean();
      return false; //how can two unsigned bits be something else??
      break;
  };

  // mpegversion, layer and bitrate are all valid
  _mp3_header_output->bitrate = _mp3_bitrates[bitrate_index][3-_tmpheader->layer][_tmpheader->bitrate_index];
  if (_mp3_header_output->bitrate == MP3BITRATE_FALSE)
  {
    this->Clean();
    return false;
  }
  _mp3_header_output->frequency = _mp3_frequencies[_tmpheader->id][_tmpheader->frequency];
  if (_mp3_header_output->frequency == MP3FREQUENCIES_Reserved)
  {
    this->Clean();
    return false;
  }

  _mp3_header_output->privatebit = (bool)_tmpheader->private_bit;
  _mp3_header_output->copyrighted = (bool)_tmpheader->copyright;
  _mp3_header_output->original = (bool)_tmpheader->original;
  _mp3_header_output->crc = (Mp3_Crc)!(bool)_tmpheader->protection_bit;

  switch (_tmpheader->mode)
  {
  case 3:
    _mp3_header_output->channelmode = MP3CHANNELMODE_SINGLE_CHANNEL;
    break;
  case 2:
    _mp3_header_output->channelmode = MP3CHANNELMODE_DUAL_CHANNEL;
    break;
  case 1:
    _mp3_header_output->channelmode = MP3CHANNELMODE_JOINT_STEREO;
    break;
  case 0:
    _mp3_header_output->channelmode = MP3CHANNELMODE_STEREO;
    break;
  default:
    this->Clean();
    return false; //wouldn't know how to handle it
    break;
  }

  if (_mp3_header_output->channelmode == MP3CHANNELMODE_JOINT_STEREO)
  {
    // these have a different meaning for different layers, better give them a generic name in the enum
    switch (_tmpheader->mode_ext)
    {
    case 3:
      _mp3_header_output->modeext = MP3MODEEXT_3;
      break;
    case 2:
      _mp3_header_output->modeext = MP3MODEEXT_2;
      break;
    case 1:
      _mp3_header_output->modeext = MP3MODEEXT_1;
      break;
    case 0:
      _mp3_header_output->modeext = MP3MODEEXT_0;
      break;
    default:
      this->Clean();
      return false; //wouldn't know how to handle it
      break;
    }
  }
  else //it's valid to have a valid false one in this case, since it's only used with joint stereo
    _mp3_header_output->modeext = MP3MODEEXT_FALSE;

  switch (_tmpheader->emphasis)
  {
  case 3:
    _mp3_header_output->emphasis = MP3EMPHASIS_CCIT_J17;
    break;
  case 2:
    _mp3_header_output->emphasis = MP3EMPHASIS_Reserved;
    break;
  case 1:
    _mp3_header_output->emphasis = MP3EMPHASIS_50_15MS;
    break;
  case 0:
    _mp3_header_output->emphasis = MP3EMPHASIS_NONE;
    break;
  default:
    this->Clean();
    return false; //wouldn't know how to handle it
    break;
  }

//http://www.mp3-tech.org/programmer/frame_header.html
  if (_mp3_header_output->bitrate != MP3BITRATE_NONE && _mp3_header_output->frequency > 0)
  {

    switch(_mp3_header_output->layer)
    {
      case MPEGLAYER_I: // Layer 1
        _mp3_header_output->framesize = 4 * (12 * _mp3_header_output->bitrate / _mp3_header_output->frequency + (_tmpheader->padding_bit ? 1 : 0));
        break;
      case MPEGLAYER_II: // Layer 2
        _mp3_header_output->framesize = 144 * _mp3_header_output->bitrate / _mp3_header_output->frequency + (_tmpheader->padding_bit ? 1 : 0);
        break;
      case MPEGLAYER_III: // Layer 3
        if(_mp3_header_output->version == MPEGVERSION_2_5)
          _mp3_header_output->framesize = 144 * _mp3_header_output->bitrate / _mp3_header_output->frequency + (_tmpheader->padding_bit ? 1 : 0); //Mpeg1
        else
          _mp3_header_output->framesize =  72000 * _mp3_header_output->bitrate / _mp3_header_output->frequency + (_tmpheader->padding_bit ? 1 : 0); //Mpeg2 + Mpeg2.5
        break;
    }
//    if (_mp3_header_output->layer == MPEGLAYER_I)
//      _mp3_header_output->framesize = fto_nearest_i((float)((48 * (float)_mp3_header_output->bitrate) / _mp3_header_output->frequency)) + (_tmpheader->padding_bit ? 4 : 0);
//    else
//      _mp3_header_output->framesize = fto_nearest_i((float)((144 * (float)_mp3_header_output->bitrate) / _mp3_header_output->frequency)) + (_tmpheader->padding_bit ? 1 : 0);
  }
  else
    _mp3_header_output->framesize = 0; //unable to determine

  const size_t CRCSIZE = 2;
  size_t sideinfo_len;

  if (_mp3_header_output->version == MPEGVERSION_1) /* MPEG 1 */
    sideinfo_len = (_mp3_header_output->channelmode == MP3CHANNELMODE_SINGLE_CHANNEL) ? 4 + 17 : 4 + 32;
  else                /* MPEG 2 */
    sideinfo_len = (_mp3_header_output->channelmode == MP3CHANNELMODE_SINGLE_CHANNEL) ? 4 + 9 : 4 + 17;

  int vbr_header_offest = beg + sideinfo_len;
  int vbr_frames = 0;

  sideinfo_len += 2; // add two for the crc itself

  if ((_mp3_header_output->crc == MP3CRC_OK) && mp3size < sideinfo_len)
    _mp3_header_output->crc = MP3CRC_ERROR_SIZE;

  if (_mp3_header_output->crc == MP3CRC_OK)
  {
    char audiodata[38 + 1]; //+1 to hold the 0 char
    uint16 crc16;
    uint16 crcstored;

    _mp3_header_output->crc = MP3CRC_MISMATCH; //as a starting point, we assume the worst

    reader.setCur(beg);

    reader.readChars(audiodata, sideinfo_len);
    audiodata[sideinfo_len] = '\0';

    crc16 = calcCRC(audiodata, sideinfo_len);

    beg = end;
    end = beg + CRCSIZE;

    reader.setCur(beg);
    crcstored = (uint16)io::readBENumber(reader, CRCSIZE);

    // a mismatch doesn't mean the file is unusable
    // it has just some bits in the wrong place
    if (crcstored == crc16)
      _mp3_header_output->crc = MP3CRC_OK;
  }

  // read xing/vbr header if present
  // derived from code in vbrheadersdk.zip 
  // from http://www.xingtech.com/developer/mp3/

  const size_t VBR_HEADER_MIN_SIZE = 8;     // "xing" + flags are fixed
  const size_t VBR_HEADER_MAX_SIZE = 116;   // frames, bytes, toc and scale are optional

  if (mp3size >= vbr_header_offest + VBR_HEADER_MIN_SIZE) 
  {
    char vbrheaderdata[VBR_HEADER_MAX_SIZE+1]; //+1 to hold the 0 char
    unsigned char *pvbrdata = (unsigned char *)vbrheaderdata;
    int vbr_filesize = 0;
    int vbr_scale = 0;
    int vbr_flags = 0;

    // get fixed part of vbr header 
    // and check if valid

    beg = vbr_header_offest;
    reader.setCur(beg);
    reader.readChars(vbrheaderdata, VBR_HEADER_MIN_SIZE);
    vbrheaderdata[VBR_HEADER_MIN_SIZE] = '\0';

    if (pvbrdata[0] == 'X' &&
        pvbrdata[1] == 'i' &&
        pvbrdata[2] == 'n' &&
        pvbrdata[3] == 'g')
    {
      // get vbr flags
      pvbrdata += 4;
      vbr_flags = ExtractI4(pvbrdata);
      pvbrdata += 4;

      //  read entire vbr header
      int vbr_header_size = VBR_HEADER_MIN_SIZE
                           + ((vbr_flags & FRAMES_FLAG)? 4:0)
                           + ((vbr_flags & BYTES_FLAG)? 4:0)
                           + ((vbr_flags & TOC_FLAG)? 100:0)
                           + ((vbr_flags & SCALE_FLAG)? 4:0);

      if (mp3size >= vbr_header_offest + vbr_header_size) 
      {
        reader.readChars(&vbrheaderdata[VBR_HEADER_MIN_SIZE], vbr_header_size - VBR_HEADER_MIN_SIZE); 
        vbrheaderdata[vbr_header_size] = '\0';

        // get frames, bytes, toc and scale

        if (vbr_flags & FRAMES_FLAG)
        {
          vbr_frames = ExtractI4(pvbrdata); 
          pvbrdata +=4;
        }

        if (vbr_flags & BYTES_FLAG)
        {
          vbr_filesize = ExtractI4(pvbrdata); 
          pvbrdata +=4;
        }

        if (vbr_flags & TOC_FLAG)
        {
          // seek offsets
          // we are not using
          // for(i=0;i<100;i++) seek_offsets[i] = pvbrdata[i];

          pvbrdata +=100;
        }

        if (vbr_flags & SCALE_FLAG)
        {
          vbr_scale = ExtractI4(pvbrdata); 
          pvbrdata +=4;
        }

        if (vbr_frames > 0)
        {
          _mp3_header_output->vbr_bitrate = (((vbr_filesize!=0) ? vbr_filesize : mp3size) / vbr_frames) * _mp3_header_output->frequency / 144;
          _mp3_header_output->vbr_bitrate -= _mp3_header_output->vbr_bitrate%1000;   // round the bitrate:
        }
      }
    }
  }

  if (_mp3_header_output->framesize > 0 && mp3size >= _mp3_header_output->framesize) // this means bitrate is not none too
  {
    if (vbr_frames == 0)
      _mp3_header_output->frames = fto_nearest_i((float)mp3size / _mp3_header_output->framesize);
    else
      _mp3_header_output->frames = vbr_frames;

    // bitrate becomes byterate (per second) if divided by 8
    if (_mp3_header_output->vbr_bitrate == 0)
      _mp3_header_output->time = fto_nearest_i( (float)mp3size / (_mp3_header_output->bitrate / 8) );
    else
      _mp3_header_output->time = fto_nearest_i( (float)mp3size / (_mp3_header_output->vbr_bitrate / 8) );
  }
  else
  {
    _mp3_header_output->frames = 0;
    _mp3_header_output->time = 0;
  }
  //if we got to here it's okay
  return true;
}


