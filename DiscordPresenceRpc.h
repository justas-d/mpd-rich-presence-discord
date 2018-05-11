#pragma once

#include <cstdint>
#include <discord_rpc.h>

class DiscordPresenceRpc
{
public:
    
    ~DiscordPresenceRpc();
    
    void setApp(const char* app);
    void send(DiscordRichPresence& presence);
    
};


