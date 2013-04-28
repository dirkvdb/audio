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

#include "audiotaglibiostream.h"

#include "utils/log.h"

#include "audio/audioreaderfactory.h"

using namespace TagLib;

namespace audio
{

TaglibIOStream::TaglibIOStream(const std::string& url)
: m_Reader(ReaderFactory::create(url))
, m_BufReader(new BufferedReader(*m_Reader, 512))
{
}

TaglibIOStream::~TaglibIOStream()
{
    m_BufReader.reset();
    m_Reader.reset();
}

FileName TaglibIOStream::name() const
{
    return m_BufReader->uri().c_str();
}

ByteVector TaglibIOStream::readBlock(ulong readLength)
{
    ByteVector data;
    data.resize(readLength);
    data.resize(m_BufReader->read(reinterpret_cast<uint8_t*>(data.data()), readLength));
    
    return data;
}

void TaglibIOStream::writeBlock(const TagLib::ByteVector&)
{
    throw std::logic_error("Writing is not supported");
}

void TaglibIOStream::insert(const TagLib::ByteVector&, ulong, ulong)
{
    throw std::logic_error("Insert is not supported");
}

void TaglibIOStream::removeBlock(ulong, ulong)
{
    throw std::logic_error("Remove is not supported");
}

bool TaglibIOStream::readOnly() const
{
    return true;
}

bool TaglibIOStream::isOpen() const
{
    return true;
}

void TaglibIOStream::seek(long offset, Position position)
{
    switch (position)
    {
        case Beginning:
            m_BufReader->seekAbsolute(offset);
            break;
        case Current:
            m_BufReader->seekRelative(offset);
            break;
        case End:
            m_BufReader->seekAbsolute((length() - 1) - offset);
            break;
        default:
            throw std::logic_error("Invalid seek position");
    }
}

void TaglibIOStream::clear()
{
    m_BufReader.reset();
}

long TaglibIOStream::tell() const
{
    return m_BufReader->currentPosition();
}

long TaglibIOStream::length()
{
    return m_BufReader->getContentLength();
}

void TaglibIOStream::truncate(long length)
{
    throw std::logic_error("Truncate is not supported");
}

}
