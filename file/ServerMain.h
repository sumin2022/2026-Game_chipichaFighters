#pragma once
#include "Common.h"
#include "DBLogin.h"
#include "ExpOver.h"
#include <vector>
#include <thread>
#include <atomic>

class ServerMain {
public:
	ServerMain();
	~ServerMain();
	bool InitServer(int port);
	void PostAccept();
	void HandleAccept(EXP_OVER* exp_over);
	void HandleRecv(int player_index, DWORD num_bytes, EXP_OVER* exp_over);
	void HandleSend(EXP_OVER* exp_over, int player_index);
	void DisconnectClient(int player_index);
	void CleanupServer();

private:
	SOCKET server;
	HANDLE h_iocp;
	DBLogin db;
};