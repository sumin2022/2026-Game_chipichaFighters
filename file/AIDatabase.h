#pragma once

#include <string>
#include <cstddef>
#include <mysql.h>


struct AIModelInfo
{
    long long modelId = 0;

    int modelVersion = 1;

    std::string modelPath;

    std::size_t trainCount = 0;

    float epsilon = 1.0f;

    int totalGames = 0;
    int wins = 0;
    int losses = 0;

    double averageReward = 0.0;
};

class AIDatabase
{
public:
    AIDatabase();
    ~AIDatabase();

    bool Connect(
        const char* host,
        const char* user,
        const char* password,
        const char* database
    );

    void Disconnect();

    bool InsertModel(AIModelInfo& info);

    bool UpdateModel(const AIModelInfo& info);

    bool LoadLatestModel(AIModelInfo& outInfo);

private:
    MYSQL* m_connection = nullptr;
};