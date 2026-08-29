#include "FeatureExtractor.h"
#include "Normalization.h"
#include "AIConfig.h"

#include <algorithm>
#include <cmath>
#include <limits>

// RoomObservation과 BotClient에서 학습에 필요한 특징을 추출한다.
// 위치, HP, 거리, 시간 등의 값을 0~1 또는 -1~1 범위로 정규화하고
// 최종적으로 RLState 또는 arma::colvec 형태의 입력 데이터를 생성한다.

RLState FeatureExtractor::Extract(
    const BotClient& bot,
    const RoomObservation& room) const
{
    // 모든 상태값을 0으로 초기화한다.
    RLState state{};

    // RoomObservation의 플레이어 배열에서
    // 현재 봇 자신에 해당하는 플레이어 정보를 찾는다.
    const NetPlayerState* self = nullptr;

    for (int i = 0; i < room.playerCount; ++i)
    {
        const NetPlayerState& player = room.players[i];

        if (player.player_id == bot.player_id)
        {
            self = &player;
            break;
        }
    }

    // 방 정보에서 자신을 찾지 못했다면
    // 기본값으로 초기화된 상태를 반환한다.
    if (self == nullptr)
        return state;

    // RoomEnter에서 저장한 현재 플레이어의 팀을 찾는다.
    const TeamType selfTeam =
        room.FindTeam(self->player_id);

    if (selfTeam == TeamType::TEAM_NONE)
        return state;

    // ========================================================
    // 1. 자신의 상태
    // ========================================================

    // HP를 0~1 범위로 정규화한다.
    // max_hp가 0인 경우 0으로 처리하여 0으로 나누는 문제를 방지한다.
    if (self->max_hp > 0.0f)
    {
        const float hpRatio = static_cast<float>(self->hp) / self->max_hp;
        state.hpRatio = std::clamp(
            hpRatio,
            0.0f,
            1.0f
        );
    }

    // 월드 X 좌표를 -1~1 범위로 정규화한다.
    state.normalizedX = NormalizeMinusOneToOne(
        self->x,
        -AIConfig::MapHalfWidth,
        AIConfig::MapHalfWidth
    );

    // 월드 Y 좌표를 -1~1 범위로 정규화한다.
    state.normalizedY = NormalizeMinusOneToOne(
        self->y,
        -AIConfig::MapHalfHeight,
        AIConfig::MapHalfHeight
    );

    // 얼굴 방향은 서버에서 이미 -1~1 방향 벡터로 관리하므로
    // 범위를 벗어나지 않도록 제한하여 그대로 사용한다.
    state.faceX = std::clamp(self->faceX, -1.0f, 1.0f);
    state.faceY = std::clamp(self->faceY, -1.0f, 1.0f);

    // 생존 상태를 신경망에서 사용하기 쉽도록
    // 살아 있으면 1, 죽어 있으면 0으로 저장한다.
    state.alive = self->alive ? 1.0f : 0.0f;

    // ========================================================
    // 2. 캐릭터 종류
    // ========================================================

    // 캐릭터 종류는 숫자 하나를 그대로 넣지 않고
    // 해당 캐릭터 위치만 1이 되는 One-Hot Encoding으로 표현한다.
    switch (self->character)
    {
    case CharacterType::CHAR_DEALER:
        state.characterDealer = 1.0f;
        break;

    case CharacterType::CHAR_ARCHER:
        state.characterArcher = 1.0f;
        break;

    case CharacterType::CHAR_TANKER:
        state.characterTanker = 1.0f;
        break;

    case CharacterType::CHAR_HEALER:
        state.characterHealer = 1.0f;
        break;

    default:
        break;
    }

    // ========================================================
    // 3. 가장 가까운 적과 아군 탐색
    // ========================================================

    const NetPlayerState* nearestEnemy = nullptr;
    const NetPlayerState* nearestAlly = nullptr;

    float nearestEnemyDistanceSquared =
        std::numeric_limits<float>::max();

    float nearestAllyDistanceSquared =
        std::numeric_limits<float>::max();

    for (int i = 0; i < room.playerCount; ++i)
    {
        const NetPlayerState& player = room.players[i];

        // 자기 자신은 탐색 대상에서 제외한다.
        if (player.player_id == self->player_id)
            continue;

        // 죽은 플레이어는 현재 이동·공격 대상에서 제외한다.
        if (!player.alive)
            continue;

        // RoomEnter에서 저장한 플레이어의 팀을 찾는다.
        const TeamType playerTeam =
            room.FindTeam(player.player_id);

        // 팀 정보를 찾지 못한 플레이어는 제외한다.
        if (playerTeam == TeamType::TEAM_NONE)
            continue;

        const float dx = player.x - self->x;
        const float dy = player.y - self->y;

        // 실제 거리 비교에는 sqrt가 필요 없으므로
        // 제곱 거리를 이용하여 가장 가까운 플레이어를 찾는다.
        const float distanceSquared =
            dx * dx + dy * dy;

        if (playerTeam != selfTeam)
        {
            // 상대 팀이면 가장 가까운 적 후보로 검사한다.
            if (distanceSquared < nearestEnemyDistanceSquared)
            {
                nearestEnemyDistanceSquared = distanceSquared;
                nearestEnemy = &player;
            }
        }
        else
        {
            // 같은 팀이면 가장 가까운 아군 후보로 검사한다.
            if (distanceSquared < nearestAllyDistanceSquared)
            {
                nearestAllyDistanceSquared = distanceSquared;
                nearestAlly = &player;
            }
        }
    }

    // ========================================================
    // 4. 가장 가까운 적 상태
    // ========================================================

    if (nearestEnemy != nullptr)
    {
        const float relativeX = nearestEnemy->x - self->x;
        const float relativeY = nearestEnemy->y - self->y;

        const float distance = std::sqrt(
            nearestEnemyDistanceSquared
        );

        // 적이 자신의 어느 방향에 있는지 나타낸다.
        // X 좌표 차이는 맵 전체 가로 길이를 기준으로 -1~1 정규화한다.
        state.enemyRelativeX = std::clamp(
            relativeX / (AIConfig::MapHalfWidth * 2.0f),
            -1.0f,
            1.0f
        );

        // Y 좌표 차이는 맵 전체 세로 길이를 기준으로 -1~1 정규화한다.
        state.enemyRelativeY = std::clamp(
            relativeY / (AIConfig::MapHalfHeight * 2.0f),
            -1.0f,
            1.0f
        );

        // 적과의 거리를 0~1 범위로 정규화한다.
        state.enemyDistance = std::clamp(
            distance / MAX_MAP_DISTANCE,
            0.0f,
            1.0f
        );

        // 가장 가까운 적의 HP를 0~1 범위로 정규화한다.
        if (nearestEnemy->max_hp > 0.0f)
        {
            const float enemyHpRatio =
                static_cast<float>(nearestEnemy->hp) / nearestEnemy->max_hp;
            state.enemyHpRatio = std::clamp(
                enemyHpRatio,
                0.0f,
                1.0f
            );
        }
    }

    // ========================================================
    // 5. 가장 가까운 아군 상태
    // ========================================================

    if (nearestAlly != nullptr)
    {
        const float relativeX = nearestAlly->x - self->x;
        const float relativeY = nearestAlly->y - self->y;

        const float distance = std::sqrt(
            nearestAllyDistanceSquared
        );

        state.allyRelativeX = std::clamp(
            relativeX / (AIConfig::MapHalfWidth * 2.0f),
            -1.0f,
            1.0f
        );

        state.allyRelativeY = std::clamp(
            relativeY / (AIConfig::MapHalfHeight * 2.0f),
            -1.0f,
            1.0f
        );

        state.allyDistance = std::clamp(
            distance / MAX_MAP_DISTANCE,
            0.0f,
            1.0f
        );

        if (nearestAlly->max_hp > 0.0f)
        {
            const float allyHpRatio = static_cast<float>(nearestAlly->hp) / nearestAlly->max_hp;
            state.allyHpRatio = std::clamp(
                allyHpRatio,
                0.0f,
                1.0f
            );
        }
    }

    // ========================================================
    // 6. 경기 상태
    // ========================================================

    // 현재 봇이 속한 팀을 기준으로
    // 아군 점수와 상대 팀 점수를 구분한다.
    int teamScore = 0;
    int enemyScore = 0;


    if (selfTeam == TeamType::TEAM_RED)
    {
        teamScore = room.redScore;
        enemyScore = room.blueScore;
    }
    else if(selfTeam == TeamType::TEAM_BLUE)
    {
        teamScore = room.blueScore;
        enemyScore = room.redScore;
    }

    // 현재 점수에 명확한 최대치가 정의되어 있지 않으므로,
    // 두 팀 점수의 합을 기준으로 현재 점수 비율을 계산한다.
    const int totalScore = teamScore + enemyScore;

    if (totalScore > 0)
    {
        state.teamScoreRatio =
            static_cast<float>(teamScore) /
            static_cast<float>(totalScore);

        state.enemyScoreRatio =
            static_cast<float>(enemyScore) /
            static_cast<float>(totalScore);
    }

    // 남은 경기 시간을 0~1 범위로 정규화한다.
    state.timeRatio = std::clamp(
        room.timeLeft / AIConfig::MatchDuration,
        0.0f,
        1.0f
    );

    // 현재 단계에서는 아이템 상태를 학습에 사용하지 않으므로
    // itemRelativeX, itemRelativeY, itemDistance, itemActive는
    // 초기값 0을 유지한다.

    return state;
}