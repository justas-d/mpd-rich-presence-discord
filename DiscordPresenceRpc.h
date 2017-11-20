#pragma once

#include <cstdint>
#include "lib/discord-rpc/include/discord-rpc.h"

class DiscordPresenceRpc
{
public:
    
    ~DiscordPresenceRpc();
    
    void setApp(const char* app);
    void send(DiscordRichPresence& presence);
    
};


