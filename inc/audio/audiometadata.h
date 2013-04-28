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

#ifndef AUDIO_METADATA_H
#define AUDIO_METADATA_H

#include <string>
#include <vector>
#include <memory>

namespace TagLib
{

class File;

}

namespace audio
{

class TaglibIOStream;

class Metadata
{
public:
    Metadata(const std::string& filepath);

    std::string getArtist();
    std::string getTitle();
    std::string getAlbum();
    std::string getAlbumArtist();
    std::string getGenre();
    std::string getComposer();
    uint32_t getDiscNr();
    uint32_t getYear();
    uint32_t getTrackNr();
    uint32_t getBitRate();
    uint32_t getSampleRate();
    uint32_t getChannels();
    uint32_t getDuration();
    bool getAlbumArt(std::vector<uint8_t>& data, const std::vector<std::string>& albumArtFileList);

private:
    void throwIfNotValid() const;
    static uint32_t parseDisc(const std::string& disc);

    std::shared_ptr<TaglibIOStream> m_IoStream;
    std::shared_ptr<TagLib::File>   m_TagFile;
};

}

#endif
