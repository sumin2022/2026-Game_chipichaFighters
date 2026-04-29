#include "ServerMain.h"


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

	EXP_OVER accept_over(IO_ACCEPT);
	// EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(over); 나중에 이렇게 별도의 EXP_OVER 객체로 만들어서 사용하기
	
	accept_over.m_client_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (accept_over.m_client_socket == INVALID_SOCKET) {
		err_display("WSASocket()");
		return;
	}
	BOOL ret = 
		AcceptEx(server, accept_over.m_client_socket, &accept_over.m_buff, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		NULL, &accept_over.m_over);

	if (ret == FALSE) {
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING) {
			err_display("AcceptEx()");
			closesocket(accept_over.m_client_socket);
			accept_over.m_client_socket = INVALID_SOCKET;
		}
	}

}