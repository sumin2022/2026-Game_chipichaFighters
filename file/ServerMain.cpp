#include "ServerMain.h"
#include "RoomManager.h"
#include <chrono>
#include "SESSION.h"

ServerMain::ServerMain() : server(INVALID_SOCKET) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		err_quit("WSAStartup() 실패");
	}
}

ServerMain::~ServerMain() {

	//listen socket 정리
	if (server != INVALID_SOCKET) {
		closesocket(server);
		server = INVALID_SOCKET;
	}
	WSACleanup();
}

// -------------------------------
// 서버 초기화
// -------------------------------

bool ServerMain::InitServer(int port) {
	
	int retval;
	
	// 소켓 생성
	server = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
	if (server == INVALID_SOCKET) {
		err_quit("socket() 실패");
		return false;
	}

	// bind
	SOCKADDR_IN serveraddr{};
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); //나중에 PORT로 바꾸기 
	serveraddr.sin_port = htons(static_cast<u_short>(port));
	retval = bind(server, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		err_quit("bind()");
		return false;
	}

	// listen
	retval = listen(server, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		err_quit("listen()");
		return false;
	}

	if (!db.ConnectDB())
	{
		std::cout << "DB 연결 실패" << "\n";
		return false;
	}

	std::cout << "DB 연결 성공" << "\n";

	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	CreateIoCompletionPort((HANDLE)server, h_iocp, -1, 0);

	PostAccept();

	// 디비 테스트
	//db.RegisterUser("player1", "1234");

	//if (db.LoginUser("player1", "1234"))
	//{
	//	std::cout << "로그인 성공\n";
	//}
	//else
	//{
	//	std::cout << "로그인 실패\n";
	//}


	return true;
}


// -------------------------------
// 클라이언트 접속 대기
// -------------------------------
void ServerMain::PostAccept() {

	EXP_OVER* accept_over = new EXP_OVER(IO_ACCEPT);

	accept_over->m_client_socket =
		WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (accept_over->m_client_socket == INVALID_SOCKET) {
		err_display("WSASocket()");
		delete accept_over;
		return;
	}

	BOOL ret = AcceptEx(
		server,
		accept_over->m_client_socket,
		accept_over->m_buff,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		NULL,
		&accept_over->m_over
	);

	if (ret == FALSE) {
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING) {
			err_display("AcceptEx()");
			closesocket(accept_over->m_client_socket);
			delete accept_over;
		}
	}

}

void ServerMain::GameThread()
{
	using namespace std::chrono;

	const auto tick = milliseconds(16); // 약 60FPS

	while (true) {
		auto start = steady_clock::now();

		RoomManager::Instance().update_all_rooms();

		auto end = steady_clock::now();
		auto elapsed = end - start;

		if (elapsed < tick) {
			std::this_thread::sleep_for(tick - elapsed);
		}
	}
}

void ServerMain::WorkerThread()
{
	while (true) {
		DWORD num_bytes = 0;
		ULONG_PTR key = 0;
		LPOVERLAPPED over = nullptr;

		BOOL ret = GetQueuedCompletionStatus(
			h_iocp,
			&num_bytes,
			&key,
			&over,
			INFINITE
		);

		if (over == nullptr) continue;

		EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(over);

		if (exp_over->m_iotype == IO_ACCEPT) {
			HandleAccept(exp_over);
			continue;
		}

		int player_index = static_cast<int>(key);

		if (!ret || num_bytes == 0) {
			DisconnectClient(player_index);
			continue;
		}

		switch (exp_over->m_iotype) {
		case IO_ACCEPT:
		{
			HandleAccept(exp_over);
			break;
		}
		case IO_RECV:
			HandleRecv(player_index, num_bytes, exp_over);
			break;

		case IO_SEND:
			HandleSend(exp_over, player_index);
			break;
		}
	}
}

void ServerMain::Run()
{
	std::thread network_thread(&ServerMain::WorkerThread, this);
	std::thread game_thread(&ServerMain::GameThread, this);
	//std::thread db_thread(&ServerMain::DBThread, this);

	network_thread.join();
	game_thread.join();
	//db_thread.join();
}

void ServerMain::HandleRecv(int player_index, DWORD num_bytes, EXP_OVER* exp_over)
{
	SESSION& session = clients[player_index];

	session.m_prev_recv += num_bytes;

	unsigned char* packet_start =
		reinterpret_cast<unsigned char*>(session.m_recv_over.m_buff);

	int remain_data = session.m_prev_recv;

	while (remain_data > 0) {
		unsigned char packet_size = packet_start[0];

		if (remain_data < packet_size)
			break;

		session.process_packet(packet_start);

		remain_data -= packet_size;
		packet_start += packet_size;
	}

	if (remain_data > 0) {
		memmove(
			session.m_recv_over.m_buff,
			packet_start,
			remain_data
		);
	}

	session.m_prev_recv = remain_data;
	session.do_recv();
}

void ServerMain::HandleSend(EXP_OVER* exp_over, int player_index)
{
	delete exp_over;
}

void ServerMain::HandleAccept(EXP_OVER* exp_over)
{
	SOCKET client_socket = exp_over->m_client_socket;

	int new_id = GetNewClientId();

	for (int i = 0; i < MAX_PLAYERS; ++i) {
		if (!clients[i].m_is_connected) {
			new_id = i;
			break;
		}
	}

	if (new_id == -1) {
		closesocket(client_socket);
		delete exp_over;
		PostAccept();
		return;
	}

	clients[new_id].m_id = new_id;
	clients[new_id].m_client = client_socket;
	clients[new_id].m_is_connected = true;
	clients[new_id].m_is_logged_in = false;
	clients[new_id].m_in_game = false;
	clients[new_id].m_prev_recv = 0;

	CreateIoCompletionPort(
		reinterpret_cast<HANDLE>(client_socket),
		h_iocp,
		new_id,
		0
	);

	clients[new_id].do_recv();

	std::cout << "Client accepted: " << new_id << "\n";

	delete exp_over;

	PostAccept();
}

int ServerMain::GetNewClientId()
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		if (!clients[i].m_is_connected) {
			return i;
		}
	}
	return -1;
}

void ServerMain::DisconnectClient(int player_index)
{
	RoomManager::Instance().leave_room(player_index);

	clients[player_index].m_is_connected = false;
	clients[player_index].m_is_logged_in = false;
	clients[player_index].m_in_game = false;
	clients[player_index].m_room_id = -1;

	closesocket(clients[player_index].m_client);
	clients[player_index].m_client = INVALID_SOCKET;

	std::cout << "Client disconnected: " << player_index << '\n';
}