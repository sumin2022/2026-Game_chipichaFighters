#pragma once

#include "RLState.h"
#include "RandomPolicy.h"
#include "ReplayBuffer.h"
#include "Action.h"
#include "AIConfig.h"

#include <mlpack.hpp>

#include <cstddef>
#include <vector>
#include <string>

// State → 각 Action의 Q값을 출력하는 신경망
using NetworkType =
mlpack::FFN<
    mlpack::MeanSquaredError,
    mlpack::HeInitialization
>;

class DQNAgent
{
public:
    DQNAgent();

    // Epsilon-Greedy 방식으로 행동 선택
    std::size_t SelectAction(
        const RLState& state,
        bool training
    );

    // ReplayBuffer에서 뽑은 Mini-Batch로 학습
    void Train(const std::vector<Experience>& batch);

    // 모델 저장 / 불러오기
    bool SaveModel(const std::string& filePath);

    bool LoadModel(const std::string& filePath);

    float GetEpsilon() const
    {
        return m_epsilon;
    }

    std::size_t GetTrainCount() const
    {
        return m_trainCount;
    }

private:
    float RandomFloat();

    // 신경망 Q값 중 가장 큰 행동 선택
    std::size_t SelectBestAction(const RLState& state);

    // Online → Target 가중치 복사
    void UpdateTargetNetwork();

    // 탐험 확률 감소
    void DecayEpsilon();


private:
    // 실제 학습되는 신경망
    NetworkType m_network;

    // NextState의 안정적인 Q Target 계산용
    NetworkType m_targetNetwork;

    RandomPolicy m_randomPolicy;

    float m_epsilon = AIConfig::EpsilonStart;
    bool m_networkReady = false;
    bool m_targetNetworkReady = false;
    std::size_t m_trainCount = 0;
};

struct DQNModelData
{
    NetworkType network;

    float epsilon = 1.0f;

    std::size_t trainCount = 0;

    template<class Archive>
    void serialize(
        Archive& ar)
    {
        ar(
            CEREAL_NVP(network),
            CEREAL_NVP(epsilon),
            CEREAL_NVP(trainCount)
        );
    }
};