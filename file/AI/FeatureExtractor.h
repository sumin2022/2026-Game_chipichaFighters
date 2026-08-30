#pragma once

#include "../Bot.h"
#include "RoomObservation.h"
#include "AIMapCollision.h"

class FeatureExtractor
{
public:
    RLState Extract(
        const BotClient& bot,
        const RoomObservation& room
    )const;
private:
    AIMapCollision m_mapCollision;
};