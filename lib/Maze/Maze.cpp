#include "Maze.h"

Maze::Maze()
{
 reset();
}


bool Maze::isValid(const CellPosition& pos) const
{
    return (pos.x >= 0 && pos.x < WIDTH && pos.y >= 0 && pos.y < HEIGHT);
}

bool Maze::hasWall(const CellPosition& pos, const Direction dir) const
{
    if (!isValid(pos)) return false;

    return (cells[pos.x][pos.y].walls & dir) != 0;
}

void Maze::setWall(const CellPosition& pos, const Direction dir)
{
    if (!isValid(pos)) return;

    cells[pos.x][pos.y].walls |= dir;
    cells[pos.x][pos.y].known |= dir;

    switch (dir)
    {
        case NORTH:
            if (pos.y + 1 < HEIGHT) cells[pos.x][pos.y + 1].walls |= SOUTH;
            if (pos.y + 1 < HEIGHT) cells[pos.x][pos.y + 1].known |= SOUTH;
            break;
        case SOUTH:
            if (pos.y > 0) cells[pos.x][pos.y - 1].walls |= NORTH;
            if (pos.y > 0) cells[pos.x][pos.y - 1].known |= NORTH;
            break;
        case EAST:
            if (pos.x + 1 < WIDTH) cells[pos.x + 1][pos.y].walls |= WEST;
            if (pos.x + 1 < WIDTH) cells[pos.x + 1][pos.y].known |= WEST;
            break;
        case WEST:
            if (pos.x > 0) cells[pos.x - 1][pos.y].walls |= EAST;
            if (pos.x > 0) cells[pos.x - 1][pos.y].known |= EAST;
            break;
    }
}

bool Maze::isKnown(const CellPosition& pos, const Direction dir) const
{
    if (!isValid(pos)) return false;

    return (cells[pos.x][pos.y].known & dir) != 0;
}

void Maze::setOpen(const CellPosition& pos, const Direction dir)
{
    if (!isValid(pos)) return;

    cells[pos.x][pos.y].known |= dir;

    switch (dir)
    {
    case NORTH:
        if (pos.y + 1 < HEIGHT) cells[pos.x][pos.y + 1].known |= SOUTH;
        break;
    case SOUTH:
        if (pos.y > 0) cells[pos.x][pos.y - 1].known |= NORTH;
        break;
    case EAST:
        if (pos.x + 1 < WIDTH) cells[pos.x + 1][pos.y].known |= WEST;
        break;
    case WEST:
        if (pos.x > 0) cells[pos.x - 1][pos.y].known |= EAST;
        break;
    }
}

bool Maze::isVisited(const CellPosition& pos) const
{
    if (isValid(pos)) return (cells[pos.x][pos.y].visited);

    return false;
}

void Maze::markVisited(const CellPosition& pos)
{
    if (isValid(pos)) cells[pos.x][pos.y].visited = true;
}

unsigned int Maze::getCost(const CellPosition& pos) const
{
    if (!isValid(pos)) return 0;

    return cells[pos.x][pos.y].cost;
}

void Maze::setCost(const CellPosition& pos, const uint8_t cost)
{
    if (isValid(pos)) cells[pos.x][pos.y].cost = cost;
}

void Maze::reset()
{
    for (unsigned int y = 0; y < HEIGHT; y++)
    {
        for (unsigned int x = 0; x < WIDTH; x++)
        {
            cells[x][y] = Cell{};
        }
    }

    for (unsigned int y = 0; y < HEIGHT; y++)
    {
        cells[0][y].walls |= WEST;
        cells[WIDTH - 1][y].walls |= EAST;
    }
    for (unsigned int x = 0; x < WIDTH; x++)
    {
        cells[x][0].walls |= SOUTH;
        cells[x][HEIGHT - 1].walls |= NORTH;
    }
}

