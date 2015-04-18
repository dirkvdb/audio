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

#ifndef PULSE_RENDERER_H
#define PULSE_RENDERER_H

#include <deque>
#include <vector>
#include <pulse/pulseaudio.h>

#include "audiobuffer.h"
#include "audio/audioformat.h"
#include "audio/audiorenderer.h"


namespace audio
{

class Frame;
struct Format;


class PulseRenderer : public IRenderer
{
public:
    PulseRenderer(const std::string& name);
    virtual ~PulseRenderer();

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
    static void contextStateCb(pa_context* pContext, void* pData);
    static void contextSubscriptionCb(pa_context* pContext, pa_subscription_event_type_t type, uint32_t index, void* pData);
    static void streamStateCb(pa_stream* pStream, void* pData);
    static void streamUnderflowCb(pa_stream* pStream, void* pData);
    static void sinkInputInfoCb(pa_context* pContext, const pa_sink_input_info* pInfo, int eol, void* pData);
    static void streamSuccessCb(pa_stream* pStream, int success, void* pData);
    static void streamUpdateTimingCb(pa_stream* pStream, int success, void* pData);

    void writeSilentData();

    bool pulseIsReady();

    pa_context*                 m_pPulseContext;
    pa_threaded_mainloop*       m_pPulseLoop;
    pa_mainloop_api*            m_pMainloopApi;
    pa_stream*                  m_pStream;
    pa_channel_map              m_ChannelMap;
    pa_sample_spec              m_SampleFormat;
    Format                      m_Format;
    pa_cvolume                  m_Volume;
    int32_t                     m_VolumeInt;
    int32_t                     m_VolumeAtMute;
    bool                        m_Muted;
    bool                        m_IsPlaying;

    double                      m_LastPts;
    pa_usec_t                   m_Latency;
    uint32_t                    m_FrameSize;
    uint32_t                    m_HWBufferSize;

    Buffer                      m_Buffer;
};

}

#endif
