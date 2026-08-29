#include "AI.h"
#include "../Bot.h"
#include "../Network.h"
#include <iostream>
#include <random>
#include "Action.h"

// ============================================================
// AI 폴더 구성
// 강화학습 기반 봇 AI의 상태 추출, 행동 선택, 보상 계산,
// 경험 저장, 학습 및 실행 흐름을 담당한다.
// ============================================================


// 외부에서 봇 AI를 실행하기 위한 최상위 진입점.
// 실제 상태 추출, Reward 계산, Experience 저장,
// 행동 선택과 실행은 TrainingManager가 담당한다.

constexpr float AI_DECISION_INTERVAL = 0.1f;

AI::AI(
    DBThread* dbThread,
    const AIModelInfo& modelInfo)
    : m_trainingManager(dbThread,modelInfo)
{}
void AI::Update(
    BotClient& bot,
    const RoomObservation& room,
    Network& network,
    float dt)
{
    // 살아 있거나 방금 죽은 봇까지 TrainingManager에 전달한다.
    // 죽은 순간의 마지막 Experience를 저장하기 위해 Dead도 허용한다.
    if (bot.state != BotState::InGame &&
        bot.state != BotState::Dead)
    {
        return;
    }

    m_trainingManager.UpdateBot(
        bot,
        room,
        dt,
        network
    );
}

//출력용 문자열 변환 함수==========================================
const char* MoveActionToString(MoveAction action)
{
    switch (action)
    {
    case MoveAction::Stop:      return "Stop";
    case MoveAction::Up:        return "Up";
    case MoveAction::Down:      return "Down";
    case MoveAction::Left:      return "Left";
    case MoveAction::Right:     return "Right";
    case MoveAction::UpLeft:    return "UpLeft";
    case MoveAction::UpRight:   return "UpRight";
    case MoveAction::DownLeft:  return "DownLeft";
    case MoveAction::DownRight: return "DownRight";
    default:                    return "Unknown";
    }
}

const char* CombatActionToString(CombatAction action)
{
    switch (action)
    {
    case CombatAction::None:   return "None";
    case CombatAction::Attack: return "Attack";
    case CombatAction::Skill:  return "Skill";
    default:                   return "Unknown";
    }
}

void AI::OnGameEnd(
    BotClient& bot,
    const SC_GameResult& result)
{
    m_trainingManager.OnGameEnd(
        bot,
        result
    );
}

//==============================================================

//void AI::Update(BotClient& bot, Network& network)
//{
//    if (bot.state != BotState::InGame)
//        return;
//
//
//    bot.decisionTimer -= dt;
//
//    if (bot.decisionTimer > 0.0f)
//        return;
//
//    bot.decisionTimer = AI_DECISION_INTERVAL;
//
//    // 상태 추출
//    // 행동 선택
//    // 패킷 전송
//    // 이전 경험 저장
//
//
//    if (randomValue < epsilon)
//    {
//        // 탐험: 무작위 행동
//        action = RandomAction();
//    }
//    else
//    {
//        // 활용: Q값이 가장 높은 행동
//        action = SelectBestAction(state);
//    }
//
//    //임시용------------------------------------------
//    static std::mt19937 rng{ std::random_device{}() };
//    static std::uniform_real_distribution<float> dirDist(-1.0f, 1.0f);
//    static std::uniform_int_distribution<int> actionDist(0, 100);
//
//    float x = dirDist(rng);
//    float y = dirDist(rng);
//
//    network.SendMove(bot, x, y);
//
//    // 나중에 적 근처에방향으로 바라보기
//   // network.SendFaceDir(bot, targetX - bot.x, targetY - bot.y);
//
//    // 일단 임시로 이동 방향과 같은 방향 바라보기
//    network.SendFaceDir(bot, x, y);
//
//    int action = actionDist(rng);
//
//    if (action < 10) {
//        network.SendAttack(bot);
//    }
//
//    if (action >= 10 && action < 15) {
//        network.SendSkill(bot, 1);
//   // }
//    //------------------------------------------
//
//    // 나중에 이동
//    // 공격
//    // 스킬
//}