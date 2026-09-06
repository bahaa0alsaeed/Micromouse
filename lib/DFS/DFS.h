#pragma once
#include "Maze.h"
#include "INavigation.h"
#include "IWallSensors.h"
#include "Directions.h"

class DFS
{
    Maze& maze;
    INavigation& navigation;
    IWallSensors& wallSensors;

    // Read the current wall configuration from the robot's sensors.
    void updateWalls();

    // Rotate the robot to face the specified direction.
    void turnTO(Direction dir);

public:
    DFS(Maze& maze, INavigation& navigation, IWallSensors& wallSensors);

    // Explore the maze using Depth-First Search with recursive backtracking.
    void explore();
};
