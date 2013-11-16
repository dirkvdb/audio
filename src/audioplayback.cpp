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

#include "audioplayback.h"

#include <iomanip>
#include <cmath>
#include <cassert>
#include <cstring>

#include "audio/audiodecoder.h"
#include "audio/audiodecoderfactory.h"
#include "audio/audiorenderer.h"
#include "audio/audioformat.h"
#include "audio/audioframe.h"
#include "audio/audiorendererfactory.h"
#include "audio/audiotrackinterface.h"
#include "audio/audioplaylistinterface.h"
#include "utils/timeoperations.h"
#include "utils/numericoperations.h"
#include "utils/log.h"

using namespace std;
using namespace std::placeholders;
using namespace utils;

namespace audio
{

Playback::Playback(IPlaylist& playlist, const std::string& appName, const std::string& audioOutput, const std::string& deviceName)
: m_Playlist(playlist)
, m_Destroy(false)
, m_Stop(false)
, m_NewTrackStarted(false)
, m_State(PlaybackState::Stopped)
, m_SkipTrack(false)
, m_SeekOccured(false)
, m_CurrentPts(0)
, m_PlaybackThread(&Playback::playbackLoop, this)
{
    try
    {
        m_pAudioRenderer.reset(audio::RendererFactory::create(appName, audioOutput, deviceName));
        m_pAudioRenderer->VolumeChanged.connect([this] (int32_t volume) { VolumeChanged(volume); }, this);
    }
    catch (std::exception&)
    {
        log::error("Failed to create audio renderer, sound is disabled");
    }

#ifdef DUMP_TO_WAVE
    log::critical("Dumping to wave is enabled, we are debugging right?");
    writeWaveHeader();
#endif
}


Playback::~Playback()
{
    stop();

#ifdef DUMP_TO_WAVE
    updateWaveHeaderSize();
#endif

    if (m_pAudioRenderer)
    {
        m_pAudioRenderer->VolumeChanged.disconnect(this);
    }
    
    {
        std::lock_guard<std::mutex> lock(m_PlaybackMutex);
        m_Destroy = true;
        m_PlaybackCondition.notify_one();
    }
    
    if (m_PlaybackThread.joinable())
    {
        log::debug("Waiting for playback thread");
        m_PlaybackThread.join();
        log::debug("Done waiting");
    }
}

bool Playback::startNewTrack()
{
    auto track = m_Playlist.dequeueNextTrack();
    if (!track || m_Stop)
    {
        stopPlayback(true);
        return false;
    }
    
    m_CurrentPts = 0.0;

    {
        std::lock_guard<std::recursive_mutex> lock(m_DecodeMutex);
        try
        {
            log::info("Play track: %s", track->getUri());
            m_pAudioDecoder.reset(audio::DecoderFactory::create(track->getUri()));
            m_CurrentTrack = track;
        }
        catch (logic_error& e)
        {
            log::error("Failed to play audio file: %s", e.what());
            m_pAudioDecoder.reset();
            return startNewTrack();
        }
    }

    uint32_t duration = static_cast<uint32_t>(m_pAudioDecoder->getDuration());
    if (duration != 0)
    {
        m_Duration = duration;
    }

    NewTrackStarted(track);
    m_NewTrackStarted = true;

    auto format = m_pAudioDecoder->getAudioFormat();
    if (m_pAudioRenderer)
    {
        m_pAudioRenderer->setFormat(format);
    }
    else
    {
        return false;
    }

    return true;
}

void Playback::playback()
{
    if (!startNewTrack())
    {
        return;
    }
    
    assert(m_pAudioDecoder);
    
    bool frameDecoded = false;
    bool firstFrame = true;

    while (!m_Stop)
    {
        while (!m_Stop && !isPaused() && rendererHasSpace(m_AudioFrame.getDataSize()))
        {
            {
                // decode the first frame
                std::lock_guard<std::recursive_mutex> lock(m_DecodeMutex);
                frameDecoded = m_pAudioDecoder->decodeAudioFrame(m_AudioFrame);
            }
        
            if (!frameDecoded)
            {
                // we could not decode a frame, end of file probably
                break;
            }
            
            if (m_SkipTrack)
            {
                if (!startNewTrack())
                {
                    return;
                }

                firstFrame = true;
                m_SkipTrack = false;
            }

            if (isPaused())
            {
                break;
            }

            std::lock_guard<std::mutex> lock(m_PlaybackMutex);
            m_pAudioRenderer->queueFrame(m_AudioFrame);
            
            #ifdef DUMP_TO_WAVE
            dumpToWav(m_AudioFrame);
            #endif
        }
        
        if (firstFrame)
        {
            // Start the renderer after adding the first frame
            firstFrame = false;
            m_pAudioRenderer->play();
        }
        
        sendProgressIfNeeded();
        
        if (!frameDecoded && !startNewTrack())
        {
            log::debug("Stop it %d", frameDecoded);
            return;
        }

        m_pAudioRenderer->flushBuffers();

        if (m_State == PlaybackState::Playing && !m_pAudioRenderer->isPlaying())
        {
            log::debug("Kick renderer");
            
            std::lock_guard<std::mutex> lock(m_PlaybackMutex);
            m_pAudioRenderer->play();
        }
        else
        {
            // don't busy wait
            double sleepTime = m_pAudioRenderer->getBufferDuration() * 1000 / 3;
            timeops::sleepMs(sleepTime);
        }
    }
}

bool Playback::rendererHasSpace(size_t dataSize)
{
    if (m_pAudioRenderer)
    {
        std::lock_guard<std::mutex> lock(m_PlaybackMutex);
        return m_pAudioRenderer->hasBufferSpace(static_cast<uint32_t>(dataSize));
    }

    return false;
}

void Playback::sendProgressIfNeeded()
{
    double pts = m_pAudioRenderer->getCurrentPts();

    if (pts <= 2.0 && m_NewTrackStarted)
    {
        m_NewTrackStarted = false;
    }

    if (static_cast<int>(pts) == 0 || m_NewTrackStarted)
    {
        //avoid sending false progress after a seek, or track that has just been started
        return;
    }

    if ((static_cast<int>(pts) != static_cast<int>(m_CurrentPts)))
    {
        m_CurrentPts = pts;
        ProgressChanged(m_CurrentPts);
    }
}

void Playback::play()
{
    std::lock_guard<std::mutex> lock(m_PlaybackMutex);

    if (!m_pAudioRenderer || m_State == PlaybackState::Playing)
    {
        return;
    }

    if (m_State == PlaybackState::Stopped)
    {
        m_PlaybackCondition.notify_all();
    }
    else if (m_State == PlaybackState::Paused)
    {
        m_SeekOccured ? m_pAudioRenderer->play() : m_pAudioRenderer->resume();
    }
    
    setPlaybackState(PlaybackState::Playing);
}

void Playback::pause()
{
    if (m_pAudioRenderer)
    {
        std::lock_guard<std::mutex> lock(m_PlaybackMutex);
        m_pAudioRenderer->pause();
        setPlaybackState(PlaybackState::Paused);
    }
}

void Playback::stop()
{
    stopPlayback(false);
}

void Playback::stopPlayback(bool drain)
{
    if (m_pAudioRenderer)
    {
        m_CurrentPts = 0.0;
        m_Stop = true;

        std::lock_guard<std::mutex> lock(m_PlaybackMutex);
        m_pAudioRenderer->stop(drain);
        setPlaybackState(PlaybackState::Stopped);
        m_SeekOccured = false;
        m_NewTrackStarted = false;
    }
}

void Playback::next()
{
    if (!m_pAudioRenderer || m_SkipTrack || m_State == PlaybackState::Stopped)
    {
        //previous skip not processed yet or we are stopped
        return;
    }

    std::lock_guard<std::mutex> lock(m_PlaybackMutex);

    m_SkipTrack = true;
    m_pAudioRenderer->stop(false);
    m_pAudioRenderer->flushBuffers();
    setPlaybackState(PlaybackState::Playing);

    m_CurrentPts = 0.0;
}

void Playback::prev()
{
}

bool Playback::isPaused() const
{
    return m_State == PlaybackState::Paused;
}

bool Playback::isPlaying() const
{
    return m_pAudioRenderer ? m_pAudioRenderer->isPlaying() : false;
}

void Playback::seek(double seconds)
{
    if (!m_pAudioDecoder || !m_pAudioRenderer)
    {
        return;
    }

    std::lock_guard<std::mutex> playbackLock(m_PlaybackMutex);
    m_pAudioRenderer->stop(false);
    m_pAudioRenderer->flushBuffers();

    {
        std::lock_guard<std::recursive_mutex> decodeLock(m_DecodeMutex);
        m_pAudioDecoder->seekAbsolute(static_cast<double>(seconds));
    }

    if (!isPaused())
    {
        m_pAudioRenderer->play();
    }
    else
    {
        m_SeekOccured = true;
    }

    m_NewTrackStarted = false;
}

double Playback::getCurrentTime() const
{
    return m_CurrentPts;
}

double Playback::getDuration() const
{
    return m_pAudioDecoder ? static_cast<double>(m_pAudioDecoder->getDuration()) : 0.0;
}

PlaybackState Playback::getState() const
{
    return m_State;
}

void Playback::setVolume(int32_t volume)
{
    if (m_pAudioRenderer)
    {
        std::lock_guard<std::mutex> lock(m_PlaybackMutex);
        m_pAudioRenderer->setVolume(volume);
        VolumeChanged(volume);
    }
}

int32_t Playback::getVolume() const
{
    std::lock_guard<std::mutex> lock(m_PlaybackMutex);
    return m_pAudioRenderer ? m_pAudioRenderer->getVolume() : 100;
}

void Playback::setMute(bool enabled)
{
    if (m_pAudioRenderer)
    {
        std::lock_guard<std::mutex> lock(m_PlaybackMutex);
        m_pAudioRenderer->setMute(enabled);
    }
}

bool Playback::getMute() const
{
    std::lock_guard<std::mutex> lock(m_PlaybackMutex);
    return m_pAudioRenderer ? m_pAudioRenderer->getMute() : false;
}

std::shared_ptr<ITrack> Playback::getTrack() const
{
    return m_CurrentTrack;
}

std::set<PlaybackAction> Playback::getAvailableActions() const
{
    std::lock_guard<std::mutex> lock(m_PlaybackMutex);
    return m_AvailableActions;
}

void Playback::playbackLoop()
{
    while (!m_Destroy)
    {
        {
            log::debug("Wait %d", m_Destroy);
            std::unique_lock<std::mutex> lock(m_PlaybackMutex);
            m_PlaybackCondition.wait(lock);
            log::debug("Condition signaled playback (stop = %d)", m_Stop);
        }

        if (m_Destroy)
        {
            break;
        }

        try
        {
            m_Stop = m_SkipTrack = false;
            playback();
        }
        catch (exception& e)
        {
            log::error("Playback error: %s", e.what());
        }
    }
}

void Playback::setPlaybackState(PlaybackState state)
{
    if (m_State != state)
    {
        m_State = state;
        PlaybackStateChanged(state);
        
        std::set<PlaybackAction> availableActions;
        
        switch (state)
        {
        case PlaybackState::Playing:
        {
            availableActions.insert(PlaybackAction::Stop);
            availableActions.insert(PlaybackAction::Pause);
            if (m_Playlist.getNumberOfTracks() > 0)
            {
                availableActions.insert(PlaybackAction::Next);
            }
            
            break;
        }
        case PlaybackState::Paused:
            availableActions.insert(PlaybackAction::Stop);
            availableActions.insert(PlaybackAction::Play);
            break;
        case PlaybackState::Stopped:
            if (m_CurrentTrack)
            {
                availableActions.insert(PlaybackAction::Play);
            }
            break;
        default:
            assert(!"Invalid action");
            break;
        }
        
        m_AvailableActions = availableActions;
        AvailableActionsChanged(availableActions);
    }
}

#ifdef DUMP_TO_WAVE

void Playback::writeWaveHeader()
{
    m_WaveFile.open("/tmp/dump.wav", ios_base::binary);
    if (!m_WaveFile.is_open())
    {
        return;
    }

    uint16_t bps = 16;
    uint32_t subchunk1Size = 16; //16 for PCM
    uint16_t audioFormat = 1; //PCM
    uint16_t channels = 2;
    uint32_t sampleRate = 44100;
    uint32_t byteRate = 44100 * 2 * (bps / 8);
    uint16_t blockAlign = 2 * (bps / 8);

    //don't know yet
    m_WaveBytes = 0;

    m_WaveFile.write("RIFF", 4);
    m_WaveFile.write((char*) &m_WaveBytes, 4);
    m_WaveFile.write("WAVEfmt ", 8);
    m_WaveFile.write((char*) &subchunk1Size, 4);
    m_WaveFile.write((char*) &audioFormat, 2);
    m_WaveFile.write((char*) &channels, 2);
    m_WaveFile.write((char*) &sampleRate, 4);
    m_WaveFile.write((char*) &byteRate, 4);
    m_WaveFile.write((char*) &blockAlign, 2);
    m_WaveFile.write((char*) &bps, 2);
    m_WaveFile.write("data", 4);
    m_WaveFile.write((char*) &m_WaveBytes, 4);
}

void Playback::updateWaveHeaderSize()
{
    m_WaveFile.seekp(4, ios_base::beg);
    m_WaveFile.write((char*) &m_WaveBytes + 36, 4);

    m_WaveFile.seekp(40, ios_base::beg);
    m_WaveFile.write((char*) &m_WaveBytes, 4);

    m_WaveFile.close();
}

void Playback::dumpToWav(Frame& frame)
{
    assert(m_WaveFile.is_open());
    m_WaveFile.write(reinterpret_cast<char*>(frame.getFrameData()), frame.getDataSize());
    m_WaveBytes += frame.getDataSize();
}

#endif
}
