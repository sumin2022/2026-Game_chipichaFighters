#pragma once
#include "Character.h"
#include "Score.h"
#include "protocol.h"
#include <array>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "Character.h"
#include "Score.h"

//가로 약 3000 세로 약 2000 중앙 0,0 
constexpr float RED_SPAWN_X = 1157.0f;
constexpr float RED_SPAWN_Y = 0.0f;

constexpr float BLUE_SPAWN_X = -1157.0f;
constexpr float BLUE_SPAWN_Y = 0.0f;
//리스폰 위치에서 약간 떨어진 위치로 리스폰 (플레이어 겹침 방지)
constexpr float RESPAWN_OFFSET = 120.0f;

struct PlayerState {
  int id = -1;

  TeamType team = TeamType::TEAM_NONE;
  float respawn_timer = 0.0f;

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

  int kill_count = 0;
  int death_count = 0;

  float moveX = 0.0f;
  float moveY = 0.0f;

  float faceX = 1.0f;
  float faceY = 0.0f;

  bool auto_attack = false;
  bool skill_requested = false;

  CharacterType character = CharacterType::CHAR_NONE;
  AttackType attack_type = AttackType::NONE;
  SkillType active_skill = SkillType::NONE;
  PassiveType passive_skill = PassiveType::NONE;

	float attack_timer = 0.0f;       // 기본 공격 쿨타임 진행 상태
	float skill_timer = 0.0f;        // 스킬 쿨타임 진행 상태

	int current_target_id = -1; // 클라 표시용
	int last_attack_target = -1;     // 마지막으로 공격한 대상
	float last_damaged_time = 0.0f;  // 마지막으로 피해받은 시간

  bool lobby_ready = false;

  bool moving = false;
  float move_input_timer = 0.0f;
};

enum class RoomState { MATCHING, LOBBY, INGAME, ENDED };

struct ItemState {
  int id;
  float x;
  float y;
  bool active;
  float respawn_timer;
};

struct Room {
	int room_id = -1; // room 번호 : 1, 2, 3 ...? 
	RoomState state = RoomState::MATCHING;
	bool active = false;
	ScoreManager score;
	ItemState items[2];

  std::array<int, MAX_ROOM_PLAYERS> players;
  std::array<PlayerState, MAX_ROOM_PLAYERS> states;
  int player_count = 0;

  Room() { players.fill(-1); }

  bool add_player(int player_id) {
    if (player_count >= MAX_ROOM_PLAYERS)
      return false;
    for (int i = 0; i < player_count; ++i) {
      if (players[i] == player_id)
        return false;
    }

    players[player_count] = player_id;

    states[player_count].id = player_id;
    states[player_count].hp = 100;
    states[player_count].alive = true;

    ++player_count;
    return true;
  }

  void remove_player(int player_id) {
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

  bool is_full() const { return player_count >= MAX_ROOM_PLAYERS; }

  bool all_ready() const {
    if (player_count < MAX_ROOM_PLAYERS)
      return false;

    for (int i = 0; i < player_count; ++i) {
      if (states[i].character == CharacterType::CHAR_NONE)
        return false;
      if (!states[i].lobby_ready)
        return false;
    }

    return true;
  }
};

class RoomManager {
public:
  static RoomManager &Instance() {
    static RoomManager instance;
    return instance;
  }

  void request_matchmaking(int player_id);
  bool join_room(int room_id, int player_id);
  void leave_room(int player_id);
  void start_room(int room_id);

	void broadcast_room(int room_id, char* packet, int size);
	void update_all_rooms(float dt);

  void select_character(int player_id, CharacterType character);
  void set_lobby_ready(int player_id, bool ready);

  void set_move_input(int player_id, float axisX, float axisY);
  void request_attack(int player_id);
  void request_skill(int player_id);

  void set_face_dir(int player_id, float faceX, float faceY);

private:
  RoomManager() = default;

  RoomManager(const RoomManager &) = delete;
  RoomManager &operator=(const RoomManager &) = delete;

  void process_pending_requests();

	void update_room(Room& room, float dt);

	void update_ai(Room& room, float dt);
	void update_movement(Room& room, float dt);
	void update_skills(Room& room, float dt);
	void update_attacks(Room& room, float dt);
	void update_items(Room& room, float dt);
	void update_respawns(Room& room, float dt);

	void check_collisions(Room& room);
	void send_room_snapshot(Room& room); //플렝이어 위치, 체력, 스킬 상태 등등 보내기 (여기서 맞나?)


  void broadcast_item_state(Room &room, int item_id, bool active);

  bool is_same_team(const PlayerState &a, const PlayerState &b) const;
  void enter_lobby(int room_id);
  void assign_teams(Room &room);
  PlayerState *find_player_state(Room &room, int player_id);

	void kill_player(Room& room, PlayerState& target, int killer_id);
	void respawn_player(Room& room, PlayerState& player);
	void set_spawn_position(Room& room, PlayerState& player); //리스폰 위치 지정
	
	void apply_character_to_player(PlayerState& state, CharacterType character);

  void end_room(Room &room);
  void init_items(Room &room);

  std::unordered_map<int, Room> m_rooms;
  std::unordered_map<int, int> m_player_room;

  int m_next_room_id = 1;

	std::mutex m_request_mutex;
	std::vector<int> m_match_requests; // 매칭요청대기업
	std::vector<int> m_leave_requests; // 퇴장 요청 대기업
};
