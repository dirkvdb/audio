//    Copyright (C) 2013 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef AUDIO_HTTP_IO_STREAM
#define AUDIO_HTTP_IO_STREAM

#include <taglib/taglib.h>
#include <taglib/tiostream.h>

#include <memory>
#include <string>

#include "utils/readerinterface.h"

namespace audio
{

class TaglibIOStream : public TagLib::IOStream
{
public:
    // Creates a stream without bufferred io
    TaglibIOStream(const std::string& url);
    // Creates a stream with bufferred io
    TaglibIOStream(const std::string& url, uint32_t bufferSize);
    virtual ~TaglibIOStream();
    
    virtual TagLib::FileName name() const;
    virtual TagLib::ByteVector readBlock(TagLib::ulong length);
    virtual void writeBlock(const TagLib::ByteVector &data);
    virtual void insert(const TagLib::ByteVector &data, TagLib::ulong start=0, TagLib::ulong replace=0);
    virtual void removeBlock(TagLib::ulong start=0, TagLib::ulong length=0);
    virtual bool readOnly() const;
    virtual bool isOpen() const;
    virtual void seek(long offset, TagLib::IOStream::Position p=Beginning);
    virtual void clear();
    virtual long tell() const;
    virtual long length();
    virtual void truncate(long length);
    
private:
    std::unique_ptr<utils::IReader> m_Reader;
    std::string                     m_Uri;
};

}

#endif
