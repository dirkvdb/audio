#include <iostream>
#include <fstream>

#include "utils/log.h"
#include "utils/fileoperations.h"
#include "utils/filereader.h"
#include "utils/bufferedreader.h"
#include "audio/audiometadata.h"
#include "audio/audiompegutils.h"

using namespace std;
using namespace utils;
using namespace audio;

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            log::error("Usage: %s filename", argv[0]);
            return -1;
        }
        
        Metadata meta(argv[1], Metadata::ReadAudioProperties::Yes);
        log::info("Artist: %s", meta.getArtist());
        log::info("Title: %s", meta.getTitle());
        
        auto data = meta.getAlbumArt();
        if (!data.data.empty())
        {
            ofstream ostr("cover.jpg", std::ios::binary);
            ostr.write(reinterpret_cast<const char*>(data.data.data()), data.data.size());
        }
        else
        {
            log::warn("No album art found");
        }
        
        BufferedReader reader(std::unique_ptr<FileReader>(new FileReader()), 512);
        reader.open(argv[1]);
        
        uint64_t filesize = reader.getContentLength();

        MpegUtils::MpegHeader mpegHeader;
        
        uint32_t id3Size = MpegUtils::skipId3Tag(reader);
        log::info("Id3 size: %d", id3Size);

        uint32_t xingPos;
        if (MpegUtils::readMpegHeader(reader, mpegHeader, xingPos) == 0)
        {
            //try a bruteforce scan in the first meg to be really sure
            if (MpegUtils::searchMpegHeader(reader, mpegHeader, id3Size, xingPos) == 0)
            {
                log::info("No mpeg header found");
                return 0;
            }
        }

        log::info("Mpeg Header");
        log::info("-----------");

        switch (mpegHeader.version)
        {
        case MpegUtils::MpegHeader::Mpeg1:
            log::info("Mpeg Version 1");
            break;
        case MpegUtils::MpegHeader::Mpeg2:
            log::info("Mpeg Version 2");
            break;
        case MpegUtils::MpegHeader::Mpeg2_5:
            log::info("Mpeg Version 2.5");
            break;
        default:
            log::info("Mpeg Version unknown");
        }

        log::info("Layer %d", mpegHeader.layer);

        switch (mpegHeader.channelMode)
        {
        case MpegUtils::MpegHeader::Mono:
            log::info("Mono");
            break;
        case MpegUtils::MpegHeader::Stereo:
            log::info("Stereo");
            break;
        case MpegUtils::MpegHeader::DualChannel:
            log::info("Dual channel");
            break;
        case MpegUtils::MpegHeader::JointStereo:
            log::info("Joint stereo");
            break;
        default:
            break;
        };

        
        MpegUtils::XingHeader xingHeader;
        
        reader.seekAbsolute(id3Size + xingPos);
        if (MpegUtils::readXingHeader(reader, xingHeader) == 0)
        {
            log::info("Bitrate: %d", mpegHeader.bitRate);
            log::info("Samplerate: %d", mpegHeader.sampleRate);
            log::info("Samples per frame: %d", mpegHeader.samplesPerFrame);
            log::info("Duration: %f", static_cast<float>((filesize - id3Size) / (mpegHeader.bitRate * 125)));

            log::info("No xing header found");
            return 0;
        }

        log::info("Bitrate: %d %s", mpegHeader.bitRate, xingHeader.vbr ? "VBR" : "CBR");
        log::info("Samplerate: %d", mpegHeader.sampleRate);
        log::info("Samples per frame: %d", mpegHeader.samplesPerFrame);

        if (xingHeader.numFrames > 0)
        {
            log::info("Duration: %d", (xingHeader.numFrames * mpegHeader.samplesPerFrame) / mpegHeader.sampleRate);
        }
        else
        {
            log::info("Duration: %d", (filesize - id3Size) / (mpegHeader.bitRate * 125));
        }
        
        MpegUtils::LameHeader lameHeader;
        if (MpegUtils::readLameHeader(reader, lameHeader) == 0)
        {
            log::info("No lame header found");
            return 0;
        }

        log::info("Lame header");
        log::info("-----------");
        log::info("Encoderdelay: %d", lameHeader.encoderDelay);
        log::info("Encoderdelay (decoderdelay added): %d", lameHeader.encoderDelay + 528 + mpegHeader.samplesPerFrame);
        log::info("Zeropadding: %d", lameHeader.zeroPadding);
    }
    catch (std::exception& e)
    {
        log::error(e.what());
    }
}
