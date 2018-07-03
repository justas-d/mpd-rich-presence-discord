#pragma once

#include <cstdint>
#include <discord_rpc.h>

class DiscordPresenceRpc
{
    bool _broadcastIdle;

public:
    
    DiscordPresenceRpc(bool broadcastIdle) 
        : _broadcastIdle(broadcastIdle)
    {
    }

    ~DiscordPresenceRpc();
    
    void setApp(const char* app);
    void send(DiscordRichPresence& presence);
    void shutdown();

    inline bool shouldBroadcastIdle()
    { return _broadcastIdle; }
};


