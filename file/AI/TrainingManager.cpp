#include "TrainingManager.h"

// 봇별 강화학습 과정을 전체적으로 관리한다.
//
// 이전 판단:
// State(t) -> Action(t)
//
// 다음 판단:
// State(t+1) 관측
// Reward 계산
// Experience 저장
// 새로운 Action 선택
// 다음 판단을 위한 상태 저장

TrainingManager::TrainingManager(
    DBThread* dbThread,
    const AIModelInfo& modelInfo)
    : m_dbThread(dbThread),
    m_modelInfo(modelInfo),
    m_rewardCalculator(RewardConfig{}),
    m_replayBuffer(50000)
{
    if (m_dqnAgent.LoadModel(m_modelInfo.modelPath))
    {
        std::cout
            << "[TRAINING MANAGER] "
            << "Existing DQN model loaded.\n";
    }
    else
    {
        std::cout
            << "[TRAINING MANAGER] "
            << "Start new DQN model.\n";
    }
}
// ============================================================
// 봇 학습 업데이트
// ============================================================

void TrainingManager::UpdateBot(
    BotClient& bot,
    const RoomObservation& room,
    float dt,
    Network& network)
{
    // ========================================================
    // 1. 판단 주기 확인
    // ========================================================

    bot.decisionTimer -= dt;

    if (bot.decisionTimer > 0.0f)
        return;

    bot.decisionTimer = AIConfig::DecisionInterval;


    // ========================================================
    // 2. 현재 봇의 Snapshot 정보 찾기
    // ========================================================

    const NetPlayerState* currentPlayer = nullptr;

    for (int i = 0; i < room.playerCount; ++i)
    {
        if (room.players[i].player_id == bot.player_id)
        {
            currentPlayer = &room.players[i];
            break;
        }
    }

    // 방 Snapshot에서 자신의 정보를 찾지 못했다면
    // 학습 데이터를 만들 수 없으므로 이번 Update는 종료한다.
    if (currentPlayer == nullptr)
        return;


    // ========================================================
    // 3. 현재 State 추출
    // ========================================================

    RLState currentState = m_featureExtractor.Extract(bot,room);

    // ========================================================
    // 4. 이전 State / Action이 있다면 Reward 계산
    // ========================================================

    if (bot.learning.hasPreviousExperience)
    {
        // 이번 판단 구간에서 발생한 적중 보상 소비
        const int attackHits = bot.pendingAttackHits.exchange(0, std::memory_order_relaxed);

        const int skillHits = bot.pendingSkillHits.exchange(0, std::memory_order_relaxed);

        const float reward =
            m_rewardCalculator.Calculate(
                bot.learning,
                *currentPlayer,
                currentState,
                room,
                bot.team,
                attackHits,
                skillHits
            );

        // 테스트용 로그
        if (attackHits > 0 || skillHits > 0)
        {
            std::cout
                << "[HIT REWARD]"
                << " Bot=" << bot.index
                << " AttackHits=" << attackHits
                << " SkillHits=" << skillHits
                << " Reward=" << reward
                << '\n';
        }

        // 이번 경기 동안 이 봇이 얻은 Reward 누적
        m_botEpisodeRewards[bot.player_id] += reward;

        // ====================================================
        // 5. Experience 생성
        // ====================================================

        Experience experience{};

        // 이전 판단 상태
        experience.state = ToArmaVector( bot.learning.previousState);

        // 이전 판단에서 실행한 행동
        experience.action = bot.learning.previousAction;

        // 이전 행동 이후 얻은 Reward
        experience.reward = reward;

        // 현재 관측한 다음 상태
        experience.nextState =ToArmaVector(currentState);

        // 현재 플레이어가 죽었다면
        // 해당 Experience를 Terminal 상태로 표시한다.
        experience.done = !currentPlayer->alive;

        experience.roomId = bot.room_id;

        experience.botId = bot.player_id;


        // ====================================================
        // 6. ReplayBuffer에 Experience 저장
        // ====================================================

        m_replayBuffer.Push(experience);

        // ====================================================
        // 7. DQN Mini-Batch 학습
        // ====================================================

        // BatchSize 이상의 경험이 쌓였다면
        // ReplayBuffer에서 무작위 경험을 뽑아 신경망을 학습한다.
        if (m_replayBuffer.Size() >= AIConfig::BatchSize)
        {
            const auto batch =
                m_replayBuffer.Sample(
                    AIConfig::BatchSize
                );

            m_dqnAgent.Train(batch);
        }

        // ====================================================
        // ReplayBuffer 동작 확인용 로그
        // ====================================================

        const std::size_t bufferSize = m_replayBuffer.Size();

        // 처음 몇 개는 전부 출력하고,
        // 이후에는 50개 단위로 출력하여 로그가 너무 많아지는 것을 방지한다.
        if (bufferSize <= 10 ||
            bufferSize % 50 == 0)
        {
            std::cout
                << "[REPLAY BUFFER]"
                << " Size=" << bufferSize
                << " Bot=" << bot.index
                << " Reward=" << reward
                << " Action="
                << experience.action
                << "\n";
        }

        // 사망 상태라면 마지막 Experience까지만 저장한다.
        //
        // 죽어있는 동안 새로운 행동을 실행하면 안 되고,
        // 같은 deathPenalty가 반복 저장되는 것도 막아야 한다.
        if (!currentPlayer->alive)
        {
            std::cout
                << "[TERMINAL DEATH]"
                << " Bot=" << bot.index
                << " Reward=" << reward
                << "\n";

            bot.learning.hasPreviousExperience = false;

            return;
        }
    }

    // 이전 경험이 없는 상태로 이미 죽어 있다면
    // 아무 행동도 하지 않는다.
    if (!currentPlayer->alive)
    {
        return;
    }

    // ========================================================
    // 7. 다음 행동 선택
    // ========================================================

    // 아직 MLpack DQN 신경망은 연결하지 않았으므로
    // 현재 DQNAgent는 사실상 RandomPolicy를 사용한다.
    const std::size_t action =m_dqnAgent.SelectAction(
            currentState,
            true
        );

    DecodedAction decoded = DecodeAction(action);

    std::cout
        << "[ACTION] Bot=" << bot.index
        << " Action=" << action
        << " Move=" << static_cast<int>(decoded.move)
        << " Combat=" << static_cast<int>(decoded.combat)
        << '\n';

    // ========================================================
    // 8. 선택한 행동 실행
    // ========================================================

    m_actionMapper.Execute(
        bot,
        action,
        network,
        room
    );


    // ========================================================
    // 9. 현재 정보를 다음 판단 시점용으로 저장
    // ========================================================

    bot.learning.previousState = currentState;

    bot.learning.previousAction = action;

    bot.learning.previousHp = currentPlayer->hp;

    bot.learning.previousKills = currentPlayer->kill_count;

    bot.learning.previousDeaths = currentPlayer->death_count;

    bot.learning.previousRedScore = room.redScore;

    bot.learning.previousBlueScore = room.blueScore;
        

    // 이제부터는 이전 State / Action이 존재하므로
    // 다음 Update부터 Experience 생성이 가능하다.
    bot.learning.hasPreviousExperience = true;
}


// ============================================================
// 현재 ReplayBuffer 크기 확인
// ============================================================

std::size_t TrainingManager::GetReplayBufferSize() const
{
    return m_replayBuffer.Size();
}


// ============================================================
// 게임 종료 처리
// ============================================================

void TrainingManager::OnGameEnd(
    BotClient& bot,
    const SC_GameResult& result)
{
    // 승/패에 따른 최종 보상
    const float reward =
        m_rewardCalculator.CalculateGameResult(
            bot.team,
            result.winner_team
        );
    // 최종 보상 경기 누적값도 추가
    m_botEpisodeRewards[bot.player_id] += reward;


    // ========================================================
    // 마지막 Action이 존재하는 경우에만
    // Terminal Experience 추가
    // ========================================================
    if (bot.learning.hasPreviousExperience)
    {
        Experience experience{};

        experience.state = ToArmaVector(bot.learning.previousState);

        experience.action = bot.learning.previousAction;

        experience.reward = reward;

        // done=true이면 DQN 학습에서 NextState의 Q값을 사용하지 않는다.
        // 하지만 입력 크기를 맞추기 위해 33개의 0 벡터를 저장한다.
        experience.nextState = arma::colvec(AIConfig::StateSize, arma::fill::zeros);

        // 경기 종료이므로 terminal state
        experience.done = true;

        experience.roomId = bot.pendingGameRoomId;

        experience.botId = bot.player_id;

        m_replayBuffer.Push(experience);

        // 충분한 Experience가 쌓였다면
        // 무작위 Mini-Batch를 뽑아 DQN 학습
        if (m_replayBuffer.Size() >= AIConfig::BatchSize)
        {
            m_dqnAgent.Train(m_replayBuffer.Sample(AIConfig::BatchSize));
        }
    }

    // ==========================================
    // 경기별 Reward 집계
    // ==========================================

    const int roomId = bot.pendingGameRoomId;

    GameRewardStat& stat = m_gameRewardStats[roomId];

    const double episodeReward = m_botEpisodeRewards[bot.player_id];

    stat.rewardSum += episodeReward;
    ++stat.rewardCount;
    // 여기 테스트
    std::cout
        << "[GAME END COUNT]"
        << " Room=" << roomId
        << " Count=" << stat.rewardCount
        << "/" << result.player_count
        << " Bot=" << bot.index
        << "\n";



	// 경기 종료 후 봇별 누적 Reward 정보 제거
    m_botEpisodeRewards.erase(bot.player_id);

    // ==========================================
    // 해당 경기 모든 봇의 종료 처리가 완료된 경우
    // ==========================================

    if (stat.rewardCount >= result.player_count)
    {
        const double gameAverageReward =
            stat.rewardSum / static_cast<double>(stat.rewardCount);

        // 한 경기이므로 딱 한 번 증가
        ++m_modelInfo.totalGames;

        // 지금까지 모든 경기의 누적 평균 Reward
        m_modelInfo.averageReward =
            (m_modelInfo.averageReward * static_cast<double>(m_modelInfo.totalGames - 1) + gameAverageReward)
            / static_cast<double>(m_modelInfo.totalGames);

        // 가장 최신 DQN 학습 상태
        m_modelInfo.trainCount = m_dqnAgent.GetTrainCount();

        m_modelInfo.epsilon = m_dqnAgent.GetEpsilon();

        // 이번 경기 학습이 전부 끝난 최종 모델을 한 번 저장
        if (m_dqnAgent.SaveModel(m_modelInfo.modelPath))
        {
            // DB를 사용하는 학습 모드에서만 경기당 한번씩
            // ai_models 메타데이터 갱신
            if (m_dbThread != nullptr)
            {
                m_dbThread->PushModelUpdate(m_modelInfo);
            }
        }

        std::cout
            << "[GAME DB UPDATE]"
            << " Room=" << roomId
            << " GameAvgReward="
            << gameAverageReward
            << " TotalAvgReward="
            << m_modelInfo.averageReward
            << " TotalGames="
            << m_modelInfo.totalGames
            << " TrainCount="
            << m_modelInfo.trainCount
            << " Epsilon="
            << m_modelInfo.epsilon
            << "\n";

        // 이 경기 집계 정보 제거
        m_gameRewardStats.erase(roomId);
    }

    std::cout
        << "[GAME TERMINAL]"
        << " Bot=" << bot.index
        << " Reward=" << reward
        << " Winner="
        << static_cast<int>(result.winner_team)
        << " ReplaySize="
        << m_replayBuffer.Size()
        << "\n";

    bot.pendingAttackHits.store(0, std::memory_order_relaxed);

    bot.pendingSkillHits.store(0, std::memory_order_relaxed);

    // 다음 게임에 이전 State/Action이 이어지지 않도록 초기화
    bot.learning = BotLearningContext{};
}