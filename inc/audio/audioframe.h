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

#ifndef AUDIO_FRAME_H
#define AUDIO_FRAME_H

#include <cinttypes>
#include <cstddef>

namespace audio
{

class Frame
{
public:
    Frame();
    virtual ~Frame();

    uint8_t* getFrameData() const;
    size_t getDataSize() const;
    double getPts() const;

    void setFrameData(uint8_t* data);
    void setDataSize(size_t size);
    void setPts(double pts);

    void allocateData(size_t size);
    void freeData();

    void clear();
    void offsetDataPtr(size_t offset);

private:
    uint8_t*    m_pFrameData;
    size_t      m_DataSize;
    bool        m_AllocatedData;
    double      m_Pts;
};

}

#endif
