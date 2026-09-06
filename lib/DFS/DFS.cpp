#include "DFS.h"
#include "RobotState.h"

DFS::DFS(Maze& maze, INavigation& navigation, IWallSensors& wallSensors)
    : maze(maze), navigation(navigation), wallSensors(wallSensors)
{
}

void DFS::updateWalls()
{
    // Read the walls relative to the robot's current orientation.
    bool front = wallSensors.wallFront();
    bool right = wallSensors.wallRight();
    bool left = wallSensors.wallLeft();


    // Convert relative sensor directions into absolute maze directions.
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

    // Turn around when the target direction is opposite to the current one.
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

    // Mark the current cell as visited and discover its walls.
    maze.markVisited(robotPos);
    updateWalls();

    // Try every direction from the current cell.
    for (Direction dir : DIRECTIONS)
    {
        if (!maze.isKnown(robotPos, dir)) continue;
        if (maze.hasWall(robotPos, dir)) continue;

        newPos = nextPosition(robotPos, dir);

        if (!maze.isValid(newPos)) continue;
        if (maze.isVisited(newPos)) continue;

        // Move to the unvisited neighboring cell.
        turnTO(dir);
        navigation.moveForward(1);
        robotPos = newPos;

        // Recursively explore the new cell.
        explore();

        /*
         * Backtrack to the previous cell after finishing
         * the exploration of this branch.
         */
        Direction backDir = opposite(dir);
        turnTO(backDir);
        robotPos = nextPosition(robotPos, robotDir);
        navigation.moveForward(1);

        // Update the maze again after returning to the cell.
        updateWalls();
    }
}
