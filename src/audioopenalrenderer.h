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

#ifndef OPENAL_RENDERER_H
#define OPENAL_RENDERER_H

#include <al.h>
#include <alc.h>

#include <deque>
#include "audio/audiorenderer.h"
#include "audioconfig.h"

#define NUM_BUFFERS 100

namespace audio
{

class Frame;
class Resampler;
struct Format;

class OpenALRenderer : public IRenderer
{
public:
    OpenALRenderer();
    virtual ~OpenALRenderer();

    // IRenderer
    void setFormat(const Format& format) override;
    void play() override;
    void pause() override;
    void resume() override;
    void stop(bool drain) override;
    void setVolume(int32_t volume) override;
    int32_t getVolume() override;
    void setMute(bool enabled) override;
    bool getMute() override;
    bool isPlaying() override;
    bool hasBufferSpace(uint32_t dataSize) override;
    double getBufferDuration() override;
    void flushBuffers() override;
    void queueFrame(const Frame& frame) override;
    double getCurrentPts() override;
    
private:
    ALCdevice*                  m_pAudioDevice;
    ALCcontext*                 m_pAlcContext;
    ALuint                      m_AudioSource;
    ALuint                      m_AudioBuffers[NUM_BUFFERS];
    int32_t                     m_CurrentBuffer;
    int32_t                     m_Volume;
    bool                        m_Muted;
    bool                        m_FloatingPoint;
    ALenum                      m_AudioFormat;
    ALsizei                     m_Frequency;
    uint32_t                    m_FrameSize; //size one queued audio frame
    uint32_t                    m_SampleSize; //size one audio sample

    std::deque<double>          m_PtsQueue;

#ifdef HAVE_FFMPEG
    std::unique_ptr<Resampler>  m_resampler;
#endif
};

}

#endif
