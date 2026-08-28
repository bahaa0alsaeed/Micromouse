#pragma once
#include <cstdint>

enum Direction
{
    NORTH = 1,
    EAST  = 2,
    SOUTH = 4,
    WEST  = 8
};

struct CellPosition
{
    unsigned int x;
    unsigned int y;
};

struct Cell
{
    uint8_t walls = 0;
    uint8_t known = 0;
    bool visited = false;
    uint8_t cost = 0;
};

class Maze
{
    static constexpr unsigned int WIDTH  = 16;
    static constexpr unsigned int HEIGHT = 16;
    Cell cells[WIDTH][HEIGHT]{};

public:
    Maze();

    [[nodiscard]] bool isValid(const CellPosition& pos) const;

    [[nodiscard]] bool hasWall(const CellPosition& pos, Direction dir) const;

    void setWall(const CellPosition& pos, Direction dir);

    [[nodiscard]] bool isKnown(const CellPosition& pos, Direction dir) const;

    void setOpen(const CellPosition& pos, Direction dir);

    [[nodiscard]] bool isVisited(const CellPosition& pos) const;

    void markVisited(const CellPosition& pos);

    [[nodiscard]] unsigned int getCost(const CellPosition& pos) const;

    void setCost(const CellPosition& pos, uint8_t cost);

    void reset();

};

