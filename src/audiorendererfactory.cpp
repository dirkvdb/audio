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

#include "audio/audiorendererfactory.h"

#include "audioconfig.h"

#if HAVE_OPENAL
#include "audioopenalrenderer.h"
#endif

#if HAVE_ALSA
#include "audioalsarenderer.h"
#endif

#if HAVE_PULSE
#include "audiopulserenderer.h"
#endif

#include <stdexcept>

namespace audio
{

IRenderer* RendererFactory::create(const std::string& applicationName, const std::string& audioBackend, const std::string& deviceName)
{
    if (audioBackend == "OpenAL")
    {
#if HAVE_OPENAL
        return new OpenALRenderer();
#else
        throw std::logic_error("AudioRendererFactory: package was not compiled with OpenAl support");
#endif
    }

    if (audioBackend == "Alsa")
    {
#if HAVE_ALSA
        return new AlsaRenderer(deviceName);
#else
        (void) deviceName;
        throw std::logic_error("AudioRendererFactory: package was not compiled with Alsa support");
#endif
    }

    if (audioBackend == "PulseAudio")
    {
#if HAVE_PULSE
        return new PulseRenderer(applicationName);
#else
        (void) applicationName;
        throw std::logic_error("AudioRendererFactory: package was not compiled with PulseAudio support");
#endif
    }

    throw std::logic_error("AudioRendererFactory: Unsupported audio output type provided: " + audioBackend);
}

}
