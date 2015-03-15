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

#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include <vector>
#include <cinttypes>

extern "C"
{
    #include <libavutil/samplefmt.h>
    #include <libswresample/swresample.h>
}

namespace audio
{

struct Format;

class Resampler
{
public:
    Resampler(const Format& source, const Format& destination);

    std::vector<uint8_t> resample(const uint8_t* pInput, size_t inputSize);

private:
    SwrContext*     m_pContext;
    uint8_t**       m_pData;
    int32_t         m_lineSize;
    uint32_t        m_sourceBits;
    uint32_t        m_sourceRate;
    uint32_t        m_destinationRate;
    int32_t         m_destinationSampleCount;
    AVSampleFormat  m_destinationSampleFormat;
    int64_t         m_destinationChannelCount;
};

}

#endif
