#include "RoomManager.h"
#include "SESSION.h"

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
	update_skills(room);
	check_player_attacks(room);
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
				start_room(target_room_id);
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

	if (room.active) return;
	if (room.player_count < MAX_ROOM_PLAYERS) return;

	room.active = true;

	for (int i = 0; i < room.player_count; ++i) {
		int player_id = room.players[i];
		if (player_id == -1) continue;

		clients[player_id].m_in_game = true;
		clients[player_id].m_room_id = room_id;

		clients[player_id].send_game_start();
	}

	std::cout << "Room " << room_id << " started\n";
}



void RoomManager::update_ai(Room& room) {}
void RoomManager::update_skills(Room& room) {}
void RoomManager::check_player_attacks(Room& room) {}
void RoomManager::check_collisions(Room& room) {}
void RoomManager::send_room_snapshot(Room& room) {}