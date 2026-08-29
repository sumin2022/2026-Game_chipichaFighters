#pragma once

#include "Bot.h"
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <thread>
#include <atomic>

class BotManager;

enum class IOType {
    RECV,
    SEND
};

struct OverlappedEx {
    WSAOVERLAPPED overlapped{};
    WSABUF wsaBuf{};
    char buffer[1024]{};
    IOType type;
};

class Network
{
public:
    bool InitializeNetwork();
    void Shutdown();

    bool Connect(BotClient& bot, const char* ip, unsigned short port);

    void PostRecv(BotClient& bot);
    void SendPacket(BotClient& bot, void* packet);

    void ProcessPacket(BotClient& bot, char* packet, int bytes);
    void HandleRecv(BotClient& bot, char* data, int bytes);

    void SendLogin(BotClient& bot, const char* name);
    void SendReady(BotClient& bot);
    void SendSelectCharacter(BotClient& bot, CharacterType character);

    void SendMove(BotClient& bot, float axisX, float axisY);
    void SendFaceDir(BotClient& bot, float axisX, float axisY);
    void SendAttack(BotClient& bot);
    void SendSkill(BotClient& bot, short skillId);

    void SendGameReady(BotClient& bot, bool ready);
    CharacterType GetRandomCharacter();

    explicit Network(BotManager& botManager);

private:
    void WorkerThread();

private:
    BotManager& m_botManager;

    HANDLE m_iocp = nullptr;
    std::thread m_worker;
    std::atomic<bool> m_running = false;
};