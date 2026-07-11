#pragma once

class MapCollision {
public:
	
	static bool is_inside_map(float x, float y);  // 월드 좌표가 맵 안쪽 범위인지 검사
	static bool is_blocked(float x, float y);     // 벽인지
	static bool is_walkable(float x, float y);   // 해당 위치로 이동할 수 있는지 검사

	// 월드 좌표를 맵 경계 안으로 제한
	static void clamp_to_map(float& x, float& y);

	static bool can_see(float x1, float y1, float x2, float y2);      // 시야 판정(벽 너머 공격 불가)
	static bool ray_cast(float x1, float y1, float x2, float y2);     // 투사체 충돌용

private:
	static constexpr float MAP_WIDTH = 3000.0f;
	static constexpr float MAP_HEIGHT = 2000.0f;
	static constexpr float MAP_BOUNDARY_MARGIN = 50.0f; // 맵 경계에서 플레이어가 벽에 붙는 것을 방지하기 위한 여유 공간

	static constexpr float MAP_MIN_X =
		-(MAP_WIDTH * 0.5f) - MAP_BOUNDARY_MARGIN;

	static constexpr float MAP_MAX_X =
		(MAP_WIDTH * 0.5f) + MAP_BOUNDARY_MARGIN;

	static constexpr float MAP_MIN_Y =
		-(MAP_HEIGHT * 0.5f) - MAP_BOUNDARY_MARGIN;

	static constexpr float MAP_MAX_Y =
		(MAP_HEIGHT * 0.5f) + MAP_BOUNDARY_MARGIN;
};