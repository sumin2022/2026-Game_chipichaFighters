#pragma once

#include <armadillo>
#include <vector>
#include <cstddef>

struct Experience
{
    arma::colvec state;

    size_t action = 0;

    float reward = 0.0f;

    arma::colvec nextState;

    bool done = false;

    int roomId = -1;
    int botId = -1;
};

class ReplayBuffer
{
public:
    explicit ReplayBuffer(
        std::size_t capacity = 50000 //초기값
    );

    void Push(
        const Experience& experience
    );

    std::vector<Experience> Sample(
        std::size_t batchSize
    ) const;

    std::size_t Size() const;

    void Clear();

private:
    std::vector<Experience> m_buffer;

    std::size_t m_capacity = 0;

    // Buffer가 가득 찬 후
    // 다음에 덮어쓸 위치
    std::size_t m_nextIndex = 0;
};