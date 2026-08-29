#pragma once

#include "../Bot.h"
#include "RoomObservation.h"

class FeatureExtractor
{
public:
    RLState Extract(
        const BotClient& bot,
        const RoomObservation& room
    )const;
};