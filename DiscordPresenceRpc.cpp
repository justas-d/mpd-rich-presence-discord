#include "DiscordPresenceRpc.h"

void DiscordPresenceRpc::shutdown() 
{
    Discord_Shutdown();
}
DiscordPresenceRpc::~DiscordPresenceRpc()
{
    shutdown();
}

void DiscordPresenceRpc::setApp(const char* app)
{
    DiscordEventHandlers h = {};
    Discord_Shutdown();
    Discord_Initialize(app, &h, 1, 0);
}

void DiscordPresenceRpc::send(DiscordRichPresence& payload)
{
    Discord_UpdatePresence(&payload);
}
