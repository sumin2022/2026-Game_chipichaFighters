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

struct DecodedAction
{
    MoveAction move = MoveAction::Stop;
    CombatAction combat = CombatAction::None;
};

constexpr std::size_t MOVE_ACTION_COUNT =
static_cast<std::size_t>(MoveAction::Count);

constexpr std::size_t COMBAT_ACTION_COUNT =
static_cast<std::size_t>(CombatAction::Count);

constexpr std::size_t TOTAL_ACTION_COUNT =
MOVE_ACTION_COUNT * COMBAT_ACTION_COUNT;

inline DecodedAction DecodeAction(std::size_t action)
{
    DecodedAction result;

    result.move = static_cast<MoveAction>(
        action / COMBAT_ACTION_COUNT
        );

    result.combat = static_cast<CombatAction>(
        action % COMBAT_ACTION_COUNT
        );

    return result;
}