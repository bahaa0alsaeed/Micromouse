#pragma once

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
    unsigned int walls;
    unsigned int known;
    bool visited;
    unsigned int cost;
};

class Maze
{
    static constexpr unsigned int WIDTH  = 16;
    static constexpr unsigned int HEIGHT = 16;
    Cell cells[HEIGHT][WIDTH]{};

    [[nodiscard]] Direction opposite(Direction direction) const;

public:
    Maze();

    [[nodiscard]] bool isValid(const CellPosition& position) const;

    [[nodiscard]] bool hasWall(const CellPosition& position, Direction direction) const;

    void setWall(const CellPosition& position, Direction direction);

    [[nodiscard]] bool isKnown(const CellPosition& position, Direction direction) const;

    void setOpen(const CellPosition& position, Direction direction);

    [[nodiscard]] bool isVisited(const CellPosition& position) const;

    void markVisited(const CellPosition& position);

    [[nodiscard]] unsigned int getCost(const CellPosition& position) const;

    void setCost(const CellPosition& position, unsigned int cost);

    void reset();

};

