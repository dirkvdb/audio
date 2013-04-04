//    Copyright (C) 2009 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#include "audio/audiobufferedreader.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

namespace audio
{

BufferedReader::BufferedReader(utils::IReader& reader, size_t bufferSize)
: m_Reader(reader)
, m_Buffer(bufferSize, '\0')
, m_BufferStartPosition(0)
, m_BufferOffset(0)
, m_CurrentPosition(0)
, m_ContentLength(0)
, m_BufferFilled(false)
{
}

void BufferedReader::open(const std::string& filename)
{
    m_Reader.open(filename);
    m_ContentLength = m_Reader.getContentLength();
    m_BufferStartPosition = 0;
    m_CurrentPosition = 0;
    m_BufferOffset = 0;
    m_BufferFilled = false;
}

uint64_t BufferedReader::getContentLength()
{
    return m_ContentLength;
}

uint64_t BufferedReader::currentPosition()
{
    return m_CurrentPosition;
}

void BufferedReader::seekAbsolute(uint64_t position)
{
    m_Reader.seekAbsolute(position);
    updateOffsetsAfterSeek(position);
}

void BufferedReader::seekRelative(uint64_t offset)
{
    m_Reader.seekAbsolute(m_CurrentPosition + offset);
    updateOffsetsAfterSeek(m_CurrentPosition + offset);
}

bool BufferedReader::eof()
{
    return m_CurrentPosition >= m_ContentLength;
}

std::string BufferedReader::uri()
{
    return m_Reader.uri();
}

uint64_t BufferedReader::read(uint8_t* pData, uint64_t size)
{
    if (!m_BufferFilled)
    {
        if (size < m_Buffer.size())
        {
            auto read = m_Reader.read(m_Buffer.data(), m_Buffer.size());
            auto advance = std::min(read, size);
            
            memcpy(pData, m_Buffer.data(), size);
            m_BufferStartPosition = m_CurrentPosition;
            m_BufferFilled = true;
            m_CurrentPosition += advance;
            m_BufferOffset = advance;
            return advance;
        }
        else
        {
            // requested data larger then buffer, read directly
            m_CurrentPosition = m_Reader.read(pData, size);
            return size;
        }
    }
    else
    {
        auto bytesInBuffer = m_Buffer.size() - m_BufferOffset;
    
        if (size < bytesInBuffer)
        {
            // sufficient data in the buffer
            memcpy(pData, &m_Buffer[m_BufferOffset], size);
            m_CurrentPosition += size;
            m_BufferOffset += size;
            
            return size;
        }
        else
        {
            // first copy the existing data from the buffer
            memcpy(pData, &m_Buffer[m_BufferOffset], bytesInBuffer);
            m_CurrentPosition += bytesInBuffer;
        
            if (size - bytesInBuffer < m_Buffer.size())
            {
                // one bufferfill is sufficient to provide the data
                
                // fill the buffer with frash data
                auto read = m_Reader.read(m_Buffer.data(), m_Buffer.size());
                auto bytesToRead = std::min(read, size - bytesInBuffer);
                
                // copy the reaming data from the buffer
                memcpy(pData + bytesInBuffer, m_Buffer.data(), bytesToRead);
                
                m_BufferStartPosition = m_CurrentPosition;
                m_CurrentPosition += bytesToRead;
                m_BufferOffset = bytesToRead;
                
                assert(m_BufferOffset <= m_Buffer.size());
                if (m_BufferOffset == m_Buffer.size())
                {
                    // buffer was completely emptied, mark it as not filled
                    m_BufferFilled = false;
                }
                
                return bytesInBuffer + bytesToRead;
            }
            else
            {
                // rest of the data is larger then 1 buffer so read it directly
                auto read = m_Reader.read(pData + bytesInBuffer, size - bytesInBuffer);
                m_CurrentPosition += read;
                m_BufferFilled = false;

                return read + bytesInBuffer;
            }
        }
    }
}

void BufferedReader::updateOffsetsAfterSeek(uint64_t newPosition)
{
    if (m_BufferFilled && (newPosition < (m_BufferStartPosition + m_Buffer.size())))
    {
        // seek is in the buffer range, update the offset
        m_BufferOffset = newPosition - m_BufferStartPosition;
    }
    else
    {
        // seek is not in the buffer range, so buffer is not valid
        m_BufferFilled = false;
    }
    
    m_CurrentPosition = newPosition;
}

}
