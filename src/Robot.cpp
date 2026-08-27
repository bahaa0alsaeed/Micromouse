#include "Robot.h"
#include "Config.h"

Robot::Robot()
    : driver(Config::driver),
      leftEncoder(Config::leftEncoder),
      rightEncoder(Config::rightEncoder),
      wallSensors(Config::wallSensors),
      positionPID(Config::positionPID),
      leftSpeedPID(Config::leftSpeedPID),
      rightSpeedPID(Config::rightSpeedPID),
      navigation(
          driver,
          wallSensors,
          leftEncoder,
          rightEncoder,
          positionPID,
          leftSpeedPID,
          rightSpeedPID,
          Config::navigation
      ),
      dfs(
          maze,
          navigation,
          wallSensors
      ),
      bfs(
          maze,
          navigation,
          wallSensors
      )
{
}

void Robot::explore()
{
}

void Robot::solve(const CellPosition& start, const CellPosition& goal)
{
}

void Robot::debug()
{
}

Maze& Robot::getMaze()
{
}