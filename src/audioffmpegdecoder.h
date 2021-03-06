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

#ifndef FFMPEG_DECODER_H
#define FFMPEG_DECODER_H

#include <string>
#include <cinttypes>

struct AVFormatContext;
struct AVCodecContext;
struct AVCodecParameters;
struct AVCodec;
struct AVPacket;
struct AVStream;

#include "audio/audiodecoder.h"

extern "C"
{
    #include <libavformat/avformat.h>
}

namespace audio
{

class Frame;
struct Format;

class FFmpegDecoder : public IDecoder
{
public:
    FFmpegDecoder(const std::string& filename);
    ~FFmpegDecoder();

    bool decodeAudioFrame(Frame& audioFrame);
    void seekAbsolute(double time);
    void seekRelative(double offset);

    Format getAudioFormat();

double getProgress();
    double getDuration();
    size_t getFrameSize();

private:
    template <typename T>
    void mergeAudioPlanes(Frame& frame);
    bool readPacket(AVPacket& packet);
    void seek(int64_t timestamp);
    void initialize();
    void destroy();
    void initializeAudio();

    int32_t                 m_AudioStream;
    AVFormatContext*        m_pFormatContext;
    AVCodecContext*         m_pAudioCodecContext;
    AVCodecParameters*      m_pAudioCodecParameters;
    AVCodec*                m_pAudioCodec;
    AVStream*               m_pAudioStream;
    AVFrame*                m_pAudioFrame;
    size_t                  m_BytesPerFrame;
};

}

#endif
