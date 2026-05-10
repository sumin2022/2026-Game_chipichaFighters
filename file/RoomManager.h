#pragma once
#include <unordered_map>
#include <array>
#include <vector>
#include <mutex>

constexpr int MAX_ROOM_PLAYERS = 6;

struct PlayerState {
	int id = -1;
	short x = 0;
	short y = 0;
	int hp = 100;
	bool alive = true;
};

struct Room {
	int room_id = -1; // room 번호 : 1, 2, 3 ...? 
	bool active = false;

	std::array<int, MAX_ROOM_PLAYERS> players;
	std::array<PlayerState, MAX_ROOM_PLAYERS> states;
	int player_count = 0;

	Room()
	{
		players.fill(-1);
	}

	bool add_player(int player_id)
	{
		if (player_count >= MAX_ROOM_PLAYERS) return false;
		for (int i = 0; i < player_count; ++i) {
			if (players[i] == player_id) return false;
		}

		players[player_count] = player_id;

		states[player_count].id = player_id;
		states[player_count].hp = 100;
		states[player_count].alive = true;

		++player_count;
		return true;
	}

	void remove_player(int player_id)
	{
		for (int i = 0; i < player_count; ++i) {
			if (players[i] == player_id) {
				players[i] = players[player_count - 1];
				states[i] = states[player_count - 1];

				players[player_count - 1] = -1;
				states[player_count - 1] = PlayerState{};
				--player_count;
				return;
			}
		}
	}

	bool is_full() const
	{
		return player_count >= MAX_ROOM_PLAYERS;
	}
};

class RoomManager {
public:
	static RoomManager& Instance()
	{
		static RoomManager instance;
		return instance;
	}

	void request_matchmaking(int player_id);
	bool join_room(int room_id, int player_id);
	void leave_room(int player_id);
	void start_room(int room_id);

	void broadcast_room(int room_id, char* packet, int size);
	void update_all_rooms();

private:
	RoomManager() = default;

	RoomManager(const RoomManager&) = delete;
	RoomManager& operator=(const RoomManager&) = delete;

	void process_pending_requests();

	void update_room(Room& room);

	void update_ai(Room& room);
	void update_skills(Room& room);
	void check_player_attacks(Room& room);
	void check_collisions(Room& room);
	void send_room_snapshot(Room& room); //플렝이어 위치, 체력, 스킬 상태 등등 보내기 (여기서 맞나?)

	std::unordered_map<int, Room> m_rooms;
	std::unordered_map<int, int> m_player_room;

	int m_next_room_id = 1;

	std::mutex m_request_mutex;
	std::vector<int> m_match_requests; // 매칭요청대기업
	std::vector<int> m_leave_requests; // 퇴장 요청 대기업
};



