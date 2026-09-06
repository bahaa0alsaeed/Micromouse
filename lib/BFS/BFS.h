#pragma once
#include <vector>
#include "Maze.h"
#include "INavigation.h"
#include "IWallSensors.h"

class BFS
{
    Maze& maze;
    INavigation& navigation;
    IWallSensors& wallSensors;

    // Stores the path found by BFS from start to goal.
    std::vector<CellPosition> path;

    // Rotate the robot to face the specified direction.
    void turnTO(Direction dir);

    // Return the neighboring cell in the specified direction.
    CellPosition newCell(const CellPosition& pos, const Direction& dir);

public:
    BFS(Maze& maze, INavigation& navigation, IWallSensors& wallSensors);

    // Find a path from start to goal using Breadth-First Search.
    void solve(const CellPosition& start, const CellPosition& goal);

    // Execute the calculated path using the navigation system.
    void executePath();

    // Return the calculated path.
    const std::vector<CellPosition>& getPath() const { return path; }
};
