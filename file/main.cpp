#include "ServerMain.h"
#include "Common.h"

int main()
{
    ServerMain server;

    if (!server.InitServer(9000)) {
        printf("InitServer() 실패\n");
        return 1;
    }

    printf("[TCP 서버] 초기화 완료. 클라이언트 접속 대기 중...\n");
    //server.AcceptClient();   // 여기서 accept() + NetThread 생성

    server.Run();


    return 0;
}