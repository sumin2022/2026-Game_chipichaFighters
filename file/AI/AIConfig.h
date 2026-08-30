#pragma once
#include <iostream>

// AI와 강화학습에서 공통으로 사용하는 설정값을 모아둔다.
// 판단 주기, 맵 크기, 경기 시간, 행동 개수,
// 보상과 패널티 값, Epsilon, 학습률,
// ReplayBuffer 크기와 Batch 크기 등을 정의한다.

constexpr float MAP_HALF_WIDTH = 1500.0f;
constexpr float MAP_HALF_HEIGHT = 1000.0f;
constexpr float MATCH_TIME = 150.0f;
constexpr float MAX_MAP_DISTANCE = 3605.55f;
// 맵 모서리에서 중앙까지 최대 거리
constexpr float MAX_CENTER_DISTANCE = 1802.78f;

struct RewardConfig
{
    float damageDealtReward = 0.02f;
    float damageTakenPenalty = -0.02f;

    float killReward = 10.0f;
    float deathPenalty = -10.0f;

    float teamScoreReward = 1.0f;
    float enemyScorePenalty = -1.0f;

    float itemPickupReward = 3.0f;

    float winReward = 30.0f;
    float losePenalty = -30.0f;

    float idlePenalty = -0.01f;

    // 적에게 가까워지는 행동 유도
    float enemyApproachReward = 0.5f;
    // 중앙 점령지로 가까워지는 행동 유도
    float captureApproachReward = 0.5f;

    float attackHitReward = 1.0f;
    float skillHitReward = 1.5f;
};

struct AIConfig
{
    static constexpr float DecisionInterval = 0.1f;

    static constexpr float MapHalfWidth = 1500.0f;
    static constexpr float MapHalfHeight = 1000.0f;

    static constexpr float MatchDuration = 150.0f;

    // DQN
    static constexpr std::size_t StateSize = 25;
    static constexpr std::size_t BatchSize = 64;

    static constexpr float Gamma = 0.99f;

    // Epsilon-Greedy
    static constexpr float EpsilonStart = 1.0f;
    static constexpr float EpsilonMin = 0.2f;
    static constexpr float EpsilonDecay = 0.999f;

    // Epsilon은 매 Train마다 낮추면 너무 빨리 감소하므로
    // 일정 학습 횟수마다 감소시킨다.
    static constexpr std::size_t EpsilonDecayInterval = 100;

    // Target Network를 Online Network와 동기화하는 주기
    static constexpr std::size_t TargetUpdateInterval = 500;

    RewardConfig reward{};
};