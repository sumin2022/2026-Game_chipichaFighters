#include "RewardCalculator.h"

#include <algorithm>
#include <cmath>

// 이전 상태와 현재 상태를 비교하여
// 강화학습 보상과 패널티를 계산한다.
//
// 현재 단계에서 계산하는 항목:
// 1. 받은 피해
// 2. 킬 증가
// 3. 데스 증가
// 4. 우리 팀 점수 증가
// 5. 상대 팀 점수 증가

float RewardCalculator::Calculate(
    const BotLearningContext& previous,
    const NetPlayerState& currentPlayer,
    const RLState& currentState,
    const RoomObservation& room,
    TeamType team,
    int attackHits,
    int skillHits) const
{
    float reward = 0.0f;

    // ========================================================
    // +적 접근 보상
    // ========================================================

    const float enemyDistanceChange = previous.previousState.enemyDistance - currentState.enemyDistance;

    // 거리가 줄어들었으면 양수,
    // 거리가 늘어났으면 음수가 된다.
    reward += enemyDistanceChange * m_config.enemyApproachReward;

    // ========================================================
    // 점령지 접근 보상
    // ========================================================

    // 이전 위치
    const float previousX = previous.previousState.normalizedX * AIConfig::MapHalfWidth;
    const float previousY = previous.previousState.normalizedY * AIConfig::MapHalfHeight;

    // 현재 위치
    const float currentX = currentPlayer.x;
    const float currentY = currentPlayer.y;

    // 중앙 점령지 (0, 0)까지 거리
    const float previousCaptureDistance =
        std::sqrt(previousX * previousX + previousY * previousY);

    const float currentCaptureDistance =
        std::sqrt(currentX * currentX + currentY * currentY);

    // 맵 최대 거리 기준으로 정규화
    const float normalizedPreviousCaptureDistance = previousCaptureDistance / MAX_CENTER_DISTANCE;

    const float normalizedCurrentCaptureDistance = currentCaptureDistance / MAX_CENTER_DISTANCE;

    const float captureDistanceChange = normalizedPreviousCaptureDistance - normalizedCurrentCaptureDistance;

    reward += captureDistanceChange * m_config.captureApproachReward;

    // ========================================================
    // +점령지 내부 유지 보상
    // ========================================================

    if (std::abs(currentPlayer.x) <= 300.0f && std::abs(currentPlayer.y) <= 300.0f)
    {
        reward += 0.1f;
    }

    // ========================================================
    // +공격 적중 보상
    // ========================================================

    if (attackHits > 0)
    {
        reward += static_cast<float>(attackHits) * m_config.attackHitReward;
    }

    // ========================================================
    // +스킬 적중 보상
    // ========================================================

    if (skillHits > 0)
    {
        reward += static_cast<float>(skillHits) * m_config.skillHitReward;
    }

    // ========================================================
    // +점령지에서 너무 멀리 있는 것에 대한 약한 패널티
    // ========================================================

    reward -= normalizedCurrentCaptureDistance * 0.01f;

    // ========================================================
    // 1. 받은 피해 패널티
    // ========================================================
    // 이전 HP보다 현재 HP가 낮아졌다면
    // 그 차이를 받은 피해량으로 판단한다.
    const int damageTaken =
        previous.previousHp - currentPlayer.hp;

    if (damageTaken > 0)
    {
        reward +=
            static_cast<float>(damageTaken)
            * m_config.damageTakenPenalty;
    }

    // ========================================================
    // 2. 킬 보상
    // ========================================================

    // 이전 판단 이후 킬 수가 증가했다면
    // 증가한 킬 수만큼 보상을 준다.
    const int killDiff =
        currentPlayer.kill_count
        - previous.previousKills;

    if (killDiff > 0)
    {
        reward +=
            static_cast<float>(killDiff)
            * m_config.killReward;
    }

    // ========================================================
    // 3. 데스 패널티
    // ========================================================

    // 이전 판단 이후 데스 수가 증가했다면
    // 증가한 데스 수만큼 패널티를 준다.
    const int deathDiff =
        currentPlayer.death_count
        - previous.previousDeaths;

    if (deathDiff > 0)
    {
        reward +=
            static_cast<float>(deathDiff)
            * m_config.deathPenalty;
    }

    // ========================================================
    // 4. 팀 점수 변화
    // ========================================================

    int previousTeamScore = 0;
    int previousEnemyScore = 0;

    int currentTeamScore = 0;
    int currentEnemyScore = 0;

    if (team == TeamType::TEAM_RED)
    {
        previousTeamScore =
            previous.previousRedScore;

        previousEnemyScore =
            previous.previousBlueScore;

        currentTeamScore =
            room.redScore;

        currentEnemyScore =
            room.blueScore;
    }
    else if (team == TeamType::TEAM_BLUE)
    {
        previousTeamScore =
            previous.previousBlueScore;

        previousEnemyScore =
            previous.previousRedScore;

        currentTeamScore =
            room.blueScore;

        currentEnemyScore =
            room.redScore;
    }

    // ========================================================
    // 5. 우리 팀 점수 증가 보상
    // ========================================================

    const int teamScoreDiff =
        currentTeamScore - previousTeamScore;

    if (teamScoreDiff > 0)
    {
        reward +=
            static_cast<float>(teamScoreDiff)
            * m_config.teamScoreReward;
    }

    // ========================================================
    // 6. 상대 팀 점수 증가 패널티
    // ========================================================

    const int enemyScoreDiff =
        currentEnemyScore - previousEnemyScore;

    if (enemyScoreDiff > 0)
    {
        reward +=
            static_cast<float>(enemyScoreDiff)
            * m_config.enemyScorePenalty;
    }

    return reward;
}

float RewardCalculator::CalculateGameResult(
    TeamType team,
    TeamType winnerTeam) const
{
    // 무승부 또는 정상적인 팀 정보가 없으면
    // 승패 보상을 주지 않는다.
    if (team == TeamType::TEAM_NONE ||
        winnerTeam == TeamType::TEAM_NONE)
    {
        return 0.0f;
    }

    // 자신의 팀이 승리
    if (team == winnerTeam)
    {
        return m_config.winReward;
    }

    // 패배
    return m_config.losePenalty;
}