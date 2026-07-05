#pragma once
#include <WinSock2.h>
#include <MSWSock.h>

constexpr int BUF_SIZE = 1024;

enum IOType { IO_SEND, IO_RECV, IO_ACCEPT };

class EXP_OVER {
public:
	WSAOVERLAPPED m_over;
	IOType  m_iotype;
	WSABUF	m_wsa;
	SOCKET  m_client_socket;
	char  m_buff[BUF_SIZE];
	EXP_OVER() :m_client_socket(INVALID_SOCKET)
	{
		ZeroMemory(&m_over, sizeof(m_over));
		m_wsa.buf = m_buff;
		m_wsa.len = BUF_SIZE;
	}
	EXP_OVER(IOType iot) : m_iotype(iot), m_client_socket(INVALID_SOCKET)
	{
		ZeroMemory(&m_over, sizeof(m_over));
		m_wsa.buf = m_buff;
		m_wsa.len = BUF_SIZE;
	}
};