#pragma once

enum Direction
{
    NORTH = 1,
    EAST  = 2,
    SOUTH = 4,
    WEST  = 8
};

inline constexpr Direction DIRECTIONS[] =
{
    NORTH,
    EAST,
    SOUTH,
    WEST
};

struct CellPosition
{
    int x = 0;
    int y = 0;
};


inline Direction opposite(const Direction dir)
{
    switch (dir)
    {
    case NORTH: return SOUTH;
    case EAST:  return WEST;
    case SOUTH: return NORTH;
    case WEST:  return EAST;
    }

    return NORTH;
}

inline Direction rightOf(const Direction dir)
{
    switch (dir)
    {
    case NORTH: return EAST;
    case EAST:  return SOUTH;
    case SOUTH: return WEST;
    case WEST:  return NORTH;
    }

    return NORTH;
}

inline Direction leftOf(const Direction dir)
{
    switch (dir)
    {
    case NORTH: return WEST;
    case WEST:  return SOUTH;
    case SOUTH: return EAST;
    case EAST:  return NORTH;
    }

    return NORTH;
}

inline CellPosition nextPosition(const CellPosition& pos, const Direction& dir)
{
    switch (dir)
    {
    case NORTH:
        return {pos.x, pos.y + 1};

    case EAST:
        return {pos.x + 1, pos.y};

    case SOUTH:
        return {pos.x, pos.y - 1};

    case WEST:
        return {pos.x - 1, pos.y};
    }

    return pos;
}
