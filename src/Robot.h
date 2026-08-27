#pragma once
#include "DRV8833.h"
#include "Encoder.h"
#include "WallSensors.h"
#include "PID.h"
#include "Navigation.h"
#include "DFS.h"
#include "BFS.h"
#include "Maze.h"


class Robot
{
    DRV8833 driver;
    Encoder leftEncoder;
    Encoder rightEncoder;
    WallSensors wallSensors;
    PID positionPID;
    PID leftSpeedPID;
    PID rightSpeedPID;
    Navigation navigation;
    DFS dfs;
    BFS bfs;
    Maze maze;

public:
    Robot();
    void explore();
    void solve(const CellPosition& start,
               const CellPosition& goal);
    void debug();
    Maze& getMaze();
};

