#pragma once
#include <unordered_map>
#include <array>

constexpr int MAX_ROOM_PLAYERS = 6;

struct Room {
	int room_id = -1; // room ¹øÈ£ : 1, 2, 3 ...? 
	bool active = false;

	std::array<int, MAX_ROOM_PLAYERS> players;
	int player_count = 0;

	void add_player(int player_id)
	{
		if (player_count >= MAX_ROOM_PLAYERS) return;
		players[player_count++] = player_id;
	}

	void remove_player(int player_id)
	{
		for (int i = 0; i < player_count; ++i) {
			if (players[i] == player_id) {
				players[i] = players[player_count - 1];
				--player_count;
				return;
			}
		}
	}
};

extern std::unordered_map<int, Room> g_rooms;
extern std::unordered_map<int, int> g_player_room;

void broadcast_room(int room_id, char* packet, int size);
void update_room(Room& room);