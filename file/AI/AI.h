#pragma once
#include "../Bot.h"
#include "RoomObservation.h"
#include "TrainingManager.h"

class Network;
class DBThread;

class AI
{
public:
    AI(DBThread* dbThread, const AIModelInfo& modelInfo);

    void Update(
        BotClient& bot,
        const RoomObservation& room,
        Network& network,
        float dt
    );

    void OnGameEnd(
        BotClient& bot,
        const SC_GameResult& result
    );

private:
    TrainingManager m_trainingManager;
};