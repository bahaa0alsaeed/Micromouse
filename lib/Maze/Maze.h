#pragma once
#include <cstdint>
#include "Directions.h"

struct Cell
{
    // Bitmask of walls that physically exist in this cell.
    uint8_t walls = 0;

    // Bitmask of directions whose state is already known.
    uint8_t known = 0;

    // Indicates whether this cell has been visited by the algorithm.
    bool visited = false;

    // Cost associated with reaching this cell.
    uint8_t cost = 0;
};

class Maze
{
    static constexpr unsigned int WIDTH  = 16;
    static constexpr unsigned int HEIGHT = 16;

    // Maze cells indexed as cells[x][y].
    Cell cells[WIDTH][HEIGHT]{};

public:
    Maze();

    // Check whether a cell position is inside the maze.
    [[nodiscard]] bool isValid(const CellPosition& pos) const;

    // Check whether a wall exists in the given direction.
    [[nodiscard]] bool hasWall(const CellPosition& pos, Direction dir) const;

    // Add a wall and mark its state as known.
    void setWall(const CellPosition& pos, Direction dir);

    // Check whether the wall state is known.
    [[nodiscard]] bool isKnown(const CellPosition& pos, Direction dir) const;

    // Mark a direction as known and open.
    void setOpen(const CellPosition& pos, Direction dir);

    // Check whether the cell has been visited.
    [[nodiscard]] bool isVisited(const CellPosition& pos) const;

    // Mark the cell as visited.
    void markVisited(const CellPosition& pos);

    // Get the traversal cost of a cell.
    [[nodiscard]] unsigned int getCost(const CellPosition& pos) const;

    // Set the traversal cost of a cell.
    void setCost(const CellPosition& pos, uint8_t cost);

    // Clear the maze and restore the boundary walls.
    void reset();
};
