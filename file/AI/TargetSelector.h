#pragma once
#include "RoomObservation.h"
#include "AIMapCollision.h"
#include "AIConfig.h"
#include <algorithm>
#include <array>
#include <cmath>

inline float EffectiveAttackRange(CharacterType character)
{
    switch(character) {
    case CharacterType::CHAR_DEALER: return AIConfig::DealerAttackRange;
    case CharacterType::CHAR_ARCHER: return AIConfig::ArcherAttackRange;
    case CharacterType::CHAR_TANKER: return AIConfig::TankerAttackRange;
    case CharacterType::CHAR_HEALER: return AIConfig::HealerAttackRange;
    default: return -1.0f;
    }
}

// 팀이 아닌 적 플레이어 ID를 최대 3개까지 반환한다.
inline std::array<int, 3> EnemySlotIds(const RoomObservation& room, TeamType team) 
{
    std::array<int, 3> result{{-1,-1,-1}};

    if (team == TeamType::TEAM_NONE) return result;
    std::array<int, MAX_ROOM_PLAYERS> ids{};
    std::size_t count = 0;

    for (int i=0; i<room.teamInfoCount && i<MAX_ROOM_PLAYERS; ++i) {
        auto other = room.FindTeam(room.playerIds[i]);
        if (other != TeamType::TEAM_NONE && other != team) ids[count++] = room.playerIds[i];
    }

    std::sort(ids.begin(), ids.begin()+count);

    auto end = std::unique(ids.begin(), ids.begin()+count); // 혹시모를 중복 제거

    count = static_cast<std::size_t>(end-ids.begin()); // 반복자를 이용한 count 계산

    for (std::size_t i=0; i<count && i<3; ++i) result[i]=ids[i];
    return result;
}

// 플레이어 ID에 해당하는 Snapshot 정보를 찾는다. 없으면 nullptr 반환
inline const NetPlayerState* FindObservedPlayer(const RoomObservation& room, int id) 
{
    if (id < 0) return nullptr;

    for (int i=0; i<room.playerCount && i<MAX_ROOM_AI; ++i)
        if (room.players[i].player_id == id) return &room.players[i];

    return nullptr;
}
