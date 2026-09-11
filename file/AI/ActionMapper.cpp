#include "ActionMapper.h"
#include "../Network.h"
#include "Action.h"
#include "TargetSelector.h"
#include <cmath>
#include <cfloat>
#include <iostream>

// DQN이나 랜덤 정책이 선택한 행동 번호를 실제 게임 명령으로 변환한다.
// 이동 방향을 axisX, axisY 값으로 바꾸고,
// 필요하면 SendMove, SendFaceDir, SendAttack, SendSkill을 호출한다.

void ActionMapper::Execute(BotClient& bot,std::size_t action, Network& network,const RoomObservation& room)
{
    // 정수로 선택된 행동을 이동 행동과 전투 행동으로 분리한다.
    const DecodedAction decoded = DecodeAction(action);

    float moveX = 0.0f;
    float moveY = 0.0f;

    // 선택된 이동 행동을 실제 이동 방향 벡터로 변환한다.
    switch (decoded.move)
    {
    case MoveAction::Stop:
        moveX = 0.0f;
        moveY = 0.0f;
        break;

    case MoveAction::Up:
        moveX = 0.0f;
        moveY = 1.0f;
        break;

    case MoveAction::Down:
        moveX = 0.0f;
        moveY = -1.0f;
        break;

    case MoveAction::Left:
        moveX = -1.0f;
        moveY = 0.0f;
        break;

    case MoveAction::Right:
        moveX = 1.0f;
        moveY = 0.0f;
        break;

    case MoveAction::UpLeft:
        moveX = -1.0f;
        moveY = 1.0f;
        break;

    case MoveAction::UpRight:
        moveX = 1.0f;
        moveY = 1.0f;
        break;

    case MoveAction::DownLeft:
        moveX = -1.0f;
        moveY = -1.0f;
        break;

    case MoveAction::DownRight:
        moveX = 1.0f;
        moveY = -1.0f;
        break;

    default:
        moveX = 0.0f;
        moveY = 0.0f;
        break;
    }

    // 대각선 이동 속도가 직선 이동보다 빨라지지 않도록 정규화한다.
    const float length =
        std::sqrt(moveX * moveX + moveY * moveY);

    if (length > 0.0f)
    {
        moveX /= length;
        moveY /= length;
    }

    constexpr float CHECK_DISTANCE = 100.0f;

    const float checkX =
        bot.x + moveX * CHECK_DISTANCE;

    const float checkY =
        bot.y + moveY * CHECK_DISTANCE;

    if ((moveX != 0.0f || moveY != 0.0f) && m_mapCollision.IsBlocked(checkX, checkY))
    {
        std::cout
            << "[AI WALL BLOCK]"
            << " Bot=" << bot.index
            << " Pos=(" << bot.x << ", " << bot.y << ")"
            << " Check=(" << checkX << ", " << checkY << ")"
            << '\n';
        // 선택한 방향이 벽이면 일단 이동하지 않는다.
        moveX = 0.0f;
        moveY = 0.0f;
    }


    // 이동 방향을 메인 서버에 전달한다.
    // Stop 행동인 경우에도 (0, 0)을 보내 이동을 중단시킨다.
    network.SendMove(bot, moveX, moveY);

    // 신경망이 선택한 슬롯만 사용합니다. 가까운 적/저체력 적으로 대체하지 않습니다.
    const auto* self = FindObservedPlayer(room, bot.player_id);

    if (!self || !self->alive) return;

    const auto ids = EnemySlotIds(room, room.FindTeam(bot.player_id));
    const auto* target = FindObservedPlayer(room, ids[static_cast<std::size_t>(decoded.target)]);

    if (!target || !target->alive ||
        !m_mapCollision.HasLineOfSight(self->x,self->y,target->x,target->y)) return;

    const float dx=target->x - self->x, dy=target->y - self->y;
    const float distance=std::sqrt(dx*dx+dy*dy);

    if (distance > 0.0001f) network.SendFaceDir(bot, dx/distance, dy/distance);

    // 선택된 전투 행동을 실제 공격 또는 스킬 패킷으로 변환한다.
    switch (decoded.combat)
    {
    case CombatAction::None:
        // 이동만 수행하고 별도의 전투 행동은 하지 않는다.
        break;

    case CombatAction::Attack:
        network.SendAttack(bot);
        break;

    case CombatAction::Skill:
        // 현재 캐릭터당 사용하는 스킬이 하나라면
        // 임시로 skill_id를 1로 고정한다.
        network.SendSkill(bot, 1);
        break;

    default:
        break;
    }

    //std::cout
    //    << "[ACTION] Bot=" << bot.index
    //    << " Action=" << action
    //    << " Move=" << static_cast<int>(decoded.move)
    //    << " Combat=" << static_cast<int>(decoded.combat)
    //    << "\n";
}