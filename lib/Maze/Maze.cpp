#include "Maze.h"

Maze::Maze()
{
}

bool Maze::isValid(const CellPosition& position) const
{
    return false;
}

bool Maze::hasWall(const CellPosition& position, Direction direction) const
{
    return false;
}

void Maze::setWall(const CellPosition& position, Direction direction)
{
}

bool Maze::isKnown(const CellPosition& position, Direction direction) const
{
    return false;
}

void Maze::setOpen(const CellPosition& position, Direction direction)
{
}

bool Maze::isVisited(const CellPosition& position) const
{
    return false;
}

void Maze::markVisited(const CellPosition& position)
{
}

unsigned int Maze::getCost(const CellPosition& position) const
{
    return 0;
}

void Maze::setCost(const CellPosition& position, unsigned int cost)
{
}

void Maze::reset()
{
}

Direction Maze::opposite(Direction direction) const
{
    return NORTH;
}