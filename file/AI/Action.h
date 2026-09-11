#pragma once

// 강화학습에서 사용할 행동(Action)을 정의한다.
// 이동 행동, 공격 행동, 스킬 행동과 전체 행동 개수를 관리하며,
// DQN이 선택한 정수 행동 값을 실제 행동 종류로 변환한다.

#include <cstddef>

enum class MoveAction : std::size_t
{
    Stop = 0,
    Up,
    Down,
    Left,
    Right,
    UpLeft,
    UpRight,
    DownLeft,
    DownRight,

    Count
};

enum class CombatAction : std::size_t
{
    None = 0,
    Attack,
    Skill,

    Count
};

// 강화학습에서 사용할 행동(Action)을 정의한다. 타겟 액션 추가
enum class TargetAction : std::size_t { Enemy0, Enemy1, Enemy2, Count };
constexpr std::size_t TARGET_ACTION_COUNT = 3;

struct DecodedAction
{
    TargetAction target = TargetAction::Enemy0;
    MoveAction move = MoveAction::Stop;
    CombatAction combat = CombatAction::None;
};

constexpr std::size_t MOVE_ACTION_COUNT =
static_cast<std::size_t>(MoveAction::Count);

constexpr std::size_t COMBAT_ACTION_COUNT =
static_cast<std::size_t>(CombatAction::Count);

constexpr std::size_t TOTAL_ACTION_COUNT =
MOVE_ACTION_COUNT * COMBAT_ACTION_COUNT * TARGET_ACTION_COUNT;
static_assert(TOTAL_ACTION_COUNT == 81);

// DQN이 선택한 정수 행동 값을 실제 행동 종류로 변환한다.
inline DecodedAction DecodeAction(std::size_t action)
{
    DecodedAction result;
    result.target = static_cast<TargetAction>(action % TARGET_ACTION_COUNT);
    result.combat = static_cast<CombatAction>((action / TARGET_ACTION_COUNT) % COMBAT_ACTION_COUNT);
    result.move = static_cast<MoveAction>(action / (TARGET_ACTION_COUNT * COMBAT_ACTION_COUNT));
    return result;
}
