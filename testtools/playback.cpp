#include <iostream>
#include <fstream>
#include <deque>
#include <mutex>
#include <condition_variable>

#include "utils/log.h"
#include "audio/audioplaybackfactory.h"
#include "audio/audioplaybackinterface.h"
#include "audio/audioplaylistinterface.h"
#include "audio/audiotrackinterface.h"

using namespace std;
using namespace utils;
using namespace audio;

class Track : public ITrack
{
public:
    Track(const std::string& uri)
    : m_uri(uri)
    {
    }

    std::string getUri() const override
    {
        return m_uri;
    }

private:
    std::string m_uri;
};

class Playlist : public IPlaylist
{
public:
    void addTrack(const std::string& track)
    {
        m_Tracks.push_back(track);
    }

    std::shared_ptr<ITrack> dequeueNextTrack() override
    {
        if (m_Tracks.empty())
        {
            return nullptr;
        }
        
        auto track = m_Tracks.front();
        m_Tracks.pop_front();
        return std::make_shared<Track>(track);
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
            log::error("Usage: {} filename", argv[0]);
            return -1;
        }
        
        Playlist playlist;
        std::unique_ptr<IPlayback> playback(PlaybackFactory::create("Custom", "Playback", "OpenAL", "Default", playlist));
        playlist.addTrack(argv[1]);

        playback->play();
        
        char key;
        log::info("Press any key to stop");
        std::cin >> key;
        
        playback->stop();
    }
    catch (std::exception& e)
    {
        log::error(e.what());
    }
}
