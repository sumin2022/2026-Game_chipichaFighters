#pragma once

#include <array>

class AIMapCollision
{
public:
    static constexpr int MAP_COLS = 60;
    static constexpr int MAP_ROWS = 40;

    static constexpr float TILE_SIZE = 50.0f;
    static constexpr float TILE_MAP_MIN_X = -1500.0f;
    static constexpr float TILE_MAP_MIN_Y = -1000.0f;

    enum class TileType
    {
        WALKABLE,
        BLOCKED
    };

    using CollisionMap =
        std::array<std::array<TileType, MAP_COLS>, MAP_ROWS>;

public:
    AIMapCollision();

    bool IsBlocked(float worldX, float worldY) const;

private:
    CollisionMap CreateCollisionMap();

private:
    CollisionMap m_map;
};