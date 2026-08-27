#pragma once
#include "Maze.h"
#include "Navigation.h"
#include "WallSensors.h"

class BFS
{
    Maze& maze;
    Navigation& navigation;
    WallSensors& wallSensors;

public:
    BFS(Maze& maze, Navigation& navigation, WallSensors& wallSensors);

    void solve(const CellPosition& start, const CellPosition& goal);
};
