#include <sstream>
#include "MpdClient.h"

#include "mpd/list.h"
#include "mpd/player.h"
#include "mpd/song.h"
#include "mpd/status.h"
#include "mpd/idle.h"
#include "mpd/client.h"

typedef struct mpd_song mpd_song_t;
typedef enum mpd_state mpd_state_t;
typedef enum mpd_error mpd_error_t;
typedef struct mpd_status mpd_status_t;

void MpdClient::connect(const std::string& password)
{
    conn_ = mpd_connection_new(hostname_.c_str(), port_, 0);
    mpd_error_t error = mpd_connection_get_error(conn_);
    
    if(error != MPD_ERROR_SUCCESS)
        throw std::runtime_error(mpd_connection_get_error_message(conn_));
    
    if(!conn_)
        throw std::runtime_error(
                "mpd_connection_get_error yielded success but mpd_connection_new yielded a null pointer.");
    
    if(!password.empty())
    {
        bool success = mpd_run_password(conn_, password.c_str());
        if(!success)
            throw std::runtime_error("mpd_run_password failed. Is your password valid?");
    }
}

void MpdClient::waitForStateChange(const std::string& password)
{
    while(true)
    {
        // this blocks
        if(mpd_run_idle_mask(conn_, MPD_IDLE_PLAYER) == 0)
        {
            // mpd really likes to kill idle connections.
            // restore connection
            mpd_connection_free(conn_);
            connect(password);
            continue;
        }
        
        // we'll only reach this if mpd_run_idle_mask returns something other than 0,
        // meaning we'll get here when the player state changes
        return;
    }
}

template<typename Fx, typename ...Args>
void assertSuccess(Fx f, const char* whyFail, Args... args)
{
    if(!f(args...))
        throw std::runtime_error(whyFail);
};

template<typename ReturnPtr, typename Fx, typename ...Args>
ReturnPtr assertNotNull(Fx f, const char* whyFail, Args... args)
{
    ReturnPtr t = f(args...);
    if(!t)
        throw std::runtime_error(whyFail);
    return t;
};

MpdClient::State MpdClient::getState()
{
    assertSuccess(mpd_send_status, "mpd_send_status in getState failed.", conn_);
    auto* mpdStatus = assertNotNull<mpd_status_t*>(mpd_recv_status, "mpd_recv_status in getState returned nullptr", conn_);
    mpd_state_t state = mpd_status_get_state(mpdStatus);
    
    switch(state)
    {
        case MPD_STATE_UNKNOWN:
        case MPD_STATE_STOP:
            return State::Idle;
        case MPD_STATE_PLAY:
            return State::Playing;
        case MPD_STATE_PAUSE:
            return State::Paused;
    }
    return State::Idle;
}

constexpr static int MAX_CHARS = 128;
#define RET_CLAMP_STRSTREAM(stream) \
     auto str = (stream).str(); \
    if(str.size() > MAX_CHARS) \
        return str.substr(0, MAX_CHARS); \
    return str

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
    
    RET_CLAMP_STRSTREAM(stream);
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
    
    RET_CLAMP_STRSTREAM(stream);
}


TrackInfo MpdClient::getCurrentTrack()
{
    assertSuccess(mpd_command_list_begin, "mpd_command_list_begin in getCurrentTrack failed.", conn_, true);
    assertSuccess(mpd_send_status, "mpd_send_status in getCurrentTrack failed.", conn_);
    assertSuccess(mpd_send_current_song, "mpd_send_current_song in getCurrentTrack failed.", conn_);
    assertSuccess(mpd_command_list_end, "mpd_command_list_end in getCurrentTrack failed.", conn_);
    
    auto* mpdStatus = assertNotNull<mpd_status_t*>(mpd_recv_status, "mpd_recv_status in getCurrentTrackr returned nullptr", conn_);
    mpd_response_next(conn_);
    auto* mpdCurrentSong = assertNotNull<mpd_song_t*>(mpd_recv_song, "mpd_recv_status in getCurrentTrackr returned nullptr", conn_);
    
    TrackInfo t;
    
    t.TotalTracks = mpd_status_get_queue_length(mpdStatus);
    t.TrackNumber = mpd_status_get_song_pos(mpdStatus) + 1;
    t.PlayTimeSeconds = mpd_status_get_elapsed_time(mpdStatus);
    t.Artist = createArtist(mpdCurrentSong);
    t.TrackName = createTitle(mpdCurrentSong);
    
    mpd_song_free(mpdCurrentSong);
    mpd_status_free(mpdStatus);
    
    return t;
}

