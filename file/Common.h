#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더
#include <windows.h>
#include <MSWSock.h>

#include <tchar.h> // _T(), ...
#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strncpy(), ...
#include <atomic>
#include <iostream>

#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "WS2_32.lib")

void err_quit(const char* msg);
void err_display(const char* msg);
void err_display(int errcode);

