#include "AIPathFinder.h"

#include <queue>
#include <array>
#include <algorithm>
#include <cmath>

bool AIPathFinder::WorldToTile(
    float worldX,
    float worldY,
    int& tileX,
    int& tileY
) const
{
    tileX = static_cast<int>(
        std::floor(
            (worldX - AIMapCollision::TILE_MAP_MIN_X)
            / AIMapCollision::TILE_SIZE));

    tileY = static_cast<int>(
        std::floor(
            (worldY - AIMapCollision::TILE_MAP_MIN_Y)
            / AIMapCollision::TILE_SIZE));

    return IsValidTile(tileX, tileY);
}

void AIPathFinder::TileToWorld(
    int tileX,
    int tileY,
    float& worldX,
    float& worldY
) const
{
    // 타일의 중앙 좌표
    worldX =
        AIMapCollision::TILE_MAP_MIN_X
        + tileX * AIMapCollision::TILE_SIZE
        + AIMapCollision::TILE_SIZE * 0.5f;

    worldY =
        AIMapCollision::TILE_MAP_MIN_Y
        + tileY * AIMapCollision::TILE_SIZE
        + AIMapCollision::TILE_SIZE * 0.5f;
}

bool AIPathFinder::IsValidTile(int tileX,int tileY) const
{
    return
        tileX >= 0 && tileX < AIMapCollision::MAP_COLS &&
        tileY >= 0 && tileY < AIMapCollision::MAP_ROWS;
}

bool AIPathFinder::IsBlockedTile(int tileX,int tileY) const
{
    if (!IsValidTile(tileX, tileY))
        return true;

    float worldX = 0.0f;
    float worldY = 0.0f;

    TileToWorld(
        tileX,
        tileY,
        worldX,
        worldY
    );

    return m_mapCollision.IsBlocked(worldX,worldY);
}

bool AIPathFinder::FindPath(
    float startWorldX,
    float startWorldY,
    float goalWorldX,
    float goalWorldY,
    std::vector<AIPathNode>& outPath
) const
{
    outPath.clear();

    int startX = 0;
    int startY = 0;

    int goalX = 0;
    int goalY = 0;

    if (!WorldToTile(
        startWorldX,
        startWorldY,
        startX,
        startY))
    {
        return false;
    }

    if (!WorldToTile(
        goalWorldX,
        goalWorldY,
        goalX,
        goalY))
    {
        return false;
    }

    if (IsBlockedTile(startX, startY))
        return false;

    if (IsBlockedTile(goalX, goalY))
        return false;

    if (startX == goalX &&
        startY == goalY)
    {
        return true;
    }

    struct Parent
    {
        int x{ -1 };
        int y{ -1 };
    };

    std::array<
        std::array<bool, AIMapCollision::MAP_COLS>,
        AIMapCollision::MAP_ROWS> visited{};

    std::array<
        std::array<Parent, AIMapCollision::MAP_COLS>,
        AIMapCollision::MAP_ROWS> parent{};

    std::queue<TilePosition> open;

    visited[startY][startX] = true;

    open.push({
        startX,
        startY
        });

    // 8방향
    static constexpr int directions[8][2] =
    {
        {  0,  1 }, // Up
        {  0, -1 }, // Down
        { -1,  0 }, // Left
        {  1,  0 }, // Right

        { -1,  1 }, // UpLeft
        {  1,  1 }, // UpRight
        { -1, -1 }, // DownLeft
        {  1, -1 }  // DownRight
    };

    bool pathFound = false;

    while (!open.empty())
    {
        TilePosition current = open.front();
        open.pop();

        if (current.x == goalX && current.y == goalY)
        {
            pathFound = true;
            break;
        }

        for (const auto& dir : directions)
        {
            const int nextX = current.x + dir[0];

            const int nextY = current.y + dir[1];

            if (!IsValidTile(nextX, nextY))
                continue;

            if (visited[nextY][nextX])
                continue;

            if (IsBlockedTile(nextX, nextY))
                continue;

            // -----------------------------
            // 대각선 이동 시 모서리 뚫기 방지
            // -----------------------------
            const bool diagonal = dir[0] != 0 && dir[1] != 0;

            if (diagonal)
            {
                const int sideX1 = current.x + dir[0];

                const int sideY1 = current.y;

                const int sideX2 = current.x;

                const int sideY2 = current.y + dir[1];

                if (IsBlockedTile(sideX1,sideY1) ||IsBlockedTile(sideX2,sideY2))
                {
                    continue;
                }
            }

            visited[nextY][nextX] = true;

            parent[nextY][nextX] = { current.x,current.y };

            open.push({ nextX,nextY });
        }
    }

    if (!pathFound)
        return false;

    // -----------------------------
    // Goal -> Start 역추적
    // -----------------------------

    int currentX = goalX;
    int currentY = goalY;

    while (!(currentX == startX && currentY == startY))
    {
        float worldX = 0.0f;
        float worldY = 0.0f;

        TileToWorld(
            currentX,
            currentY,
            worldX,
            worldY
        );

        AIPathNode node{};

        node.tileX = currentX;
        node.tileY = currentY;

        node.worldX = worldX;
        node.worldY = worldY;

        outPath.push_back(node);

        const Parent& p = parent[currentY][currentX];

        if (p.x == -1 || p.y == -1)
        {
            outPath.clear();
            return false;
        }

        currentX = p.x;
        currentY = p.y;
    }

    // 현재는 Goal -> Start 순서이므로 뒤집기
    std::reverse(outPath.begin(), outPath.end());

    return true;
}