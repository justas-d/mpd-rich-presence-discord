#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include "MpdClient.h"
#include "unistd.h"
#include <cstdint>
#include <discord_rpc.h>

static const char* AppIdPlaying= "381948295830044683";
static const char* AppIdPaused = "382303152327753739";
static const char* AppIdIdle = "382302420073709568";
static const char* AppIdMulti = "474605546457137157";

struct AppState
{
    bool BroadcastIdle;
    bool UseMultipleApps;
    uint64_t PrevAppIdHash = 0;
};

struct PresencePayload
{
    DiscordRichPresence Presence;
    const char* AppId;
};

static inline
uint64_t FNVHash(const char* str, uint64_t hash = 14695981039346656037UL)
{
    for(const char *c = str; *c != '\0'; c++)
    {
        hash = hash ^ *c;
        hash = hash * 1099511628211UL;
    }
    return hash;
}

PresencePayload makePresence(const AppState& app, const TrackInfo* track, bool isPaused) 
{
    PresencePayload payload;
    memset(&payload, 0, sizeof(payload));

    DiscordRichPresence& rp = payload.Presence;

    rp.largeImageKey = "mpd_large";
    rp.largeImageText = "Music Player Daemon";

    if(track) 
    {
        rp.state = track->Artist.c_str();
        rp.details = track->TrackName.c_str();
        rp.partySize = track->TrackNumber;
        rp.partyMax = track->TotalTracks;

        if(isPaused) 
        {
            rp.smallImageKey = "pause-circle_png";
            rp.smallImageText = "Paused";

            if(app.UseMultipleApps)
                payload.AppId = AppIdPaused;
        } 
        else
        {
            rp.startTimestamp = time(0) - track->PlayTimeSeconds;
            rp.smallImageKey = "play-circle_png";
            rp.smallImageText = "Playing";

            if(app.UseMultipleApps)
                payload.AppId = AppIdPlaying;
        }
    } 
    else 
    {
        rp.state = "Idle";
        rp.smallImageKey = "refresh-cw_png";
        rp.smallImageText = "Zzz...";
   
        if(app.UseMultipleApps)
            payload.AppId = AppIdIdle;
    }

    if(!app.UseMultipleApps)
        payload.AppId = AppIdMulti;

    return payload;
}

void updatePresence(MpdClient& mpd, TrackInfo* trackBuffer, AppState& app)
{
    MpdClient::State state = mpd.getState();
    PresencePayload p;

    if(state == MpdClient::Idle)
    {
        if(app.BroadcastIdle)
        {
            p = makePresence(app, nullptr, true);
        }
        else 
        {
            app.PrevAppIdHash = 0;
            Discord_Shutdown();
            return;
        }
    }
    else if(state == MpdClient::Playing || state == MpdClient::Paused) 
    {
        *trackBuffer = mpd.getCurrentTrack();
        p = makePresence(app, trackBuffer, state == MpdClient::Paused);
    }

    auto appIdHash = FNVHash(p.AppId);
    if(app.PrevAppIdHash != appIdHash)
    {
        // reinitialize with new app id
        DiscordEventHandlers h = {};
        Discord_Shutdown();
        Discord_Initialize(p.AppId, &h, 1, 0);
    }

    app.PrevAppIdHash = appIdHash;
    Discord_UpdatePresence(&p.Presence);
}

bool isFlagSet(const std::vector<std::string>& args, const std::string& flag)
{
    for(const auto& arg : args)
    {
        auto start = arg.find(flag);
        if(start == std::string::npos)
            continue;

        return true;
    }
    return false;
}

std::string getParam(const std::vector<std::string>& args, const std::string& param)
{
    for(const auto& arg : args)
    {
        auto start = arg.find(param);
        if(start == std::string::npos)
            continue;
        
        return arg.substr(start + param.size() + 1);
    }
    
    return {};
}

bool shouldBroadcastIdle(const std::vector<std::string>& args)
{
    return !isFlagSet(args, "--no-idle");
}

bool shouldUseMultiAppModel(const std::vector<std::string>& args)
{
    return isFlagSet(args, "--idle");
}

std::string getHostname(const std::vector<std::string>& args)
{
    auto host = getParam(args, "-h");
    if(host.empty())
        return "127.0.0.1";
    return  host;
}

unsigned getPort(const std::vector<std::string>& args)
{
    auto rawPort = getParam(args, "-p");
    if(rawPort.empty())
        return 6600;
    return static_cast<unsigned int>(std::stoi(rawPort));
}

std::string getPassword(const std::vector<std::string>& args)
{
    return getParam(args, "-P");
}

int main(int argc, char** args)
{

    auto vecArgs = std::vector<std::string>(args+1, args+argc);


    if(isFlagSet(vecArgs, "--help") || isFlagSet(vecArgs, "-help") || isFlagSet(vecArgs, "help"))
    {
#define LINE(what) std::cout << what << std::endl
        LINE("MPD Rich Presence for Discord");
        LINE("https://github.com/SSStormy/mpd-rich-presence-discord/");
        LINE("");
        LINE("Arguments:");
        LINE("  -h=ADDDRESS             the address where the MPD server (defaults to 127.0.0.1)");
        LINE("  -p=PORT                 the port on which the target MPD server is listening (defaults to 6600)");
        LINE("  -P=PASSWORD             the password to be sent after connection to the MPD server has been established in hopes of acquiring higher permissions. (default is empty, therfore no password sent.)");
        LINE("  --fork                  forks the process into the background.");
        LINE("  --no-idle               Disables broadcasting of the idle state.");
        LINE("  --use-multiple-apps     Uses the Multi App mode.");
#undef LINE
        return 0;
    }

    auto host = getHostname(vecArgs);
    auto pass = getPassword(vecArgs);
    auto port = getPort(vecArgs);

    AppState state = {};
    state.BroadcastIdle     = !isFlagSet(vecArgs, "--no-idle");
    state.UseMultipleApps   = isFlagSet(vecArgs, "--use-multiple-apps");
    
    // forking
    auto isForked = false;
    {
        int pid;

        if(isFlagSet(vecArgs, "--fork"))
        {
            pid = fork();
            if (pid != 0)
            {
                if (pid < 0)
                {
                    std::cerr << "Failed to fork." << std::endl;
                    return -1;
                }
                else
                {
                    std::cout << "Forked. PID: " << pid << std::endl;
                    return 0;
                }
            }
            isForked = true;
        }
    }
        
    int errCount = 0;
    const static int MaxExceptionsWhenForked = 10;
    
    while(true)
    {
        try
        {
            MpdClient mpd(host, port);
            mpd.connect(pass);

            // we store the track way back here since it's easier then allocating
            // the track info on the heap when we want to get it in
            // updatePresenece.
            //
            // Storing the track in the updatePresence stack will cause
            // grabled strings to get send to discord due to that memory
            // being freed when the discord rpc backend sends it over the network.
            TrackInfo trackBuffer;
            while(true)
            {
                updatePresence(mpd, &trackBuffer, state);
                mpd.waitForStateChange(pass);
            }
        }
        catch(const std::runtime_error& e)
        {
            Discord_Shutdown();

            std::cout << "Exception: " << e.what() << ". reconnecting to MPD in 5 seconds." << std::endl;
    
            if(isForked && errCount++ == MaxExceptionsWhenForked)
                return -1;
            
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}
