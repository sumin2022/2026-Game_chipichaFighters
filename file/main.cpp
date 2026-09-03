#include <iostream>
#include <string>
#include <random>
#include <memory>

#include "Network.h"
#include "BotManager.h"
#include "AI/AI.h"
#include "AIDatabase.h"
#include "DBThread.h"

//봇 개수 입력
//Network.Initailize();
//botCount 만큼 WSAConnect() 호출
//열결 성공시 CS_LOGIN
// SC_LOGIN, SC_AVATAR_INFO 수신
// BotState::LoggedIn
//콘솔에서 match 입력시 CS_READY 전송

int main() {

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // ==============================================
    // Game Server
    // ==============================================
    std::string gameServerIp;

    std::cout << "Game Server IP: ";
    std::cin >> gameServerIp;

    // ==============================================
    // DB 사용 여부
    // ==============================================
    char useDatabase;

    std::cout << "Use Database? (y/n): ";
    std::cin >> useDatabase;

    BotManager botManager;
    Network network(botManager);

    AIDatabase database;

    // ==============================================
    // 기본 모델 정보
    // ==============================================
    // DB 없이도 .bin 파일 찾기
    AIModelInfo modelInfo{};

    modelInfo.modelVersion = 1;
    modelInfo.modelPath = "models/dqn_model.bin";
    modelInfo.trainCount = 0;
    modelInfo.epsilon = AIConfig::EpsilonStart;

    // DB Thread
    std::unique_ptr<DBThread> dbThread;

    // ==============================================
    // Database Training Mode
    // ==============================================
    if (useDatabase == 'y' || useDatabase == 'Y')
    {
        std::string dbServerIp;
        std::string dbUser;
        std::string dbPassword;
        std::string dbName;

        std::cout << "DB Server IP: ";
        std::cin >> dbServerIp;

        std::cout << "DB User: ";
        std::cin >> dbUser;

        std::cout << "DB Password: ";
        std::cin >> dbPassword;

        std::cout << "DB Name (default: ai_db): ";
        std::cin >> dbName;


        if (!database.Connect(dbServerIp.c_str(),
            dbUser.c_str(),
            dbPassword.c_str(),
            dbName.c_str()))
        {
            std::cout << "DB connection failed\n";
            return 1;
        }

        // DB에 기존 모델 정보가 있다면 사용
        if (database.LoadLatestModel(modelInfo))
        {
            std::cout
                << "[DB] Existing model info loaded"
                << " ID=" << modelInfo.modelId
                << " TrainCount=" << modelInfo.trainCount
                << " Epsilon=" << modelInfo.epsilon
                << "\n";
        }
        else
        {
            // DB에는 없으므로 새 모델 레코드 생성
            if (!database.InsertModel(modelInfo))
            {
                std::cout << "[DB] Initial model insert failed\n";
                return 1;
            }

            std::cout
                << "[DB] New model inserted"
                << " ID=" << modelInfo.modelId
                << "\n";
        }

        dbThread = std::make_unique<DBThread>(database);

        dbThread->Start();

        std::cout << "[MODE] Database Training Mode\n";
    }
    else    // Local Test Mode
    {
        std::cout
            << "[MODE] Local Test Mode\n"
            << "[MODEL] "
            << modelInfo.modelPath
            << "\n";
    }

    if (database.LoadLatestModel(modelInfo))
    {
        std::cout
            << "[DB] Existing model info loaded"
            << " ID=" << modelInfo.modelId
            << " TrainCount=" << modelInfo.trainCount
            << " Epsilon=" << modelInfo.epsilon
            << "\n";
    }
    else
    {
        //modelInfo.modelVersion = 1;
        //modelInfo.modelPath = "models/dqn_model.bin";
        //modelInfo.trainCount = 0;
        //modelInfo.epsilon = AIConfig::EpsilonStart;

        //if (!database.InsertModel(modelInfo))
        //{
        //    std::cout << "[DB] Initial model insert failed\n";
        //    return 1;
        //}

        //std::cout
        //    << "[DB] New model inserted"
        //    << " ID=" << modelInfo.modelId
        //    << "\n";
    }

    AI ai(dbThread.get(), modelInfo);

    if (!network.InitializeNetwork()) {
        return 1;
    }

    int botCount;
    std::cout << "Bot count: ";
    std::cin >> botCount;

    if (botCount > MAX_BOTS) botCount = MAX_BOTS;

    //const char* serverIp = "127.0.0.1";
    //std::string ip;
    //std::cout << "Server IP : ";
    //std::cin >> ip;

    for (int i = 0; i < botCount; ++i) {
        BotClient& bot = botManager.GetBot(i);
        bot.index = i;

        std::cout << "[BOT " << i << "] Connecting...\n";

        //network.Connect(bot, ip.c_str(), PORT);
        if (network.Connect(bot, gameServerIp.c_str(), PORT)) {

            std::cout << "[BOT " << i << "] Connected\n";

            std::string name = "Bot" + std::to_string(i + 1);
            network.SendLogin(bot, name.c_str());

            std::cout << "[BOT " << i << "] CS_LOGIN sent\n";
        }
        else
        {
            std::cout << "[BOT " << i << "] CONNECT FAILED\n";
        }
    }

    // 로그인 완료 대기
    while (true) {
        int loggedInCount = 0;

        for (int i = 0; i < botCount; ++i) {
            BotClient& bot = botManager.GetBot(i);

            if (bot.state == BotState::LoggedIn) {
                ++loggedInCount;
            }
        }

        if (loggedInCount == botCount) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\nAll bots logged in.\n";
    std::cout << "type 'match' to start matchmaking: ";

    std::string cmd;
    std::cin >> cmd;

    if (cmd == "match") {
        for (int i = 0; i < botCount; ++i) {
            BotClient& bot = botManager.GetBot(i);

            if (bot.state != BotState::LoggedIn)
                continue;

            network.SendReady(bot);
            bot.state = BotState::MatchWaiting;

            std::cout << "[BOT " << bot.index << "] CS_READY sent\n";
        }

        std::cout << "match request sent\n";
    }

    while (true)
    {
        for (int i = 0; i < botCount; ++i)
        {
            BotClient& bot = botManager.GetBot(i);

            // =====================================================
            // 게임 종료 Reward 처리
            // =====================================================

            if (bot.hasPendingGameResult)
            {
                bot.hasPendingGameResult = false;

                ai.OnGameEnd(
                    bot,
                    bot.pendingGameResult
                );
            }

            // 사망 순간의 terminal Experience도 처리해야 하므로
            // Dead 상태까지 AI Update에 전달한다.
            if (bot.state != BotState::InGame &&
                bot.state != BotState::Dead)
            {
                continue;
            }

            if (bot.room_id < 0)
                continue;

            RoomObservation room{};

            if (!botManager.GetRoomObservation(
                bot.room_id,
                room))
            {
                continue;
            }

            ai.Update(
                bot,
                room,
                network,
                0.1f
            );
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(100)
        );

    }
    network.Shutdown();
    if (dbThread)
        dbThread->Stop();
    //
    return 0;
}