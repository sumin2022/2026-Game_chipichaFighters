#include "RoomManager.h"
#include "SESSION.h"

std::unordered_map<int, Room> g_rooms;
std::unordered_map<int, int> g_player_room;

void broadcast_room(int room_id, char* packet, int size)
{
	auto room_it = g_rooms.find(room_id);
	if (room_it == g_rooms.end()) return;

	Room& room = room_it->second;

	for (int i = 0; i < room.player_count; ++i) {
		int player_id = room.players[i];

		SESSION& session = clients[player_id];

		if (session.m_is_connected) {
			session.do_send(size, packet);
		}
	}
}

void update_room(Room& room)
{
	if (!room.active) return;

	for (int i = 0; i < room.player_count; ++i) {
		int attacker_id = room.players[i];

		for (int j = 0; j < room.player_count; ++j) {
			if (i == j) continue;

			int target_id = room.players[j];

			// 공격 판정
			// 충돌 체크
		}
	}
}