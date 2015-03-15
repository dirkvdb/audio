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

#include "audioalsarenderer.h"
#include "audio/audioframe.h"

#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cassert>
#include <cmath>

#include "utils/log.h"
#include "utils/numericoperations.h"

using namespace std;
using namespace utils;

namespace audio
{

AlsaRenderer::AlsaRenderer(const std::string& deviceName)
: m_pAudioDevice(nullptr)
, m_bufferSize(0)
, m_periodSize(0)
, m_bufferTime(1000000) //1 second buffer
, m_periodTime(200000)
, m_volume(100)
, m_muted(false)
, m_frameSize(0)
, m_lastPts(0.0)
, m_supportPause(true)
, m_buffer(1024 * 1024)
{
    throwOnError(snd_pcm_open(&m_pAudioDevice, deviceName.c_str(), SND_PCM_STREAM_PLAYBACK, 0), "Error opening PCM device " + deviceName);
}

AlsaRenderer::~AlsaRenderer()
{
    if (m_pAudioDevice)
    {
        stop(false);
        snd_pcm_close(m_pAudioDevice);
    }
}

void AlsaRenderer::setHardwareParams(snd_pcm_format_t format, uint32_t channels, uint32_t rate)
{
    snd_pcm_hw_params_t* pHwParams = nullptr;
    snd_pcm_hw_params_alloca(&pHwParams);

    int dir;

    throwOnError(snd_pcm_hw_params_any(m_pAudioDevice, pHwParams), "Broken configuration for playback: no configurations available");
    throwOnError(snd_pcm_hw_params_set_rate_resample(m_pAudioDevice, pHwParams, 1), "Resampling setup failed for playback");
    throwOnError(snd_pcm_hw_params_set_access(m_pAudioDevice, pHwParams, SND_PCM_ACCESS_RW_INTERLEAVED), "Access type not available for playback");
    throwOnError(snd_pcm_hw_params_set_format(m_pAudioDevice, pHwParams, format), "Sample format not available for playback");
    throwOnError(snd_pcm_hw_params_set_channels(m_pAudioDevice, pHwParams, channels), "Channels count (%i) not available for playback");

    uint32_t rrate = rate;
    throwOnError(snd_pcm_hw_params_set_rate_near(m_pAudioDevice, pHwParams, &rrate, 0), "Rate not available for playback");
    if (rrate != rate)
    {
        throwOnError(-1, "Rate doesn't match");
    }

    throwOnError(snd_pcm_hw_params_set_buffer_time_near(m_pAudioDevice, pHwParams, &m_bufferTime, &dir), "Unable to set buffer time for playback");
    throwOnError(snd_pcm_hw_params_get_buffer_size(pHwParams, &m_bufferSize), "Unable to get buffer size for playback");

    uint32_t maxPeriodTime = m_bufferTime / 2;
    throwOnError(snd_pcm_hw_params_set_period_time_max(m_pAudioDevice, pHwParams, &maxPeriodTime, &dir), "Failed to set max period time");
    throwOnError(snd_pcm_hw_params_set_period_time_near(m_pAudioDevice, pHwParams, &m_periodTime, &dir), "Unable to set period time for playback");
    throwOnError(snd_pcm_hw_params_get_period_size(pHwParams, &m_periodSize, &dir), "Unable to get period size for playback");
    throwOnError(snd_pcm_hw_params(m_pAudioDevice, pHwParams), "Unable to set hw params for playback");

    if (!snd_pcm_hw_params_can_pause(pHwParams))
    {
        log::warn("Sound card does not support pause");
        m_supportPause = false;
    }
}

void AlsaRenderer::setSoftwareParams()
{
    snd_pcm_sw_params_t* pSwParams = nullptr;
    snd_pcm_sw_params_alloca(&pSwParams);

    throwOnError(snd_pcm_sw_params_current(m_pAudioDevice, pSwParams), "Unable to determine current pSwParams for playback");

    /* start the transfer when the buffer is almost full: */
    /* (buffer_size / avail_min) * avail_min */
    throwOnError(snd_pcm_sw_params_set_start_threshold(m_pAudioDevice, pSwParams, (m_bufferSize / m_periodSize) * m_periodSize), "Unable to set start threshold mode for playback");
    /* allow the transfer when at least period_size samples can be processed */
    throwOnError(snd_pcm_sw_params_set_avail_min(m_pAudioDevice, pSwParams, m_periodSize), "Unable to set avail min for playback");
    /* write the parameters to the playback device */
    throwOnError(snd_pcm_sw_params(m_pAudioDevice, pSwParams), "Unable to set sw params for playback");
}

void AlsaRenderer::setFormat(const Format& format)
{
    if (format == m_format)
    {
        log::debug("Format is the same");
        return;
    }

    if (format.numChannels == 0 || format.rate == 0)
    {
        log::warn("Format contains nonsense values, ignoring");
        return;
    }

    log::debug("Format has changed %d %d %d %d %d", format.bits, format.rate, format.numChannels, format.framesPerPacket, format.floatingPoint);
    snd_pcm_format_t formatType;
    if (!format.floatingPoint)
    {
        switch (format.bits)
        {
        case 32:
            formatType = SND_PCM_FORMAT_S32_LE;
            break;
        case 24:
            formatType = SND_PCM_FORMAT_S24_LE;
            break;
        case 16:
            formatType = SND_PCM_FORMAT_S16_LE;
            break;
        default:
            throw logic_error("AlsaRenderer: unsupported format");
        }
    }
    else
    {
        switch (format.bits)
        {
        case 32:
            formatType = SND_PCM_FORMAT_FLOAT_LE;
            break;
        default:
            throw logic_error("AlsaRenderer: unsupported format");   
        }
    }

    setHardwareParams(formatType, format.numChannels, format.rate);
    setSoftwareParams();

    //log::debug("Buffer size:", m_BufferSize, "Period size:", m_PeriodSize);

    int bytesPerSample = snd_pcm_format_width(formatType) / 8;
    m_frameSize = format.numChannels * bytesPerSample;
    
    m_format = format;
}

void AlsaRenderer::play()
{
    snd_pcm_state_t status = getDeviceStatus();
    switch (status)
    {
    case SND_PCM_STATE_PAUSED:
        log::debug("Alsa renderer paused, no good");
        throwOnError(snd_pcm_pause(m_pAudioDevice, 0), "Error resuming playback");
        break;
    case SND_PCM_STATE_SETUP:
        log::debug("Alsa renderer in setup state");
        snd_pcm_prepare(m_pAudioDevice);
    case SND_PCM_STATE_PREPARED:
    {
        log::debug("Alsa renderer prepared, starting playback");
        //throwOnError(snd_pcm_start(m_pAudioDevice), "Error starting playback");
        snd_pcm_start(m_pAudioDevice);
        break;
    }
    case SND_PCM_STATE_RUNNING:
        break;
    case SND_PCM_STATE_XRUN:
        snd_pcm_prepare(m_pAudioDevice);
        play();
    default:
        log::debug("Alsa renderer in unexpected state: %s", getDeviceStatusString(status));
    }
}

std::string AlsaRenderer::getDeviceStatusString(snd_pcm_state_t status)
{
    switch (status)
    {
    case SND_PCM_STATE_PAUSED:
        return "SND_PCM_STATE_PAUSED";
    case SND_PCM_STATE_SETUP:
        return "SND_PCM_STATE_SETUP";
    case SND_PCM_STATE_PREPARED:
        return "SND_PCM_STATE_PREPARED";
    case SND_PCM_STATE_SUSPENDED:
        return "SND_PCM_STATE_SUSPENDED";
    case SND_PCM_STATE_DISCONNECTED:
        return "SND_PCM_STATE_DISCONNECTED";
    case SND_PCM_STATE_XRUN:
        return "SND_PCM_STATE_XRUN";
    case SND_PCM_STATE_DRAINING:
        return "SND_PCM_STATE_DRAINING";
    case SND_PCM_STATE_RUNNING:
        return "SND_PCM_STATE_RUNNING";
    default:
        return "UNKNOWN";
    }
}

void AlsaRenderer::pause()
{
    if (getDeviceStatus() == SND_PCM_STATE_RUNNING)
    {
        if (m_supportPause)
        {
            throwOnError(snd_pcm_pause(m_pAudioDevice, 1), "Error pausing playback");
        }
        else
        {
            stop(true);
        }
    }
}

void AlsaRenderer::resume()
{
    if (m_supportPause)
    {
        if (getDeviceStatus() == SND_PCM_STATE_PAUSED)
        {
            throwOnError(snd_pcm_pause(m_pAudioDevice, 0), "Error resuming playback");
        }
    }
    else
    {
        if (getDeviceStatus() == SND_PCM_STATE_SETUP)
        {
            play();
        }
    }
}

void AlsaRenderer::stop(bool drain)
{
    if (getDeviceStatus() == SND_PCM_STATE_RUNNING)
    {
        m_buffer.clear();
        m_lastPts = 0.0;
        
        if (drain)
        {
            throwOnError(snd_pcm_drain(m_pAudioDevice), "Error stopping playback");
        }
        else
        {
            throwOnError(snd_pcm_drop(m_pAudioDevice), "Error stopping playback");
        }
    }
}

void AlsaRenderer::setVolume(int32_t volume)
{
    numericops::clip(volume, 0, 100);
    m_volume = volume;
}

int32_t AlsaRenderer::getVolume()
{
    return m_volume;
}

void AlsaRenderer::setMute(bool enabled)
{
    if (enabled == m_muted)
    {
        return;
    }
    
    m_muted = enabled;
    if (m_muted)
    {
        m_volumeAtMute  = m_volume;
        m_volume        = 0;
    }
    else
    {
        m_volume = m_volumeAtMute;
    }
}

bool AlsaRenderer::getMute()
{
    return m_muted;
}

snd_pcm_state_t AlsaRenderer::getDeviceStatus()
{
    snd_pcm_state_t state = snd_pcm_state(m_pAudioDevice);
    return state;
}

bool AlsaRenderer::hasBufferSpace(uint32_t dataSize)
{
    if (getDeviceStatus() == SND_PCM_STATE_PAUSED)
    {
        return false;
    }

    return m_buffer.bytesFree() >= static_cast<uint32_t>(dataSize);
}

double AlsaRenderer::getBufferDuration()
{
    snd_pcm_sframes_t available = snd_pcm_avail_update(m_pAudioDevice);
    uint32_t availableBytes = snd_pcm_frames_to_bytes(m_pAudioDevice, available);
    uint32_t bufferSize = snd_pcm_frames_to_bytes(m_pAudioDevice, m_bufferSize);

    auto bytesInBuffer = bufferSize - availableBytes;
    return static_cast<double>(bytesInBuffer / (m_format.bits / 8.0)) / m_format.rate;
}

bool AlsaRenderer::isPlaying()
{
    return getDeviceStatus() == SND_PCM_STATE_RUNNING;
}

void AlsaRenderer::flushBuffers()
{
    int deviceStatus = getDeviceStatus();
    if (deviceStatus == SND_PCM_STATE_XRUN)
    {
        log::debug("Recover from xrun");
        snd_pcm_prepare(m_pAudioDevice);
        snd_pcm_start(m_pAudioDevice);
    }
    
    snd_pcm_sframes_t available = snd_pcm_avail_update(m_pAudioDevice);
    uint32_t availableBytes = snd_pcm_frames_to_bytes(m_pAudioDevice, available);
    
    //log::debug("Alsa av: %d Buf av: %d (%d - %d)", availableBytes, m_Buffer.bytesUsed(), m_PeriodSize, m_BufferSize);

    if (availableBytes != 0 && m_buffer.bytesUsed() > availableBytes)
    {
        uint32_t size = availableBytes;
        uint8_t* pData = m_buffer.getData(size);
        
        applyVolume(pData, size);
    
        snd_pcm_sframes_t dataFrames = snd_pcm_bytes_to_frames(m_pAudioDevice, size);

        if (size > availableBytes)
        {
            log::warn("Frame is bigger than available size: frameSize: %d Avail: %d", dataFrames, available);
        }

        //log::debug("Write frame: alsaAvB: %d AudioBufAvB %d %d", availableBytes, size, m_Buffer.bytesUsed());
        snd_pcm_sframes_t status = snd_pcm_writei(m_pAudioDevice, pData, dataFrames);

        if (status < 0)
        {
            if (status == -EPIPE)
            {
                log::warn("Alsa: Failed to write frame data: underrun occured (%s) %d", snd_strerror(status), m_buffer.bytesUsed());
                snd_pcm_prepare(m_pAudioDevice);
                snd_pcm_start(m_pAudioDevice);
            }
            else if (status == -EBADFD)
            {
                log::error("Alsa: Failed to write frame data: stream not in right state (%s)", snd_strerror(status));
                snd_pcm_prepare(m_pAudioDevice);
                snd_pcm_start(m_pAudioDevice);
            }
            else if (status == -ESTRPIPE)
            {
                log::warn("AlsaCallback: Failed to write frame data: suspend event occured %s %d", snd_strerror(status), m_buffer.bytesUsed());
            }
            else if (status == -EAGAIN)
            {
                log::warn("AlsaCallback: Failed to write frame data: try again (%s)", snd_strerror(status));
                if (snd_pcm_writei(m_pAudioDevice, pData, dataFrames) < 0)
                {
                    log::error("AlsaCallback: Failed again, too bad (%s)", snd_strerror(status));
                }
            }
            else
            {
                log::error("AlsaCallback: unknown error: %s", snd_strerror(status));
            }
        }
        
        flushBuffers();
    }
}

void AlsaRenderer::queueFrame(const Frame& frame)
{
    if (m_frameSize == 0)
    {
        throw logic_error("Alsarenderer: Audio format was never set");
    }

    m_buffer.writeData(frame.getFrameData(), frame.getDataSize());
    m_lastPts = frame.getPts();

    flushBuffers();
}

void AlsaRenderer::applyVolume(uint8_t* pData, uint32_t dataSize)
{
    if (m_volume == 100)
    {
        return;
    }

    if (m_format.bits == 16)
    {
        int scaleFactor = (m_volume * 256) / 100;

        for (uint32_t i = 0; i < dataSize; i+=2)
        {
            short* pSample = reinterpret_cast<short*>(&pData[i]);
            int sample = ((*pSample) * scaleFactor + 128) >> 8;
            numericops::clip(sample, -32768, 32767);
            *pSample = sample;
        }
    }
    else if (m_format.bits == 32)
    {
        float scaleFactor = m_volume / 100.0;

        for (uint32_t i = 0; i < dataSize; i+=4)
        {
            float* pSample = reinterpret_cast<float*>(pData[i]);
            *pSample = static_cast<float>(*pSample * scaleFactor);
        }
    }
    else
    {
        assert(false); //implement volume scaling for this bitsize
    }
}

double AlsaRenderer::getCurrentPts()
{
    double bufferDelay = 0.0;

    snd_pcm_sframes_t frames;
    if (snd_pcm_delay(m_pAudioDevice, &frames) == 0)
    {
        bufferDelay = static_cast<double>(frames) / m_format.rate;
    }
    
    bufferDelay += m_buffer.bytesUsed() / static_cast<double>(m_frameSize * m_format.rate);

    return std::max(0.0, m_lastPts - bufferDelay);
}

void AlsaRenderer::throwOnError(int err, const string& message)
{
    if (err < 0)
    {
        throw logic_error("AlsaRenderer: " + message + ": " + string(snd_strerror(err)));
    }
}

}

