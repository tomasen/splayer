// $Id: io_decorators.cpp,v 1.4 2002/07/02 22:13:40 t1mpy Exp $

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

#if defined HAVE_CONFIG_H
#include <config.h>
#endif



#include "id3/io_decorators.h" //has "readers.h" "io_helpers.h" "utils.h"
#include "zlib.h"

using namespace dami;

void io::WindowedReader::setWindow(pos_type beg, size_type size)
{
  ID3D_NOTICE( "WindowedReader::setWindow() [beg, size] = [" << 
               this->getBeg() << ", " << size << "]" );
  pos_type cur = this->getCur();
  
  // reset the end marker so as to avoid errors
  this->setEnd(_reader.getEnd());
  
  // set the beginning marker
  this->setBeg(beg);
  
  // since the characters might be more than a byte in size, we need to 
  // manually get all the chars to set the window appropriately
  this->setCur(beg);
  ID3D_NOTICE( "WindowedReader::setWindow(): after setCur(beg), cur = "<<
               this->getCur() );
  
  this->skipChars(size);
  ID3D_NOTICE( "WindowedReader::setWindow(): after skipChars, cur = " <<
               this->getCur() );
  
  this->setEnd(this->getCur());
  
  ID3D_NOTICE( "WindowedReader::setWindow() [beg, cur, end] = [" << this->getBeg() << ", " << this->getCur() << ", " << this->getEnd() << "]" );
  
  
  // reset the stream
  this->setCur(cur);
}

ID3_Reader::pos_type io::WindowedReader::setBeg(pos_type beg)
{
  // make sure the position we want to set to isn't past the current
  // end position or the superclass's beginning position
  if (beg <= this->getEnd() && beg >= _reader.getBeg())
  {
    _beg = beg;
  }
  else if (beg > this->getEnd())
  {
    ID3D_WARNING( "WindowedReader::setBeg() failed, [beg, _end] = " << 
                  beg << ", " << this->getEnd() << "]" );
  }
  else
  {
    ID3D_WARNING( "WindowedReader::setBeg() failed, [beg, _beg] = " << 
                  beg << ", " << this->getBeg() << "]" );
  }
  return _beg;
}

ID3_Reader::pos_type io::WindowedReader::setEnd(pos_type end)
{
  // make sure the position we want to set to isn't beforen the current
  // beginning position or the superclass's end position
  if (this->getBeg() <= end && end <= _reader.getEnd())
  {
    _end = end;
  }
  else
  {
    ID3D_WARNING( "WindowedReader::setEnd() failed, end = " << end );
    ID3D_WARNING( "WindowedReader::setEnd() failed, beg = " << 
                  this->getBeg() );
    ID3D_WARNING( "WindowedReader::setEnd() failed, super.end = " << 
                  _reader.getEnd() );
    
  }
  return _end;
}

ID3_Reader::int_type io::WindowedReader::readChar()
{
  int_type ch = END_OF_READER;
  if (this->inWindow())
  {
    ch = _reader.readChar();
  }
  else
  {
    ID3D_WARNING( "io::WindowedReader::readChar: not in window, " << 
                  "pos = " << this->getCur() << ", window = [" << 
                  this->getBeg() << ", " << this->getEnd() << "]");
  }
  return ch;
}

ID3_Reader::int_type io::WindowedReader::peekChar()
{
  int_type ch = END_OF_READER;
  if (this->inWindow())
  {
    ch = _reader.peekChar();
  }
  return ch;
}

ID3_Reader::size_type io::WindowedReader::readChars(char_type buf[], size_type len)
{
  pos_type cur = this->getCur();
  size_type size = 0;
  if (this->inWindow(cur))
  {
    size = _reader.readChars(buf, min<size_type>(len, _end - cur));
  }
  return size;
}

ID3_Reader::size_type io::CharReader::readChars(char_type buf[], size_type len)
{
  size_type numChars = 0;
  ID3D_NOTICE( "CharReader::readChars(): len = " << len );
  for (; numChars < len; ++numChars)
  {
    if (this->atEnd())
    {
      break;
    }
    char_type ch = this->readChar();
    if (buf != NULL)
    {
      buf[numChars] = ch;
    }
  }
  ID3D_NOTICE( "CharReader::readChars(): numChars = " << len );
  return numChars;
}

ID3_Reader::int_type io::LineFeedReader::readChar()
{
  if (this->atEnd())
  {
    return END_OF_READER;
  }
  char_type ch = _reader.readChar();
  if (ch == 0x0D && this->peekChar() == 0x0A)
  {
    ID3D_NOTICE( "LineFeedReader::readChar(): found CRLF at pos " << 
                 this->getCur() );
    ch = _reader.readChar();
  }
  return ch;
};

ID3_Reader::int_type io::UnsyncedReader::readChar()
{
  if (this->atEnd())
  {
    return END_OF_READER;
  }
  char_type ch = _reader.readChar();
  if (ch == 0xFF && this->peekChar() == 0x00)
  {
    ID3D_NOTICE( "UnsyncedReader::readChar(): found sync at pos " << 
                 this->getCur() );
    _reader.readChar();
  }
  return ch;
}

io::CompressedReader::CompressedReader(ID3_Reader& reader, size_type newSize)
  : _uncompressed(new char_type[newSize])
{
  size_type oldSize = reader.remainingBytes();
  
  BString binary = readBinary(reader, oldSize);
  
  ::uncompress(_uncompressed,
               reinterpret_cast<luint*>(&newSize),
               reinterpret_cast<const uchar*>(binary.data()),
               oldSize);
  this->setBuffer(_uncompressed, newSize);
}

io::CompressedReader::~CompressedReader()
{ 
  delete [] _uncompressed; 
}

ID3_Writer::int_type io::UnsyncedWriter::writeChar(char_type ch)
{
  if (_last == 0xFF && (ch == 0x00 || ch >= 0xE0))
  {
    _writer.writeChar('\0');
    _numSyncs++;
  }
  _last = _writer.writeChar(ch);
  return _last;
}

void io::UnsyncedWriter::flush()
{
  if (_last == 0xFF)
  {
    _last = _writer.writeChar('\0');
    _numSyncs++;
  }
  _writer.flush();
}

ID3_Writer::size_type 
io::UnsyncedWriter::writeChars(const char_type buf[], size_type len)
{
  pos_type beg = this->getCur();
  ID3D_NOTICE( "UnsyncedWriter::writeChars(): len = " << len );
  for (size_t i = 0; i < len; ++i)
  {
    if (this->atEnd())
    {
      break;
    }
    this->writeChar(buf[i]);
  }
  size_type numChars = this->getCur() - beg;
  ID3D_NOTICE( "CharWriter::writeChars(): numChars = " << numChars );
  return numChars;
}

void io::CompressedWriter::flush()
{
  if (_data.size() == 0)
  {
    return;
  }
  const char_type* data = reinterpret_cast<const char_type*>(_data.data());
  size_type dataSize = _data.size();
  _origSize = dataSize;
  // The zlib documentation specifies that the destination size needs to
  // be an unsigned long at least 0.1% larger than the source buffer,
  // plus 12 bytes
  unsigned long newDataSize = dataSize + (dataSize / 10) + 12;
  char_type* newData = new char_type[newDataSize];
  if (::compress(newData, &newDataSize, data, dataSize) != Z_OK)
  {
    // log this
    ID3D_WARNING("io::CompressedWriter: error compressing");
    _writer.writeChars(data, dataSize);
  }
  else if (newDataSize < dataSize)
  {
    ID3D_NOTICE("io::CompressedWriter: compressed size = " << newDataSize << ", original size = " << dataSize ); 
    _writer.writeChars(newData, newDataSize);
  }
  else
  {
    ID3D_NOTICE("io::CompressedWriter: no compression!compressed size = " << newDataSize << ", original size = " << dataSize ); 
    _writer.writeChars(data, dataSize);
  }
  delete [] newData;
  _data.erase();
}

ID3_Writer::size_type 
io::CompressedWriter::writeChars(const char_type buf[], size_type len)
{ 
  ID3D_NOTICE("io::CompressedWriter: writing chars: " << len );
  _data.append(buf, len);
  return len;
}

