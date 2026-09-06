#include "BFS.h"
#include <algorithm>
#include "RobotState.h"
#include "queue"

BFS::BFS(Maze& maze, INavigation& navigation, IWallSensors& wallSensors)
    : maze(maze), navigation(navigation), wallSensors(wallSensors)
{
}

void BFS::turnTO(const Direction dir)
{
    if (robotDir == dir)
        return;

    // Turn right when the target direction is immediately to the right.
    if (rightOf(robotDir) == dir)
    {
        navigation.turnRight();
        robotDir = dir;
        return;
    }

    // Turn left when the target direction is immediately to the left.
    if (leftOf(robotDir) == dir)
    {
        navigation.turnLeft();
        robotDir = dir;
        return;
    }

    // A 180-degree turn requires two left turns.
    if (opposite(robotDir) == dir)
    {
        navigation.turnLeft();
        navigation.turnLeft();
        robotDir = dir;
        return;
    }
}

CellPosition BFS::newCell(const CellPosition& pos, const Direction& dir)
{
    int x = 0;
    int y = 0;

    // Calculate the neighboring cell in the given direction.
    if (dir == NORTH) { x = pos.x;     y = pos.y + 1; }
    if (dir == EAST)  { x = pos.x + 1; y = pos.y; }
    if (dir == SOUTH) { x = pos.x;     y = pos.y - 1; }
    if (dir == WEST)  { x = pos.x - 1; y = pos.y; }

    return {x, y};
}

void BFS::solve(const CellPosition& start, const CellPosition& goal)
{
    CellPosition parent[16][16] = {};
    bool hasParent[16][16] = {};
    bool visited[16][16] = {};

    std::queue<CellPosition> queue;

    visited[start.x][start.y] = true;
    queue.push(start);

    while (!queue.empty())
    {
        CellPosition pos = queue.front();
        queue.pop();

        if (pos == goal)
            break;

        // Explore all reachable neighboring cells.
        for (Direction dir : DIRECTIONS)
        {
            if (maze.hasWall(pos, dir))
                continue;

            CellPosition next = newCell(pos, dir);

            if (visited[next.x][next.y])
                continue;

            visited[next.x][next.y] = true;

            // Store the predecessor to reconstruct the path later.
            parent[next.x][next.y] = pos;
            hasParent[next.x][next.y] = true;

            queue.push(next);
        }
    }

    // No path exists if the goal was never reached.
    if (start != goal && !hasParent[goal.x][goal.y])
        return;

    path.clear();

    CellPosition pos = goal;

    // Follow parent links from the goal back to the start.
    while (pos != start)
    {
        path.push_back(pos);
        pos = parent[pos.x][pos.y];
    }

    path.push_back(start);

    // Parent links were followed backwards, so reverse the path.
    std::reverse(path.begin(), path.end());
}

void BFS::executePath()
{
    if (path.size() < 2)
        return;

    for (size_t i = 0; i + 1 < path.size(); ++i)
    {
        const CellPosition cur  = path[i];
        const CellPosition next = path[i + 1];

        const int dx = next.x - cur.x;
        const int dy = next.y - cur.y;

        // Every consecutive pair must represent an adjacent cell.
        if (std::abs(dx) + std::abs(dy) != 1)
            return;

        Direction dir = NORTH;

        if      (dx == 1)  dir = EAST;
        else if (dx == -1) dir = WEST;
        else if (dy == 1)  dir = NORTH;
        else if (dy == -1) dir = SOUTH;

        // Face the direction of the next cell, then move one cell forward.
        turnTO(dir);
        navigation.moveForward(1);

        robotPos = next;
    }
}
