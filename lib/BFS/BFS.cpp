#include "BFS.h"

BFS::BFS(Maze& maze, Navigation& navigation, WallSensors& wallSensors)
    : maze(maze), navigation(navigation), wallSensors(wallSensors)
{
}

void BFS::solve(const CellPosition& start, const CellPosition& goal)
{
}