#include "DQNAgent.h"
#include <fstream>
#include <filesystem>
#include <cereal/archives/binary.hpp>
#include <random>
#include <algorithm>
#include <iostream>
#include <limits>
#include <cstdint>
#include <stdexcept>

// MLpack을 이용한 DQN 강화학습 모델을 관리한다.
// 현재 상태에서 행동별 Q값을 계산하고,
// Epsilon-Greedy 방식으로 탐험 또는 최적 행동을 선택한다.
// ReplayBuffer의 경험을 이용해 모델을 학습하고,
// 학습된 모델의 저장과 불러오기를 담당한다.

DQNAgent::DQNAgent()
{
    // 입력 State 64개
    // ↓
    // Hidden Layer 128
    // ↓
    // Hidden Layer 128
    // ↓
    // 행동 81개의 Q값 출력

   // Online Network
    m_network.Add<mlpack::Linear>(128);
    m_network.Add<mlpack::ReLU>();

    m_network.Add<mlpack::Linear>(128);
    m_network.Add<mlpack::ReLU>();

    m_network.Add<mlpack::Linear>(
        TOTAL_ACTION_COUNT
    );


    // Target Network
    // Online Network와 완전히 동일한 구조여야 한다.
    m_targetNetwork.Add<mlpack::Linear>(128);
    m_targetNetwork.Add<mlpack::ReLU>();

    m_targetNetwork.Add<mlpack::Linear>(128);
    m_targetNetwork.Add<mlpack::ReLU>();

    m_targetNetwork.Add<mlpack::Linear>(
        TOTAL_ACTION_COUNT
    );
}

namespace {
std::size_t RandomValidAction(const arma::colvec& input)
{
    std::vector<std::size_t> valid;
    for (std::size_t a=0; a<TOTAL_ACTION_COUNT; ++a)
		if (IsTargetActionValid(input, a)) valid.push_back(a); // 유효한 행동만 선택

    if (valid.empty()) throw std::runtime_error("No valid action / state schema mismatch");

    static thread_local std::mt19937 rng{std::random_device{}()}; //스레드마다 생성한 난수 유지
    std::uniform_int_distribution<std::size_t> distribution(0,valid.size()-1);

	return valid[distribution(rng)]; // 랜덤하게 유효한 행동 선택
}
std::size_t BestValidAction(const arma::mat& values, const arma::colvec& input) // Q값 중 가장 큰 유효한 행동 선택
{
    if (values.n_rows != TOTAL_ACTION_COUNT || values.n_cols != 1)
        throw std::runtime_error("DQN output must contain 81 Q values");

    std::size_t best = TOTAL_ACTION_COUNT;
	double score = -std::numeric_limits<double>::infinity(); // Q값은 음수일 수 있으므로 -무한대로 초기화

    for(std::size_t a=0; a<TOTAL_ACTION_COUNT; ++a) {
        if(!IsTargetActionValid(input,a)) continue;
        if(best == TOTAL_ACTION_COUNT || values(a,0)>score) { best=a; score=values(a,0); }
		// best : 가장 좋은 행동, score : 가장 좋은 Q값
		// best == TOTAL_ACTION_COUNT - > 아직 유효한 행동이 없음을 의미
    }

    if(best == TOTAL_ACTION_COUNT) throw std::runtime_error("No valid action");

    return best;
}

constexpr std::uint32_t MODEL_MAGIC = 0x54473831; // TG81 모델 식별자
constexpr std::uint32_t MODEL_SCHEMA = 1; // 모델 파일 형식의 버전 번호
}

std::size_t DQNAgent::SelectAction(const RLState& state, bool training)
{
    if (!m_networkReady || (training && RandomFloat() < m_epsilon))
        return RandomValidAction(ToArmaVector(state));
    return SelectBestAction(state);
}

float DQNAgent::RandomFloat()
{
    static thread_local std::mt19937 rng{ std::random_device{}() };

    static thread_local std::uniform_real_distribution<float> dist(
        0.0f,
        1.0f
    );

    return dist(rng);
}

std::size_t DQNAgent::SelectBestAction(const RLState& state)
{
    // 아직 신경망 학습이 시작되지 않았다면
    // 랜덤 행동을 사용한다.
    // 나중에 m_targetNetwork.Predict(...)
    if (!m_networkReady)
    {
        return RandomValidAction(ToArmaVector(state));
    }

    // RLState를 MLpack 입력 벡터로 변환
    arma::colvec input = ToArmaVector(state);

    arma::mat output;

    // State를 신경망에 넣어
    // 행동별 Q값을 얻는다.
    m_network.Predict(input, output);

    return BestValidAction(output, input);
}

void DQNAgent::Train(const std::vector<Experience>& batch)
{
    if (batch.empty())
        return;

    for (const auto& experience : batch) {
        if (experience.state.n_elem != AIConfig::StateSize || experience.nextState.n_elem != AIConfig::StateSize ||
            !IsTargetActionValid(experience.state, experience.action))
            throw std::runtime_error("Replay state/action schema mismatch");
    }
    const std::size_t batchSize = batch.size();

    // 입력:
    // 64 x BatchSize
    arma::mat states(AIConfig::StateSize, batchSize);

    // 정답 Q값:
    // 81 x BatchSize
    arma::mat targets(
        TOTAL_ACTION_COUNT,
        batchSize,
        arma::fill::zeros
    );


    // ========================================================
    // Batch 데이터를 하나씩 처리
    // ========================================================

    for (std::size_t i = 0; i < batchSize; ++i)
    {
        const Experience& experience = batch[i];

        states.col(i) = experience.state;


        // --------------------------------------------
        // 현재 State의 Q값
        // --------------------------------------------

        arma::mat currentQ;

        if (m_networkReady)
        {
            m_network.Predict(experience.state, currentQ);
        }
        else
        {
            // 첫 학습 전에는 Q값을 0으로 시작
            currentQ = arma::zeros<arma::mat>(TOTAL_ACTION_COUNT, 1);
        }


        // 현재 Q값을 Target의 기본값으로 복사
        targets.col(i) = currentQ.col(0);


        // --------------------------------------------
        // DQN Target 계산
        // --------------------------------------------

        float targetValue = experience.reward;

        // NextState의 Q값은 Target Network
        if (!experience.done && m_networkReady)
        {
            arma::mat nextQ;

            // NextState Q값은 Online이 아니라
            // Target Network를 이용한다.
            m_targetNetwork.Predict(experience.nextState, nextQ);

			const auto bestNext = BestValidAction(nextQ, experience.nextState); // 다음 상태에서 선택 가능한 것 중 가장 좋은 행동 선택
            const float maxNextQ = static_cast<float>(nextQ(bestNext, 0)); //그 해당 행동번호에 해당하는 Q값

            targetValue += AIConfig::Gamma * maxNextQ;
        }


        // 실제 실행한 Action의 Q값만
        // Bellman Target으로 변경한다.
        targets(experience.action, i) = targetValue;
    }


    // ========================================================
    // MLpack 신경망 학습
    // ========================================================

    ens::Adam optimizer(
        0.001,      // learning rate
        32,         // batch size
        0.9,        // beta1
        0.999,      // beta2
        1e-8,       // epsilon
        1000,       // max iterations
        1e-8,       // tolerance
        true        // shuffle
    );

    m_network.Train(states, targets, optimizer);

    m_networkReady = true;

    ++m_trainCount;

    // 처음 학습한 직후에는
// Online Network를 Target Network에 그대로 복사한다.
    if (!m_targetNetworkReady)
    {
        UpdateTargetNetwork();
    }


    // 일정 학습 횟수마다 Target Network 갱신
    if (m_trainCount %
        AIConfig::TargetUpdateInterval == 0)
    {
        UpdateTargetNetwork();
    }


    // 일정 학습 횟수마다 Epsilon 감소
    if (m_trainCount %
        AIConfig::EpsilonDecayInterval == 0)
    {
        DecayEpsilon();
    }



    if (m_trainCount <= 10 || m_trainCount % 100 == 0)
    {
        std::cout
            << "[DQN TRAIN]"
            << " Count=" << m_trainCount
            << " Batch=" << batchSize
            << " Epsilon=" << m_epsilon
            << "\n";
    }
}

void DQNAgent::UpdateTargetNetwork()
{
    if (!m_networkReady)
        return;

    // Online Network의 가중치를
    // Target Network로 복사한다.
    m_targetNetwork.Parameters() = m_network.Parameters();

    m_targetNetworkReady = true;

    std::cout
        << "[TARGET UPDATE]"
        << " TrainCount=" << m_trainCount
        << "\n";
}

void DQNAgent::DecayEpsilon()
{
    if (m_epsilon <= AIConfig::EpsilonMin)
    {
        m_epsilon = AIConfig::EpsilonMin;
        return;
    }

    m_epsilon *= AIConfig::EpsilonDecay;

    // 최소 탐험 확률 이하로 내려가지 않도록 제한
    if (m_epsilon < AIConfig::EpsilonMin)
    {
        m_epsilon = AIConfig::EpsilonMin;
    }
}

bool DQNAgent::SaveModel(const std::string& filePath)
{
    // 아직 한 번도 학습되지 않은 모델은 저장하지 않는다.
    if (!m_networkReady)
    {
        std::cout << "[MODEL SAVE] Network is not ready.\n";
        return false;
    }

    try
    {
        // models/ 같은 상위 폴더가 없다면 생성한다.
        std::filesystem::path path(filePath);

        if (path.has_parent_path())
        {
            std::filesystem::create_directories(
                path.parent_path()
            );
        }

        // 바이너리 파일 생성
        std::ofstream file(filePath, std::ios::binary);

        if (!file.is_open())
        {
            std::cout
                << "[MODEL SAVE FAIL] "
                << filePath
                << "\n";

            return false;
        }

        const std::uint32_t header[] = {MODEL_MAGIC, MODEL_SCHEMA,
            static_cast<std::uint32_t>(AIConfig::StateSize), static_cast<std::uint32_t>(TOTAL_ACTION_COUNT)};
        file.write(reinterpret_cast<const char*>(header), sizeof(header));

        // 모델 파일에 저장할 정보
        DQNModelData data{};

        data.network = m_network;
        data.epsilon = m_epsilon;
        data.trainCount = m_trainCount;

        // Cereal을 이용하여 바이너리 저장
        cereal::BinaryOutputArchive archive(file);

        archive(data);

        std::cout
            << "[MODEL SAVE]"
            << " Path=" << filePath
            << " TrainCount=" << m_trainCount
            << " Epsilon=" << m_epsilon
            << "\n";

        return true;
    }
    catch (const std::exception& e)
    {
        std::cout
            << "[MODEL SAVE ERROR] "
            << e.what()
            << "\n";

        return false;
    }
}

bool DQNAgent::LoadModel(const std::string& filePath)
{
    // 모델 파일이 존재하지 않으면
    // 새 모델로 학습을 시작한다.
    if (!std::filesystem::exists(filePath))
    {
        std::cout
            << "[MODEL LOAD] File not found: "
            << filePath
            << "\n";

        return false;
    }

    try
    {
        std::ifstream file(filePath, std::ios::binary);

        if (!file.is_open())
        {
            std::cout
                << "[MODEL LOAD FAIL] "
                << filePath
                << "\n";

            return false;
        }

        DQNModelData data{};

        std::uint32_t header[4]{};
        file.read(reinterpret_cast<char*>(header), sizeof(header));
        if (!file || header[0] != MODEL_MAGIC || header[1] != MODEL_SCHEMA ||
            header[2] != AIConfig::StateSize || header[3] != TOTAL_ACTION_COUNT) {
            std::cout << "[MODEL LOAD SKIP] Expected target81 schema (64 inputs / 81 actions).\n";
            return false;
        }
        // 저장된 모델 데이터 복원
        cereal::BinaryInputArchive archive(file);

        archive(data);

        arma::mat validationInput(AIConfig::StateSize, 1, arma::fill::zeros);
        arma::mat validationOutput;
        data.network.Predict(validationInput, validationOutput);
        if (validationOutput.n_rows != TOTAL_ACTION_COUNT || validationOutput.n_cols != 1)
            throw std::runtime_error("Loaded model output shape mismatch");

        // Online Network 복원
        m_network = std::move(data.network);

        // 학습 진행 상태 복원
        m_epsilon = data.epsilon;

        m_trainCount = data.trainCount;

        m_networkReady = true;

        // ====================================================
        // Target Network 초기화
        // ====================================================

        // Target Network가 입력 크기를 알 수 있도록
        // 임시 State를 한 번 통과시킨다.
        arma::mat dummyInput(
            AIConfig::StateSize,
            1,
            arma::fill::zeros
        );

        arma::mat dummyOutput;

        m_targetNetwork.Predict(dummyInput, dummyOutput);

        // Load된 Online Network의 가중치를
        // Target Network에 복사한다.
        m_targetNetwork.Parameters() = m_network.Parameters();

        m_targetNetworkReady = true;


        std::cout
            << "[MODEL LOAD]"
            << " Path=" << filePath
            << " TrainCount=" << m_trainCount
            << " Epsilon=" << m_epsilon
            << "\n";

        return true;
    }
    catch (const std::exception& e)
    {
        std::cout
            << "[MODEL LOAD ERROR] "
            << e.what()
            << "\n";

        return false;
    }
}