//ReplayBuffer.cpp

// 강화학습 경험 데이터를 메모리에 저장한다.
// State, Action, Reward, NextState, Done으로 구성된 Experience를 보관하고,
// 학습 시 무작위 Mini-Batch를 추출한다.

#include "ReplayBuffer.h"

#include <algorithm>
#include <random>
#include <iterator>


// ============================================================
// 생성자
// ============================================================
//
// ReplayBuffer가 저장할 수 있는 최대 Experience 개수를 설정한다.
// reserve()를 사용하여 필요한 메모리를 미리 확보하지만,
// 실제 vector의 size가 증가하는 것은 Push()가 호출될 때이다.
ReplayBuffer::ReplayBuffer(
    std::size_t capacity)
    : m_capacity(capacity)
{
    m_buffer.reserve(m_capacity);
}


// ============================================================
// Experience 저장
// ============================================================
//
// 새로운 강화학습 경험을 ReplayBuffer에 저장한다.
//
// Buffer에 공간이 남아 있는 동안에는 뒤에 계속 추가하고,
// 최대 용량에 도달한 이후에는 가장 오래된 Experience부터
// 새로운 Experience로 덮어쓴다.
//
// 이를 통해 학습이 오래 진행되어도
// ReplayBuffer의 메모리 사용량이 계속 증가하지 않도록 한다.
void ReplayBuffer::Push(
    const Experience& experience)
{
    // 최대 용량이 0이면 Experience를 저장할 수 없다.
    if (m_capacity == 0)
        return;

    // ========================================================
    // 아직 Buffer가 가득 차지 않은 경우
    // ========================================================

    // 최대 용량에 도달하기 전까지는
    // 새로운 Experience를 vector 뒤에 순서대로 추가한다.
    if (m_buffer.size() < m_capacity)
    {
        m_buffer.push_back(experience);
        return;
    }

    // ========================================================
    // Buffer가 가득 찬 경우
    // ========================================================

    // 더 이상 vector의 크기를 증가시키지 않고
    // 가장 오래된 Experience가 저장된 위치를
    // 새로운 Experience로 덮어쓴다.
    m_buffer[m_nextIndex] = experience;

    // 다음 Push에서 덮어쓸 위치로 이동한다.
    //
    // 마지막 위치까지 도달하면 % 연산을 통해
    // 다시 0번 위치로 돌아간다.
    //
    // 예:
    // capacity = 5
    //
    // [0][1][2][3][4]
    //  ↑
    // nextIndex
    //
    // 0 → 1 → 2 → 3 → 4 → 0 → ...
    //
    // 이러한 방식을 Circular Buffer 방식이라고 볼 수 있다.
    m_nextIndex =
        (m_nextIndex + 1)
        % m_capacity;
}


// ============================================================
// Mini-Batch 추출
// ============================================================
//
// ReplayBuffer에 저장된 Experience 중에서
// DQN 학습에 사용할 경험을 무작위로 선택한다.
//
// 시간 순서대로 연속된 데이터를 학습하는 대신
// 서로 다른 시점의 Experience를 무작위로 섞어서 학습하여
// 데이터 사이의 시간적 상관관계를 줄이는 것이 목적이다.
std::vector<Experience> ReplayBuffer::Sample(
    std::size_t batchSize) const
{
    std::vector<Experience> batch;

    // 저장된 Experience가 없거나
    // 요청한 Batch 크기가 0이면 빈 vector를 반환한다.
    if (m_buffer.empty() || batchSize == 0)
        return batch;

    // 요청한 batchSize가 현재 저장된 Experience보다 크다면
    // 현재 가지고 있는 Experience 개수만큼만 추출한다.
    //
    // 예:
    // Buffer Size = 30
    // Batch Size  = 64
    //
    // 실제 추출 개수 = 30
    const std::size_t sampleCount =
        (std::min)(
            batchSize,
            m_buffer.size()
            );

    // 앞으로 저장할 Experience 개수를 알고 있으므로
    // 필요한 메모리를 미리 확보한다.
    batch.reserve(sampleCount);

    // 각 스레드에서 독립적으로 사용하는 난수 생성기
    // Sample을 호출할 때 Experience를 무작위로 선택하는 데 사용한다.
    static thread_local std::mt19937 rng{
        std::random_device{}()
    };

    // ========================================================
    // Experience 무작위 추출
    // ========================================================

    // m_buffer에서 sampleCount개의 Experience를
    // 중복 없이 무작위로 선택하여 batch에 저장한다.
    //
    // 예:
    //
    // ReplayBuffer
    // [E0][E1][E2][E3][E4][E5][E6]
    //
    // Sample(3)
    //
    //          ↓ Random
    //
    // [E5][E1][E6]
    //
    // 이렇게 선택된 Mini-Batch를 이후 DQN 학습에 사용한다.
    std::sample(
        m_buffer.begin(),
        m_buffer.end(),
        std::back_inserter(batch),
        sampleCount,
        rng
    );

    return batch;
}


// ============================================================
// 현재 저장된 Experience 개수 반환
// ============================================================
//
// ReplayBuffer의 최대 용량이 아니라
// 현재 실제로 저장되어 있는 Experience 개수를 반환한다.
//
// 이후 TrainingManager에서
//
// if (replayBuffer.Size() >= batchSize)
//
// 같은 방식으로 학습을 시작할 수 있는지 확인할 때 사용한다.
std::size_t ReplayBuffer::Size() const
{
    return m_buffer.size();
}


// ============================================================
// ReplayBuffer 초기화
// ============================================================
//
// 현재 저장되어 있는 모든 Experience를 제거하고
// Circular Buffer의 다음 저장 위치도 처음으로 되돌린다.
//
// 모델을 새로 학습하거나
// 기존 경험 데이터를 완전히 버려야 할 때 사용할 수 있다.
void ReplayBuffer::Clear()
{
    m_buffer.clear();

    // Buffer가 비었으므로
    // 다음 덮어쓰기 위치 역시 처음부터 시작한다.
    m_nextIndex = 0;
}