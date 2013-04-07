#include <iostream>
#include <fstream>
#include <deque>
#include <mutex>
#include <condition_variable>

#include "utils/log.h"
#include "audio/audioplaybackfactory.h"
#include "audio/audioplaybackinterface.h"
#include "audio/audioplaylistinterface.h"

using namespace std;
using namespace utils;
using namespace audio;

class Playlist : public IPlaylist
{
public:
    void addTrack(const std::string& track)
    {
        m_Tracks.push_back(track);
    }

    virtual bool dequeueNextTrack(std::string& track)
    {
        if (m_Tracks.empty())
        {
            return false;
        }
        
        track = m_Tracks.front();
        m_Tracks.pop_front();
        return true;
    }
    
    virtual size_t getNumberOfTracks() const
    {
        return m_Tracks.size();
    }
    
private:
    std::deque<std::string>    m_Tracks;
};

static std::mutex g_Mutex;
static std::condition_variable g_Condition;

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            log::error("Usage: %s filename", argv[0]);
            return -1;
        }
        
        Playlist playlist;
        std::unique_ptr<IPlayback> playback(PlaybackFactory::create("FFmpeg", "OpenAL", "Default", playlist));
        playback->PlaybackStateChanged.connect([&] (PlaybackState state) {
            if (state == PlaybackState::Stopped)
            {
                std::lock_guard<std::mutex> lock(g_Mutex);
                g_Condition.notify_all();
            }
        }, playback.get());
        
        playlist.addTrack(argv[1]);
        
        usleep(10000);
        playback->play();
        
        std::unique_lock<std::mutex> lock(g_Mutex);
        g_Condition.wait(lock);
    }
    catch (std::exception& e)
    {
        log::error(e.what());
    }
}
