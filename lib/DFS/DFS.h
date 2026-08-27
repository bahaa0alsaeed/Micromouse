#pragma once
#include "Maze.h"
#include "Navigation.h"
#include "WallSensors.h"

class DFS
{
    Maze& maze;
    Navigation& navigation;
    WallSensors& wallSensors;

public:
    DFS(Maze& maze, Navigation& navigation, WallSensors& wallSensors);
    void explore();
};
