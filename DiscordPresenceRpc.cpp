#include "DiscordPresenceRpc.h"

DiscordPresenceRpc::~DiscordPresenceRpc()
{
    Discord_Shutdown();
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