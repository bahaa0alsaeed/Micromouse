#pragma once
#include "Maze.h"
#include "Navigation.h"
#include "WallSensors.h"

class DFS
{
    Maze& maze;
    Navigation& navigation;
    WallSensors& wallSensors;

    CellPosition robotPos = {0,0};
    Direction robotDir = NORTH;

    void updateWalls();
    void turnTO(Direction dir);

public:
    DFS(Maze& maze, Navigation& navigation, WallSensors& wallSensors);
    void explore();
};
