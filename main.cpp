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
 *  TODO : pause support
 *  TODO : rate limiting
 *  TODO : killDiscord doesn't actually kill presence.
 */

typedef struct mpd_connection mpd_connection_t;
typedef struct mpd_song mpd_song_t;
typedef enum mpd_state mpd_state_t;
typedef struct mpd_status mpd_status_t;

void updateState(uint64_t elapsedTime, const std::string& detail, const std::string& state)
{
    
    printf("updating stats\n");
    
    DiscordRichPresence discordPresence = {};
    
    discordPresence.state = state.c_str();
    discordPresence.details = detail.c_str();
    
    discordPresence.startTimestamp = time(0) - elapsedTime;
    
    discordPresence.largeImageKey = "skeltal-big";
    discordPresence.smallImageKey= "mpd-small2";
    
    discordPresence.partySize = 2;
    discordPresence.partyMax = 3;

    discordPresence.instance = 1;
    
    Discord_UpdatePresence(&discordPresence);
}


constexpr static int MAX_CHARS = 128;
#define RET_CLAMP_STRSTREAM(stream) \
     auto str = stream.str(); \
    if(str.size() > MAX_CHARS) \
        return str.substr(0, MAX_CHARS); \
    return str;

std::string createDetail(mpd_song_t* song)
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


static bool discordIsAlive = false;
void startDiscord()
{
    
    if(discordIsAlive ) return;
    discordIsAlive = true;
    DiscordEventHandlers handlers {};
    Discord_Initialize("381948295830044683", &handlers, 1, 0);
}

void killDiscord()
{
    if(!discordIsAlive)return;
    discordIsAlive = false;
    Discord_Shutdown();
}

std::string createStatus(mpd_song_t* song)
{
    std::stringstream stream;
    
    const char* artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
    const char* album = mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
    if(artist)
        stream << "by " << artist;
    if(album)
    {
        if(artist) stream << " ,";
        stream << album;
    }
    
    RET_CLAMP_STRSTREAM(stream)
}

static bool queryMpdAndUpdateDiscord(mpd_connection_t* conn)
{
    if(!mpd_send_status(conn))
    {
        std::cout << "mpd status failed\n";
        return false;
    }
    
    mpd_status_t* status = mpd_recv_status(conn);
    if(!status)
    {
        std::cout << "mpd status was null.\n";
        return false;
    }
    
    mpd_state_t state = mpd_status_get_state(status);
    if(state == MPD_STATE_STOP || state == MPD_STATE_UNKNOWN)
    {
        std::cout << "Nothing is playing\n";
        return false;
    }
    
    
    uint64_t time = mpd_status_get_elapsed_time(status);
    mpd_status_free(status);
    
    if(!mpd_send_current_song(conn))
    {
        std::cout << "failed sending current song\n";
        return false;
    }
    
    mpd_song_t* song = mpd_recv_song(conn);
    if(!song)
    {
        std::cout << "failed to receive song.\n";
        return false;
    }
    
    startDiscord();
    updateState(time, createDetail(song), createStatus(song));
    
    mpd_song_free(song);
    
    std::cout << "Updated presence";
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
    
    if(!conn)
    {
        std::cout << "failed connecting to mpd.";
        return nullptr;
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
            conn = makeConnection();
        }
        queryMpdAndUpdateDiscordWrapped(conn);
    }
    

    mpd_song_t* song = mpd_recv_song(conn);
    
    printf(mpd_song_get_tag(song, MPD_TAG_TITLE, 0));
    
    
    return 0;
    
    
    while(1)
    {
    
    }
    
    Discord_Shutdown();
    
    
    //mpd_status_t* status = mpd_recv_status(conn);
    
    /*
    if (mpd_status_get_state(status) == MPD_STATE_PLAY ||
        mpd_status_get_state(status) == MPD_STATE_PAUSE) {
        if (!mpd_response_next(conn))
            printErrorAndExit(conn);
        
        struct mpd_song *song = mpd_recv_song(conn);
        if (song != NULL) {
            pretty_print_song(song);
            printf("\n");
            
            mpd_song_free(song);
        }
        
        my_finishCommand(conn);
    }
    
    mpd_status_free(status);
     */
    return 0;
}