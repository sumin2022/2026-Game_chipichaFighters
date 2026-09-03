#pragma once

#include <vector>
#include "AIMapCollision.h"

struct AIPathNode
{
    int tileX{};
    int tileY{};

    float worldX{};
    float worldY{};
};

class AIPathFinder
{
public:
    AIPathFinder() = default;

    bool FindPath(
        float startWorldX,
        float startWorldY,
        float goalWorldX,
        float goalWorldY,
        std::vector<AIPathNode>& outPath
    ) const;

private:
    struct TilePosition
    {
        int x{};
        int y{};
    };

private:
    bool WorldToTile(
        float worldX,
        float worldY,
        int& tileX,
        int& tileY
    ) const;

    void TileToWorld(
        int tileX,
        int tileY,
        float& worldX,
        float& worldY
    ) const;

    bool IsValidTile(
        int tileX,
        int tileY
    ) const;

    bool IsBlockedTile(
        int tileX,
        int tileY
    ) const;

private:
    AIMapCollision m_mapCollision;
};