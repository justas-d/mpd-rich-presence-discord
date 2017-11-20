#include <stdio.h>

#include <string>
#include <cstring>
#include "mpd/list.h"
#include "mpd/player.h"
#include "mpd/connection.h"
#include "mpd/song.h"
#include "mpd/status.h"
#include <discord-rpc.h>
#include <idle.h>
#include <sstream>
#include <iostream>
#include <client.h>

/*
 *  TODO : rate limiting
 *  TODO : mpd_status_get_queue_length is not what we need
 */

#define LOG(...) std::cout << __VA_ARGS__ << std::endl

typedef struct mpd_connection mpd_connection_t;
typedef struct mpd_song mpd_song_t;
typedef enum mpd_state mpd_state_t;
typedef enum mpd_error mpd_error_t;
typedef struct mpd_status mpd_status_t;

static bool discordIsAlive = false;
constexpr static int MAX_CHARS = 128;

struct mpd_share_state
{
    const char* application;
    const char* track;
    const char* artist;
    int trackNumber;
    int maxTrackNumbers;
    uint64_t currentPlayTime;
    
    bool isPaused;
};

void updateState(mpd_share_state* state)
{
    Discord_Shutdown();
    DiscordEventHandlers handlers {};
    Discord_Initialize(state->application, &handlers, 1, 0);
    
    DiscordRichPresence discordPresence = {};
    
    discordPresence.state = state->artist;
    discordPresence.details = state->track;
    
    if(!state->isPaused)
        discordPresence.startTimestamp = time(0) - state->currentPlayTime;
    
    discordPresence.largeImageKey = "mpd_large";
    
    discordPresence.partySize = state->trackNumber;
    discordPresence.partyMax = 54; //state->maxTrackNumbers;

    discordPresence.instance = 1;
    
    Discord_UpdatePresence(&discordPresence);
    LOG("updated presence state");
}

#define RET_CLAMP_STRSTREAM(stream) \
     auto str = (stream).str(); \
    if(str.size() > MAX_CHARS) \
        return str.substr(0, MAX_CHARS); \
    return str;

std::string createTitle(mpd_song_t* song)
{
    std::stringstream stream;
    
    const char* name = mpd_song_get_tag(song, MPD_TAG_NAME, 0);
    const char* uri = mpd_song_get_uri(song);
    const char* title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
    if(title)
        stream << title;
    else if(name)
        stream << name;
    else if(uri)
        stream << uri;
    else
        stream << "Unknown";
    
    RET_CLAMP_STRSTREAM(stream)
}

std::string createArtist(mpd_song_t* song)
{
    std::stringstream stream;
    
    const char* artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
    const char* album = mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
    if(artist)
        stream << "by " << artist;
    if(album)
    {
        if(artist) stream << " (";
        stream << "album: " << album;
        if(artist) stream << ")";
    }
    
    RET_CLAMP_STRSTREAM(stream)
}


void killDiscord()
{
    if(!discordIsAlive)
    {
        LOG("Attempted to killDiscord with no live connection.");
        return;
    }
    
    discordIsAlive = false;
    Discord_Shutdown();
    // BUG : killDiscord doesn't actually kill presence.
    
    LOG("Killed discord.");
}

static bool queryMpdAndUpdateDiscord(mpd_connection_t* conn)
{
    if(!mpd_send_status(conn))
    {
        LOG("mpd_send_status failed.");
        return false;
    }
    
    mpd_status_t* status = mpd_recv_status(conn);
    if(!status)
    {
        LOG("mpd_recv_status returned null");
        return false;
    }
    
    mpd_state_t state = mpd_status_get_state(status);
    
    if(state == MPD_STATE_STOP || state == MPD_STATE_UNKNOWN)
    {
        LOG("stopped playing");
        return false;
    }
    
    uint64_t time = mpd_status_get_elapsed_time(status);
    mpd_status_free(status);
    
    if(!mpd_send_current_song(conn))
    {
        LOG("mpd_send_current_song failed.");
        return false;
    }
    
    mpd_song_t* song = mpd_recv_song(conn);
    if(!song)
    {
        LOG("mpd_recv_song returned null.");
        return false;
    }
    
    
    std::string artist = createArtist(song);
    std::string title = createTitle(song);
    
    bool isPaused = state == MPD_STATE_PAUSE;
    mpd_share_state share = {};
    share.application = (isPaused ? "381994109981687810" : "381948295830044683");
    share.isPaused = isPaused;
    share.artist = artist.c_str();
    share.trackNumber = mpd_song_get_pos(song);
    share.maxTrackNumbers =  mpd_status_get_queue_length(status);
    share.track = title.c_str();
    share.currentPlayTime = time;
    
    updateState(&share);
    
    mpd_song_free(song);
    
    return true;
}

static void queryMpdAndUpdateDiscordWrapped(mpd_connection_t* conn)
{
    if(!queryMpdAndUpdateDiscord(conn))
        killDiscord();
}

mpd_connection_t* makeConnection()
{
    mpd_connection_t* conn = mpd_connection_new("127.0.0.1", 6600, 0);
    mpd_error_t err = mpd_connection_get_error(conn);
    
    if(err != MPD_ERROR_SUCCESS)
    {
        LOG(mpd_connection_get_error_message(conn));
        std::exit(1);
    }
    
    if(!conn)
    {
        LOG("failed connecting to mpd.");
        std::exit(1);
    }
    return conn;
}

int main()
{
    mpd_connection_t* conn = makeConnection();
    
    queryMpdAndUpdateDiscordWrapped(conn);
    while(true)
    {
        if(mpd_run_idle_mask(conn, MPD_IDLE_PLAYER) == 0)
        {
            mpd_connection_free(conn);
            conn = makeConnection();
            LOG("restored mpd connection.");
        }
        queryMpdAndUpdateDiscordWrapped(conn);
    }
}