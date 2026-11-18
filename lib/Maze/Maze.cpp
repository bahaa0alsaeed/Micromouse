#include "Maze.h"

Maze::Maze()
{
    reset();
}

bool Maze::isValid(const CellPosition& pos) const
{
    // Check whether the position is inside the maze boundaries.
    return (pos.x >= 0 && pos.x < WIDTH && pos.y >= 0 && pos.y < HEIGHT);
}

bool Maze::hasWall(const CellPosition& pos, const Direction dir) const
{
    if (!isValid(pos)) return false;

    // Check if the specified wall exists in this cell.
    return (cells[pos.x][pos.y].walls & dir) != 0;
}

void Maze::setWall(const CellPosition& pos, const Direction dir)
{
    if (!isValid(pos)) return;//0

    // Store the wall and mark it as known.
    cells[pos.x][pos.y].walls |= dir;
    cells[pos.x][pos.y].known |= dir;

    // A wall is shared by two adjacent cells, so update both sides.
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
        default: return;    
    }
}

bool Maze::isKnown(const CellPosition& pos, const Direction dir) const
{
    if (!isValid(pos)) return false;

    // Check whether this wall has already been discovered.
    return (cells[pos.x][pos.y].known & dir) != 0;
}

void Maze::setOpen(const CellPosition& pos, const Direction dir)
{
    if (!isValid(pos)) return;

    // Clear the wall and mark the direction as known in the current cell.
    cells[pos.x][pos.y].walls &= ~dir;
    cells[pos.x][pos.y].known |= dir;

    // Clear the shared wall and mark the side as known in the neighboring cell.
    switch (dir)
    {
    case NORTH:
        if (pos.y + 1 < HEIGHT) {
            cells[pos.x][pos.y + 1].walls &= ~SOUTH;
            cells[pos.x][pos.y + 1].known |= SOUTH;
        }
        break;

    case SOUTH:
        if (pos.y > 0) {
            cells[pos.x][pos.y - 1].walls &= ~NORTH;
            cells[pos.x][pos.y - 1].known |= NORTH;
        }
        break;

    case EAST:
        if (pos.x + 1 < WIDTH) {
            cells[pos.x + 1][pos.y].walls &= ~WEST;
            cells[pos.x + 1][pos.y].known |= WEST;
        }
        break;

    case WEST:
        if (pos.x > 0) {
            cells[pos.x - 1][pos.y].walls &= ~EAST;
            cells[pos.x - 1][pos.y].known |= EAST;
        }
        break;
    default: return;    
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

uint8_t Maze::getCost(const CellPosition& pos) const
{
    if (!isValid(pos)) return UINT8_MAX; 

    return cells[pos.x][pos.y].cost;
 }

void Maze::setCost(const CellPosition& pos, const uint8_t cost)
{
    if (isValid(pos)) cells[pos.x][pos.y].cost = cost;
}

void Maze::reset()
{
    // Clear all cell data and restore the default state.
    for (unsigned int y = 0; y < HEIGHT; y++)
    {
        for (unsigned int x = 0; x < WIDTH; x++)
        {
            cells[x][y] = Cell{};
        }
    }

    // Add the outer boundary walls.
    
for (unsigned int y = 0; y < HEIGHT; y++)
{
    cells[0][y].walls |= WEST;
    cells[0][y].known |= WEST;
    cells[WIDTH - 1][y].walls |= EAST;
    cells[WIDTH - 1][y].known |= EAST;
}

for (unsigned int x = 0; x < WIDTH; x++)
{
    cells[x][0].walls |= SOUTH;
    cells[x][0].known |= SOUTH;
    cells[x][HEIGHT - 1].walls |= NORTH;
    cells[x][HEIGHT - 1].known |= NORTH;
}
}