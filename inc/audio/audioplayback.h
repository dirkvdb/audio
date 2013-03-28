//    Copyright (C) 2013 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#ifndef AUDIO_PLAYBACK_H
#define AUDIO_PLAYBACK_H

#include <set>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>

#include "audio/audioframe.h"
#include "utils/signal.h"


//#define DUMP_TO_WAVE
//#include <fstream>

namespace audio
{

class IDecoder;
class IRenderer;
class IPlaylist;


class Playback
{
public:
    enum class State
    {
        Stopped,
        Playing,
        Paused
    };

    enum class Action
    {
        Play,
        Pause,
        Stop,
        Prev,
        Next
    };

    Playback(IPlaylist& playlist);
    virtual ~Playback();

    void play();
    void pause();
    void stop();
    void prev();
    void next();
    bool isPaused();
    bool isPlaying();

    void seek(double seconds);
    double getCurrentTime();
    double getDuration();
    State getState() const;

    void setVolume(int32_t volume);
    int32_t getVolume() const;
    void setMute(bool enabled);
    bool getMute() const;
    
    std::set<Action> getAvailableActions() const;
    
    utils::Signal<void(State)>              PlaybackStateChanged;
    utils::Signal<void(std::set<Action>)>   AvailableActionsChanged;
    utils::Signal<void(double)>             ProgressChanged;

private:
    void stopPlayback(bool drain);
    bool startNewTrack();
    void sendProgressIfNeeded();
    void playback();
    void playbackLoop();
    bool rendererHasSpace(uint32_t dataSize);
    void setPlaybackState(State state);

    std::unique_ptr<audio::IDecoder>        m_pAudioDecoder;
    std::unique_ptr<audio::IRenderer>       m_pAudioRenderer;

    std::thread                             m_PlaybackThread;
    std::condition_variable                 m_PlaybackCondition;
    mutable std::mutex                      m_PlaybackMutex;
    mutable std::recursive_mutex            m_DecodeMutex;
    
    
    IPlaylist&                              m_Playlist;
    bool                                    m_Destroy;
    bool                                    m_Stop;
    bool                                    m_NewTrackStarted;
    State                                   m_State;
    bool                                    m_SkipTrack;
    bool                                    m_SeekOccured;
    double                                  m_CurrentPts;
    audio::Frame                            m_AudioFrame;
    std::set<Action>                        m_AvailableActions;
    std::string                             m_CurrentTrack;

#ifdef DUMP_TO_WAVE
    std::ofstream                           m_WaveFile;
    uint32_t                                m_WaveBytes;

    void writeWaveHeader();
    void updateWaveHeaderSize();
    void dumpToWav(AudioFrame& frame);
#endif
};

inline std::ostream& operator<< (std::ostream& os, const Playback::Action& action)
{
    switch (action)
    {
        case Playback::Action::Play:        return os << "Play";
        case Playback::Action::Pause:       return os << "Pause";
        case Playback::Action::Stop:        return os << "Stop";
        case Playback::Action::Prev:        return os << "Previous";
        case Playback::Action::Next:        return os << "Next";
    }
    
    return os;
}

}

#endif
