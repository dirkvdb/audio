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

#ifndef ALSA_RENDERER_H
#define ALSA_RENDERER_H

#include "audio/audioformat.h"
#include "audio/audiorenderer.h"
#include "audiobuffer.h"

#include <alsa/asoundlib.h>
#include <string>
#include <deque>

namespace audio
{

class AlsaRenderer : public IRenderer
{
public:
    AlsaRenderer(const std::string& deviceName);
    ~AlsaRenderer();

    void setFormat(const Format& format);

    void play();
    void pause();
    void resume();
    void stop(bool drain);
    void setVolume(int32_t volume);
    int32_t getVolume();
    void setMute(bool enabled);
    bool getMute();

    bool hasBufferSpace(uint32_t dataSize);
    double getBufferDuration() override;
    void flushBuffers();
    void queueFrame(const Frame& frame);

    double getCurrentPts();

    bool isPlaying();

private:
    void throwOnError(int err, const std::string& message);
    snd_pcm_state_t getDeviceStatus();
    std::string getDeviceStatusString(snd_pcm_state_t status);
    void setHardwareParams(snd_pcm_format_t format, uint32_t channels, uint32_t rate);
    void setSoftwareParams();
    void applyVolume(uint8_t* pData, uint32_t dataSize);

    snd_pcm_t*              m_pAudioDevice;
    snd_pcm_uframes_t       m_bufferSize;
    snd_pcm_uframes_t       m_periodSize;
    uint32_t                m_bufferTime;
    uint32_t                m_periodTime;

    Format                  m_format;
    int                     m_volume;
    int                     m_volumeAtMute;
    bool                    m_muted;
    
    int                     m_frameSize;
    double                  m_lastPts;

    bool                    m_supportPause;

    Buffer                  m_buffer;
};

}

#endif //ALSA_RENDERER_H
