#include "RoomManager.h"

void ScoreManager::update(Room& room, float dt)
{
    game_timer -= dt;
}

bool ScoreManager::is_time_over() const
{
    return game_timer <= 0.0f;
}

void ScoreManager::make_result(Room& room, SC_GameResult& packet)
{
    packet.size = sizeof(SC_GameResult);
    packet.type = SC_GAME_RESULT;

    packet.red_score = static_cast<int>(red_score);
    packet.blue_score = static_cast<int>(blue_score);

    packet.winner_team =
        (red_score > blue_score) ? TEAM_RED : TEAM_BLUE;

    packet.player_count = room.player_count;

    for (int i = 0; i < room.player_count; i++) {
        packet.player_ids[i] = room.states[i].id;
        packet.kills[i] = room.states[i].kill_count;
        packet.deaths[i] = room.states[i].death_count;
    }
}