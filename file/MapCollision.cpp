#include "MapCollision.h"
#include <algorithm>
#include <cmath>

bool MapCollision::is_inside_map(float x, float y)
{
	return x >= MAP_MIN_X &&
		x <= MAP_MAX_X &&
		y >= MAP_MIN_Y &&
		y <= MAP_MAX_Y;
}

void MapCollision::clamp_to_map(float& x, float& y)
{
	x = std::clamp(x, MAP_MIN_X, MAP_MAX_X);
	y = std::clamp(y, MAP_MIN_Y, MAP_MAX_Y);
}

bool MapCollision::is_walkable(float x, float y)
{
	// 전체 맵 경계를 벗어났는지 먼저 검사
	if (!is_inside_map(x, y))
		return false;

	// 내부 벽 타일이 아니라면 이동 가능
	return !is_blocked(x, y);
}

bool MapCollision::is_blocked(float x, float y)
{
	int tileX = 0;
	int tileY = 0;

	// 타일 맵 범위 바깥은 막힌 곳으로 처리
	if (!world_to_tile(x, y, tileX, tileY))
		return true;

	return collision_map[tileY][tileX] == TileType::BLOCKED;
}

bool MapCollision::can_see(float x1, float y1, float x2, float y2)
{
	return true;
}
bool MapCollision::ray_cast(float x1, float y1, float x2, float y2)
{
	return true;
}

const MapCollision::CollisionMap MapCollision::collision_map =
MapCollision::create_collision_map();

MapCollision::CollisionMap MapCollision::create_collision_map()
{
	CollisionMap map{};

	// 전체를 이동 가능으로 초기화
	for (auto& row : map) {
		row.fill(TileType::WALKABLE);
	}

	auto set_blocked_world_rect =
		[&](float centerX, float centerY, float sizeX, float sizeY)
		{
			const float minX = centerX - sizeX * 0.5f;
			const float maxX = centerX + sizeX * 0.5f;

			const float minY = centerY - sizeY * 0.5f;
			const float maxY = centerY + sizeY * 0.5f;

			int minTileX = static_cast<int>(
				std::floor((minX - TILE_MAP_MIN_X) / TILE_SIZE)
				);

			int maxTileX = static_cast<int>(
				std::floor((maxX - 0.001f - TILE_MAP_MIN_X) / TILE_SIZE)
				);

			int minTileY = static_cast<int>(
				std::floor((minY - TILE_MAP_MIN_Y) / TILE_SIZE)
				);

			int maxTileY = static_cast<int>(
				std::floor((maxY - 0.001f - TILE_MAP_MIN_Y) / TILE_SIZE)
				);

			minTileX = std::clamp(minTileX, 0, MAP_COLS - 1);
			maxTileX = std::clamp(maxTileX, 0, MAP_COLS - 1);
			minTileY = std::clamp(minTileY, 0, MAP_ROWS - 1);
			maxTileY = std::clamp(maxTileY, 0, MAP_ROWS - 1);

			for (int y = minTileY; y <= maxTileY; ++y) {
				for (int x = minTileX; x <= maxTileX; ++x) {
					map[y][x] = TileType::BLOCKED;
				}
			}
		};

	// 중앙 구조물
	set_blocked_world_rect(360.0f, -800.0f, 60.0f, 400.0f);
	set_blocked_world_rect(300.0f, -600.0f, 200.0f, 60.0f);

	set_blocked_world_rect(-360.0f, -800.0f, 60.0f, 400.0f);
	set_blocked_world_rect(-300.0f, -600.0f, 200.0f, 60.0f);

	set_blocked_world_rect(360.0f, 800.0f, 60.0f, 400.0f);
	set_blocked_world_rect(300.0f, 600.0f, 200.0f, 60.0f);

	set_blocked_world_rect(-360.0f, 800.0f, 60.0f, 400.0f);
	set_blocked_world_rect(-300.0f, 600.0f, 200.0f, 60.0f);

	// X 음수 쪽 리스폰 구조물
	set_blocked_world_rect(-900.0f, -450.0f, 60.0f, 500.0f);
	set_blocked_world_rect(-900.0f, 450.0f, 60.0f, 500.0f);
	set_blocked_world_rect(-1170.0f, 680.0f, 500.0f, 60.0f);
	set_blocked_world_rect(-1450.0f, 450.0f, 60.0f, 500.0f);
	set_blocked_world_rect(-1450.0f, -450.0f, 60.0f, 500.0f);
	set_blocked_world_rect(-1170.0f, -680.0f, 500.0f, 60.0f);

	// X 양수 쪽 리스폰 구조물
	set_blocked_world_rect(900.0f, -450.0f, 60.0f, 500.0f);
	set_blocked_world_rect(900.0f, 450.0f, 60.0f, 500.0f);
	set_blocked_world_rect(1170.0f, 680.0f, 500.0f, 60.0f);
	set_blocked_world_rect(1450.0f, 450.0f, 60.0f, 500.0f);
	set_blocked_world_rect(1450.0f, -450.0f, 60.0f, 500.0f);
	set_blocked_world_rect(1170.0f, -680.0f, 500.0f, 60.0f);

	return map;
}

bool MapCollision::world_to_tile(
	float worldX,
	float worldY,
	int& tileX,
	int& tileY)
{
	// 내부 타일 맵 범위는 정확히 3000 × 2000
	constexpr float TILE_MAP_MAX_X =
		TILE_MAP_MIN_X + MAP_WIDTH;

	constexpr float TILE_MAP_MAX_Y =
		TILE_MAP_MIN_Y + MAP_HEIGHT;

	if (worldX < TILE_MAP_MIN_X ||
		worldX >= TILE_MAP_MAX_X ||
		worldY < TILE_MAP_MIN_Y ||
		worldY >= TILE_MAP_MAX_Y) {
		return false;
	}

	tileX = static_cast<int>(
		(worldX - TILE_MAP_MIN_X) / TILE_SIZE
		);

	tileY = static_cast<int>(
		(worldY - TILE_MAP_MIN_Y) / TILE_SIZE
		);

	return tileX >= 0 &&
		tileX < MAP_COLS &&
		tileY >= 0 &&
		tileY < MAP_ROWS;
}
