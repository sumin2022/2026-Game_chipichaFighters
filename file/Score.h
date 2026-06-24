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
  float game_timer = 150.0f; // 2분 30초
  int red_score = 0;
  int blue_score = 0;

  CaptureZone zone;
};