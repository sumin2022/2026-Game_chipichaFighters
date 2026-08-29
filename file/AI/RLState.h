#pragma once

// 신경망에 입력할 강화학습 상태(State)를 정의한다.
// 자신의 HP와 위치, 캐릭터 종류, 적·아군과의 거리,
// 점수, 남은 시간, 아이템 정보 등을 정규화된 값으로 저장한다.

#include <cstddef>
#include <armadillo>
#include "AIConfig.h"

struct RLState
{
    // 자신
    float hpRatio = 0.0f;
    float normalizedX = 0.0f;
    float normalizedY = 0.0f;

    float faceX = 0.0f;
    float faceY = 0.0f;

    float alive = 0.0f;

    // 캐릭터 종류
    float characterDealer = 0.0f;
    float characterArcher = 0.0f;
    float characterTanker = 0.0f;
    float characterHealer = 0.0f;

    // 가장 가까운 적
    float enemyRelativeX = 0.0f;
    float enemyRelativeY = 0.0f;
	float enemyDistance = 1.0f; // 0~1 범위로 정규화 + 기본은 거리가 멀게 설정
    float enemyHpRatio = 0.0f;

    // 가장 가까운 아군
    float allyRelativeX = 0.0f;
    float allyRelativeY = 0.0f;
    float allyDistance = 1.0f; // 0~1 범위로 정규화 + 기본은 거리가 멀게 설정
    float allyHpRatio = 0.0f;

    // 경기
    float teamScoreRatio = 0.0f;
    float enemyScoreRatio = 0.0f;
    float timeRatio = 0.0f;

    // 아이템
    float itemRelativeX = 0.0f;
    float itemRelativeY = 0.0f;
    float itemDistance = 0.0f;
    float itemActive = 0.0f;
};

// RLState 구조체에 저장된 강화학습 상태값을
// MLpack 신경망과 ReplayBuffer에서 사용할 수 있는
// Armadillo 열 벡터(arma::colvec) 형태로 변환한다.
//
// RLState의 각 멤버를 항상 동일한 순서로 벡터에 저장해야 하며,
// 이후 DQN 신경망의 입력 노드 역시 이 순서를 기준으로 사용한다.
inline arma::colvec ToArmaVector(
    const RLState& state)
{
    // 현재 RLState에서 사용하는 특징값은 총 25개이다.
    arma::colvec result(AIConfig::StateSize);

    // 각 특징값을 순서대로 저장하기 위한 인덱스
    std::size_t i = 0;

    // ========================================================
    // 1. 자신의 상태
    // ========================================================

    result(i++) = state.hpRatio;       // 현재 HP 비율 (0~1)
    result(i++) = state.normalizedX;   // 정규화된 X 위치 (-1~1)
    result(i++) = state.normalizedY;   // 정규화된 Y 위치 (-1~1)

    result(i++) = state.faceX;         // 현재 바라보는 X 방향
    result(i++) = state.faceY;         // 현재 바라보는 Y 방향

    result(i++) = state.alive;         // 생존 여부 (생존 1, 사망 0)

    // ========================================================
    // 2. 캐릭터 종류
    // One-Hot Encoding으로 표현한다.
    // ========================================================

    result(i++) = state.characterDealer;
    result(i++) = state.characterArcher;
    result(i++) = state.characterTanker;
    result(i++) = state.characterHealer;

    // ========================================================
    // 3. 가장 가까운 적 정보
    // ========================================================

    result(i++) = state.enemyRelativeX; // 자신 기준 적의 상대 X 위치
    result(i++) = state.enemyRelativeY; // 자신 기준 적의 상대 Y 위치
    result(i++) = state.enemyDistance;  // 가장 가까운 적까지의 정규화된 거리
    result(i++) = state.enemyHpRatio;   // 가장 가까운 적의 HP 비율

    // ========================================================
    // 4. 가장 가까운 아군 정보
    // ========================================================

    result(i++) = state.allyRelativeX; // 자신 기준 아군의 상대 X 위치
    result(i++) = state.allyRelativeY; // 자신 기준 아군의 상대 Y 위치
    result(i++) = state.allyDistance;  // 가장 가까운 아군까지의 정규화된 거리
    result(i++) = state.allyHpRatio;   // 가장 가까운 아군의 HP 비율

    // ========================================================
    // 5. 현재 경기 정보
    // ========================================================

    result(i++) = state.teamScoreRatio;  // 우리 팀 점수 비율
    result(i++) = state.enemyScoreRatio; // 상대 팀 점수 비율
    result(i++) = state.timeRatio;       // 남은 경기 시간 비율

    // ========================================================
    // 6. 아이템 정보
    // 현재는 실제 학습에 사용하지 않지만
    // 이후 아이템 행동을 추가하기 위해 상태 공간에 포함한다.
    // ========================================================

    result(i++) = state.itemRelativeX;
    result(i++) = state.itemRelativeY;
    result(i++) = state.itemDistance;
    result(i++) = state.itemActive;

    return result;
}