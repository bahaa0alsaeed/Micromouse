#include "DFS.h"
#include "RobotState.h"

DFS::DFS(Maze& maze, INavigation& navigation, IWallSensors& wallSensors)
    : maze(maze), navigation(navigation), wallSensors(wallSensors)
{
}

void DFS::updateWalls()
{
    bool front = wallSensors.wallFront();
    bool right = wallSensors.wallRight();
    bool left = wallSensors.wallLeft();


    Direction frontDir = robotDir;
    Direction rightDir = rightOf(robotDir);
    Direction leftDir  = leftOf(robotDir);
    Direction backDir  = opposite(robotDir);

    if (front)
        maze.setWall(robotPos, frontDir);
    else
        maze.setOpen(robotPos, frontDir);

    if (right)
        maze.setWall(robotPos, rightDir);
    else
        maze.setOpen(robotPos, rightDir);

    if (left)
        maze.setWall(robotPos, leftDir);
    else
        maze.setOpen(robotPos, leftDir);

    /*
     * The rear direction is the path through which
     * the robot came, so it is guaranteed to be open.
     */
    maze.setOpen(robotPos, backDir);
}

void DFS::turnTO(Direction dir)
{
    if (robotDir == dir)
        return;

    if (rightOf(robotDir) == dir)
    {
        navigation.turnRight();
        robotDir = dir;
        return;
    }

    if (leftOf(robotDir) == dir)
    {
        navigation.turnLeft();
        robotDir = dir;
        return;
    }

    if (opposite(robotDir) == dir)
    {
        navigation.turnLeft();
        navigation.turnLeft();
        robotDir = dir;
        return;
    }
}

void DFS::explore()
{
    CellPosition newPos;

    maze.markVisited(robotPos);
    updateWalls();

    for (Direction dir : DIRECTIONS)
    {

        if (!maze.isKnown(robotPos, dir)) continue;
        if (maze.hasWall(robotPos, dir)) continue;

        newPos = nextPosition(robotPos, dir);
        if (!maze.isValid(newPos)) continue;
        if (maze.isVisited(newPos)) continue;

        turnTO(dir);
        navigation.moveForward(1);
        robotPos = newPos;

        explore();

        Direction backDir = opposite(dir);
        turnTO(backDir);
        robotPos = nextPosition(robotPos, robotDir);
        navigation.moveForward(1);

        updateWalls();
    }
}

