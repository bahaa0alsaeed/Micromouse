#include "unity.h"
#include "Maze.h"
#include "Directions.h"

// ---------------------------------------------------------
// isValid()
// ---------------------------------------------------------

void test_isValid_accepts_valid_positions(void)
{
    Maze maze;

    TEST_ASSERT_TRUE(maze.isValid({0, 0}));
    TEST_ASSERT_TRUE(maze.isValid({15, 15}));
    TEST_ASSERT_TRUE(maze.isValid({7, 8}));
}

void test_isValid_rejects_invalid_positions(void)
{
    Maze maze;

    TEST_ASSERT_FALSE(maze.isValid({16, 0}));
    TEST_ASSERT_FALSE(maze.isValid({0, 16}));
    TEST_ASSERT_FALSE(maze.isValid({16, 16}));
}


// ---------------------------------------------------------
// reset() / outer walls
// ---------------------------------------------------------

void test_reset_creates_outer_walls(void)
{
    Maze maze;

    // Left boundary
    TEST_ASSERT_TRUE(maze.hasWall({0, 5}, WEST));

    // Right boundary
    TEST_ASSERT_TRUE(maze.hasWall({15, 5}, EAST));

    // Bottom boundary
    TEST_ASSERT_TRUE(maze.hasWall({5, 0}, SOUTH));

    // Top boundary
    TEST_ASSERT_TRUE(maze.hasWall({5, 15}, NORTH));
}

void test_reset_does_not_create_internal_walls(void)
{
    Maze maze;

    TEST_ASSERT_FALSE(maze.hasWall({5, 5}, NORTH));
    TEST_ASSERT_FALSE(maze.hasWall({5, 5}, EAST));
    TEST_ASSERT_FALSE(maze.hasWall({5, 5}, SOUTH));
    TEST_ASSERT_FALSE(maze.hasWall({5, 5}, WEST));
}


// ---------------------------------------------------------
// setWall()
// ---------------------------------------------------------

void test_setWall_creates_wall(void)
{
    Maze maze;
    CellPosition pos{5, 5};

    maze.setWall(pos, NORTH);

    TEST_ASSERT_TRUE(maze.hasWall(pos, NORTH));
}

void test_setWall_updates_opposite_cell(void)
{
    Maze maze;
    CellPosition pos{5, 5};

    maze.setWall(pos, NORTH);

    TEST_ASSERT_TRUE(maze.hasWall({5, 6}, SOUTH));
}

void test_setWall_marks_direction_as_known(void)
{
    Maze maze;
    CellPosition pos{5, 5};

    maze.setWall(pos, EAST);

    TEST_ASSERT_TRUE(maze.isKnown(pos, EAST));
    TEST_ASSERT_TRUE(maze.isKnown({6, 5}, WEST));
}

void test_setWall_works_for_all_directions(void)
{
    Maze maze;

    maze.setWall({5, 5}, NORTH);
    maze.setWall({5, 5}, EAST);
    maze.setWall({5, 5}, SOUTH);
    maze.setWall({5, 5}, WEST);

    TEST_ASSERT_TRUE(maze.hasWall({5, 5}, NORTH));
    TEST_ASSERT_TRUE(maze.hasWall({5, 5}, EAST));
    TEST_ASSERT_TRUE(maze.hasWall({5, 5}, SOUTH));
    TEST_ASSERT_TRUE(maze.hasWall({5, 5}, WEST));
}

void test_setWall_does_not_crash_on_invalid_position(void)
{
    Maze maze;

    maze.setWall({16, 5}, NORTH);
    maze.setWall({5, 16}, NORTH);
}

void test_setWall_boundary_does_not_modify_outside(void)
{
    Maze maze;

    maze.setWall({0, 0}, WEST);
    maze.setWall({15, 15}, EAST);
    maze.setWall({0, 0}, SOUTH);
    maze.setWall({15, 15}, NORTH);

    TEST_ASSERT_TRUE(maze.hasWall({0, 0}, WEST));
    TEST_ASSERT_TRUE(maze.hasWall({15, 15}, EAST));
    TEST_ASSERT_TRUE(maze.hasWall({0, 0}, SOUTH));
    TEST_ASSERT_TRUE(maze.hasWall({15, 15}, NORTH));
}

// ---------------------------------------------------------
// setOpen()
// ---------------------------------------------------------

void test_setOpen_marks_direction_as_known(void)
{
    Maze maze;
    CellPosition pos{5, 5};

    maze.setOpen(pos, NORTH);

    TEST_ASSERT_TRUE(maze.isKnown(pos, NORTH));
}

void test_setOpen_does_not_create_wall(void)
{
    Maze maze;
    CellPosition pos{5, 5};

    maze.setOpen(pos, NORTH);

    TEST_ASSERT_FALSE(maze.hasWall(pos, NORTH));
}

void test_setOpen_updates_opposite_cell(void)
{
    Maze maze;
    CellPosition pos{5, 5};

    maze.setOpen(pos, EAST);

    TEST_ASSERT_TRUE(maze.isKnown({6, 5}, WEST));
}

void test_setOpen_works_for_all_directions(void)
{
    Maze maze;

    maze.setOpen({5, 5}, NORTH);
    maze.setOpen({5, 5}, EAST);
    maze.setOpen({5, 5}, SOUTH);
    maze.setOpen({5, 5}, WEST);

    TEST_ASSERT_TRUE(maze.isKnown({5, 5}, NORTH));
    TEST_ASSERT_TRUE(maze.isKnown({5, 5}, EAST));
    TEST_ASSERT_TRUE(maze.isKnown({5, 5}, SOUTH));
    TEST_ASSERT_TRUE(maze.isKnown({5, 5}, WEST));
}


// ---------------------------------------------------------
// visited
// ---------------------------------------------------------

void test_new_cell_is_not_visited(void)
{
    Maze maze;

    TEST_ASSERT_FALSE(maze.isVisited({5, 5}));
}

void test_markVisited_marks_cell(void)
{
    Maze maze;
    CellPosition pos{5, 5};

    maze.markVisited(pos);

    TEST_ASSERT_TRUE(maze.isVisited(pos));
}

void test_markVisited_does_not_affect_other_cells(void)
{
    Maze maze;

    maze.markVisited({5, 5});

    TEST_ASSERT_TRUE(maze.isVisited({5, 5}));
    TEST_ASSERT_FALSE(maze.isVisited({5, 6}));
    TEST_ASSERT_FALSE(maze.isVisited({6, 5}));
}


// ---------------------------------------------------------
// cost
// ---------------------------------------------------------

void test_new_cell_has_zero_cost(void)
{
    Maze maze;

    TEST_ASSERT_EQUAL_UINT(0, maze.getCost({5, 5}));
}

void test_setCost_changes_cost(void)
{
    Maze maze;
    CellPosition pos{5, 5};

    maze.setCost(pos, 42);

    TEST_ASSERT_EQUAL_UINT(42, maze.getCost(pos));
}

void test_setCost_does_not_affect_other_cells(void)
{
    Maze maze;

    maze.setCost({5, 5}, 42);

    TEST_ASSERT_EQUAL_UINT(42, maze.getCost({5, 5}));
    TEST_ASSERT_EQUAL_UINT(0, maze.getCost({5, 6}));
}


// ---------------------------------------------------------
// invalid positions
// ---------------------------------------------------------

void test_invalid_position_hasWall_returns_false(void)
{
    Maze maze;

    TEST_ASSERT_FALSE(maze.hasWall({16, 5}, NORTH));
}

void test_invalid_position_isKnown_returns_false(void)
{
    Maze maze;

    TEST_ASSERT_FALSE(maze.isKnown({16, 5}, NORTH));
}

void test_invalid_position_isVisited_returns_false(void)
{
    Maze maze;

    TEST_ASSERT_FALSE(maze.isVisited({16, 5}));
}

void test_invalid_position_getCost_returns_zero(void)
{
    Maze maze;

    TEST_ASSERT_EQUAL_UINT(0, maze.getCost({16, 5}));
}


// ---------------------------------------------------------
// reset()
// ---------------------------------------------------------

void test_reset_clears_maze_state(void)
{
    Maze maze;

    CellPosition pos{5, 5};

    maze.setWall(pos, NORTH);
    maze.setOpen(pos, EAST);
    maze.markVisited(pos);
    maze.setCost(pos, 100);

    maze.reset();

    // Internal state should be cleared
    TEST_ASSERT_FALSE(maze.hasWall(pos, NORTH));
    TEST_ASSERT_FALSE(maze.isKnown(pos, NORTH));
    TEST_ASSERT_FALSE(maze.isKnown(pos, EAST));
    TEST_ASSERT_FALSE(maze.isVisited(pos));
    TEST_ASSERT_EQUAL_UINT(0, maze.getCost(pos));
}

void test_reset_restores_outer_walls(void)
{
    Maze maze;

    maze.reset();

    TEST_ASSERT_TRUE(maze.hasWall({0, 5}, WEST));
    TEST_ASSERT_TRUE(maze.hasWall({15, 5}, EAST));
    TEST_ASSERT_TRUE(maze.hasWall({5, 0}, SOUTH));
    TEST_ASSERT_TRUE(maze.hasWall({5, 15}, NORTH));
}


// ---------------------------------------------------------
// Tests
// ---------------------------------------------------------

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_isValid_accepts_valid_positions);
    RUN_TEST(test_isValid_rejects_invalid_positions);

    RUN_TEST(test_reset_creates_outer_walls);
    RUN_TEST(test_reset_does_not_create_internal_walls);

    RUN_TEST(test_setWall_creates_wall);
    RUN_TEST(test_setWall_updates_opposite_cell);
    RUN_TEST(test_setWall_marks_direction_as_known);
    RUN_TEST(test_setWall_works_for_all_directions);
    RUN_TEST(test_setWall_does_not_crash_on_invalid_position);
    RUN_TEST(test_setWall_boundary_does_not_modify_outside);

    RUN_TEST(test_setOpen_marks_direction_as_known);
    RUN_TEST(test_setOpen_does_not_create_wall);
    RUN_TEST(test_setOpen_updates_opposite_cell);
    RUN_TEST(test_setOpen_works_for_all_directions);

    RUN_TEST(test_new_cell_is_not_visited);
    RUN_TEST(test_markVisited_marks_cell);
    RUN_TEST(test_markVisited_does_not_affect_other_cells);

    RUN_TEST(test_new_cell_has_zero_cost);
    RUN_TEST(test_setCost_changes_cost);
    RUN_TEST(test_setCost_does_not_affect_other_cells);

    RUN_TEST(test_invalid_position_hasWall_returns_false);
    RUN_TEST(test_invalid_position_isKnown_returns_false);
    RUN_TEST(test_invalid_position_isVisited_returns_false);
    RUN_TEST(test_invalid_position_getCost_returns_zero);

    RUN_TEST(test_reset_clears_maze_state);
    RUN_TEST(test_reset_restores_outer_walls);

    return UNITY_END();
}