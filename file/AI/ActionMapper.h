#pragma once

#include "../Bot.h"
#include "RoomObservation.h"
#include "../Network.h"
#include "AIMapCollision.h"

class ActionMapper
{
public:
    void Execute(
        BotClient& bot,
        size_t action,
        Network& network,
        const RoomObservation& room
    );
private:
    AIMapCollision m_mapCollision;
};