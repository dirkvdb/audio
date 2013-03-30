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

#include "audio/audioreaderfactory.h"

#include <stdexcept>

#include "audio/audiofilereader.h"
#include "utils/log.h"

#include "audioconfig.h"

namespace audio
{

std::vector<std::unique_ptr<utils::IReaderBuilder>> ReaderFactory::m_Builders;

void ReaderFactory::registerBuilder(std::unique_ptr<utils::IReaderBuilder> builder)
{
    m_Builders.push_back(std::move(builder));
}

utils::IReader* ReaderFactory::create(const std::string& uri)
{
    for (auto& builder : m_Builders)
    {
        if (builder->supportsUri(uri))
        {
            return builder->build(uri);
        }
    }

    // by default just try to read it from the filesystem
    return new FileReader();
}

}
   
