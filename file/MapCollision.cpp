#include "MapCollision.h"
#include <algorithm>

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
	// 우선은 맵 경계만 검사
	if (!is_inside_map(x, y))
		return false;

	// 나중에 타일 충돌 검사 추가
	// int tileX = ...
	// int tileY = ...
	// return collisionMap[tileY][tileX] == 0;

	return true;
}

// 나중에 타일 충돌 검사 추가
bool MapCollision::is_blocked(float x, float y)
{
	return false;
}

bool MapCollision::can_see(float x1, float y1, float x2, float y2)
{
	return true;
}
bool MapCollision::ray_cast(float x1, float y1, float x2, float y2)
{
	return true;
}
