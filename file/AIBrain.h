#pragma once

#include "Bot.h"

class AIBrain
{
public:
    void LoadModel();
    int DecideAction(const BotClient& bot);
    void Train();
};