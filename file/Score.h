#pragma once
#include "protocol.h"

struct Room;

struct TeamScore {
  int red_score = 0;
  int blue_score = 0;
};

struct CaptureZone {
  float min_x = 0.0f;
  float max_x = 0.0f;
  float min_y = 0.0f;
  float max_y = 0.0f;
};

class ScoreManager {
public:
  void start_game();
  void update(Room &room, float dt);

  bool is_time_over() const;
  void make_result(Room &room, SC_GameResult &packet);

  int get_red_score() const { return static_cast<int>(red_score); }
  int get_blue_score() const { return static_cast<int>(blue_score); }
  float get_time_left() const { return game_timer; }

private:
	static constexpr float GAME_DURATION = 15.0f; // 테스트용
	float game_timer = GAME_DURATION;
    float red_score = 0;
    float blue_score = 0;

    CaptureZone zone{ //점령지 위치
    -310.0f,  // min_x = centerX - 300
     290.0f,  // max_x = centerX + 300
    -300.0f,  // min_y = centerY - 300
     300.0f   // max_y = centerY + 300
    };

	bool is_in_capture_zone(float x, float y) const; // 점령지 안에 있는지 확인
	float get_capture_multiplier(int count) const;  // 점령지 안에 있는 플레이어 수에 따른 점수 배율 계산

};
