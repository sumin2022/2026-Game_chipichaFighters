#include "AIMapCollision.h"
#include <cmath>
#include <algorithm>

AIMapCollision::AIMapCollision()
    : m_map(CreateCollisionMap())
{
}

bool AIMapCollision::IsBlocked(float worldX, float worldY) const
{
    const int tileX = static_cast<int>(
        std::floor((worldX - TILE_MAP_MIN_X) / TILE_SIZE));

    const int tileY = static_cast<int>(
        std::floor((worldY - TILE_MAP_MIN_Y) / TILE_SIZE));

    if (tileX < 0 || tileX >= MAP_COLS || tileY < 0 || tileY >= MAP_ROWS)
    {
        return true;
    }

    return m_map[tileY][tileX] == TileType::BLOCKED;
}

AIMapCollision::CollisionMap AIMapCollision::CreateCollisionMap()
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

// 선분과 차단 타일의 AABB 교차 검사. 코너에 닿는 선도 보수적으로 차단합니다.
bool AIMapCollision::HasLineOfSight(float x0, float y0, float x1, float y1) const
{
    if (!std::isfinite(x0) || !std::isfinite(y0) || !std::isfinite(x1) || !std::isfinite(y1)) return false;
    if (IsBlocked(x0,y0) || IsBlocked(x1,y1)) return false;
    const float dx=x1-x0, dy=y1-y0;
    for(int y=0;y<MAP_ROWS;++y) for(int x=0;x<MAP_COLS;++x) {
        if(m_map[y][x] != TileType::BLOCKED) continue;
        const float left=TILE_MAP_MIN_X+x*TILE_SIZE, bottom=TILE_MAP_MIN_Y+y*TILE_SIZE;
        float lo=0, hi=1;
        auto slab=[&](float start,float delta,float a,float b) {
            if(std::abs(delta)<1e-8f) return start>=a && start<=b;
            float t0=(a-start)/delta, t1=(b-start)/delta;
            if(t0>t1) std::swap(t0,t1);
            lo=std::max(lo,t0); hi=std::min(hi,t1);
            return lo<=hi;
        };
        if(slab(x0,dx,left,left+TILE_SIZE) && slab(y0,dy,bottom,bottom+TILE_SIZE)) return false;
    }
    return true;
}
