#pragma once

#include "AI/RoomObservation.h"
#include <unordered_map>
#include <mutex>
#include <array>
#include "Bot.h"

constexpr int MAX_BOTS = 200;
class BotManager
{
public:
    BotClient& GetBot(int index);

    bool GetRoomObservation(
        int roomId,
        RoomObservation& outRoom
    ) const;

    void UpdateRoomObservation(
        int roomId,
        const RoomObservation& observation
    );

    // 방 입장 패킷에서 받은 플레이어별 팀 정보를 저장한다.
    void SetRoomTeamInfo(
        int roomId,
        const SC_RoomEnter& packet
    );

private:
    std::array<BotClient, 200> m_bots{};

    std::unordered_map<int, RoomObservation> m_roomObservations;

    mutable std::mutex m_roomMutex;
};