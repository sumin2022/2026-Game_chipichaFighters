#pragma once

#include "AIDatabase.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

enum class DBJobType
{
    InsertModel,
    UpdateModel
};

struct DBJob
{
    DBJobType type;
    AIModelInfo info;
};

class DBThread
{
public:
    explicit DBThread(AIDatabase& database);

    ~DBThread();

    void Start();
    void Stop();

    void PushModelUpdate(const AIModelInfo& info);

    void PushModelInsert(const AIModelInfo& info);

private:
    void ThreadLoop();

private:
    AIDatabase& m_database;

    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;

    std::queue<DBJob> m_jobs;

    std::atomic<bool> m_running{false};
};