#pragma once
#include <array>
#include <cstdint>

enum class TileType : std::uint8_t {
	WALKABLE = 0,
	BLOCKED = 1
};

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

	// 내부 충돌 타일은 정확한 3000 × 2000 범위만 사용
	static constexpr float TILE_MAP_MIN_X = -1500.0f;
	static constexpr float TILE_MAP_MIN_Y = -1000.0f;

	static constexpr int TILE_SIZE = 50;

	static constexpr int MAP_COLS = 60; // X축, 가로
	static constexpr int MAP_ROWS = 40; // Y축, 세로

	using CollisionMap =
		std::array<std::array<TileType, MAP_COLS>, MAP_ROWS>;

	static bool world_to_tile(
		float worldX,
		float worldY,
		int& tileX,
		int& tileY
	);

	static CollisionMap create_collision_map();

	static const CollisionMap collision_map;
};
