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

#include "audio/audioplaybackfactory.h"

#include "audioconfig.h"

#ifdef HAVE_GSTREAMER
#include "gstreamerengine.h"
#endif

#include "audioplayback.h"
#include <stdexcept>

namespace audio
{

IPlayback* PlaybackFactory::create(const std::string& engine, const std::string& audioOutput, const std::string& audioDevice, audio::IPlaylist& playlist)
{
    if (engine == "GStreamer")
    {
#ifdef HAVE_GSTREAMER
        return new audio::GStreamerEngine(playlist);
#else
        throw std::logic_error("PlaybackEngineFactory: package was not compiled with Gstreamer support");
#endif
    }

    if (engine == "FFmpeg")
    {
        return new audio::Playback(playlist, audioOutput, audioDevice);
    }

    throw std::logic_error("PlaybackFactory: Unsupported playback engine type provided: " + engine);
}

}
