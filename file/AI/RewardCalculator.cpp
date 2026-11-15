#include "RewardCalculator.h"
#include "Action.h"

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

RewardCalculator::Result RewardCalculator::Calculate(
    const BotLearningContext& previous,
    const NetPlayerState& currentPlayer,
    const RLState& currentState,
    const RoomObservation& room,
    TeamType team,
    CharacterType character,
    int attackHits,
    int skillHits) const
{
    Result result{};
    float& reward = result.total;

    const CharacterCombatRewardConfig* characterConfig = nullptr;
    const bool melee = character == CharacterType::CHAR_DEALER ||
                       character == CharacterType::CHAR_TANKER;
    switch (character)
    {
    case CharacterType::CHAR_DEALER: characterConfig = &m_config.dealer; break;
    case CharacterType::CHAR_TANKER: characterConfig = &m_config.tanker; break;
    case CharacterType::CHAR_ARCHER: characterConfig = &m_config.archer; break;
    case CharacterType::CHAR_HEALER: characterConfig = &m_config.healer; break;
    default: break;
    }

    // ========================================================
    // +적 접근 보상
    // ========================================================

    const auto& oldState = previous.previousState;
    const auto slot = static_cast<std::size_t>(DecodeAction(previous.previousAction).target);
    const auto& oldEnemy = oldState.enemies[slot];
    const EnemyObservation* newEnemy = nullptr;

    for (const auto& enemy : currentState.enemies)
        if (oldEnemy.playerId >= 0 && enemy.playerId == oldEnemy.playerId) newEnemy = &enemy;

    // 같은 방/대상/생명 주기에만 비교한다. 타겟 변경 자체로는 보상을 얻지 않는다.
    if (oldState.roomId == currentState.roomId && oldState.alive > 0.5f && currentPlayer.alive &&
        previous.previousDeaths == currentPlayer.death_count &&
        oldState.effectiveRange > 0 && oldState.effectiveRange == currentState.effectiveRange &&
        oldEnemy.exists > 0.5f && oldEnemy.alive > 0.5f && oldEnemy.visible > 0.5f && newEnemy &&
        newEnemy->exists > 0.5f && newEnemy->alive > 0.5f && newEnemy->visible > 0.5f &&
        oldEnemy.deaths == newEnemy->deaths)
    {
        const float range = oldState.effectiveRange;
        const float previousDistance = oldEnemy.distance * MAX_MAP_DISTANCE;
        const float currentDistance = newEnemy->distance * MAX_MAP_DISTANCE;
        const float previousOutsideRange = std::max(0.0f, previousDistance - range);
        const float currentOutsideRange = std::max(0.0f, currentDistance - range);

        if (characterConfig != nullptr)
        {
            float distanceReward = 0.0f;
            if (melee)
            {
                // 사거리 밖에서는 적극적으로 접근하되, 진입 뒤 밀착을 보상하지 않는다.
                distanceReward += (previousOutsideRange - currentOutsideRange) / MAX_MAP_DISTANCE
                    * characterConfig->meleeApproachReward;
                if (previousOutsideRange > 0.0f && currentOutsideRange <= 0.0f)
                    distanceReward += characterConfig->meleeEnterRangeReward;
                if (previousOutsideRange <= 0.0f && currentOutsideRange > 0.0f)
                    distanceReward += characterConfig->meleeRetreatPenalty
                        * std::min(1.0f, currentOutsideRange / range);
            }
            else
            {
                const float safeDistance = range * characterConfig->safeDistanceRatio;
                const float preferredDistance = range * characterConfig->preferredDistanceRatio;
                const float tolerance = range * characterConfig->preferredDistanceToleranceRatio;
                if (previousOutsideRange > 0.0f)
                    distanceReward += (previousOutsideRange - currentOutsideRange) / MAX_MAP_DISTANCE
                        * characterConfig->rangedApproachReward;
                if (previousDistance < safeDistance && currentDistance > previousDistance)
                    distanceReward += (currentDistance - previousDistance) / range
                        * characterConfig->rangedRetreatReward;
                if (currentDistance >= safeDistance && currentDistance <= range &&
                    std::abs(currentDistance - preferredDistance) <= tolerance)
                    distanceReward += characterConfig->rangedMaintainReward;
            }
            reward += distanceReward;
            result.characterDistance += distanceReward;
        }
    }

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

    const float captureApproach = captureDistanceChange * m_config.common.captureApproachReward;
    reward += captureApproach;
    result.capture += captureApproach;

    // ========================================================
    // +점령지 내부 유지 보상
    // ========================================================

    const bool wasInCapture = std::abs(previousX) <= 300.0f && std::abs(previousY) <= 300.0f;
    const bool isInCapture = std::abs(currentPlayer.x) <= 300.0f && std::abs(currentPlayer.y) <= 300.0f;
    if (isInCapture)
    {
        const float captureReward = m_config.common.captureInsideReward +
            (!wasInCapture ? m_config.common.captureEntryReward : 0.0f);
        reward += captureReward;
        result.capture += captureReward;
    }

    // ========================================================
    // +공격 적중 보상
    // ========================================================

    if (attackHits > 0)
    {
        reward += static_cast<float>(attackHits) * m_config.common.attackHitReward;
    }

    // ========================================================
    // +스킬 적중 보상
    // ========================================================

    if (skillHits > 0)
    {
        reward += static_cast<float>(skillHits) * m_config.common.skillHitReward;
    }

    // ========================================================
    // +점령지에서 너무 멀리 있는 것에 대한 약한 패널티
    // ========================================================

    reward -= normalizedCurrentCaptureDistance * m_config.common.captureDistancePenalty;

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
            * m_config.common.damageTakenPenalty;
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
            * m_config.common.killReward;
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
            * m_config.common.deathPenalty;
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
            * m_config.common.teamScoreReward;
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
            * m_config.common.enemyScorePenalty;
    }

    result.common = result.total - result.characterDistance - result.capture;
    return result;
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
        return m_config.common.winReward;
    }

    // 패배
    return m_config.common.losePenalty;
}
