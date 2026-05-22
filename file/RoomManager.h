#pragma once
#include <unordered_map>
#include <array>
#include <vector>
#include <mutex>
#include "protocol.h"
#include "Character.h"

constexpr int MAX_ROOM_PLAYERS = 2; // 원래 6명

struct PlayerState {
	int id = -1;

	float x = 0.0f;
	float y = 0.0f;

	int hp = 100;
	int max_hp = 100;

	int mp = 0;
	int max_mp = 0;

	int attack_damage = 10;
	float attack_cooldown = 1.0f;
	float attack_range = 150.0f;
	float move_speed = 300.0f;

	bool alive = true;

	CharacterType character = CHAR_NONE;
	AttackType attack_type = AttackType::NONE;
	SkillType active_skill = SkillType::NONE;
	PassiveType passive_skill = PassiveType::NONE;

	float attack_timer = 0.0f;       // 기본 공격 쿨타임 진행 상태
	float skill_timer = 0.0f;        // 스킬 쿨타임 진행 상태

	int last_attack_target = -1;     // 마지막으로 공격한 대상
	float last_damaged_time = 0.0f;  // 마지막으로 피해받은 시간

	bool lobby_ready = false;
};

enum class RoomState {
	MATCHING,
	LOBBY,
	INGAME,
	ENDED
};

struct Room {
	int room_id = -1; // room 번호 : 1, 2, 3 ...? 
	RoomState state = RoomState::MATCHING;
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

	bool all_ready() const
	{
		if (player_count < MAX_ROOM_PLAYERS) return false;

		for (int i = 0; i < player_count; ++i) {
			if (states[i].character == CHAR_NONE) return false;
			if (!states[i].lobby_ready) return false;
		}

		return true;
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

	void select_character(int player_id, CharacterType character);
	void set_lobby_ready(int player_id, bool ready);

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

	void enter_lobby(int room_id);
	PlayerState* find_player_state(Room& room, int player_id);

	void apply_character_to_player(PlayerState& state, CharacterType character);

	std::unordered_map<int, Room> m_rooms;
	std::unordered_map<int, int> m_player_room;

	int m_next_room_id = 1;

	std::mutex m_request_mutex;
	std::vector<int> m_match_requests; // 매칭요청대기업
	std::vector<int> m_leave_requests; // 퇴장 요청 대기업
};



