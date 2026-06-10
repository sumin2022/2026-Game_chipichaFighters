#include "SESSION.h"
#include <algorithm>
#include "RoomManager.h"

std::array<SESSION, MAX_PLAYERS> clients;

SESSION::SESSION()
{
	m_is_connected = false;
	m_id = 999;
	m_client = INVALID_SOCKET;
	m_recv_over.m_iotype = IO_RECV;
	m_x = 0;
	m_y = 0;
	m_prev_recv = 0;
	memset(m_username, 0, sizeof(m_username));
}

SESSION::~SESSION()
{
	if (m_is_connected)
		closesocket(m_client);
}

void SESSION::do_recv()
{
	DWORD recv_flag = 0;
	memset(&m_recv_over.m_over, 0, sizeof(m_recv_over.m_over));
	m_recv_over.m_wsa.buf = m_recv_over.m_buff + m_prev_recv;
	m_recv_over.m_wsa.len = sizeof(m_recv_over.m_buff) - m_prev_recv;

	WSARecv(m_client, &m_recv_over.m_wsa, 1, 0, &recv_flag, &m_recv_over.m_over, nullptr);
}

void SESSION::do_send(int num_bytes, char* mess)
{
	EXP_OVER* o = new EXP_OVER(IO_SEND);
	o->m_wsa.len = num_bytes;
	memcpy(o->m_buff, mess, num_bytes);
	WSASend(m_client, &o->m_wsa, 1, 0, 0, &o->m_over, nullptr);
}

void SESSION::send_avatar_info()
{
	SC_AvatarInfo packet;
	packet.size = sizeof(SC_AvatarInfo);
	packet.type = SC_AVATAR_INFO;
	packet.playerId = m_id;
	packet.x = m_x;
	packet.y = m_y;
	do_send(packet.size, reinterpret_cast<char*>(&packet));
}

void SESSION::send_add_player(int player_id)
{
	SC_AddPlayer packet;
	packet.size = sizeof(SC_AddPlayer);
	packet.type = SC_ADD_PLAYER;
	packet.playerId = player_id;

	SESSION& pl = clients[player_id];
	memcpy(packet.username, pl.m_username, sizeof(packet.username));
	packet.x = pl.m_x;
	packet.y = pl.m_y;

	do_send(packet.size, reinterpret_cast<char*>(&packet));
}

void SESSION::send_login_success()
{
	SC_LoginResult packet;
	packet.size = sizeof(SC_LoginResult);
	packet.type = SC_LOGIN_RESULT;
	packet.success = true;
	strncpy_s(packet.message, "Login successful.", sizeof(packet.message));
	do_send(packet.size, reinterpret_cast<char*>(&packet));
}

void SESSION::send_remove_player(int player_id)
{
	SC_RemovePlayer packet;
	packet.size = sizeof(SC_RemovePlayer);
	packet.type = SC_REMOVE_PLAYER;
	packet.playerid = player_id;
	do_send(packet.size, reinterpret_cast<char*>(&packet));
}

void SESSION::send_move_packet(int mover)
{
	SC_MovePlayer packet;
	packet.size = sizeof(SC_MovePlayer);
	packet.type = SC_MOVE_PLAYER;
	packet.playerId = mover;
	packet.x = clients[mover].m_x;
	packet.y = clients[mover].m_y;
	do_send(packet.size, reinterpret_cast<char*>(&packet));
}

void SESSION::process_packet(unsigned char* p)
{
	PACKET_TYPE type = *reinterpret_cast<PACKET_TYPE*>(&p[1]);

	switch (type) {
	case CS_LOGIN:
	{
		// DB 확인
		// 성공하면 m_is_logged_in = true;
		// m_username 저장
		// SC_LOGIN_RESULT 전송
		CS_Login* packet = reinterpret_cast<CS_Login*>(p);
		strncpy_s(m_username, packet->username, MAX_NAME_LEN);

		m_is_logged_in = true;

		std::cout << "Player[" << m_id << "] logged in as " << m_username << std::endl;

		send_login_success();
		send_avatar_info();

		//for (auto& other : clients) {
		//	if (!other.m_is_connected) continue;
		//	if (other.m_id == m_id) continue;
		//	send_add_player(other.m_id);
		//}

		//for (auto& other : clients) {
		//	if (!other.m_is_connected) continue;
		//	if (other.m_id == m_id) continue;
		//	other.send_add_player(m_id);
		//}
		break;
	}
	case CS_READY:
	{
		if (!m_is_logged_in) return;

		std::cout << "[RECV] CS_READY / player=" << m_id
			<< " matchmaking request\n";

		// RoomManager에 매칭 요청
		RoomManager::Instance().request_matchmaking(m_id);

		break;
	}
	case CS_SELECT_CHARACTER:
	{
		CS_SelectCharacter* packet = reinterpret_cast<CS_SelectCharacter*>(p);

		std::cout << "[RECV] CS_SELECT_CHARACTER / player=" << m_id
			<< " character=" << (int)packet->character << "\n";

		RoomManager::Instance().select_character(m_id, packet->character);

		break;
	}

	case CS_GAME_READY:
	{
		CS_GameReady* packet = reinterpret_cast<CS_GameReady*>(p);

		std::cout << "[RECV] CS_GAME_READY / player=" << m_id
			<< " ready=" << packet->ready << "\n";


		RoomManager::Instance().set_lobby_ready(m_id, packet->ready);

		break;
	}
	case CS_MOVE:
	{
		if (!m_in_game) return;

		CS_Move* packet = reinterpret_cast<CS_Move*>(p);
		RoomManager::Instance().set_move_input(m_id, packet->axisX, packet->axisY);
		break;

		//std::cout << "Player[" << m_id << "] moved to (" << m_x << ", " << m_y << ")\n";

		//for (auto& cl : clients) {
		//	if (cl.m_is_connected)
		//		cl.send_move_packet(m_id);
		//}
		// 나중에 방에	있는 플레이어한테만 보내도록 수정하기
		break;
	}
	case CS_FACE_DIR:
	{
		if (!m_in_game) return;

		CS_FaceDir* packet = reinterpret_cast<CS_FaceDir*>(p);

		RoomManager::Instance().set_face_dir(
			m_id,
			packet->faceX,
			packet->faceY
		);

		break;
	}
	case CS_ATTACK:
	{
		if (!m_in_game) return;
		// 공격 요청 저장
		CS_Attack* packet = reinterpret_cast<CS_Attack*>(p);

		RoomManager::Instance().request_attack(m_id);

		//std::cout << "Player[" << m_id << "] attack dir: "
		//	<< aimX << ", " << aimY << "\n";
		break;
	}
	case CS_SKILL:
	{
		if (!m_in_game) return;
		// 스킬 요청 저장
		CS_Skill* packet = reinterpret_cast<CS_Skill*>(p);

		RoomManager::Instance().request_skill(m_id);

		//std::cout << "Player[" << m_id << "] skill[" << skillId << "] dir: "
		//	<< aimX << ", " << aimY << "\n";

		break;
	}
	default:
		std::cout << "Unknown packet type received from player[" << m_id << "].\n";
		break;
	}
}

void send_login_fail(SOCKET client, const char* message)
{
	SC_LoginResult packet;
	packet.size = sizeof(SC_LoginResult);
	packet.type = SC_LOGIN_RESULT;
	packet.success = false;
	strncpy_s(packet.message, message, sizeof(packet.message));

	WSABUF wsa_buf;
	wsa_buf.buf = reinterpret_cast<char*>(&packet);
	wsa_buf.len = packet.size;

	WSASend(client, &wsa_buf, 1, 0, 0, nullptr, nullptr);// ?이거 왜 WSASend임? do_send?
}

void SESSION::send_game_start()
{
	SC_GameStart packet;
	packet.size = sizeof(SC_GameStart);
	packet.type = SC_GAME_START;

	do_send(packet.size, reinterpret_cast<char*>(&packet));
}

void SESSION::send_death(int dead_id, int killer_id)
{
	SC_Death packet;
	packet.size = sizeof(SC_Death);
	packet.type = SC_DEATH;
	packet.dead_id = dead_id;
	packet.killer_id = killer_id;

	do_send(packet.size, reinterpret_cast<char*>(&packet));
}

void SESSION::send_respawn(int player_id, float x, float y, int hp)
{
	SC_Respawn packet;
	packet.size = sizeof(SC_Respawn);
	packet.type = SC_RESPAWN;
	packet.player_id = player_id;
	packet.x = x;
	packet.y = y;
	packet.hp = hp;

	do_send(packet.size, reinterpret_cast<char*>(&packet));
}

void SESSION::send_game_result()
{
	SC_GameResult packet;
	packet.size = sizeof(SC_GameResult);
	packet.type = SC_GAME_RESULT;

	do_send(packet.size, reinterpret_cast<char*>(&packet));
}