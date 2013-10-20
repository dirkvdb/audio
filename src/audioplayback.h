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
#include "audio/audioplaybackinterface.h"
#include "utils/signal.h"


//#define DUMP_TO_WAVE
//#include <fstream>

namespace audio
{

class IDecoder;
class IRenderer;
class IPlaylist;
class ITrack;

class Playback : public IPlayback
{
public:
    Playback(IPlaylist& playlist, const std::string& audioOutput, const std::string& deviceName);
    virtual ~Playback();

    void play();
    void pause();
    void stop();
    void prev();
    void next();
    bool isPaused() const;
    bool isPlaying() const;

    void seek(double seconds);
    double getCurrentTime() const;
    double getDuration() const;
    PlaybackState getState() const;

    void setVolume(int32_t volume);
    int32_t getVolume() const;
    void setMute(bool enabled);
    bool getMute() const;
    
    std::shared_ptr<ITrack> getTrack() const;
    std::set<PlaybackAction> getAvailableActions() const;   

private:
    void stopPlayback(bool drain);
    bool startNewTrack();
    void sendProgressIfNeeded();
    void playback();
    void playbackLoop();
    bool rendererHasSpace(size_t dataSize);
    void setPlaybackState(PlaybackState state);

    std::unique_ptr<IDecoder>               m_pAudioDecoder;
    std::unique_ptr<IRenderer>              m_pAudioRenderer;

    std::thread                             m_PlaybackThread;
    std::condition_variable                 m_PlaybackCondition;
    mutable std::mutex                      m_PlaybackMutex;
    mutable std::recursive_mutex            m_DecodeMutex;
    
    
    IPlaylist&                              m_Playlist;
    bool                                    m_Destroy;
    bool                                    m_Stop;
    bool                                    m_NewTrackStarted;
    PlaybackState                           m_State;
    bool                                    m_SkipTrack;
    bool                                    m_SeekOccured;
    double                                  m_CurrentPts;
    double                                  m_Duration;
    Frame                                   m_AudioFrame;
    std::set<PlaybackAction>                m_AvailableActions;
    std::shared_ptr<ITrack>                 m_CurrentTrack;

#ifdef DUMP_TO_WAVE
    std::ofstream                           m_WaveFile;
    uint32_t                                m_WaveBytes;

    void writeWaveHeader();
    void updateWaveHeaderSize();
    void dumpToWav(Frame& frame);
#endif
};

}

#endif
