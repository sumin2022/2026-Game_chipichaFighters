#include "RoomManager.h"

void ScoreManager::start_game() // 게임 시작 시 초기화
{
	game_timer = GAME_DURATION;
    red_score = 0.0f;
    blue_score = 0.0f;
}

bool ScoreManager::is_in_capture_zone(float x, float y) const // 점령지 안에 있는지 확인
{
    return x >= zone.min_x &&
        x <= zone.max_x &&
        y >= zone.min_y &&
        y <= zone.max_y;
}

float ScoreManager::get_capture_multiplier(int count) const // 점령지 안에 있는 플레이어 수에 따른 점수 배율 계산
{
    switch (count) {
    case 1:
        return 1.0f;
    case 2:
        return 1.5f;
    case 3:
        return 2.0f;
    default:
        return 0.0f;
    }
}

void ScoreManager::update(Room& room, float dt) // 게임 시간 업데이트
{
    game_timer -= dt;

    int red_count = 0;
    int blue_count = 0;

    for (int i = 0; i < room.player_count; ++i) {
        PlayerState& player = room.states[i];

        if (!player.alive) continue;

        if (!is_in_capture_zone(player.x, player.y)) continue;

        if (player.team == TeamType::TEAM_RED) {
            ++red_count;
        }
        else if (player.team == TeamType::TEAM_BLUE) {
            ++blue_count;
        }
    }

    if (red_count > 0) {
        red_score += get_capture_multiplier(red_count) * dt;
    }

    if (blue_count > 0) {
        blue_score += get_capture_multiplier(blue_count) * dt;
    }
}

bool ScoreManager::is_time_over() const // 게임 시간이 끝났는지 확인
{
    return game_timer <= 0.0f;
}

void ScoreManager::make_result(Room& room, SC_GameResult& packet) // 게임 결과 패킷 생성
{
    packet.size = sizeof(SC_GameResult);
    packet.type = PACKET_TYPE::SC_GAME_RESULT;

    packet.red_score = static_cast<int>(red_score);
    packet.blue_score = static_cast<int>(blue_score);

    packet.winner_team =
        (red_score > blue_score) ? TeamType::TEAM_RED : TeamType::TEAM_BLUE;

    packet.player_count = room.player_count;

    for (int i = 0; i < room.player_count; i++) {
        packet.player_ids[i] = room.states[i].id;
        packet.kills[i] = room.states[i].kill_count;
        packet.deaths[i] = room.states[i].death_count;
    }
}
