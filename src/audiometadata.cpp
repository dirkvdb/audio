//    Copyright (C) 2009 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modifys
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

#include "audio/audiometadata.h"

#include "audioconfig.h"
#include "audiotaglibiostream.h"

#include "utils/log.h"
#include "utils/stringoperations.h"
#include "utils/fileoperations.h"

#include <cmath>
#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/asffile.h>
#include <taglib/mpegfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/flacfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/mpcfile.h>
#include <taglib/mp4file.h>
#include <taglib/wavpackfile.h>
#include <taglib/speexfile.h>
#include <taglib/trueaudiofile.h>
#include <taglib/aifffile.h>
#include <taglib/wavfile.h>
#include <taglib/apefile.h>
#include <taglib/modfile.h>
#include <taglib/flacfile.h>
#include <taglib/s3mfile.h>
#include <taglib/itfile.h>
#include <taglib/xmfile.h>
#include <taglib/tiostream.h>
#include <taglib/id3v2framefactory.h>
#include <taglib/attachedpictureframe.h>

using namespace utils;
using namespace TagLib;

namespace audio
{

static std::shared_ptr<TagLib::File> createFile(TagLib::IOStream& iostream, bool readAudioProperties, AudioProperties::ReadStyle audioPropertiesStyle);

Metadata::Metadata(const std::string& uri)
: m_IoStream(new TaglibIOStream(uri))
, m_TagFile(createFile(*m_IoStream, false, AudioProperties::Fast))
{
    throwIfNotValid();
}

std::string Metadata::getArtist()
{
    return stringops::trim(m_TagFile->tag()->artist().to8Bit(true));
}

std::string Metadata::getTitle()
{
    return stringops::trim(m_TagFile->tag()->title().to8Bit(true));
}

std::string Metadata::getAlbum()
{
    return stringops::trim(m_TagFile->tag()->album().to8Bit(true));
}

std::string Metadata::getAlbumArtist()
{
    if (TagLib::MPEG::File* pMpegFile = dynamic_cast<TagLib::MPEG::File*>(m_TagFile.get()))
    {
        if (pMpegFile->ID3v2Tag())
        {
            const TagLib::ID3v2::FrameList& tpe2List = pMpegFile->ID3v2Tag()->frameList("TPE2");
            if (!tpe2List.isEmpty())
            {
                return stringops::trim(tpe2List.front()->toString().to8Bit(true));
            }
        }
    }
    
    return "";
}

std::string Metadata::getGenre()
{
    return stringops::trim(m_TagFile->tag()->genre().to8Bit(true));
}

std::string Metadata::getComposer()
{
    if (TagLib::MPEG::File* pMpegFile = dynamic_cast<TagLib::MPEG::File*>(m_TagFile.get()))
    {
        if (pMpegFile->ID3v2Tag())
        {
            const TagLib::ID3v2::FrameList& tcomList = pMpegFile->ID3v2Tag()->frameList("TCOM");
            if (!tcomList.isEmpty())
            {
                return stringops::trim(tcomList.front()->toString().to8Bit(true));
            }
        }
    }
    else if (TagLib::Ogg::Vorbis::File* pVorbisFile = dynamic_cast<TagLib::Ogg::Vorbis::File*>(m_TagFile.get()))
    {
        if (pVorbisFile->tag())
        {
            const TagLib::Ogg::FieldListMap& listMap = pVorbisFile->tag()->fieldListMap();
            if (!listMap["COMPOSER"].isEmpty())
            {
                return stringops::trim(listMap["COMPOSER"].front().to8Bit(true));
            }
        }
    }
    else if (TagLib::FLAC::File* pFlacFile = dynamic_cast<TagLib::FLAC::File*>(m_TagFile.get()))
    {
        if (pFlacFile->xiphComment())
        {
            const TagLib::Ogg::FieldListMap& listMap = pFlacFile->xiphComment()->fieldListMap();
            if (!listMap["COMPOSER"].isEmpty())
            {
                return stringops::trim(listMap["COMPOSER"].front().to8Bit(true));
            }
        }
    }
    
    return "";
}

uint32_t Metadata::getDiscNr()
{
    if (TagLib::MPEG::File* pMpegFile = dynamic_cast<TagLib::MPEG::File*>(m_TagFile.get()))
    {
        if (pMpegFile->ID3v2Tag())
        {
            const TagLib::ID3v2::FrameList& tposList = pMpegFile->ID3v2Tag()->frameList("TPOS");
            if (!tposList.isEmpty())
            {
                return parseDisc(stringops::trim(tposList.front()->toString().to8Bit(true)));
            }
        }
    }
    else if (TagLib::Ogg::Vorbis::File* pVorbisFile = dynamic_cast<TagLib::Ogg::Vorbis::File*>(m_TagFile.get()))
    {
        if (pVorbisFile->tag())
        {
            const TagLib::Ogg::FieldListMap& listMap = pVorbisFile->tag()->fieldListMap();
            if (!listMap["DISCNUMBER"].isEmpty())
            {
                return parseDisc(stringops::trim(listMap["DISCNUMBER"].front().to8Bit(true)));
            }
        }
    }
    else if (TagLib::FLAC::File* pFlacFile = dynamic_cast<TagLib::FLAC::File*>(m_TagFile.get()))
    {
        if (pFlacFile->xiphComment())
        {
            const TagLib::Ogg::FieldListMap& listMap = pFlacFile->xiphComment()->fieldListMap();
            if (!listMap["DISCNUMBER"].isEmpty())
            {
                return parseDisc(stringops::trim(listMap["DISCNUMBER"].front().to8Bit(true)));
            }
        }
    }

    return 0;
}

uint32_t Metadata::getYear()
{
    return m_TagFile->tag()->year();
}

uint32_t Metadata::getTrackNr()
{
    return m_TagFile->tag()->track();
}

uint32_t Metadata::getBitRate()
{
    return m_TagFile->audioProperties() ? m_TagFile->audioProperties()->bitrate() : 0;
}

uint32_t Metadata::getSampleRate()
{
    return m_TagFile->audioProperties() ? m_TagFile->audioProperties()->sampleRate() : 0;
}

uint32_t Metadata::getChannels()
{
    return m_TagFile->audioProperties() ? m_TagFile->audioProperties()->channels() : 0;
}

uint32_t Metadata::getDuration()
{
    return m_TagFile->audioProperties() ? m_TagFile->audioProperties()->length() : 0;
}

static Metadata::ImageFormat imageFormatFromMimetype(const std::string& mimetype)
{
    if (mimetype == "image/png")    return Metadata::ImageFormat::Png;
    if (mimetype == "image/jpeg")   return Metadata::ImageFormat::Jpeg;
    if (mimetype == "image/bmp")    return Metadata::ImageFormat::Bitmap;
    if (mimetype == "image/gif")    return Metadata::ImageFormat::Gif;
    
    return Metadata::ImageFormat::Unknown;
}

static Metadata::ImageFormat imageFormatFromMp4Format(MP4::CoverArt::Format format)
{
    switch (format)
    {
    case MP4::CoverArt::PNG:    return Metadata::ImageFormat::Png;
    case MP4::CoverArt::JPEG:   return Metadata::ImageFormat::Jpeg;
    case MP4::CoverArt::BMP:    return Metadata::ImageFormat::Bitmap;
    case MP4::CoverArt::GIF:    return Metadata::ImageFormat::Gif;
    default:                    return Metadata::ImageFormat::Unknown;
    }
}

Metadata::AlbumArt Metadata::getAlbumArt()
{
    AlbumArt art;

    if (MPEG::File* pMpegFile = dynamic_cast<MPEG::File*>(m_TagFile.get()))
    {
        if (pMpegFile->ID3v2Tag())
        {
            TagLib::ID3v2::AttachedPictureFrame* pAlbumArt = nullptr;

            TagLib::ID3v2::FrameList list = pMpegFile->ID3v2Tag()->frameList("APIC");
            for (TagLib::ID3v2::FrameList::Iterator iter = list.begin(); iter != list.end(); ++iter)
            {
                TagLib::ID3v2::AttachedPictureFrame* pCurPicture = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(*iter);

                if (pCurPicture->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover)
                {
                    pAlbumArt = pCurPicture;
                    art.format = imageFormatFromMimetype(pCurPicture->mimeType().toCString());
                    break;
                }
                else if (pAlbumArt == nullptr)
                {
                    pAlbumArt = pCurPicture;
                    art.format = imageFormatFromMimetype(pCurPicture->mimeType().toCString());
                }
            }

            if (pAlbumArt != nullptr)
            {
                art.data.resize(pAlbumArt->picture().size());
                memcpy(art.data.data(), pAlbumArt->picture().data(), art.data.size());
            }
        }
    }
    else if (FLAC::File* pFlacFile = dynamic_cast<FLAC::File*>(m_TagFile.get()))
    {
        auto picList = pFlacFile->pictureList();
        if (!picList.isEmpty())
        {
            TagLib::FLAC::Picture* pic = picList[0];
            auto picData = pic->data();
            art.data.resize(picData.size());
            memcpy(art.data.data(), picData.data(), art.data.size());
            art.format = imageFormatFromMimetype(pic->mimeType().toCString());
        }
    }
    else if (MP4::File* pMp4File = dynamic_cast<MP4::File*>(m_TagFile.get()))
    {
        auto& coverItem = pMp4File->tag()->itemListMap()["covr"];
        auto coverList = coverItem.toCoverArtList();
        
        if (!coverList.isEmpty())
        {
            auto data = coverList.front().data();
        
            art.data.resize(data.size());
            memcpy(art.data.data(), data.data(), art.data.size());
            art.format = imageFormatFromMp4Format(coverList.front().format());
        }
    }

    return art;
}

uint32_t Metadata::parseDisc(const std::string& disc)
{
    size_t pos = disc.find('/');
    return stringops::toNumeric<uint32_t>(pos == std::string::npos ? disc : disc.substr(0, pos));
}

void Metadata::throwIfNotValid() const
{
    if (!m_TagFile || !m_TagFile->isValid())
    {
        throw std::logic_error("Invalid url");
    }
}

static std::shared_ptr<TagLib::File> createFile(TagLib::IOStream& ioStream, bool readAudioProperties, AudioProperties::ReadStyle audioPropertiesStyle)
{
    std::string ext = stringops::uppercase(fileops::getFileExtension(ioStream.name()));

    if (ext == "MP3")
    {
        return std::make_shared<MPEG::File>(&ioStream, ID3v2::FrameFactory::instance(), readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "OGG")
    {
        return std::make_shared<Ogg::Vorbis::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "FLAC")
    {
        return std::make_shared<FLAC::File>(&ioStream, ID3v2::FrameFactory::instance(), readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "MPC")
    {
        return std::make_shared<MPC::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "WV")
    {
        return std::make_shared<WavPack::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "SPX")
    {
        return std::make_shared<Ogg::Speex::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "TTA")
    {
        return std::make_shared<TrueAudio::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "WMA" || ext == "ASF")
    {
        return std::make_shared<ASF::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "AIF" || ext == "AIFF")
    {
        return std::make_shared<RIFF::AIFF::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "WAV")
    {
        return std::make_shared<RIFF::WAV::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "APE")
    {
        return std::make_shared<APE::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "S3M")
    {
        return std::make_shared<S3M::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "IT")
    {
        return std::make_shared<IT::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "XM")
    {
        return std::make_shared<XM::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }

    if (ext == "MOD" || ext == "MODULE" || ext == "NST" || ext == "WOW")
    {
        return std::make_shared<Mod::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "M4A" || ext == "M4R" || ext == "M4B" || ext == "M4P" || ext == "MP4" || ext == "3G2")
    {
        return std::make_shared<MP4::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    if (ext == "OGA")
    {
        // oga can be any audio in the Ogg container. First try FLAC, then Vorbis.
        auto file = std::make_shared<Ogg::FLAC::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
        if (file->isValid())
        {
            return file;
        }
        
        return std::make_shared<Ogg::Vorbis::File>(&ioStream, readAudioProperties, audioPropertiesStyle);
    }
    
    throw  std::logic_error((std::string("Failed to determine file type from url: ") + ioStream.name()).c_str());
}

}
