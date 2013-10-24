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

#ifndef AUDIO_READER_FACTORY_H
#define AUDIO_READER_FACTORY_H

#include <string>
#include <vector>
#include <memory>

#include "utils/readerinterface.h"

namespace audio
{

class ReaderFactory
{
public:
    static void registerBuilder(std::unique_ptr<utils::IReaderBuilder> builder);
    static utils::IReader* create(const std::string& uri);
    static utils::IReader* createBuffered(const std::string& filepath, uint32_t bufferSize);
    
private:
    static std::vector<std::unique_ptr<utils::IReaderBuilder>>  m_Builders;
};

}

#endif
