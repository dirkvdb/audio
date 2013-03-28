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

#ifndef AUDIO_HTTP_READER_H
#define AUDIO_HTTP_READER_H

#include <string>

#include "utils/readerinterface.h"

namespace audio
{

class HttpReader : public utils::IReader
{
public:
    HttpReader();
    ~HttpReader();
    
    virtual void open(const std::string& url);

    virtual uint64_t getContentLength();
    virtual uint64_t currentPosition();
    virtual bool eof();
    virtual std::string uri();
    
    virtual void seekAbsolute(uint64_t position);
    virtual void seekRelative(uint64_t offset);
    virtual uint64_t read(uint8_t* pData, uint64_t size);
    
private:
    std::string m_Url;
    uint64_t    m_ContentLength;
    uint64_t    m_CurrentPosition;
};

}

#endif
