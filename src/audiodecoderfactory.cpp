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

#include "audio/audiodecoderfactory.h"

#include <stdexcept>

#include "utils/fileoperations.h"
#include "utils/stringoperations.h"

#include "audioconfig.h"

#ifdef HAVE_MAD
#include "audiomaddecoder.h"
#endif

#ifdef HAVE_FLAC
#include "audioflacdecoder.h"
#endif

#ifdef HAVE_FFMPEG
#include "audioffmpegdecoder.h"
#endif

using namespace utils;

namespace audio
{

IDecoder* DecoderFactory::create(const std::string& filepath)
{
    std::string extension = fileops::getFileExtension(filepath);
    str::lowercase_in_place(extension);

    if (extension == "mp3")
    {
#ifdef HAVE_MAD
        return new MadDecoder(filepath);
#endif
    }
    else if (extension == "flac")
    {
#if defined(HAVE_FLAC)
        return new FlacDecoder(filepath);
#elif defined(HAVE_FFMPEG)
        return new FFmpegDecoder(filepath);
#else
        throw std::logic_error("Flac support not enabled");
#endif
    }

#ifdef HAVE_FFMPEG
    return new FFmpegDecoder(filepath);
#else
    throw std::logic_error("No valid decoder found for: " + filepath);
#endif
}

} // namespace audio
