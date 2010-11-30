// $Id: tag.cpp,v 1.55 2003/03/02 13:35:58 t1mpy Exp $

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

//#include "readers.h"
#include "writers.h"
#include "tag_impl.h" //has <stdio.h> "tag.h" "header_tag.h" "frame.h" "field.h" "spec.h" "id3lib_strings.h" "utils.h"

using namespace dami;

/** \mainpage id3lib Library Documentation
 **
 ** \section tutorial Quick Tutorial
 **
 ** This tutorial will quickly get you up and running with id3lib.
 **
 ** \subsection download Downloading id3lib
 **
 ** First, id3lib must be a part of your development environment.  The latest
 ** files can always be downloaded from the <a href="http://id3lib.sourceforge.net">id3lib homepage</a>.
 **
 ** \subsection include Preparing your source code
 **
 ** To use the basic functionality of id3lib in your C++ code, a single
 ** \c #include is necessary.
 **
 ** \code
 **   #include <id3/tag.h>
 ** \endcode
 **
 ** There are other files that must be included to access more advanced
 ** functionality, but this will do most of the core functionality.
 **
 ** \subsection creation Creating a tag
 **
 ** Almost all functionality occurs via an ID3_Tag object.  An ID3_Tag object
 ** basically encapsulates two things: a collection of ID3_Frame objects and
 ** file information.  The goal is to populate an ID3_Tag object with ID3_Frame
 ** objects, and the easiest way to do this is to associate the tag with a
 ** file.  This is done primarily via the ID3_Tag constructor, like so:
 **
 ** \code
 **   ID3_Tag myTag("song.mp3");
 ** \endcode
 **
 ** This constructor links, or associates, the object \c myTag with the file
 ** "song.mp3".  In doing so, the tagging information from "song.mp3" is parsed
 ** and added to \c myTag.  This association can also be accomplished by creating
 ** an empty tag and making an explicit call to Link().
 **
 ** \code
 **   ID3_Tag myTag;
 **   myTag.Link("song.mp3");
 ** \endcode
 **
 ** The default behavior of Link() is to parse all possible tagging information
 ** and convert it into ID3v2 frames.  The tagging information parsed can be
 ** limited to a particular type (or types) of tag by passing an ID3_TagType
 ** (or combination of ID3_TagTypes).  For example, to read only the ID3v1
 ** tag, pass in the constant ID3TT_ID3V1.
 **
 ** \code
 **   myTag.Link("song.mp3", ID3TT_ID3V1);
 ** \endcode
 **
 ** Another example would be to read in all tags that could possibly appear at
 ** the end of the file.
 **
 ** \code
 **   myTag.Link("song.mp3", ID3TT_ID3V1 | ID3TT_LYRICS3V2 | ID3TT_MUSICMATCH);
 ** \endcode
 **
 ** \section accessing Accessing the Tag Data
 **
 ** After linking with a file, the object \c myTag now contains some or all of
 ** the tagging information present in the file "song.mp3", represented as
 ** ID3v2 frames.  How can that information be accessed?  There are a variety of
 ** ways to do this.  One is to iterate through all the frames in the tag.
 **
 ** \code
 **   // use an std::auto_ptr here to handle object cleanup automatically
 **   ID3_Tag::Iterator* iter = myTag.CreateIterator();
 **   ID3_Frame* myFrame = NULL;
 **   while (NULL != (myFrame = iter->GetNext()))
 **   {
 **     // do something with myFrame
 **   }
 **   delete iter;
 ** \endcode
 **
 ** Another way to access tagging information is by searching for specific
 ** frames using the Find() method.  For example, the album frame can be found
 ** in the following manner:
 **
 ** \code
 **   ID3_Frame* myFrame = myTag.Find(ID3FID_ALBUM);
 **   if (NULL != myFrame)
 **   {
 **     // do something with myFrame
 **   }
 ** \endcode
 **
 ** The Find() method can be used to search for frames with specific
 ** information.  For example, the following code can be used to find the frame
 ** with the title "Nirvana".
 **
 ** \code
 **   ID3_Frame* myFrame = myTag.Find(ID3FID_TITLE, ID3FN_TEXT, "Nirvana")));
 **   if (NULL != myFrame)
 **   {
 **     // do something with myFrame
 **   }
 ** \endcode
 **
 ** As indicated, the Find() method will return a NULL pointer if no such frame
 ** can be found.  If more than one frame meets the search criteria, subsequent
 ** calls to Find() with the same parameters will return the other matching
 ** frames.  The Find() method is guaranteed to return all matching frames
 ** before it wraps around to return the first matching frame.
 **
 ** All ID3_Frame objects are comprised of a collection of ID3_Field objects.
 ** These fields can represent text, numbers, or binary data.  As with frames,
 ** fields can be accessed in a variety of manners.  The fields of a frame
 ** can be iterated over in much the same manner of the frames of a tag.
 **
 ** \code
 **   // use an std::auto_ptr here to handle object cleanup automatically
 **   ID3_Frame::Iterator* iter = myFrame->CreateIterator();
 **   ID3_Field* myField = NULL;
 **   while (NULL != (myField = iter->GetNext()))
 **   {
 **     // do something with myField
 **   }
 **   delete iter;
 ** \endcode
 **
 ** If you know which field type you're looking for, you can access it
 ** directly.
 **
 ** \code
 **   ID3_Field* myField = myFrame->GetField(ID3FN_TEXT);
 **   while (NULL != myField)
 **   {
 **     // do something with myField
 **   }
 ** \endcode
 **
 ** Note: The ID3_FrameInfo class provides information about the frame types known
 ** to id3lib.
 **
 ** The ID3_Field represents a single piece of data within an ID3v2 frame.  As
 ** mentioned, an ID3_Field can represent three possible types of
 ** data: integers, binary data, and text strings.  The type of a particular
 ** field object is immutable; it is determined at the time of its construction
 ** (almost always when a frame is constructed) and can't be changed.  If in
 ** doubt, the field type can be accessed through its GetType() method.
 **
 ** Having an ID3_Field object isn't much use if you cannot access and/or
 ** alter its data.  Luckily, the id3lib API provides overloaded \c Set and
 ** \c Get methods for all data types.
 **
 ** If the field is an integer, the following methods can be used to access
 ** the data.
 **
 ** \code
 **   uint32 val = myField->Get();
 **   myField->Set(5);
 **   (*myField) = 10;
 ** \endcode
 **
 ** All text data is accessed in a slightly different manner.  The following
 ** code example best illustrates these differences.
 **
 ** \code
 **   // for ascii strings
 **   char str1[1024];
 **   const char* p1 = "My String";
 **   const char* p2 = "My Other String";
 **
 **   myField->Set(p1);
 **   (*myField) = p2;  // equivalent to Set
 **
 **   myField->Get(str1, 1024); // copies up to 1024 bytes of the field data into str1
 **   p1 = myField->GetRawText(); // returns a pointer to the internal string
 ** \endcode
 **
 ** Binary data is similar to text data, except that its base type is a pointer
 ** to an unsigned, rather than a signed, char.
 **
 ** \code
 **   // for binary strings
 **   uchar data[1024];
 **   const uchar *p1 = getBinaryData(); // not an id3lib function
 **   size_t size = getBinarySize();     // not an id3lib function
 **
 **   myField->Set(p1, size);
 **
 **   myField->Get(data, 1024); // copies up to 1024 bytes of the field data into str1
 **   p1 = myField->GetRawBinary(); // returns a pointer to the internal string
 ** \endcode
 **
 ** \section updating Updating the Tag
 **
 ** When you're ready to save your changes back to the file, a single call to
 ** Update() is sufficient.
 **
 ** \code
 **   tag.Update();
 ** \endcode
 **
 **/

/** \class ID3_Tag tag.h id3/tag.h
 ** \brief The representative class of an id3 tag.
 **
 ** The ID3_Tag is, at its simplest, a container for ID3v2 frames.  At its
 ** most complicated, it's a kitchen-sink, monolithic "catch-all" class for
 ** handling almost every task associated with creating, parsing, rendering,
 ** and manipulating digital audio data using id3lib.
 **
 ** This simple \c #include does it all.  In order to read an existing tag, do
 ** the following:
 **
 ** \code
 **   ID3_Tag myTag;
 **   myTag.Link("something.mp3");
 ** \endcode
 **
 ** That is all there is to it.  Now all you have to do is use the Find()
 ** method to locate the frames you are interested in is the following:
 **
 ** \code
 **   ID3_Frame* myFrame = myTag.Find(ID3FID_TITLE);
 **   if (NULL != myFrame)
 **   {
 **     const char* title = myFrame->GetField(ID3FN_TEXT)->GetText();
 **     cout << "Title: " << title << endl;
 **   }
 ** \endcode
 **
 ** This code snippet locates the ID3FID_TITLE frame and displays the
 ** text field.
 **
 ** When using the ID3_Tag::Link() method, you automatically gain access to any
 ** ID3v1/1.1, ID3v2, Lyrics3 v2.0, and MusicMatch tags present in the file.
 ** The class will automaticaly parse and convert any of these foreign tag
 ** formats into ID3v2 tags.  Also, id3lib will correctly parse any correctly
 ** formatted 'CDM' frames from the unreleased ID3v2 2.01 draft specification.
 **
 ** \author Dirk Mahoney
 ** \version $Id: tag.cpp,v 1.55 2003/03/02 13:35:58 t1mpy Exp $
 ** \sa ID3_Frame
 ** \sa ID3_Field
 ** \sa ID3_Err
 **/

/** Default constructor; it can accept an optional filename as a parameter.
 **
 ** If this file exists, it will be opened and all id3lib-supported tags will
 ** be parsed and converted to ID3v2 if necessary.  After the conversion, the
 ** file will remain unchanged, and will continue to do so until you use the
 ** Update() method on the tag (if you choose to Update() at all).
 **
 ** \param name The filename of the mp3 file to link to
 **/
ID3_Tag::ID3_Tag(const char *name)
  : _impl(new ID3_TagImpl(name))
{
}

/** Standard copy constructor.
 **
 ** \param tag What is copied into this tag
 **/
ID3_Tag::ID3_Tag(const ID3_Tag &tag)
  : _impl(new ID3_TagImpl(tag))
{
}

ID3_Tag::~ID3_Tag()
{
  delete _impl;
}

/** Clears the object and disassociates it from any files.
 **
 ** Frees any resources for which the object is responsible, including all
 ** frames and files.  After a call to Clear(), the object can be used
 ** again for any new or existing tag.
 **/
void ID3_Tag::Clear()
{
  _impl->Clear();
}


/** Indicates whether the tag has been altered since the last parse, render,
 ** or update.
 **
 ** If you have a tag linked to a file, you do not need this method since the
 ** Update() method will check for changes before writing the tag.
 **
 ** This method is primarily intended as a status indicator for applications
 ** and for applications that use the Parse() and Render() methods.
 **
 ** Setting a field, changed the ID of an attached frame, setting or grouping
 ** or encryption IDs, and clearing a frame or field all constitute a change
 ** to the tag, as do calls to the SetUnsync(), SetExtendedHeader(), and
 ** SetPadding() methods.
 **
 ** \code
 **   if (myTag.HasChanged())
 **   {
 **     // render and output the tag
 **   }
 ** \endcode
 **
 ** \return Whether or not the tag has been altered.
 **/
bool ID3_Tag::HasChanged() const
{
  return _impl->HasChanged();
}

/** Returns an over estimate of the number of bytes required to store a
 ** binary version of a tag.
 **
 ** When using Render() to render a binary tag to a
 ** memory buffer, first use the result of this call to allocate a buffer of
 ** unsigned chars.
 **
 ** \code
 **   if (myTag.HasChanged())
 **   {
 **     size_t tagSize; = myTag.Size();
 **     if (tagSize > 0)
 **     {
 **       uchar *buffer = new uchar[tagSize];
 **       if (NULL != buffer)
 **       {
 **         size_t actualSize = myTag.Render(buffer);
 **         // do something useful with the first
 **         // 'actualSize' bytes of the buffer,
 **         // like push it down a socket
 **         delete [] buffer;
 **       }
 **     }
 **   }
 ** \endcode
 **
 ** @see #Render
 ** @return The (overestimated) number of bytes required to store a binary
 **         version of a tag
 **/
size_t ID3_Tag::Size() const
{
  return _impl->Size();
}

/** Turns unsynchronization on or off, dependant on the value of the boolean
 ** parameter.
 **
 ** If you call this method with 'false' as the parameter, the
 ** binary tag will not be unsync'ed, regardless of whether the tag should
 ** be.  This option is useful when the file is only going to be used by
 ** ID3v2-compliant software.  See the ID3v2 standard document for futher
 ** details on unsync.
 **
 ** Be default, tags are created without unsync.
 **
 ** \code
 **   myTag.SetUnsync(false);
 ** \endcode
 **
 ** \param bSync Whether the tag should be unsynchronized
 **/
bool ID3_Tag::SetUnsync(bool b)
{
  return _impl->SetUnsync(b);
}


/** Turns extended header rendering on or off, dependant on the value of the
 ** boolean parameter.
 **
 ** This option is currently ignored as id3lib doesn't yet create extended
 ** headers.  This option only applies when rendering tags for ID3v2 versions
 ** that support extended headers.
 **
 ** \code
 **   myTag.SetExtendedHeader(true);
 ** \endcode
 **
 ** \param bExt Whether to render an extended header
 **/
bool ID3_Tag::SetExtendedHeader(bool ext)
{
  return _impl->SetExtended(ext);
}

/** Turns padding on or off, dependant on the value of the boolean
 ** parameter.
 **
 ** When using ID3v2 tags in association with files, id3lib can optionally
 ** add padding to the tags to ensure minmal file write times when updating
 ** the tag in the future.
 **
 ** When the padding option is switched on, id3lib automatically creates
 ** padding according to the 'ID3v2 Programming Guidelines'.  Specifically,
 ** enough padding will be added to round out the entire file (song plus
 ** tag) to an even multiple of 2K.  Padding will only be created when the
 ** tag is attached to a file and that file is not empty (aside from a
 ** pre-existing tag).
 **
 ** id3lib's addition to the guidelines for padding, is that if frames are
 ** removed from a pre-existing tag (or the tag simply shrinks because of
 ** other reasons), the new tag will continue to stay the same size as the
 ** old tag (with padding making the difference of course) until such time as
 ** the padding is greater than 4K.  When this happens, the padding will be
 ** reduced and the new tag will be smaller than the old.
 **
 ** By default, padding is switched on.
 **
 ** \code
 **   myTag.SetPadding(false);
 ** \endcode
 **
 ** \param bPad Whether or not render the tag with padding.
 **/
bool ID3_Tag::SetPadding(bool pad)
{
  return _impl->SetPadding(pad);
}

bool ID3_Tag::SetExperimental(bool exp)
{
  return _impl->SetExperimental(exp);
}

bool ID3_Tag::GetUnsync() const
{
  return _impl->GetUnsync();
}

bool ID3_Tag::GetExtendedHeader() const
{
  return _impl->GetExtended();
}

bool ID3_Tag::GetExperimental() const
{
  return _impl->GetExperimental();
}

void ID3_Tag::AddFrame(const ID3_Frame& frame)
{
  _impl->AddFrame(frame);
}

/** Attaches a frame to the tag; the tag doesn't take responsibility for
 ** releasing the frame's memory when tag goes out of scope.
 **
 ** Optionally, operator<< can also be used to attach a frame to a tag.  To
 ** use, simply supply its parameter a pointer to the ID3_Frame object you wish
 ** to attach.
 **
 ** \code
 **   ID3_Frame myFrame;
 **   myTag.AddFrame(&myFrame);
 ** \endcode
 **
 ** As stated, this method attaches the frames to the tag---the tag does
 ** not create its own copy of the frame.  Frames created by an application
 ** must exist until the frame is removed or the tag is finished with it.
 **
 ** \param pFrame A pointer to the frame that is being added to the tag.
 ** \sa ID3_Frame
 **/
void ID3_Tag::AddFrame(const ID3_Frame* frame)
{
  _impl->AddFrame(frame);
}

/** Attaches a frame to the tag; the tag takes responsibility for
 ** releasing the frame's memory when tag goes out of scope.
 **
 ** This method accepts responsibility for the attached frame's memory, and
 ** will delete the frame and its contents when the tag goes out of scope or is
 ** deleted.  Therefore, be sure the frame isn't "Attached" to other tags.
 **
 ** \code
 **   ID3_Frame *frame = new ID3_Frame;
 **   myTag.AttachFrame(frame);
 ** \endcode
 **
 ** \param frame A pointer to the frame that is being added to the tag.
 **/
bool ID3_Tag::AttachFrame(ID3_Frame *frame)
{
  return _impl->AttachFrame(frame);
}


/** Removes a frame from the tag.
 **
 ** If you already own the frame object in question, then you should already
 ** have a pointer to the frame you want to delete.  If not, or if you wish to
 ** delete a pre-existing frame (from a tag you have parsed, for example), the
 ** use one of the Find methods to obtain a frame pointer to pass to this
 ** method.
 **
 ** \code
 **   ID3_Frame *someFrame;
 **   if (someFrame = myTag.Find(ID3FID_TITLE))
 **   {
 **     myTag.RemoveFrame(someFrame);
 **   }
 ** \endcode
 **
 ** \sa ID3_Tag#Find
 ** \param pOldFrame A pointer to the frame that is to be removed from the
 **                  tag
 **/
ID3_Frame* ID3_Tag::RemoveFrame(const ID3_Frame *frame)
{
  return _impl->RemoveFrame(frame);
}

bool ID3_Tag::Parse(ID3_Reader& reader)
{
  return id3::v2::parse(*_impl, reader);
}

size_t ID3_Tag::Parse(const uchar* buffer, size_t bytes)
{
  ID3_MemoryReader mr(buffer, bytes);
  ID3_Reader::pos_type beg = mr.getCur();
  id3::v2::parse(*_impl, mr);
  return mr.getEnd() - beg;
}

/** Turns a binary tag into a series of ID3_Frame objects attached to the
 ** tag.
 **
 ** \code
 **   ID3_Tag myTag;
 **   uchar header[ID3_TAGHEADERSIZE];
 **   uchar *buffer;
 **   luint tagSize;
 **
 **   // get ID3_TAGHEADERSIZE from a socket or somewhere
 **   // ...
 **
 **   if ((tagSize = ID3_IsTagHeader(ourSourceBuffer)) > -1)
 **   {
 **     // read a further 'tagSize' bytes in
 **     // from our data source
 **     // ...
 **
 **     if (buffer = new uchar[tagSize])
 **     {
 **       // now we will call ID3_Tag::Parse()
 **       // with these values (explained later)
 **       myTag.Parse(header, buffer);
 **
 **       // do something with the objects,
 **       // like look for titles, artists, etc.
 **       // ...
 **
 **       // free the buffer
 **       delete [] buffer;
 **     }
 **   }
 ** \endcode
 **
 ** \sa ID3_Frame
 ** @param header The byte header read in from the data source.
 ** @param buffer The remainder of the tag (not including the data source)
 **               read in from the data source.
 **/
size_t ID3_Tag::Parse(const uchar header[ID3_TAGHEADERSIZE], const uchar *buffer)
{
  size_t size = ID3_Tag::IsV2Tag(header);
  if (0 == size)
  {
    return 0;
  }
  BString buf;
  buf.reserve(ID3_TagHeader::SIZE + size);
  buf.append(reinterpret_cast<const BString::value_type *>(header),
             ID3_TagHeader::SIZE);
  buf.append(reinterpret_cast<const BString::value_type *>(buffer), size);
  return this->Parse(buf.data(), buf.size());
}

/** Renders the tag and writes it to the attached file; the type of tag
 ** rendered can be specified as a parameter.  The default is to update only
 ** the ID3v2 tag.  See the ID3_TagType enumeration for the constants that
 ** can be used.
 **
 ** Make sure the rendering parameters are set before calling the method.
 ** See the Link documentation for an example of this method in use.
 **
 ** \sa ID3_TagType
 ** \sa Link
 ** \param tt The type of tag to update.
 **/
/** Renders a binary image of the tag into the supplied buffer.
 **
 ** See Size() for an example.  This method returns the actual number of the
 ** bytes of the buffer used to store the tag.  This will be no more than the
 ** size of the buffer itself, because Size() over estimates the required
 ** buffer size when padding is enabled.
 **
 ** Before calling this method, it is advisable to call HasChanged() first as
 ** this will let you know whether you should bother rendering the tag.
 **
 ** @see    ID3_IsTagHeader
 ** @see    ID3_Tag#HasChanged
 ** @return The actual number of the bytes of the buffer used to store the
 **         tag
 ** @param  buffer The buffer that will contain the rendered tag.
 **/
size_t ID3_Tag::Render(uchar* buffer, ID3_TagType tt) const
{
  ID3_MemoryWriter mw(buffer, -1);
  return this->Render(mw, tt);
}

size_t ID3_Tag::Render(ID3_Writer& writer, ID3_TagType tt) const
{
  ID3_Writer::pos_type beg = writer.getCur();
  if (ID3TT_ID3V2 & tt)
  {
    id3::v2::render(writer, *this);
  }
  else if (ID3TT_ID3V1 & tt)
  {
    id3::v1::render(writer, *this);
  }
  return writer.getCur() - beg;
}


/** Attaches a file to the tag, parses the file, and adds any tag information
 ** found in the file to the tag.
 **
 ** Use this method if you created your ID3_Tag object without supplying a
 ** parameter to the constructor (maybe you created an array of ID3_Tag
 ** pointers).  This is the preferred method of interacting with files, since
 ** id3lib can automatically do things like parse foreign tag formats and
 ** handle padding when linked to a file.  When a tag is linked to a file, you
 ** do not need to use the Size(), Render(const uchar*, size_t), or
 ** Parse(ID3_Reader&) methods or the IsV2Tag(ID3_Reader&) static function--
 ** id3lib will take care of those details for you.  The single parameter is a
 ** pointer to a file name.
 **
 ** Link returns the size of the the ID3v2 tag (if any) that begins the file.
 **
 ** \code
 **   ID3_Tag myTag;
 **   myTag.Link("mysong.mp3");
 **
 **   // do whatever we want with the tag
 **   // ...
 **
 **   // setup all our rendering parameters
 **   myTag->SetUnsync(false);
 **   myTag->SetExtendedHeader(true);
 **   myTag->SetCompression(true);
 **   myTag->SetPadding(true);
 **
 **   // write any changes to the file
 **   myTag->Update()
 **
 ** \endcode
 **
 ** @see IsV2Tag
 ** @param fileInfo The filename of the file to link to.
 **/
size_t ID3_Tag::Link(const char *fileInfo, flags_t flags)
{
  return _impl->Link(fileInfo, flags);
}

/**
 ** Same as above, but takes a ID3_Reader as argument.
 */
size_t ID3_Tag::Link(ID3_Reader &reader, flags_t flags)
{
  return _impl->Link(reader, flags);
}

flags_t ID3_Tag::Update(flags_t flags)
{
  return _impl->Update(flags);
}

/**
 ** Get's the mp3 Info like bitrate, mpeg version, etc.
 ** Can be run after Link(<filename>)
 **
 **/
const Mp3_Headerinfo* ID3_Tag::GetMp3HeaderInfo() const
{
  return _impl->GetMp3HeaderInfo();
}

/** Strips the tag(s) from the attached file. The type of tag stripped
 ** can be specified as a parameter.  The default is to strip all tag types.
 **
 ** \param tt The type of tag to strip
 ** \sa ID3_TagType
 **/
flags_t ID3_Tag::Strip(flags_t flags)
{
  return _impl->Strip(flags);
}

size_t ID3_Tag::GetPrependedBytes() const
{
  return _impl->GetPrependedBytes();
}

size_t ID3_Tag::GetAppendedBytes() const
{
  return _impl->GetAppendedBytes();
}

size_t ID3_Tag::GetFileSize() const
{
  return _impl->GetFileSize();
}

const char* ID3_Tag::GetFileName() const
{
  //fix because GetFileName need to return a pointer which keeps to be valid
  String fn = _impl->GetFileName();
  if (fn.size())
  {
    memset((char *)_tmp_filename, 0, ID3_PATH_LENGTH);
    memmove((char *)_tmp_filename, fn.c_str(), fn.size());
    return _tmp_filename; //_impl->GetFileName().c_str();
  }
  else
    return NULL;
}

/// Finds frame with given frame id
  /** Returns a pointer to the next ID3_Frame with the given ID3_FrameID;
   ** returns NULL if no such frame found.
   **
   ** If there are multiple frames in the tag with the same ID (which, for some
   ** frames, is allowed), then subsequent calls to Find() will return
   ** subsequent frame pointers, wrapping if necessary.
   **
   ** \code
   **   ID3_Frame *myFrame;
   **   if (myFrame = myTag.Find(ID3FID_TITLE))
   **   {
   **     // do something with the frame, like copy
   **     // the contents into a buffer, display the
   **     // contents in a window, etc.
   **     // ...
   **   }
   ** \endcode
   **
   ** You may optionally supply to more parameters ot this method, being an
   ** ID3_FieldID and a value of some sort.  Depending on the field name/ID you
   ** supply, you may supply an integer, a char* or a unicode_t* as the third
   ** parameter.  If you supply an ID3_FrameID, you must also supply a data
   ** value to compare against.
   **
   ** This method will then return the first frame that has a matching frame
   ** ID, and which has a field with the same name as that which you supplied
   ** in the second parameter, whose calue matches that which you supplied as
   ** the third parameter.  For example:
   **
   ** \code
   **   ID3_Frame *myFrame;
   **   if (myFrame = myTag.Find(ID3FID_TITLE, ID3FN_TEXT, "Nirvana"))
   **   {
   **     // found it, do something with it.
   **     // ...
   **   }
   ** \endcode
   **
   ** This example will return the first TITLE frame and whose TEXT field is
   ** 'Nirvana'.  Currently there is no provision for things like 'contains',
   ** 'greater than', or 'less than'.  If there happens to be more than one of
   ** these frames, subsequent calls to the Find() method will return
   ** subsequent frames and will wrap around to the beginning.
   **
   ** Another example...
   **
   ** \code
   **   ID3_Frame *myFrame;
   **   if (myFrame = myTag.Find(ID3FID_COMMENT, ID3FN_TEXTENC, ID3TE_UNICODE))
   **   {
   **     // found it, do something with it.
   **     // ...
   **   }
   ** \endcode
   **
   ** This returns the first COMMENT frame that uses Unicode as its text
   ** encdoing.
   **
   ** @name   Find
   ** @param  id The ID of the frame that is to be located
   ** @return A pointer to the first frame found that has the given frame id,
   **         or NULL if no such frame.
   **/
ID3_Frame* ID3_Tag::Find(ID3_FrameID id) const
{
  return _impl->Find(id);
}

/// Finds frame with given frame id, fld id, and integer data
ID3_Frame* ID3_Tag::Find(ID3_FrameID id, ID3_FieldID fld, uint32 data) const
{
  return _impl->Find(id, fld, data);
}

/// Finds frame with given frame id, fld id, and ascii data
ID3_Frame* ID3_Tag::Find(ID3_FrameID id, ID3_FieldID fld, const char* data) const
{
  String str(data);
  return _impl->Find(id, fld, str);
}

/// Finds frame with given frame id, fld id, and unicode data
ID3_Frame* ID3_Tag::Find(ID3_FrameID id, ID3_FieldID fld, const unicode_t* data) const
{
  WString str = toWString(data, ucslen(data));
  return _impl->Find(id, fld, str);
}

/** Returns the number of frames present in the tag object.
 **
 ** This includes only those frames that id3lib recognises.  This is used as
 ** the upper bound on calls to the GetFrame() and operator[]() methods.
 **
 ** \return The number of frames present in the tag object.
 **/
size_t ID3_Tag::NumFrames() const
{
  return _impl->NumFrames();
}

/** Returns a pointer to the frame with the given index; returns NULL if
 ** there is no such frame at that index.
 **
 ** Optionally, operator[](size_t) can be used as an alternative to this
 ** method.  Indexing is 0-based (that is, the first frame is number 0, and the
 ** last frame in a tag that holds n frames is n-1).
 **
 ** If you wish to have a more comlex searching facility, then at least for
 ** now you will have to devise it yourself and implement it useing these
 ** methods.
 **
 ** @param nIndex The index of the frame that is to be retrieved
 ** @return A pointer to the requested frame, or NULL if no such frame.
 **/
/*
ID3_Frame* ID3_Tag::GetFrameNum(size_t num) const
{
  const size_t numFrames = this->NumFrames();
  if (num >= numFrames)
  {
    return NULL;
  }

  ID3_Frame* frame = NULL;
  size_t curNum = 0;
  // search from the cursor to the end
  for (ID3_TagImpl::const_iterator cur = _impl->begin(); cur != _impl->end(); ++cur)
  {
    if (curNum++ == num)
    {
      frame = *cur;
      break;
    }
  }

  return frame;
}
*/

/** Returns a pointer to the frame with the given index; returns NULL if
 ** there is no such frame at that index.
 **
 ** @name operator[]
 ** @param index The index of the frame that is to be retrieved
 ** @return A pointer to the requested frame, or NULL if no such frame.
 ** @see #GetFrameNum
 **/
/*
ID3_Frame* ID3_Tag::operator[](size_t index) const
{
  return this->GetFrameNum(index);
}
*/

ID3_Tag& ID3_Tag::operator=( const ID3_Tag &rTag )
{
  if (this != &rTag)
  {
    *_impl = rTag;
  }
  return *this;
}

bool ID3_Tag::HasTagType(ID3_TagType tt) const
{
  return _impl->HasTagType(tt);
}

ID3_V2Spec ID3_Tag::GetSpec() const
{
  return _impl->GetSpec();
}

bool ID3_Tag::SetSpec(ID3_V2Spec spec)
{
  return _impl->SetSpec(spec);
}

/** Analyses a buffer to determine if we have a valid ID3v2 tag header.
 ** If so, return the total number of bytes (including the header) to
 ** read so we get all of the tag
 **/
size_t ID3_Tag::IsV2Tag(const uchar* const data)
{
  ID3_MemoryReader mr(data, ID3_TagHeader::SIZE);
  return ID3_TagImpl::IsV2Tag(mr);
}

size_t ID3_Tag::IsV2Tag(ID3_Reader& reader)
{
  return ID3_TagImpl::IsV2Tag(reader);
}

/// Deprecated
void ID3_Tag::AddNewFrame(ID3_Frame* f)
{
  _impl->AttachFrame(f);
}

/** Copies an array of frames to the tag.
 **
 ** This method copies each frame in an array to the tag.  As in
 ** AddFrame, the tag adds a copy of the frame, and it assumes responsiblity
 ** for freeing the frames' memory when the tag goes out of scope.
 **
 ** \code
 **   ID3_Frame myFrames[10];
 **   myTag.AddFrames(myFrames, 10);
 ** \endcode
 **
 ** \sa ID3_Frame
 ** \sa ID3_Frame#AddFrame
 ** \param pNewFrames A pointer to an array of frames to be added to the tag.
 ** \param nFrames The number of frames in the array pNewFrames.
 **/
void ID3_Tag::AddFrames(const ID3_Frame *frames, size_t numFrames)
{
  for (int i = numFrames - 1; i >= 0; i--)
  {
    this->AddFrame(frames[i]);
  }
}

size_t ID3_Tag::Link(const char *fileInfo, bool parseID3v1, bool parseLyrics3)
{
  return _impl->Link(fileInfo, parseID3v1, parseLyrics3);
}

void ID3_Tag::SetCompression(bool b)
{
  ;
}

bool ID3_Tag::HasLyrics() const
{
  return this->HasTagType(ID3TT_LYRICS);
}
bool ID3_Tag::HasV2Tag()  const
{
  return this->HasTagType(ID3TT_ID3V2);
}
bool ID3_Tag::HasV1Tag()  const
{
  return this->HasTagType(ID3TT_ID3V1);
}

/** Copies a frame to the tag.  The frame parameter can thus safely be deleted
 ** or allowed to go out of scope.
 **
 ** Operator<< supports the addition of a pointer to a frame object, or
 ** the frame object itself.
 **
 ** \code
 **   ID3_Frame *pFrame, frame;
 **   p_frame = &frame;
 **   myTag << pFrame;
 **   myTag << frame;
 ** \endcode
 **
 ** Both these methods copy the given frame to the tag---the tag creates its
 ** own copy of the frame.
 **
 ** \name operator<<
 ** \param frame The frame to be added to the tag.
 **/
ID3_Tag& ID3_Tag::operator<<(const ID3_Frame& frame)
{
  this->AddFrame(frame);
  return *this;
}


ID3_Tag& ID3_Tag::operator<<(const ID3_Frame* frame)
{
  if (frame)
  {
    this->AddFrame(frame);
  }
  return *this;
}

int32 ID3_IsTagHeader(const uchar data[ID3_TAGHEADERSIZE])
{
  size_t size = ID3_Tag::IsV2Tag(data);

  if (!size)
  {
    return -1;
  }

  return size - ID3_TagHeader::SIZE;
}


namespace
{
  class IteratorImpl : public ID3_Tag::Iterator
  {
    ID3_TagImpl::iterator _cur;
    ID3_TagImpl::iterator _end;
  public:
    IteratorImpl(ID3_TagImpl& tag)
      : _cur(tag.begin()), _end(tag.end())
    {
    }

    ID3_Frame* GetNext()
    {
      ID3_Frame* next = NULL;
      while (next == NULL && _cur != _end)
      {
        next = *_cur;
        ++_cur;
      }
      return next;
    }
  };


  class ConstIteratorImpl : public ID3_Tag::ConstIterator
  {
    ID3_TagImpl::const_iterator _cur;
    ID3_TagImpl::const_iterator _end;
  public:
    ConstIteratorImpl(ID3_TagImpl& tag)
      : _cur(tag.begin()), _end(tag.end())
    {
    }
    const ID3_Frame* GetNext()
    {
      ID3_Frame* next = NULL;
      while (next == NULL && _cur != _end)
      {
        next = *_cur;
        ++_cur;
      }
      return next;
    }
  };
}

ID3_Tag::Iterator*
ID3_Tag::CreateIterator()
{
  return new IteratorImpl(*_impl);
}

ID3_Tag::ConstIterator*
ID3_Tag::CreateIterator() const
{
  return new ConstIteratorImpl(*_impl);
}

