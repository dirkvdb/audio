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

#ifdef HAVE_UPNP
    #include "audiohttpreader.h"
#endif

namespace audio
{

utils::IReader* ReaderFactory::create(const std::string& uri)
{
	utils::log::info("ReaderFactory: %s", uri);
    if (uri.substr(0, 7) == "http://")
    {
#ifdef HAVE_UPNP
        auto reader = new HttpReader();
        reader->open(uri);
        return reader;
#else
        throw std::logic_error("Not compiled with support for urls as input");
#endif
    }

    return new FileReader(uri);
}

}
   
