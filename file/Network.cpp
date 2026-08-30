#include "Common.h"
#include "Network.h"
#include <cstring>
#include <random>
#include <chrono>
#include "BotManager.h"

Network::Network(BotManager& botManager)
    : m_botManager(botManager)
{}

bool Network::InitializeNetwork()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cout << "WSAStartup failed\n";
        return false;
    }

    m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
    if (m_iocp == nullptr) {
        std::cout << "CreateIoCompletionPort failed\n";
        WSACleanup();
        return false;
    }

    m_running = true;
    m_worker = std::thread(&Network::WorkerThread, this);

    return true;
}

bool Network::Connect(BotClient& bot, const char* ip, unsigned short port)
{
    bot.socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (bot.socket == INVALID_SOCKET) {
        std::cout << "WSASocket failed\n";
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(bot.socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "connect failed\n";
        closesocket(bot.socket);
        bot.socket = INVALID_SOCKET;
        bot.state = BotState::Disconnected;
        return false;
    }

    CreateIoCompletionPort(
        reinterpret_cast<HANDLE>(bot.socket),
        m_iocp,
        reinterpret_cast<ULONG_PTR>(&bot),
        0
    );

    bot.state = BotState::Connected;

    PostRecv(bot);

    return true;
}

void Network::PostRecv(BotClient& bot)
{
    auto* ex = new OverlappedEx;
    ex->type = IOType::RECV;
    ex->wsaBuf.buf = ex->buffer;
    ex->wsaBuf.len = sizeof(ex->buffer);

    DWORD flags = 0;
    DWORD recvBytes = 0;

    int ret = WSARecv(
        bot.socket,
        &ex->wsaBuf,
        1,
        &recvBytes,
        &flags,
        &ex->overlapped,
        nullptr
    );

    if (ret == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            std::cout << "WSARecv failed: " << err << "\n";
            delete ex;
            closesocket(bot.socket);
            bot.socket = INVALID_SOCKET;
            bot.state = BotState::Disconnected;
        }
    }
}

void Network::HandleRecv(BotClient& bot, char* data, int bytes)
{
    if (bot.recvBytes + bytes > sizeof(bot.recvBuffer)) {
        closesocket(bot.socket);
        bot.socket = INVALID_SOCKET;
        bot.state = BotState::Disconnected;
        return;
    }

    memcpy(bot.recvBuffer + bot.recvBytes, data, bytes);
    bot.recvBytes += bytes;

    unsigned char* packetStart =
        reinterpret_cast<unsigned char*>(bot.recvBuffer);

    int remainData = bot.recvBytes;

    while (remainData >= sizeof(TZPacketHeader))
    {
        TZPacketHeader header{};
        memcpy(&header, packetStart, sizeof(TZPacketHeader));

        std::uint16_t packetSize = header.size;

        if (packetSize < sizeof(TZPacketHeader))
            break;

        if (remainData < packetSize)
            break;

        ProcessPacket(bot, reinterpret_cast<char*>(packetStart), packetSize);

        remainData -= packetSize;
        packetStart += packetSize;
    }

    if (remainData > 0) {
        memmove(bot.recvBuffer, packetStart, remainData);
    }

    bot.recvBytes = remainData;
}

void Network::SendPacket(BotClient& bot, void* packet)
{
    if (!bot.IsConnected()) return;

    std::uint16_t size = *(std::uint16_t*)packet;

    auto* ex = new OverlappedEx;
    ex->type = IOType::SEND;
    memcpy(ex->buffer, packet, size);
    ex->wsaBuf.buf = ex->buffer;
    ex->wsaBuf.len = size;

    DWORD sendBytes = 0;

    int ret = WSASend(
        bot.socket,
        &ex->wsaBuf,
        1,
        &sendBytes,
        0,
        &ex->overlapped,
        nullptr
    );

    if (ret == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            std::cout << "WSASend failed: " << err << "\n";
            delete ex;
            bot.state = BotState::Disconnected;
        }
    }
}

void Network::WorkerThread()
{
    while (m_running) {
        DWORD bytes = 0;
        ULONG_PTR key = 0;
        LPOVERLAPPED overlapped = nullptr;

        BOOL result = GetQueuedCompletionStatus(
            m_iocp,
            &bytes,
            &key,
            &overlapped,
            INFINITE
        );

        if (!m_running) break;

        BotClient* bot = reinterpret_cast<BotClient*>(key);
        OverlappedEx* ex = reinterpret_cast<OverlappedEx*>(overlapped);

        if (bot == nullptr || ex == nullptr)
            continue;

        if (!result || bytes == 0) {

            if (bot) {
                closesocket(bot->socket);
                bot->socket = INVALID_SOCKET;
                bot->state = BotState::Disconnected;
            }

            if (ex)
                delete ex;

            continue;
        }

        if (ex->type == IOType::RECV) {
            HandleRecv(*bot, ex->buffer, bytes);
            delete ex;

            if (bot->IsConnected()) {
                PostRecv(*bot);
            }
        }
        else if (ex->type == IOType::SEND) {
            delete ex;
        }
    }
}

void Network::ProcessPacket(BotClient& bot, char* packet, int bytes)
{
    if (bytes < sizeof(TZPacketHeader))
        return;

    TZPacketHeader* header = reinterpret_cast<TZPacketHeader*>(packet);


    switch (header->type) {
        using enum PACKET_TYPE;
    case SC_LOGIN_RESULT:
    {
        SC_LoginResult* pkt = reinterpret_cast<SC_LoginResult*>(packet);

        if (pkt->success) {
            bot.state = BotState::LoggedIn;
            std::cout << "[BOT " << bot.index << "] Login success\n";
        }
        else {
            bot.state = BotState::Disconnected;
            std::cout << "[BOT " << bot.index << "] Login failed: "
                << pkt->message << "\n";
        }
        break;
    }
    case SC_ROOM_ENTER:
    {
        auto* pkt =
            reinterpret_cast<SC_RoomEnter*>(packet);

        bot.room_id = pkt->room_id;
        bot.state = BotState::InLobby;

        std::cout << "\n[ROOM ENTER] room_id="
            << pkt->room_id << "\n";

        // 방 전체 플레이어의 ID와 팀 정보를 BotManager에 저장한다.
        m_botManager.SetRoomTeamInfo(
            pkt->room_id,
            *pkt
        );

        const int playerCount = (std::min)(
            pkt->player_count,
            MAX_ROOM_PLAYERS
            );

        for (int i = 0; i < playerCount; ++i)
        {
            const int playerId = pkt->player_ids[i];

            const TeamType team =
                static_cast<TeamType>(pkt->teams[i]);

            std::cout << "player_id=" << playerId
                << " team=" << static_cast<int>(team)
                << "\n";

            // 현재 봇 자신의 팀 저장
            if (playerId == bot.player_id)
            {
                bot.team = team;
            }
        }

        CharacterType ch = GetRandomCharacter();
        bot.character = ch;

        SendSelectCharacter(bot, ch);
        SendGameReady(bot, true);

        std::cout << "[BOT " << bot.index
            << "] Enter lobby / select character = "
            << static_cast<int>(ch)
            << " / game ready\n";

        break;
    }
    case SC_GAME_START:
    {
        bot.state = BotState::InGame;
        std::cout << "[BOT " << bot.index << "] Game Start / InGame\n";
        break;
    }
    case SC_DEATH:
    {
        auto* pkt = reinterpret_cast<SC_Death*>(packet);

        std::cout << "[DEATH] dead_id=" << pkt->dead_id
            << " killer_id=" << pkt->killer_id << "\n";

        if (bot.player_id == pkt->dead_id) {
            bot.state = BotState::Dead;
            bot.alive = false;
        }

        break;
    }
    case SC_RESPAWN:
    {
        auto* pkt = reinterpret_cast<SC_Respawn*>(packet);

        std::cout << "[RESPAWN] player_id=" << pkt->player_id
            << " x=" << pkt->x
            << " y=" << pkt->y
            << " hp=" << pkt->hp << "\n";

        if (bot.player_id == pkt->player_id) {
            bot.state = BotState::InGame;
            bot.alive = true;
            bot.x = pkt->x;
            bot.y = pkt->y;
            bot.hp = pkt->hp;
        }

        break;
    }
    case SC_AVATAR_INFO:
    {
        SC_AvatarInfo* pkt =
            reinterpret_cast<SC_AvatarInfo*>(packet);

        bot.player_id = pkt->playerId;
        bot.x = static_cast<float>(pkt->x);
        bot.y = static_cast<float>(pkt->y);

        std::cout << "[BOT " << bot.index
            << "] player_id = "
            << bot.player_id << "\n";
        break;
    }
    case SC_ROOM_SNAPSHOT:
    {
        auto* pkt = reinterpret_cast<SC_RoomSnapshot*>(packet);

        static auto lastPrintTime = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();

        bool shouldPrint =
            std::chrono::duration_cast<std::chrono::seconds>(now - lastPrintTime).count() >= 1;

        // 현재 봇 자신의 상태 갱신
        for (int i = 0; i < pkt->count; ++i) {
            const NetPlayerState& p = pkt->players[i];

            if (p.player_id == bot.player_id) {
                bot.x = p.x;
                bot.y = p.y;
                bot.hp = p.hp;
                bot.alive = p.alive;
                bot.character = p.character;

                bot.state = p.alive
                    ? BotState::InGame
                    : BotState::Dead;

                break;
            }
        }

        // 강화학습에 사용할 방 전체 관측 정보 생성
        if (bot.room_id >= 0)
        {
            RoomObservation observation{};

            observation.roomId = bot.room_id;
            observation.redScore = pkt->red_score;
            observation.blueScore = pkt->blue_score;
            observation.timeLeft = pkt->time_left;

            observation.playerCount = (std::min)(
                pkt->count,
                static_cast<int>(observation.players.size())
                );

            for (int i = 0; i < observation.playerCount; ++i)
            {
                observation.players[i] = pkt->players[i];
            }

            m_botManager.UpdateRoomObservation(
                bot.room_id,
                observation
            );
        }

        if (shouldPrint) {
            lastPrintTime = now;

            std::cout << "\n========== ROOM SNAPSHOT ==========\n";
            std::cout << "Red Score: " << pkt->red_score
                << " / Blue Score: " << pkt->blue_score
                << " / Time Left: " << pkt->time_left << "\n";

            for (int i = 0; i < pkt->count; ++i) {
                const NetPlayerState& p = pkt->players[i];

                std::cout << "Player[" << p.player_id << "] "
                    << "HP " << p.hp << "/" << p.max_hp
                    << " Pos(" << p.x << ", " << p.y << ") "
                    << "Alive=" << p.alive
                    << " Char=" << static_cast<int>(p.character)
                    << " K/D=" << p.kill_count << "/" << p.death_count
                    << " Target=" << p.current_target_id
                    << "\n";
            }

            std::cout << "===================================\n";
        }

        break;
    }
    case SC_ATTACK:
    {
        auto* pkt = reinterpret_cast<SC_Attack*>(packet);

        // 이 봇이 공격자인 경우에만 적중 횟수 증가
        if (pkt->attacker_id == bot.player_id)
        {
            bot.pendingAttackHits.fetch_add(1, std::memory_order_relaxed);
        }

        break;
    }
    case SC_SKILL_HIT:
    {
        auto* pkt = reinterpret_cast<SC_SkillHit*>(packet);

        // 이 봇이 스킬 시전자인 경우에만 적중 횟수 증가
        if (pkt->caster_id == bot.player_id)
        {
            bot.pendingSkillHits.fetch_add(1, std::memory_order_relaxed);
        }

        break;
    }
    case SC_GAME_RESULT:
    {
        auto* pkt = reinterpret_cast<SC_GameResult*>(packet);

        std::cout
            << "\n[GAME RESULT]"
            << " BOT=" << bot.index
            << " RED=" << pkt->red_score
            << " BLUE=" << pkt->blue_score
            << " WINNER=" << static_cast<int>(pkt->winner_team)
            << "\n";

        // =====================================================
        // AI Main Thread에서 처리할 게임 결과 저장
        // =====================================================

        bot.pendingGameResult = *pkt;

        bot.pendingGameRoomId = bot.room_id;

        bot.hasPendingGameResult = true;

        // 이전 게임의 방 정보 제거/초기화가 필요하다면 여기서 처리
        bot.room_id = -1;

        // 로그인 연결 자체는 유지되고 있으므로
        // 다시 매칭 가능한 상태로 변경
        bot.state = BotState::LoggedIn;

        // 다음 게임 자동 매칭
        SendReady(bot);

        bot.state = BotState::MatchWaiting;

        std::cout
            << "[BOT " << bot.index
            << "] rematch CS_READY sent\n";

        break;
    }
    default:
        break;
    }
}

void Network::Shutdown()
{
    m_running = false;

    if (m_iocp) {
        PostQueuedCompletionStatus(m_iocp, 0, 0, nullptr);
    }

    if (m_worker.joinable()) {
        m_worker.join();
    }

    if (m_iocp) {
        CloseHandle(m_iocp);
        m_iocp = nullptr;
    }

    WSACleanup();
}

void Network::SendLogin(BotClient& bot, const char* name)
{
    CS_Login packet{};
    packet.size = sizeof(CS_Login);
    packet.type = PACKET_TYPE::CS_LOGIN;

    strncpy_s(packet.username, MAX_NAME_LEN, name, _TRUNCATE);

    SendPacket(bot, &packet);
}
void Network::SendReady(BotClient& bot)
{
    CS_Ready packet{};
    packet.size = sizeof(CS_Ready);
    packet.type = PACKET_TYPE::CS_READY;

    SendPacket(bot, &packet);
}
void Network::SendSelectCharacter(BotClient& bot, CharacterType character)
{
    CS_SelectCharacter packet{};
    packet.size = sizeof(CS_SelectCharacter);
    packet.type = PACKET_TYPE::CS_SELECT_CHARACTER;
    packet.character = character;

    SendPacket(bot, &packet);
}

void Network::SendMove(BotClient& bot, float axisX, float axisY)
{
    CS_Move packet{};
    packet.size = sizeof(CS_Move);
    packet.type = PACKET_TYPE::CS_MOVE;
    packet.axisX = axisX;
    packet.axisY = axisY;

    SendPacket(bot, &packet);
}

void Network::SendFaceDir(BotClient& bot, float x, float y)
{
    CS_FaceDir packet;
    packet.size = sizeof(packet);
    packet.type = PACKET_TYPE::CS_FACE_DIR;
    packet.faceX = x;
    packet.faceY = y;

    SendPacket(bot, &packet);
}

void Network::SendAttack(BotClient& bot)
{
    CS_Attack packet{};
    packet.size = sizeof(CS_Attack);
    packet.type = PACKET_TYPE::CS_ATTACK;

    SendPacket(bot, &packet);
}

void Network::SendSkill(BotClient& bot, short skillId)
{
    CS_Skill packet{};
    packet.size = sizeof(CS_Skill);
    packet.type = PACKET_TYPE::CS_SKILL;
    packet.skill_id = skillId;

    SendPacket(bot, &packet);
}

CharacterType Network::GetRandomCharacter()
{
    static std::mt19937 rng{ std::random_device{}() };
    static std::uniform_int_distribution<int> dist(1, 4);

    return static_cast<CharacterType>(dist(rng));
}

void Network::SendGameReady(BotClient& bot, bool ready)
{
    CS_GameReady packet{};
    packet.size = sizeof(CS_GameReady);
    packet.type = PACKET_TYPE::CS_GAME_READY;
    packet.ready = ready;

    SendPacket(bot, &packet);
}