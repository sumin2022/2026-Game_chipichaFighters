#pragma once
#include "Common.h"
#include "ExpOver.h"
#include "protocol.h"

class EXP_OVER;

class SESSION {
public:
	SOCKET m_client;
	int m_id;
	bool m_is_connected;
	EXP_OVER m_recv_over;
	int m_prev_recv;
	char m_username[MAX_NAME_LEN];
	short m_x, m_y;

	SESSION();
	~SESSION();
	void do_recv();
	void do_send(int num_bytes, char* mess);
	void send_avatar_info();
	void send_move_packet(int mover);
	void send_add_player(int player_id);
	void send_remove_player(int player_id);
	void send_login_success();
	void process_packet(unsigned char* p);

	void send_current_state();
	void send_death(int player_id);
	void send_respawn(int player_id);
	void send_game_start();
	void send_game_result();
};

extern std::array<SESSION, MAX_PLAYERS> clients;

void send_login_fail(SOCKET client, const char* message);