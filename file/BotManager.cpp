#include "BotManager.h"
#include <algorithm>

BotClient& BotManager::GetBot(int index)
{
    return m_bots[index];
}

bool BotManager::GetRoomObservation(
    int roomId,
    RoomObservation& outRoom) const
{
    std::lock_guard<std::mutex> lock(m_roomMutex);

    auto it = m_roomObservations.find(roomId);

    if (it == m_roomObservations.end())
        return false;

    outRoom = it->second;
    return true;
}

void BotManager::UpdateRoomObservation(
    int roomId,
    const RoomObservation& observation)
{
    if (roomId < 0)
        return;

    std::lock_guard<std::mutex> lock(m_roomMutex);
    // 기존 방 정보를 가져온다.
    // 방이 없으면 새로 생성된다.
    RoomObservation& room =
        m_roomObservations[roomId];

    // Snapshot으로 계속 변하는 정보만 갱신한다.
    room.roomId = roomId;
    room.redScore = observation.redScore;
    room.blueScore = observation.blueScore;
    room.timeLeft = observation.timeLeft;
    room.playerCount = observation.playerCount;
    room.players = observation.players;

    // playerIds, playerTeams, teamInfoCount는
    // SC_ROOM_ENTER에서 받은 값이므로 그대로 유지한다.
}

void BotManager::SetRoomTeamInfo(
    int roomId,
    const SC_RoomEnter& packet)
{
    std::lock_guard<std::mutex> lock(m_roomMutex);

    RoomObservation& room =
        m_roomObservations[roomId];

    room.roomId = roomId;

    room.teamInfoCount = (std::min)(
        packet.player_count,
        static_cast<int>(room.playerIds.size())
        );

    for (int i = 0; i < room.teamInfoCount; ++i)
    {
        room.playerIds[i] = packet.player_ids[i];

        room.playerTeams[i] =
            static_cast<TeamType>(packet.teams[i]);
    }
}