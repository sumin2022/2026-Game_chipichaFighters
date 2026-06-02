#include "RoomManager.h"
#include "SESSION.h"
#include <algorithm>
#include <cmath>

void RoomManager::broadcast_room(int room_id, char* packet, int size)
{
	auto room_it = m_rooms.find(room_id);
	if (room_it == m_rooms.end()) return;

	Room& room = room_it->second;

	for (int i = 0; i < room.player_count; ++i) {
		int player_id = room.players[i];
		if (player_id == -1) continue;

		SESSION& session = clients[player_id];

		if (session.m_is_connected) {
			session.do_send(size, packet);
		}
	}
}

void RoomManager::update_all_rooms()
{
	process_pending_requests();

	for (auto& [id, room] : m_rooms) {
		update_room(room);
	}
}

void RoomManager::update_room(Room& room)
{
	if (!room.active) return;

	update_ai(room);
	update_movement(room);
	update_skills(room);
	update_attacks(room);
	check_collisions(room);
	send_room_snapshot(room);

}

void RoomManager::request_matchmaking(int player_id) //그냥 요청만
{
	{
		std::lock_guard<std::mutex> lock(m_request_mutex);
		m_match_requests.push_back(player_id);
	}

	std::cout << "Matchmaking requested by player " << player_id << '\n';
}

void RoomManager::leave_room(int player_id) //그냥 요청만	, 실제로 방에서 나가는 건 update_room에서 처리
{
	{
		std::lock_guard<std::mutex> lock(m_request_mutex);
		m_leave_requests.push_back(player_id);
	}

	std::cout << "Leave room requested by player " << player_id << '\n';
}

void RoomManager::process_pending_requests()
{
	std::vector<int> match_requests;
	std::vector<int> leave_requests;

	{
		std::lock_guard<std::mutex> lock(m_request_mutex);

		match_requests.swap(m_match_requests);
		leave_requests.swap(m_leave_requests);
	}

	for (int player_id : leave_requests) {
		auto player_room_it = m_player_room.find(player_id);
		if (player_room_it == m_player_room.end()) continue;

		int room_id = player_room_it->second;

		auto room_it = m_rooms.find(room_id);
		if (room_it == m_rooms.end()) {
			m_player_room.erase(player_id);
			continue;
		}

		Room& room = room_it->second;

		std::cout << "Player " << player_id << " left room " << room_id << '\n';

		// 게임 중인 방에서 누가 나가면 이 방은 재사용하지 않고 제거
		if (room.active) {
			for (int i = 0; i < room.player_count; ++i) {
				int pid = room.players[i];
				if (pid == -1) continue;

				clients[pid].m_in_game = false;
				clients[pid].m_room_id = -1;

				if (pid != player_id && clients[pid].m_is_connected) {
					clients[pid].send_game_result();
				}

				m_player_room.erase(pid);
			}

			m_rooms.erase(room_id);

			std::cout << "Active room " << room_id << " removed\n";
			continue;
		}

		// 아직 매칭 중인 방이면 해당 플레이어만 제거
		room.remove_player(player_id);
		m_player_room.erase(player_id);

		clients[player_id].m_in_game = false;
		clients[player_id].m_room_id = -1;

		if (room.player_count == 0) {
			m_rooms.erase(room_id);
			std::cout << "Empty room " << room_id << " removed\n";
		}
	}

	for (int player_id : match_requests) {
		if (!clients[player_id].m_is_connected) continue;
		if (!clients[player_id].m_is_logged_in) continue;
		if (clients[player_id].m_in_game) continue;

		if (m_player_room.find(player_id) != m_player_room.end()) {
			continue;
		}

		int target_room_id = -1;

		for (auto& [room_id, room] : m_rooms) {
			if (!room.active && !room.is_full()) {
				target_room_id = room_id;
				break;
			}
		}

		if (target_room_id == -1) {
			target_room_id = m_next_room_id++;

			Room new_room;
			new_room.room_id = target_room_id;

			m_rooms.emplace(target_room_id, new_room);

			std::cout << "Room created: " << target_room_id << '\n';
		}

		join_room(target_room_id, player_id);

		auto room_it = m_rooms.find(target_room_id);
		if (room_it != m_rooms.end()) {
			if (room_it->second.is_full()) {
				enter_lobby(target_room_id);
			}
		}
	}
}

bool RoomManager::join_room(int room_id, int player_id)
{
	auto room_it = m_rooms.find(room_id);
	if (room_it == m_rooms.end()) return false;

	Room& room = room_it->second;

	if (room.active) return false;
	if (room.is_full()) return false;

	if (!room.add_player(player_id)) return false;

	m_player_room[player_id] = room_id;

	clients[player_id].m_room_id = room_id;
	clients[player_id].m_in_game = false;

	std::cout << "Player " << player_id
		<< " joined room " << room_id
		<< " (" << room.player_count << "/" << MAX_ROOM_PLAYERS << ")\n";

	return true;
}

void RoomManager::start_room(int room_id)
{
	auto room_it = m_rooms.find(room_id);
	if (room_it == m_rooms.end()) return;

	Room& room = room_it->second;

	if (room.state != RoomState::LOBBY) return;
	if (!room.all_ready()) return;

	room.state = RoomState::INGAME;
	room.active = true;

	for (int i = 0; i < room.player_count; ++i) {
		int player_id = room.players[i];
		if (player_id == -1) continue;

		clients[player_id].m_in_game = true;
		clients[player_id].m_room_id = room_id;

		clients[player_id].send_game_start();
		std::cout << "Room " << room_id << " started\n";
	}

	std::cout << "Room " << room_id << " started\n";
}

void RoomManager::enter_lobby(int room_id)
{
	auto room_it = m_rooms.find(room_id);
	if (room_it == m_rooms.end()) return;

	Room& room = room_it->second;

	if (room.state != RoomState::MATCHING) return;
	if (!room.is_full()) return;

	room.state = RoomState::LOBBY;

	SC_RoomEnter packet;
	packet.size = sizeof(SC_RoomEnter);
	packet.type = SC_ROOM_ENTER;
	packet.room_id = room_id;
	packet.player_count = room.player_count;

	broadcast_room(room_id, reinterpret_cast<char*>(&packet), packet.size);

	std::cout << "Room " << room_id << " entered lobby\n";
}

void RoomManager::select_character(int player_id, CharacterType character)
{
	if (character < CHAR_DEALER || character > CHAR_HEALER) return;

	auto it = m_player_room.find(player_id);
	if (it == m_player_room.end()) return;

	int room_id = it->second;

	auto room_it = m_rooms.find(room_id);
	if (room_it == m_rooms.end()) return;

	Room& room = room_it->second;

	if (room.state != RoomState::LOBBY) return;

	PlayerState* state = find_player_state(room, player_id);
	if (state == nullptr) return;

	apply_character_to_player(*state, character);

	SC_CharacterSelected packet;
	packet.size = sizeof(SC_CharacterSelected);
	packet.type = SC_CHARACTER_SELECTED;
	packet.player_id = player_id;
	packet.character = character;

	broadcast_room(room_id, reinterpret_cast<char*>(&packet), packet.size);
}

void RoomManager::set_lobby_ready(int player_id, bool ready)
{
	auto it = m_player_room.find(player_id);
	if (it == m_player_room.end()) return;

	int room_id = it->second;

	auto room_it = m_rooms.find(room_id);
	if (room_it == m_rooms.end()) return;

	Room& room = room_it->second;

	if (room.state != RoomState::LOBBY) return;

	PlayerState* state = find_player_state(room, player_id);
	if (state == nullptr) return;

	if (state->character == CHAR_NONE) return;

	state->lobby_ready = ready;

	SC_LobbyReadyState packet;
	packet.size = sizeof(SC_LobbyReadyState);
	packet.type = SC_LOBBY_READY_STATE;
	packet.player_id = player_id;
	packet.ready = ready;

	broadcast_room(room_id, reinterpret_cast<char*>(&packet), packet.size);

	if (room.all_ready()) {
		start_room(room_id);
	}
}

PlayerState* RoomManager::find_player_state(Room& room, int player_id)
{
	for (int i = 0; i < room.player_count; ++i) {
		if (room.states[i].id == player_id) {
			return &room.states[i];
		}
	}

	return nullptr;
}

void RoomManager::apply_character_to_player(PlayerState& state, CharacterType character)
{
	const CharacterStats& stats = CharacterManager::GetStats(character);

	state.character = character;

	state.max_hp = stats.max_hp;
	state.hp = stats.max_hp;

	state.max_mp = stats.max_mp;
	state.mp = stats.max_mp;

	state.attack_damage = stats.attack_damage;
	state.attack_cooldown = stats.attack_cooldown;
	state.attack_range = stats.attack_range;
	state.move_speed = stats.move_speed;

	state.attack_type = stats.attack_type;
	state.active_skill = stats.skill.type;
	state.passive_skill = stats.passive.type;

	state.attack_timer = 0.0f;
	state.skill_timer = 0.0f;

	state.last_attack_target = -1;
	state.last_damaged_time = 0.0f;

	state.alive = true;
	state.lobby_ready = false;
}

void RoomManager::update_movement(Room& room)
{
	constexpr float DT = 0.016f; // GameThread 16ms 기준

	for (int i = 0; i < room.player_count; ++i) {
		PlayerState& p = room.states[i];

		if (!p.alive) continue;

		float dx = p.moveX;
		float dy = p.moveY;

		float len = sqrtf(dx * dx + dy * dy);
		if (len > 0.0f) {
			dx /= len;
			dy /= len;

			p.x += dx * p.move_speed * DT;
			p.y += dy * p.move_speed * DT;

			p.faceX = dx;
			p.faceY = dy;
		}

		p.x = std::clamp(p.x, 0.0f, static_cast<float>(WORLD_WIDTH - 1));
		p.y = std::clamp(p.y, 0.0f, static_cast<float>(WORLD_HEIGHT - 1));

		clients[p.id].m_x = static_cast<short>(p.x);
		clients[p.id].m_y = static_cast<short>(p.y);
	}
}

void RoomManager::update_attacks(Room& room)
{
	constexpr float DT = 0.016f;

	for (int i = 0; i < room.player_count; ++i) {
		PlayerState& attacker = room.states[i];

		if (!attacker.alive) continue;
		if (!attacker.auto_attack) continue;

		attacker.attack_timer = (std::max)(0.0f, attacker.attack_timer - DT);
		if (attacker.attack_timer > 0.0f) continue;

		PlayerState* target = nullptr;
		float bestDistSq = attacker.attack_range * attacker.attack_range;

		for (int j = 0; j < room.player_count; ++j) {
			if (i == j) continue;

			PlayerState& enemy = room.states[j];
			if (!enemy.alive) continue;

			float dx = enemy.x - attacker.x;
			float dy = enemy.y - attacker.y;
			float distSq = dx * dx + dy * dy;

			if (distSq > bestDistSq) continue;

			float len = sqrtf(distSq);
			if (len <= 0.0f) continue;

			float dirX = dx / len;
			float dirY = dy / len;

			float dot = dirX * attacker.faceX + dirY * attacker.faceY;

			// 0.7 정도면 전방 약 90도 안쪽
			if (dot < 0.7f) continue;

			bestDistSq = distSq;
			target = &enemy;
		}

		if (target == nullptr) continue;

		attacker.last_attack_target = target->id;

		target->hp -= attacker.attack_damage;
		attacker.attack_timer = attacker.attack_cooldown;

		std::cout << "[ATTACK] " << attacker.id
			<< " -> " << target->id
			<< " damage=" << attacker.attack_damage
			<< " hp=" << target->hp << "\n";

		if (target->hp <= 0) {
			target->hp = 0;
			target->alive = false;

			for (int k = 0; k < room.player_count; ++k) {
				clients[room.players[k]].send_death(target->id);
			}
		}
	}
}

void RoomManager::request_attack(int player_id, float aimX, float aimY) 
{
	auto it = m_player_room.find(player_id);
	if (it == m_player_room.end()) return;

	Room& room = m_rooms[it->second];
	PlayerState* state = find_player_state(room, player_id);
	if (state == nullptr) return;

	float len = sqrtf(aimX * aimX + aimY * aimY);
	if (len > 0.0f) {
		state->faceX = aimX / len;
		state->faceY = aimY / len;
	}

	state->auto_attack = true;
}

void RoomManager::set_move_input(int player_id, float axisX, float axisY)
{
	auto it = m_player_room.find(player_id);
	if (it == m_player_room.end()) return;

	int room_id = it->second;

	auto room_it = m_rooms.find(room_id);
	if (room_it == m_rooms.end()) return;

	Room& room = room_it->second;

	for (int i = 0; i < room.player_count; ++i) {
		if (room.players[i] == player_id) {
			PlayerState& state = room.states[i];

			state.moveX = axisX;
			state.moveY = axisY;

			break;
		}
	}
}

void RoomManager::request_skill(int player_id, float aimX, float aimY)
{
	auto it = m_player_room.find(player_id);
	if (it == m_player_room.end()) return;

	int room_id = it->second;

	auto room_it = m_rooms.find(room_id);
	if (room_it == m_rooms.end()) return;

	Room& room = room_it->second;

	for (int i = 0; i < room.player_count; ++i) {
		if (room.players[i] == player_id) {
			PlayerState& state = room.states[i];

			state.skill_requested = true;
			state.skillAimX = aimX;
			state.skillAimY = aimY;

			return;
		}
	}
}

void RoomManager::update_skills(Room& room)
{
	constexpr float DT = 0.016f;

	for (int i = 0; i < room.player_count; ++i) {
		PlayerState& caster = room.states[i];

		if (!caster.alive) continue;

		caster.skill_timer = (std::max)(0.0f, caster.skill_timer - DT);

		if (!caster.skill_requested) continue;

		caster.skill_requested = false;

		if (caster.skill_timer > 0.0f) continue;
		if (caster.active_skill == SkillType::NONE) continue;

		switch (caster.active_skill) {
		case SkillType::DEALER_SKILL:
			// 자기 주변 원형 범위 공격
			break;

		case SkillType::ARCHER_SKILL:
			// 방향으로 화살 판정
			break;

		case SkillType::TANKER_SKILL:
			// 타겟팅 된 상대에게 투척 판정
			break;

		case SkillType::HEALER_SKILL:
			// 방향 사거리 위치에 장판 생성 후 범위 힐
			break;

		default:
			break;
		}

		caster.skill_timer = 5.0f; // 임시 쿨타임
	}
}

void RoomManager::update_ai(Room& room) {}
void RoomManager::check_collisions(Room& room) {}
void RoomManager::send_room_snapshot(Room& room) {}