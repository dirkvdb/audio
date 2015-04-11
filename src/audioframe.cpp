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

#include "audio/audioframe.h"

#include <cassert>

#include "utils/log.h"

namespace audio
{

Frame::Frame()
: m_pFrameData(nullptr)
, m_DataSize(0)
, m_AllocatedData(false)
, m_Pts(0.0)
{
}

Frame::~Frame()
{
    freeData();
}

uint8_t* Frame::getFrameData() const
{
    return m_pFrameData;
}

size_t Frame::getDataSize() const
{
    return m_DataSize;
}

double Frame::getPts() const
{
    return m_Pts;
}

void Frame::setFrameData(uint8_t* data)
{
    freeData();
    m_pFrameData = data;
}

void Frame::setDataSize(size_t size)
{
    m_DataSize = size;
}

void Frame::setPts(double pts)
{
    m_Pts = pts;
}

void Frame::clear()
{
    freeData();

    m_pFrameData = nullptr;
    m_DataSize = 0;
    m_Pts = 0.0;
}

void Frame::offsetDataPtr(size_t offset)
{
    if (offset >= m_DataSize)
    {
        m_pFrameData = nullptr;
        m_DataSize = 0;
    }
    else
    {
        m_pFrameData += offset;
        m_DataSize -= offset;
    }
}

void Frame::allocateData(size_t size)
{
    if (m_AllocatedData && m_DataSize == size)
    {
        return;
    }
    
    freeData();
    m_pFrameData = new uint8_t[size];
    m_DataSize = size;
    m_AllocatedData = true;
}

void Frame::freeData()
{
    if (m_AllocatedData)
    {
        delete[] m_pFrameData;
        m_DataSize = 0;
        m_AllocatedData = false;
    }
} 

}
