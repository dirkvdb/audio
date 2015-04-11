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

#include "audioopenalrenderer.h"

#include "audio/audioframe.h"
#include "audio/audioformat.h"
#include "utils/numericoperations.h"
#include "utils/log.h"

#ifdef HAVE_FFMPEG
#include "audioresampler.h"
#endif

#include <cassert>
#include <vector>
#include <stdexcept>

using namespace std;
using namespace utils;

namespace audio
{

OpenALRenderer::OpenALRenderer()
: m_pAudioDevice(nullptr)
, m_pAlcContext(nullptr)
, m_AudioSource(0)
, m_CurrentBuffer(0)
, m_Volume(100)
, m_Muted(false)
, m_FloatingPoint(false)
, m_AudioFormat(AL_FORMAT_STEREO16)
, m_Frequency(0)
{
    m_pAudioDevice = alcOpenDevice(nullptr);

    if (m_pAudioDevice)
    {
        m_pAlcContext = alcCreateContext(m_pAudioDevice, nullptr);
        alcMakeContextCurrent(m_pAlcContext);
    }

    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        log::warn("Openal creation error {}", err);
    }

    alGenBuffers(NUM_BUFFERS, m_AudioBuffers);
    alGenSources(1, &m_AudioSource);
}

OpenALRenderer::~OpenALRenderer()
{
    alSourceStop(m_AudioSource);
    alDeleteSources(1, &m_AudioSource);
    alDeleteBuffers(NUM_BUFFERS, m_AudioBuffers);

    if (m_pAudioDevice)
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(m_pAlcContext);
        alcCloseDevice(m_pAudioDevice);
    }
}

void OpenALRenderer::setFormat(const Format& format)
{
    m_FloatingPoint = false;
    m_Frequency = format.rate;
    m_SampleSize = format.bits / 8;

#ifdef HAVE_FFMPEG
    m_resampler.reset();
#endif

    switch (format.bits)
    {
    case 8:
        m_AudioFormat = format.numChannels == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
        break;
    case 16:
        m_AudioFormat = format.numChannels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        break;
#ifdef HAVE_FFMPEG
    case 24:
    case 32:
        if (format.floatingPoint == false)
        {
            m_AudioFormat = format.numChannels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
            auto destFormat = format;
            destFormat.bits = 16;
            m_resampler = std::make_unique<Resampler>(format, destFormat);
        }
        else
        {
            m_AudioFormat = format.numChannels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
            m_FloatingPoint = true;
        }
        m_SampleSize = 16 / 8;
        break;
#endif
    default:
        throw logic_error(fmt::format("OpenAlRenderer: unsupported format bitdepth ({})", format.bits));
    }
}

bool OpenALRenderer::hasBufferSpace(uint32_t /*dataSize*/)
{
    int queued = 0;
    alGetSourcei(m_AudioSource, AL_BUFFERS_QUEUED, &queued);

    if (queued == 0)
    {
        log::debug("OpenalRenderer: xrun");
    }

    return queued < NUM_BUFFERS;
}

double OpenALRenderer::getBufferDuration()
{
    int queued = 0;
    alGetSourcei(m_AudioSource, AL_BUFFERS_QUEUED, &queued);

    double singleBufferDuration = static_cast<double>(m_FrameSize / static_cast<double>(m_SampleSize)) / m_Frequency;
    return queued * singleBufferDuration;
}

void OpenALRenderer::queueFrame(const Frame& frame)
{
#ifdef HAVE_FFMPEG
    if (m_resampler)
    {
        auto frameData = m_resampler->resample(frame.getFrameData(), frame.getDataSize());
        alBufferData(m_AudioBuffers[m_CurrentBuffer], m_AudioFormat, frameData.data(), static_cast<ALsizei>(frameData.size()), m_Frequency);
    }
    else
#endif
    if (m_FloatingPoint)
    {
        std::vector<int16_t> frameData;
        frameData.resize(frame.getDataSize() / sizeof(float));

        auto pData = reinterpret_cast<const float*>(frame.getFrameData());
        for (auto i = 0u; i < frameData.size(); ++i)
        {
            float sample = numericops::clip(*pData++, -1.f, 1.f);
            frameData[i] = static_cast<int16_t>(sample * 32768.f);
        }

        alBufferData(m_AudioBuffers[m_CurrentBuffer], m_AudioFormat, frameData.data(), static_cast<ALsizei>(frameData.size() * sizeof(uint16_t)), m_Frequency);
        m_FrameSize = static_cast<uint32_t>(frameData.size());
    }
    else
    {
        assert(frame.getFrameData());
        alBufferData(m_AudioBuffers[m_CurrentBuffer], m_AudioFormat, frame.getFrameData(), static_cast<ALsizei>(frame.getDataSize()), m_Frequency);
        m_FrameSize = static_cast<uint32_t>(frame.getDataSize());
    }

    alSourceQueueBuffers(m_AudioSource, 1, &m_AudioBuffers[m_CurrentBuffer]);
    m_PtsQueue.push_back(frame.getPts());

    ++m_CurrentBuffer;
    m_CurrentBuffer %= NUM_BUFFERS;

    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        log::warn("Openal queueFrame error {}", err);
    }
}

void OpenALRenderer::flushBuffers()
{
    int processed = 0;
    alGetSourcei(m_AudioSource, AL_BUFFERS_PROCESSED, &processed);

    while(processed--)
    {
        ALuint buffer;
        alSourceUnqueueBuffers(m_AudioSource, 1, &buffer);
        m_PtsQueue.pop_front();

        ALenum err = alGetError();
        if (err != AL_NO_ERROR)
        {
            log::warn("Openal flushBuffers error {}", err);
        }
    }
}

bool OpenALRenderer::isPlaying()
{
    ALenum state;
    alGetSourcei(m_AudioSource, AL_SOURCE_STATE, &state);

    return (state == AL_PLAYING);
}

void OpenALRenderer::play()
{
    if (!isPlaying() && !m_PtsQueue.empty())
    {
        alSourcePlay(m_AudioSource);
    }
}

void OpenALRenderer::pause()
{
    if (isPlaying())
    {
        alSourcePause(m_AudioSource);
    }
}

void OpenALRenderer::resume()
{
    play();
}

void OpenALRenderer::stop(bool /*drain*/)
{
    alSourceStop(m_AudioSource);
    flushBuffers();
}

void OpenALRenderer::setVolume(int32_t volume)
{
    numericops::clip(m_Volume, 0, 100);
    m_Volume = volume;

    if (!m_Muted)
    {
        alSourcef(m_AudioSource, AL_GAIN, m_Volume / 100.f);
    }
}

int32_t OpenALRenderer::getVolume()
{
    return m_Volume;
}

void OpenALRenderer::setMute(bool enabled)
{
    if (enabled == m_Muted)
    {
        return;
    }

    m_Muted = enabled;
    alSourcef(m_AudioSource, AL_GAIN, m_Muted ? 0.f : m_Volume / 100.f);
}

bool OpenALRenderer::getMute()
{
    return m_Muted;
}

double OpenALRenderer::getCurrentPts()
{
    return m_PtsQueue.empty() ? 0 : m_PtsQueue.front();
}

}
