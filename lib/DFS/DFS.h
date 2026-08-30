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


    void updateWalls();
    void turnTO(Direction dir);

public:
    DFS(Maze& maze, INavigation& navigation, IWallSensors& wallSensors);
    void explore();

};
