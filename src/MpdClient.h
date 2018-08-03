#pragma once

#include <string>
#include "mpd/connection.h"

typedef struct mpd_connection mpd_connection_t;

struct TrackInfo
{
    std::string Artist;
    uint64_t PlayTimeSeconds;
    
    std::string TrackName;
    int TrackNumber;
    int TotalTracks;
};

class MpdClient
{

public:
    
    MpdClient(const std::string& hostname, unsigned port) :
            hostname_(hostname),
            port_(port),
            conn_(0)
    {}
    
    enum State
    {
        Playing,
        Paused,
        Idle
    };
    
    void connect(const std::string& password = std::string());
    
    State getState();
    void waitForStateChange(const std::string& password = std::string());
    TrackInfo getCurrentTrack();

private:

    std::string hostname_;
    unsigned port_;
    mpd_connection_t* conn_;
};


