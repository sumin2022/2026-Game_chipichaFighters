#include "SESSION.h"
#include <algorithm>

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
		CS_Login* packet = reinterpret_cast<CS_Login*>(p);
		strncpy_s(m_username, packet->username, MAX_NAME_LEN);

		std::cout << "Player[" << m_id << "] logged in as " << m_username << std::endl;

		send_login_success();
		send_avatar_info();

		for (auto& other : clients) {
			if (!other.m_is_connected) continue;
			if (other.m_id == m_id) continue;
			send_add_player(other.m_id);
		}

		for (auto& other : clients) {
			if (!other.m_is_connected) continue;
			if (other.m_id == m_id) continue;
			other.send_add_player(m_id);
		}
		break;
	}
	case CS_MOVE:
	{
		CS_Move* packet = reinterpret_cast<CS_Move*>(p);
		DIRECTION dir = packet->dir;

		switch (dir) {
		case UP:    m_y = std::max<short>(0, m_y - 1); break;
		case DOWN:  m_y = std::min<short>(WORLD_HEIGHT - 1, m_y + 1); break;
		case LEFT:  m_x = std::max<short>(0, m_x - 1); break;
		case RIGHT: m_x = std::min<short>(WORLD_WIDTH - 1, m_x + 1); break;
		}

		std::cout << "Player[" << m_id << "] moved to (" << m_x << ", " << m_y << ")\n";

		for (auto& cl : clients) {
			if (cl.m_is_connected)
				cl.send_move_packet(m_id);
		}
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

	WSASend(client, &wsa_buf, 1, 0, 0, nullptr, nullptr);
}