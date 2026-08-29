#pragma once

#include <array>
#include "../../../GameServer/file/protocol.h"

// 한 개 방의 현재 관측 정보를 저장한다.
// 방 ID, 플레이어 목록, 팀 점수, 남은 시간,
// 아이템 활성 상태 등 Snapshot으로 받은 방 전체 상태를 관리한다.
// 방 입장 시 전달받은 팀 정보 등을 관리한다.

struct RoomObservation
{
    int roomId = -1;

    int redScore = 0;
    int blueScore = 0;
    float timeLeft = 0.0f;

    // Snapshot으로 계속 갱신되는 플레이어 전투 상태
    std::array<NetPlayerState, MAX_ROOM_AI> players{};
    int playerCount = 0;

    // SC_ROOM_ENTER에서 한 번 전달받은 플레이어 ID와 팀 정보
    // Snapshot의 플레이어 순서가 달라져도 ID로 팀을 찾을 수 있도록
    // 플레이어 ID와 팀을 함께 보관한다.
    std::array<int, MAX_ROOM_PLAYERS> playerIds{};
    std::array<TeamType, MAX_ROOM_PLAYERS> playerTeams{};
    int teamInfoCount = 0;

    std::array<bool, 2> itemActive{};

    // 플레이어 ID에 해당하는 팀을 찾는다.
    // 찾지 못하면 -1을 반환한다.
    TeamType FindTeam(int playerId) const
    {
        for (int i = 0; i < teamInfoCount; ++i)
        {
            if (playerIds[i] == playerId)
                return playerTeams[i];
        }

        return TeamType::TEAM_NONE;
    }
};