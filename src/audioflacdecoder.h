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

#ifndef AUDIO_FLAC_DECODER_H
#define AUDIO_FLAC_DECODER_H

#include <string>
#include <vector>
#include <memory>

#include <cassert>
#include <cinttypes>

#include "audio/audiodecoder.h"
#include "audio/audioformat.h"

#include "utils/readerinterface.h"

#include <FLAC++/decoder.h>

namespace audio
{

class Frame;
class IReader;

class FlacDecoder : public IDecoder
                  , public FLAC::Decoder::Stream
{
public:
    FlacDecoder(const std::string& uri);
    ~FlacDecoder();

    bool decodeAudioFrame(Frame& audioFrame);
    void seekAbsolute(double time);
    void seekRelative(double offset);

    Format getAudioFormat();

    double  getProgress();
    double  getDuration();
    size_t getFrameSize();

protected:
    FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[], size_t* pBytes);
    FLAC__StreamDecoderSeekStatus seek_callback(FLAC__uint64 pPosition);
    FLAC__StreamDecoderTellStatus tell_callback(FLAC__uint64* pCurrentPosition);
    FLAC__StreamDecoderLengthStatus length_callback(FLAC__uint64* pStreamLength);
    bool eof_callback();
    FLAC__StreamDecoderWriteStatus write_callback(const FLAC__Frame* pFrame, const FLAC__int32* const pBuffer[]);
    void metadata_callback(const FLAC__StreamMetadata* pMetadata);
    void error_callback(FLAC__StreamDecoderErrorStatus status);

private:
    std::vector<uint8_t>            m_AudioBuffer;
    size_t                          m_BytesPerFrame;
    uint64_t                        m_NumSamples;
    Format                          m_Format;
    std::unique_ptr<utils::IReader> m_Reader;
};

}

#endif
