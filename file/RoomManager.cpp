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

	constexpr float DT = 0.016f;

	room.score.update(room, DT);

	if (room.score.is_time_over()) {
		end_room(room);
		return;
	}

	update_ai(room);
	update_respawns(room);
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

	assign_teams(room);

	room.state = RoomState::LOBBY;

	SC_RoomEnter packet;
	packet.size = sizeof(SC_RoomEnter);
	packet.type = SC_ROOM_ENTER;
	packet.room_id = room_id;
	packet.player_count = room.player_count;

	for (int i = 0; i < room.player_count; ++i) {
		packet.player_ids[i] = room.players[i];
		packet.teams[i] = static_cast<int>(room.states[i].team);
	}

	broadcast_room(room_id, reinterpret_cast<char*>(&packet), packet.size);

	std::cout << "Room " << room_id << " entered lobby\n";
}

void RoomManager::assign_teams(Room& room)
{
	for (int i = 0; i < room.player_count; ++i) {
		if (i < room.player_count / 2)
			room.states[i].team = TEAM_RED;
		else
			room.states[i].team = TEAM_BLUE;
	}
}

bool RoomManager::is_same_team(const PlayerState& a,
	const PlayerState& b) const
{
	return a.team == b.team;
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

			//p.faceX = dx;
			//p.faceY = dy;
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

		if (!attacker.alive) {
			attacker.current_target_id = -1;
			continue;
		}

		if (!attacker.auto_attack) {
			attacker.current_target_id = -1;
			continue;
		}

		attacker.attack_timer = (std::max)(0.0f, attacker.attack_timer - DT);

		PlayerState* target = nullptr;
		float bestDistSq = attacker.attack_range * attacker.attack_range;

		for (int j = 0; j < room.player_count; ++j) {
			if (i == j) continue;

			PlayerState& enemy = room.states[j];
			if (!enemy.alive) continue;

			if (is_same_team(attacker, enemy))
				continue;

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

		attacker.current_target_id = target ? target->id : -1;

		if (target == nullptr) continue;
		if (attacker.attack_timer > 0.0f) continue;

		attacker.last_attack_target = target->id;

		target->hp -= attacker.attack_damage;
		attacker.attack_timer = attacker.attack_cooldown;

		std::cout << "[ATTACK] " << attacker.id
			<< " -> " << target->id
			<< " damage=" << attacker.attack_damage
			<< " hp=" << target->hp << "\n";

		if (target->hp <= 0) {
			kill_player(room, *target, attacker.id);
		}
	}
}

void RoomManager::request_attack(int player_id) 
{
	auto it = m_player_room.find(player_id);
	if (it == m_player_room.end()) return;

	Room& room = m_rooms[it->second];
	PlayerState* state = find_player_state(room, player_id);
	if (state == nullptr) return;

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

void RoomManager::request_skill(int player_id)
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
			return;
		}
	}
}

void RoomManager::set_face_dir(int player_id, float faceX, float faceY)
{
	auto it = m_player_room.find(player_id);
	if (it == m_player_room.end()) return;

	auto room_it = m_rooms.find(it->second);
	if (room_it == m_rooms.end()) return;

	Room& room = room_it->second;

	PlayerState* state = find_player_state(room, player_id);
	if (state == nullptr) return;
	if (!state->alive) return;

	float len = sqrtf(faceX * faceX + faceY * faceY);
	if (len <= 0.0f) return;

	state->faceX = faceX / len;
	state->faceY = faceY / len;
}

//스킬 판정시 거리 계산용
static float dist_sq(float x1, float y1, float x2, float y2)
{
	float dx = x1 - x2;
	float dy = y1 - y2;
	return dx * dx + dy * dy;
}
//스킬 데미지 적용
static void apply_damage(PlayerState& target, int damage)
{
	if (!target.alive) return;

	target.hp -= damage;

	if (target.hp <= 0) {
		target.hp = 0;
	}
}
//스킬 힐 적용
static void apply_heal(PlayerState& target, int heal)
{
	if (!target.alive) return;

	target.hp += heal;
	if (target.hp > target.max_hp)
		target.hp = target.max_hp;
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

		const CharacterStats& stats = CharacterManager::GetStats(caster.character);
		const SkillStats& skill = stats.skill;

		if (caster.mp < skill.mana_cost) continue;

		bool skill_casted = false;

		switch (caster.active_skill) {
		case SkillType::DEALER_SKILL:
		{
			// 자기 주변 원형 범위 공격
			// skill.damage, skill.dealer_area_range 사용
			float range_sq = skill.dealer_area_range * skill.dealer_area_range;

			for (int j = 0; j < room.player_count; ++j) {
				if (i == j) continue;

				PlayerState& target = room.states[j];
				if (!target.alive) continue;

				if (is_same_team(caster, target))
					continue;

				if (dist_sq(caster.x, caster.y, target.x, target.y) <= range_sq) {
					apply_damage(target, skill.damage);
					if (target.hp <= 0) {
						kill_player(room, target, caster.id);
					}
				}
			}

			skill_casted = true;
			break;
		}
		case SkillType::ARCHER_SKILL:
		{
			// 방향으로 화살 판정
			// skill.penetration_damage, skill.range 사용
			// 방향으로 관통 화살 판정
			float dirX = caster.faceX;
			float dirY = caster.faceY;

			float len = sqrtf(dirX * dirX + dirY * dirY);
			if (len <= 0.0f) {
				dirX = caster.faceX;
				dirY = caster.faceY;
			}
			else {
				dirX /= len;
				dirY /= len;
			}

			float current_damage = skill.penetration_damage;
			float hit_width = 50.0f; // 화살 판정 폭, 임시값
			float hit_width_sq = hit_width * hit_width;

			for (int j = 0; j < room.player_count; ++j) {
				if (i == j) continue;

				PlayerState& target = room.states[j];
				if (!target.alive) continue;

				if (is_same_team(caster, target))
					continue;

				float vx = target.x - caster.x;
				float vy = target.y - caster.y;

				float forward_dist = vx * dirX + vy * dirY;

				if (forward_dist < 0.0f) continue;
				if (forward_dist > skill.range) continue;

				float closestX = caster.x + dirX * forward_dist;
				float closestY = caster.y + dirY * forward_dist;

				if (dist_sq(target.x, target.y, closestX, closestY) <= hit_width_sq) {
					apply_damage(target, static_cast<int>(current_damage));
					if (target.hp <= 0) {
						kill_player(room, target, caster.id);
					}

					current_damage *= (1.0f - skill.damage_reduce_per_hit);
					if (current_damage <= 0.0f)
						break;
				}
			}

			skill_casted = true;
			break;
		}
		case SkillType::TANKER_SKILL:
		{
			// 타겟팅 된 상대에게 투척 판정
			// skill.damage, skill.extra_damage, skill.range 사용
			// 기본 공격처럼 가장 가까운 상대 타겟팅
			PlayerState* target = nullptr;
			float best_dist_sq = skill.range * skill.range;

			for (int j = 0; j < room.player_count; ++j) {
				if (i == j) continue;

				PlayerState& enemy = room.states[j];
				if (!enemy.alive) continue;

				if (is_same_team(caster, enemy))
					continue;

				float d = dist_sq(caster.x, caster.y, enemy.x, enemy.y);

				if (d <= best_dist_sq) {
					best_dist_sq = d;
					target = &enemy;
				}
			}

			if (target != nullptr) {
				apply_damage(*target, skill.damage + skill.extra_damage);

				if (target->hp <= 0) {
					kill_player(room, *target, caster.id);
				}
				// stun_duration은 현재 PlayerState에 stun_timer 같은 게 없으면 아직 적용 불가
				// target->stun_timer = skill.stun_duration;

				skill_casted = true;
			}

			break;
		}
		case SkillType::HEALER_SKILL:
		{
			// 방향 사거리 위치에 장판 생성 후 범위 힐
			// skill.heal, skill.range, skill.heal_area_range 사용
			// 방향 사거리 위치에 장판 생성 후 범위 힐
			float dirX = caster.faceX;
			float dirY = caster.faceY;

			float len = sqrtf(dirX * dirX + dirY * dirY);
			if (len <= 0.0f) {
				dirX = caster.faceX;
				dirY = caster.faceY;
			}
			else {
				dirX /= len;
				dirY /= len;
			}

			float areaX = caster.x + dirX * skill.range;
			float areaY = caster.y + dirY * skill.range;

			float area_range_sq = skill.heal_area_range * skill.heal_area_range;

			for (int j = 0; j < room.player_count; ++j) {
				PlayerState& target = room.states[j];
				if (!target.alive) continue;

				if (dist_sq(areaX, areaY, target.x, target.y) > area_range_sq)
					continue;

				if (is_same_team(caster, target)) {
					apply_heal(target, skill.heal);
				}
				else {
					apply_damage(target, skill.damage);

					if (target.hp <= 0) {
						kill_player(room, target, caster.id);
					}
				}
			}

			skill_casted = true;
			break;
		}
		default:
			break;
		}
		if (skill_casted) {
			caster.mp -= static_cast<int>(skill.mana_cost);
			caster.skill_timer = skill.cooldown;
		}
	}
}

void RoomManager::kill_player(Room& room, PlayerState& target, int killer_id)
{
	if (!target.alive) return;

	target.hp = 0;
	target.alive = false;
	target.auto_attack = false;
	target.skill_requested = false;
	target.respawn_timer = 5.0f; // 임시 리스폰 시간
	target.death_count++;

	PlayerState* killer = find_player_state(room, killer_id);
	if (killer != nullptr && killer->id != target.id) {
		killer->kill_count++;
	}

	for (int i = 0; i < room.player_count; ++i) {
		clients[room.players[i]].send_death(target.id, killer_id);
	}

	std::cout << "[DEATH] player=" << target.id << "\n";
}

void RoomManager::update_respawns(Room& room)
{
	constexpr float DT = 0.016f;

	for (int i = 0; i < room.player_count; ++i) {
		PlayerState& player = room.states[i];

		if (player.alive)
			continue;

		player.respawn_timer -= DT;

		if (player.respawn_timer <= 0.0f) {
			respawn_player(room, player);
		}
	}
}

void RoomManager::respawn_player(Room& room, PlayerState& player)
{
	player.alive = true;
	player.hp = player.max_hp;

	player.respawn_timer = 0.0f;

	player.moveX = 0.0f;
	player.moveY = 0.0f;

	player.auto_attack = false;
	player.skill_requested = false;

	if (player.team == TEAM_RED) //현재는 임시 리스폰 위치 값
	{
		player.x = 50;
		player.y = 200;
	}
	else
	{
		player.x = 350;
		player.y = 200;
	}

	for (int i = 0; i < room.player_count; ++i) {
		clients[room.players[i]].send_respawn(
			player.id,
			player.x,
			player.y,
			player.hp
		);
	}

	std::cout << "[RESPAWN] player=" << player.id << "\n";
}

void RoomManager::send_room_snapshot(Room& room) 
{
	SC_RoomSnapshot packet;
	packet.size = sizeof(SC_RoomSnapshot);
	packet.type = SC_ROOM_SNAPSHOT;
	packet.count = room.player_count;

	for (int i = 0; i < room.player_count; ++i) {
		PlayerState& p = room.states[i];

		packet.players[i].player_id = p.id;
		packet.players[i].x = p.x;
		packet.players[i].y = p.y;
		packet.players[i].faceX = p.faceX;
		packet.players[i].faceY = p.faceY;
		packet.players[i].current_target_id = p.current_target_id;
		packet.players[i].hp = p.hp;
		packet.players[i].max_hp = p.max_hp;
		packet.players[i].alive = p.alive;
		packet.players[i].character = p.character;
		packet.red_score = room.score.get_red_score();
		packet.blue_score = room.score.get_blue_score();
		packet.time_left = room.score.get_time_left();
		//킬뎃 실시간 적용?
		packet.players[i].kill_count = p.kill_count;
		packet.players[i].death_count = p.death_count;
	}

	broadcast_room(room.room_id, reinterpret_cast<char*>(&packet), packet.size);
}

void RoomManager::end_room(Room& room)
{
	if (room.state == RoomState::ENDED) return;

	room.state = RoomState::ENDED;
	room.active = false;

	SC_GameResult packet;
	room.score.make_result(room, packet);

	broadcast_room(room.room_id, reinterpret_cast<char*>(&packet), packet.size);

	for (int i = 0; i < room.player_count; ++i) {
		int pid = room.players[i];
		if (pid == -1) continue;

		clients[pid].m_in_game = false;
		clients[pid].m_room_id = -1;
		m_player_room.erase(pid);
	}

	m_rooms.erase(room.room_id);
}

void RoomManager::update_ai(Room& room) {}
void RoomManager::check_collisions(Room& room) {}
