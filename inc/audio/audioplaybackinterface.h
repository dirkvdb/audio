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

#ifndef AUDIO_PLAYBACK_INTERFACE_H
#define AUDIO_PLAYBACK_INTERFACE_H

#include "utils/signal.h"
#include <set>

namespace audio
{

class ITrack;

enum class PlaybackState
{
    Stopped,
    Playing,
    Paused
};

enum class PlaybackAction
{
    Play,
    Pause,
    Stop,
    Prev,
    Next
};

class IPlayback
{
public:
    virtual ~IPlayback() {}

    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void prev() = 0;
    virtual void next() = 0;

    virtual PlaybackState getState() const = 0;
    virtual bool isPlaying() const = 0;

    virtual void seek(double seconds) = 0;
    virtual double getCurrentTime() const = 0;
    virtual double getDuration() const = 0;

    virtual void setVolume(int32_t volume) = 0;
    virtual int32_t getVolume() const = 0;
    virtual void setMute(bool mute) = 0;
    virtual bool getMute() const = 0;

    virtual std::shared_ptr<ITrack> getTrack() const = 0;
    virtual std::set<PlaybackAction> getAvailableActions() const = 0;
    
    utils::Signal<PlaybackState>              PlaybackStateChanged;
    utils::Signal<std::set<PlaybackAction>>   AvailableActionsChanged;
    utils::Signal<double>                     ProgressChanged;
    utils::Signal<int32_t>                    VolumeChanged;
    utils::Signal<std::shared_ptr<ITrack>>    NewTrackStarted;
};

}

#endif
