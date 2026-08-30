#pragma once

#include <winsock2.h>
#include "../../GameServer/file/protocol.h"
#include <armadillo>
#include "AI/RLState.h"
#include <atomic>

enum class BotState {
    Disconnected,
    Connected,
    LoggedIn,
    MatchWaiting,
    InLobby,
    InGame,
    Dead
};

struct BotLearningContext
{
    // 이전 판단 시점 상태
    RLState previousState{};

    // 이전 판단 시점 행동
    std::size_t previousAction = 0;

    // 이전 경험 존재 여부
    bool hasPreviousExperience = false;

    // 보상 계산용 이전 값
    int previousHp = 0;
    int previousKills = 0;
    int previousDeaths = 0;

    int previousRedScore = 0;
    int previousBlueScore = 0;

};

struct BotCooldownState
{
    float attackCooldownLeft = 0.0f;
    float skillCooldownLeft = 0.0f;
};

struct BotClient {
    int index = -1;
    int player_id = -1;
    int room_id = -1;
    TeamType team = TeamType::TEAM_NONE;

    SOCKET socket = INVALID_SOCKET;

    BotState state = BotState::Disconnected;

    char recvBuffer[4096]{};
    int recvBytes = 0;

    float x = 0.0f;
    float y = 0.0f;

    int hp = 100;
    bool alive = true;

    CharacterType character = CharacterType::CHAR_NONE;

    float decisionTimer = 0.0f;

    BotCooldownState Bot_cooldown;

    BotLearningContext learning;

    bool IsConnected() const {
        return state != BotState::Disconnected;
    }

    // Network Thread에서 받은 게임 결과를
    // Main AI Thread가 처리하기 위해 임시 저장한다.
    SC_GameResult pendingGameResult{};
    bool hasPendingGameResult = false;
    int pendingGameRoomId = -1;

    // 공격과 스킬이 적중했는지 여부를 판단 후 보상 계산에 사용하기 위해 임시 저장한다.
    // Network Thread -> AI Thread 전달용
    std::atomic<int> pendingAttackHits{ 0 };
    std::atomic<int> pendingSkillHits{ 0 };
};

