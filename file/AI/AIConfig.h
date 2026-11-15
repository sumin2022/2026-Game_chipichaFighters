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
    // 모든 캐릭터가 공유하는 보상이다. 실제로 계산되는 항목만 둔다.
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

    // 중앙 점령지로 가까워지는 행동 유도
    float captureApproachReward = 2.0f;
    // 점령지 안에서 얻는 작은 유지/진입 보상. 전투 보상보다 커지지 않게 둔다.
    float captureInsideReward = 0.25f;
    float captureEntryReward = 0.35f;
    float captureDistancePenalty = 0.01f;

    float attackHitReward = 1.0f;
    float skillHitReward = 1.5f;
};

// 공격 사거리 자체가 아니라, Reward 계산에만 쓰는 거리 정책이다.
struct CharacterCombatRewardConfig
{
    // 근거리: 사거리 밖 접근, 사거리 진입, 불필요한 후퇴
    float meleeApproachReward = 2.0f;
    float meleeEnterRangeReward = 0.45f;
    float meleeRetreatPenalty = -0.8f;

    // 원거리: 사거리 밖 접근, 너무 가까울 때 이탈, 선호 거리 유지
    float rangedApproachReward = 1.0f;
    float rangedRetreatReward = 1.25f;
    float rangedMaintainReward = 0.18f;
    float safeDistanceRatio = 0.45f;
    float preferredDistanceRatio = 0.75f;
    float preferredDistanceToleranceRatio = 0.12f;
};

// 공통 보상과 캐릭터별 거리 보상을 한 곳에서 독립적으로 조정한다.
struct CharacterRewardConfig
{
    RewardConfig common{};
    CharacterCombatRewardConfig dealer{};
    CharacterCombatRewardConfig tanker{};
    CharacterCombatRewardConfig archer{};
    CharacterCombatRewardConfig healer{};
};

struct AIConfig
{
    static constexpr float DecisionInterval = 0.1f;

    static constexpr float MapHalfWidth = 1500.0f;
    static constexpr float MapHalfHeight = 1000.0f;

    static constexpr float MatchDuration = 150.0f;

    // 메인 서버 기본공격 사거리(월드 단위)
    static constexpr float DealerAttackRange = 150.0f;
    static constexpr float ArcherAttackRange = 600.0f;
    static constexpr float TankerAttackRange = 150.0f;
    static constexpr float HealerAttackRange = 625.0f;
    // DQN
    static constexpr std::size_t StateSize = 64;
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

    CharacterRewardConfig rewards{};

    // 점령지 복귀는 시작/종료 반경을 분리해 경계 토글을 막는다.
    static constexpr float CaptureReturnStartDistance = 850.0f;
    static constexpr float CaptureReturnEndDistance = 300.0f;
    static constexpr float ReturnPathNodeReachDistance = 30.0f;
    static constexpr int ReturnPathStallDecisionLimit = 10;
    static constexpr float ReturnPathRetryCooldown = 2.0f;
};
