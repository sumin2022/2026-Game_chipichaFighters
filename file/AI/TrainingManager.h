#pragma once

#include "../Bot.h"
#include "RoomObservation.h"
#include "../Network.h"
#include "../AIDatabase.h"
#include "../DBThread.h"

#include "FeatureExtractor.h"
#include "RewardCalculator.h"
#include "ReplayBuffer.h"
#include "ActionMapper.h"
#include "DQNAgent.h"

#include <cstddef>
#include <unordered_map>

// 봇별 강화학습 흐름을 관리한다.
//
// 현재 상태 추출
// 이전 행동의 Reward 계산
// Experience 저장
// 다음 행동 선택
// 실제 행동 실행

class DBThread;

class TrainingManager
{
public:
    TrainingManager(
        DBThread* dbThread,
        const AIModelInfo& modelInfo
    );

    void UpdateBot(
        BotClient& bot,
        const RoomObservation& room,
        float dt,
        Network& network
    );

    void OnGameEnd(
        BotClient& bot,
        const SC_GameResult& result
    );

    // 테스트용
    // 현재 ReplayBuffer에 저장된 Experience 개수를 확인한다.
    std::size_t GetReplayBufferSize() const;
private:

    struct GameRewardStat
    {
        double rewardSum = 0.0;
        int rewardCount = 0;
    };

    std::unordered_map<int, GameRewardStat> m_gameRewardStats;

    DBThread* m_dbThread = nullptr;
    AIModelInfo m_modelInfo;
	std::unordered_map<int, double> m_botEpisodeRewards; //누적 봇별 reward
    // 봇 ID를 기준으로 각 봇의 이전 상태와 행동을 따로 관리한다.
    // 여러 봇이 하나의 DQN 모델을 공유하더라도
    // 이전 경험 정보는 봇마다 독립적으로 저장되어야 한다.
    FeatureExtractor m_featureExtractor;
    RewardCalculator m_rewardCalculator;
    ReplayBuffer m_replayBuffer;
    ActionMapper m_actionMapper;
    DQNAgent m_dqnAgent;
};