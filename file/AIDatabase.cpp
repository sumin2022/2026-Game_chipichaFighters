#include "AIDatabase.h"
#include <iostream>
#include <sstream>

AIDatabase::AIDatabase()
{
}

AIDatabase::~AIDatabase()
{
    Disconnect();
}


// ============================================================
// MySQL 연결
// ============================================================

bool AIDatabase::Connect(const char* host)
{
    // 이미 연결되어 있으면 다시 연결하지 않는다.
    if (m_connection != nullptr)
        return true;

    m_connection = mysql_init(nullptr);

    if (m_connection == nullptr)
    {
        std::cout
            << "[AI DATABASE] mysql_init failed\n";

        return false;
    }

    // 기존 MySQL 설정에 맞게 수정
    const char* user = "root";
    const char* password = "pungbear2018";
    const char* database = "ai_db";

    const unsigned int port = 3306;

    if (mysql_real_connect(
        m_connection,
        host,
        user,
        password,
        database,
        port,
        nullptr,
        0
    ) == nullptr)
    {
        std::cout
            << "[AI DATABASE] Connect failed: "
            << mysql_error(m_connection)
            << "\n";

        mysql_close(m_connection);
        m_connection = nullptr;

        return false;
    }

    std::cout
        << "[AI DATABASE] Connected to ai_db\n";

    return true;
}


// ============================================================
// MySQL 연결 종료
// ============================================================

void AIDatabase::Disconnect()
{
    if (m_connection == nullptr)
        return;

    mysql_close(m_connection);

    m_connection = nullptr;

    std::cout
        << "[AI DATABASE] Disconnected\n";
}


// ============================================================
// 새로운 AI 모델 등록
// ============================================================

bool AIDatabase::InsertModel(AIModelInfo& info)
{
    if (m_connection == nullptr)
        return false;


    // modelPath에 ', \ 등이 들어가도 SQL이 깨지지 않도록 처리
    std::string escapedPath;

    escapedPath.resize(
        info.modelPath.size() * 2 + 1
    );

    unsigned long escapedLength =
        mysql_real_escape_string(
            m_connection,
            escapedPath.data(),
            info.modelPath.c_str(),
            static_cast<unsigned long>(
                info.modelPath.size()
                )
        );

    escapedPath.resize(escapedLength);


    std::ostringstream query;

    query
        << "INSERT INTO ai_models "
        << "("
        << "model_version, "
        << "model_path, "
        << "train_count, "
        << "epsilon, "
        << "total_games, "
        << "wins, "
        << "losses, "
        << "average_reward"
        << ") VALUES ("

        << info.modelVersion << ", "
        << "'" << escapedPath << "', "
        << info.trainCount << ", "
        << info.epsilon << ", "
        << info.totalGames << ", "
        << info.wins << ", "
        << info.losses << ", "
        << info.averageReward

        << ")";


    if (mysql_query(
        m_connection,
        query.str().c_str()) != 0)
    {
        std::cout
            << "[AI DATABASE] InsertModel failed: "
            << mysql_error(m_connection)
            << "\n";

        return false;
    }


    std::cout
        << "[AI DATABASE] InsertModel"
        << " version=" << info.modelVersion
        << "\n";

    info.modelId =static_cast<long long>(mysql_insert_id(m_connection));

    return true;
}


// ============================================================
// 기존 모델 정보 갱신
// ============================================================

bool AIDatabase::UpdateModel(
    const AIModelInfo& info)
{
    if (m_connection == nullptr)
        return false;


    std::string escapedPath;

    escapedPath.resize(
        info.modelPath.size() * 2 + 1
    );

    unsigned long escapedLength =
        mysql_real_escape_string(
            m_connection,
            escapedPath.data(),
            info.modelPath.c_str(),
            static_cast<unsigned long>(
                info.modelPath.size()
                )
        );

    escapedPath.resize(escapedLength);


    std::ostringstream query;

    query
        << "UPDATE ai_models SET "

        << "model_version="
        << info.modelVersion << ", "

        << "model_path='"
        << escapedPath << "', "

        << "train_count="
        << info.trainCount << ", "

        << "epsilon="
        << info.epsilon << ", "

        << "total_games="
        << info.totalGames << ", "

        << "wins="
        << info.wins << ", "

        << "losses="
        << info.losses << ", "

        << "average_reward="
        << info.averageReward << " "

        << "WHERE model_id="
        << info.modelId;


    if (mysql_query(
        m_connection,
        query.str().c_str()) != 0)
    {
        std::cout
            << "[AI DATABASE] UpdateModel failed: "
            << mysql_error(m_connection)
            << "\n";

        return false;
    }


    std::cout
        << "[AI DATABASE] UpdateModel"
        << " id=" << info.modelId
        << " version=" << info.modelVersion
        << " trainCount=" << info.trainCount
        << " epsilon=" << info.epsilon
        << "\n";

    return true;
}


// ============================================================
// 가장 최근 모델 불러오기
// ============================================================

bool AIDatabase::LoadLatestModel(
    AIModelInfo& outInfo)
{
    if (m_connection == nullptr)
        return false;


    const char* query =
        "SELECT "
        "model_id, "
        "model_version, "
        "model_path, "
        "train_count, "
        "epsilon, "
        "total_games, "
        "wins, "
        "losses, "
        "average_reward "
        "FROM ai_models "
        "ORDER BY model_id DESC "
        "LIMIT 1";


    if (mysql_query(
        m_connection,
        query) != 0)
    {
        std::cout
            << "[AI DATABASE] LoadLatestModel failed: "
            << mysql_error(m_connection)
            << "\n";

        return false;
    }


    MYSQL_RES* result =
        mysql_store_result(m_connection);

    if (result == nullptr)
    {
        std::cout
            << "[AI DATABASE] No result\n";

        return false;
    }


    MYSQL_ROW row =
        mysql_fetch_row(result);

    if (row == nullptr)
    {
        // 아직 저장된 모델이 없는 경우
        mysql_free_result(result);

        return false;
    }


    outInfo.modelId =
        std::stoll(row[0]);

    outInfo.modelVersion =
        std::stoi(row[1]);

    outInfo.modelPath =
        row[2] != nullptr
        ? row[2]
        : "";

    outInfo.trainCount =
        static_cast<std::size_t>(
            std::stoull(row[3])
            );

    outInfo.epsilon =
        std::stof(row[4]);

    outInfo.totalGames =
        std::stoi(row[5]);

    outInfo.wins =
        std::stoi(row[6]);

    outInfo.losses =
        std::stoi(row[7]);

    outInfo.averageReward =
        std::stod(row[8]);


    mysql_free_result(result);


    std::cout
        << "[AI DATABASE] Latest model loaded"
        << " id=" << outInfo.modelId
        << " version=" << outInfo.modelVersion
        << " trainCount=" << outInfo.trainCount
        << "\n";


    return true;
}