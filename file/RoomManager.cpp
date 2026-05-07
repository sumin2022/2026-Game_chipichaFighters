#include "RoomManager.h"
#include "SESSION.h"

std::unordered_map<int, Room> g_rooms;
std::unordered_map<int, int> g_player_room;

static void update_ai(Room& room);
static void update_skills(Room& room);
static void check_player_attacks(Room& room);
static void check_collisions(Room& room);
static void send_room_snapshot(Room& room); //플렝이어 위치, 체력, 스킬 상태 등등 보내기 (여기서 맞나?)

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

void update_all_rooms()
{
	for (auto& [id, room] : g_rooms) {
		update_room(room);
	}
}

void update_room(Room& room)
{
	if (!room.active) return;

	update_ai(room);
	update_skills(room);
	check_player_attacks(room);
	check_collisions(room);
	send_room_snapshot(room);

}

static void update_ai(Room& room) {}
static void update_skills(Room& room) {}
static void check_player_attacks(Room& room) {}
static void check_collisions(Room& room) {}
static void send_room_snapshot(Room& room) {}