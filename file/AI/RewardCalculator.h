#pragma once

#include "AIConfig.h"
#include "RoomObservation.h"
#include "../Bot.h"

// 이전 판단 시점과 현재 상태를 비교하여
// 강화학습에 사용할 Reward를 계산한다.
//
// 현재 단계에서는 Snapshot만으로 확실히 판단할 수 있는
// 받은 피해, 킬, 데스, 팀 점수 변화를 기준으로 보상을 계산한다.
class RewardCalculator
{
public:
    explicit RewardCalculator(
        const RewardConfig& config
    )
        : m_config(config)
    {
    }

    // 이전 학습 상태와 현재 방 상태를 비교하여
    // 현재 행동에 대한 보상값을 계산한다.
    float Calculate(
        const BotLearningContext& previous,
        const NetPlayerState& currentPlayer,
        const RLState& currentState,
        const RoomObservation& room,
        TeamType team
    ) const;

    // 게임 종료 시 승/패 보상을 계산한다.
    float CalculateGameResult(
        TeamType team,
        TeamType winnerTeam
    ) const;

private:
    RewardConfig m_config;
};