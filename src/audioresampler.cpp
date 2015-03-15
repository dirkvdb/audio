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

#include "audioresampler.h"
#include "utils/format.h"
#include "audio/audioformat.h"

#include <cassert>

extern "C"
{
    #include <libavutil/opt.h>
    #include <libavcodec/avcodec.h>
}

namespace audio
{

namespace
{

int64_t channelLayoutFromNumChannels(uint32_t numChannels)
{
    switch (numChannels)
    {
        case 0:     throw std::invalid_argument("Audio format with 0 channels");
        case 1:     return AV_CH_LAYOUT_MONO;
        case 2:     return AV_CH_LAYOUT_STEREO;
        default:    return AV_CH_LAYOUT_SURROUND;
    }
}

AVSampleFormat sampleFormatFromFormat(const Format& fmt)
{
    switch (fmt.bits)
    {
        case 8:     return AV_SAMPLE_FMT_U8;
        case 16:    return AV_SAMPLE_FMT_S16;
        case 24:
        case 32:    return fmt.floatingPoint ? AV_SAMPLE_FMT_FLT : AV_SAMPLE_FMT_S32;
        default:
            throw std::logic_error(fmt::format("Audio resampler: unsupported format bitdepth ({})", fmt.bits));
    }
}

}

Resampler::Resampler(const Format& source, const Format& destination)
: m_pContext(swr_alloc())
, m_pData(nullptr)
, m_lineSize(0)
, m_sourceBits(source.bits)
, m_sourceRate(source.rate)
, m_destinationRate(destination.rate)
, m_destinationSampleCount(0)
, m_destinationSampleFormat(sampleFormatFromFormat(destination))
, m_destinationChannelCount(channelLayoutFromNumChannels(destination.numChannels))
{
    assert(m_pContext);

    av_opt_set_int(m_pContext,          "in_channel_layout",    channelLayoutFromNumChannels(source.numChannels), 0);
    av_opt_set_int(m_pContext,          "in_sample_rate",       source.rate, 0);
    av_opt_set_sample_fmt(m_pContext,   "in_sample_fmt",        sampleFormatFromFormat(source), 0);

    av_opt_set_int(m_pContext,          "out_channel_layout",   m_destinationChannelCount, 0);
    av_opt_set_int(m_pContext,          "out_sample_rate",      destination.rate, 0);
    av_opt_set_sample_fmt(m_pContext,   "out_sample_fmt",       m_destinationSampleFormat, 0);

    if (swr_init(m_pContext) < 0)
    {
        swr_free(&m_pContext);
        throw std::runtime_error("Failed to initialize the resampling context");
    }
}

std::vector<uint8_t> Resampler::resample(const uint8_t* pInput, size_t inputSize)
{
    // compute destination number of samples
    m_destinationSampleCount = av_rescale_rnd(swr_get_delay(m_pContext, m_sourceRate) + inputSize / (m_sourceBits / 8), m_destinationRate, m_sourceRate, AV_ROUND_UP);

    int64_t channels = 0;
    av_get_channel_layout_nb_channels(av_opt_get_int(m_pContext, "out_channel_layout", 0, &channels));
    auto ret = av_samples_alloc_array_and_samples(&m_pData, &m_lineSize, channels, m_destinationSampleCount, m_destinationSampleFormat, 0);
    if (ret < 0)
    {
        swr_free(&m_pContext);
        throw std::runtime_error("Failed to allocate destination resample buffer");
    }

    ret = swr_convert(m_pContext, m_pData, m_destinationSampleCount, (const uint8_t**)(pInput), inputSize / (m_sourceBits / 8));
    if (ret < 0)
    {
        throw std::runtime_error("Error while resampling");
    }

    auto bufsize = av_samples_get_buffer_size(&m_lineSize, m_destinationChannelCount, ret, m_destinationSampleFormat, 1);
    std::vector<uint8_t> result(bufsize);
    
    return result;
}

}
