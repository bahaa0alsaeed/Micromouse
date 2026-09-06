#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "API.h"
#include "DFS.h"
#include "BFS.h"
#include "Maze.h"
#include "Directions.h"
#include "RobotState.h"

std::string MAZE_PATH =
    "C:/Users/LEGION/Downloads/Micromouse/mazefiles-master/classic/alljapan-033-2012-exp-pre.txt";

constexpr int MAZE_WIDTH = 16;
constexpr int MAZE_HEIGHT = 16;


// ============================================================
// Adapters
// ============================================================
//
// The algorithms depend on abstract interfaces rather than
// the simulator API directly. These adapters translate calls
// such as moveForward() and wallFront() into MMS API commands.
//
// This keeps DFS/BFS independent from the simulator and allows
// the same algorithms to be used with real robot hardware.
// ============================================================

class MMSNavigation : public INavigation
{
    API& api;

public:
    explicit MMSNavigation(API& api) : api(api) {}

    void moveForward(float distance, float speed = -1) override
    {
        // The simulator currently ignores the speed parameter.
        (void)speed;

        api.moveForward(static_cast<int>(distance));
    }

    void turnLeft() override  { api.turnLeft(); }
    void turnRight() override { api.turnRight(); }
};


class MMSWallSensors : public IWallSensors
{
    API& api;

public:
    explicit MMSWallSensors(API& api) : api(api) {}

    // Forward the sensor requests to the simulator API.
    bool wallFront() override { return api.wallFront(); }
    bool wallRight() override { return api.wallRight(); }
    bool wallLeft()  override { return api.wallLeft(); }
};


// ============================================================
// Shared test objects
// ============================================================
//
// DFS explores the original maze and builds discoveredMaze.
//
// BFS then uses the same discoveredMaze to find the shortest
// path. This intentionally mirrors the real Micromouse flow:
//
//      Original Maze
//           |
//          DFS
//           |
//      Discovered Maze
//           |
//          BFS
//           |
//      Shortest Path
//           |
//        Execute
// ============================================================

Maze discoveredMaze;
API api;

MMSNavigation navigation(api);
MMSWallSensors sensors(api);

DFS dfs(discoveredMaze, navigation, sensors);
BFS bfs(discoveredMaze, navigation, sensors);


// ============================================================
// Helpers
// ============================================================

std::string directionToString(Direction dir)
{
    // Convert a direction enum into readable text for test output.
    switch (dir)
    {
        case NORTH: return "NORTH";
        case EAST:  return "EAST";
        case SOUTH: return "SOUTH";
        case WEST:  return "WEST";
    }

    return "?";
}


// ============================================================
// Load the original maze from a text file
// ============================================================
//
// The maze file uses an ASCII representation:
//
//      o---o---o
//      |       |
//      o   o---o
//
// The file is stored from top to bottom, while the internal
// maze representation uses y = 0 at the bottom and increasing
// y values going upward.
//
// Therefore, the rows must be flipped while loading.
// ============================================================

bool loadOriginalMaze(
    const std::string& path,
    Maze& originalMaze
)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cerr
            << "ERROR: cannot open maze file: "
            << path
            << std::endl;

        return false;
    }

    // Read the complete maze file into memory first.
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line))
        lines.push_back(line);

    file.close();


    // A 16x16 maze uses 33 lines:
    // 16 cell rows + 16 wall rows + 1 final boundary row.
    const int expectedLines = MAZE_HEIGHT * 2 + 1;

    if (static_cast<int>(lines.size()) != expectedLines)
    {
        std::cerr
            << "ERROR: invalid maze height (expected "
            << expectedLines
            << ", got "
            << lines.size()
            << ")"
            << std::endl;

        return false;
    }


    // Each cell occupies 4 characters horizontally,
    // plus one character for the final boundary.
    const int expectedWidth = MAZE_WIDTH * 4 + 1;

    for (const auto& currentLine : lines)
    {
        if (static_cast<int>(currentLine.size()) != expectedWidth)
        {
            std::cerr
                << "ERROR: invalid maze width (expected "
                << expectedWidth
                << ", got "
                << currentLine.size()
                << ")"
                << std::endl;

            return false;
        }
    }


    // The file is drawn from top to bottom,
    // while our y+ direction points upward.
    // Flip the rows to match the internal coordinate system.
    for (int fileRow = 0; fileRow < MAZE_HEIGHT; ++fileRow)
    {
        const int y = MAZE_HEIGHT - 1 - fileRow;


        // ----------------------------------------------------
        // Horizontal walls (NORTH)
        // ----------------------------------------------------
        //
        // Horizontal walls are stored on even-numbered lines.
        // Each cell uses three '-' characters for a wall.
        // ----------------------------------------------------

        const std::string& horizontal = lines[fileRow * 2];

        for (int x = 0; x < MAZE_WIDTH; ++x)
        {
            const int index = x * 4 + 1;

            bool wall =
                horizontal[index] == '-' &&
                horizontal[index + 1] == '-' &&
                horizontal[index + 2] == '-';

            if (wall)
                originalMaze.setWall({x, y}, NORTH);
        }


        // ----------------------------------------------------
        // Vertical walls (WEST)
        // ----------------------------------------------------
        //
        // Vertical walls are stored on odd-numbered lines.
        // The '|' character at the beginning of each cell
        // represents its WEST wall.
        // ----------------------------------------------------

        const std::string& vertical = lines[fileRow * 2 + 1];

        for (int x = 0; x < MAZE_WIDTH; ++x)
        {
            if (vertical[x * 4] == '|')
                originalMaze.setWall({x, y}, WEST);
        }


        // The final '|' on the line represents the EAST wall
        // of the rightmost cell.
        if (vertical[MAZE_WIDTH * 4] == '|')
        {
            originalMaze.setWall(
                {MAZE_WIDTH - 1, y},
                EAST
            );
        }
    }


    // --------------------------------------------------------
    // Bottom boundary
    // --------------------------------------------------------
    //
    // The final line contains the SOUTH walls of the bottom row.
    // --------------------------------------------------------

    const std::string& bottom =
        lines[MAZE_HEIGHT * 2];

    for (int x = 0; x < MAZE_WIDTH; ++x)
    {
        const int index = x * 4 + 1;

        bool wall =
            bottom[index] == '-' &&
            bottom[index + 1] == '-' &&
            bottom[index + 2] == '-';

        if (wall)
            originalMaze.setWall({x, 0}, SOUTH);
    }

    return true;
}


// ============================================================
// Maze printing / saving
// ============================================================

std::string mazeToString(const Maze& map)
{
    std::ostringstream output;


    // --------------------------------------------------------
    // Print the NORTH walls of the top row.
    // --------------------------------------------------------

    for (int x = 0; x < MAZE_WIDTH; ++x)
    {
        output << "o";

        output << (
            map.hasWall(
                {x, MAZE_HEIGHT - 1},
                NORTH
            )
                ? "---"
                : "   "
        );
    }

    output << "o\n";


    // --------------------------------------------------------
    // Print each row from top to bottom.
    // --------------------------------------------------------

    for (int y = MAZE_HEIGHT - 1; y >= 0; --y)
    {
        // Print WEST walls and cell spaces.
        for (int x = 0; x < MAZE_WIDTH; ++x)
        {
            CellPosition pos = {x, y};

            output << (
                map.hasWall(pos, WEST)
                    ? "|"
                    : " "
            );

            output << "   ";
        }


        // Print the EAST boundary of the last cell.
        output << (
            map.hasWall(
                {MAZE_WIDTH - 1, y},
                EAST
            )
                ? "|"
                : " "
        );

        output << "\n";


        // ----------------------------------------------------
        // Print SOUTH walls for the current row.
        // ----------------------------------------------------

        for (int x = 0; x < MAZE_WIDTH; ++x)
        {
            output << "o";

            output << (
                map.hasWall({x, y}, SOUTH)
                    ? "---"
                    : "   "
            );
        }

        output << "o\n";
    }

    return output.str();
}


void printMaze(
    const std::string& title,
    const Maze& map
)
{
    std::cerr
        << "--- "
        << title
        << " ---\n"
        << mazeToString(map)
        << std::endl;
}


bool saveMaze(
    const Maze& map,
    const std::string& path
)
{
    std::ofstream file(path);

    if (!file.is_open())
    {
        std::cerr
            << "ERROR: cannot create file: "
            << path
            << std::endl;

        return false;
    }

    // Save the same ASCII representation used for console output.
    file << mazeToString(map);

    return true;
}


// ============================================================
// Compare the discovered maze against the original one
// ============================================================
//
// DFS does not necessarily need to visit every cell in an
// arbitrary maze. Some cells may be completely unreachable.
//
// Therefore, the comparison first determines which cells are
// reachable from the starting position in the ORIGINAL maze.
// Only those cells are then used for validation.
//
// This prevents unreachable cells from being incorrectly
// reported as DFS failures.
// ============================================================

std::vector<std::vector<bool>> computeReachable(
    Maze original,
    CellPosition start
)
{
    std::vector<std::vector<bool>> reachable(
        MAZE_WIDTH,
        std::vector<bool>(MAZE_HEIGHT, false)
    );

    // Use a simple stack to perform graph traversal.
    std::vector<CellPosition> stack = {start};

    reachable[start.x][start.y] = true;


    while (!stack.empty())
    {
        CellPosition pos = stack.back();
        stack.pop_back();


        // Check all four possible neighboring cells.
        for (Direction dir : {
            NORTH,
            EAST,
            SOUTH,
            WEST
        })
        {
            // A wall blocks movement in this direction.
            if (original.hasWall(pos, dir))
                continue;

            CellPosition next =
                nextPosition(pos, dir);

            // Ignore positions outside the maze.
            if (!original.isValid(next))
                continue;

            // Skip cells that were already discovered as reachable.
            if (reachable[next.x][next.y])
                continue;

            reachable[next.x][next.y] = true;
            stack.push_back(next);
        }
    }

    return reachable;
}


bool compareMazes(
    const Maze& original,
    const Maze& discovered
)
{
    // Determine which cells can actually be reached by the robot.
    std::vector<std::vector<bool>> reachable =
        computeReachable(
            original,
            {0, 0}
        );

    int mismatchCount = 0;
    int unknownCount = 0;


    for (int x = 0; x < MAZE_WIDTH; ++x)
    {
        for (int y = 0; y < MAZE_HEIGHT; ++y)
        {
            // Cells the robot could never reach are excluded.
            // DFS not visiting them is expected, not a failure.
            if (!reachable[x][y])
                continue;

            CellPosition pos = {x, y};


            // Checking only NORTH and EAST is sufficient because
            // every wall is shared by two adjacent cells.
            //
            // Checking SOUTH/WEST as well would compare the same
            // physical walls twice.
            for (Direction dir : {
                NORTH,
                EAST
            })
            {
                // A reachable wall that DFS never discovered
                // means the exploration is incomplete.
                if (!discovered.isKnown(pos, dir))
                {
                    ++unknownCount;

                    std::cerr
                        << "UNKNOWN ("
                        << x << ","
                        << y << ") "
                        << directionToString(dir)
                        << std::endl;

                    continue;
                }


                bool originalWall =
                    original.hasWall(pos, dir);

                bool discoveredWall =
                    discovered.hasWall(pos, dir);


                // Compare the actual wall state against
                // the state discovered by DFS.
                if (originalWall != discoveredWall)
                {
                    ++mismatchCount;

                    std::cerr
                        << "MISMATCH ("
                        << x << ","
                        << y << ") "
                        << directionToString(dir)
                        << ": original="
                        << (originalWall ? "#" : ".")
                        << " discovered="
                        << (discoveredWall ? "#" : ".")
                        << std::endl;
                }
            }
        }
    }


    // DFS passes only when every reachable wall has been
    // correctly discovered and no wall state is incorrect.
    bool passed =
        (mismatchCount == 0) &&
        (unknownCount == 0);


    std::cerr
        << "mismatches="
        << mismatchCount
        << " unknown="
        << unknownCount
        << " -> "
        << (passed ? "PASSED" : "FAILED")
        << std::endl;

    return passed;
}


// ============================================================
// Reset robot before BFS
// ============================================================
//
// DFS may finish with the robot facing any direction and at
// any reachable position. BFS execution needs a known initial
// robot state.
//
// The discovered maze itself is NOT reset because it is the
// input map that BFS must use.
// ============================================================

void resetRobotForBFS(const CellPosition& start)
{
    robotPos = start;


    // Keep the physical robot and software direction synchronized.
    //
    // Rotate the robot until it faces NORTH so that both the
    // simulator and RobotState start BFS from a known orientation.
    while (robotDir != NORTH)
    {
        navigation.turnLeft();
        robotDir = leftOf(robotDir);
    }
}


// ============================================================
// DFS Test
// ============================================================

bool runDfsTest(const Maze& originalMaze)
{
    std::cerr
        << "\n========== DFS TEST ==========\n";


    // DFS explores the maze using the simulated sensors
    // and stores the discovered walls in discoveredMaze.
    dfs.explore();


    // Print the result so the discovered map can be inspected
    // visually during development.
    printMaze(
        "DISCOVERED",
        discoveredMaze
    );


    // Save the discovered map for later inspection or comparison.
    saveMaze(
        discoveredMaze,
        "discovered_maze.txt"
    );


    // Compare the discovered map with the original maze.
    bool passed =
        compareMazes(
            originalMaze,
            discoveredMaze
        );


    if (passed)
        std::cerr << "DFS PASSED\n";
    else
        std::cerr << "DFS FAILED\n";


    return passed;
}


// ============================================================
// BFS path validation
// ============================================================
//
// Finding a path is not enough. The test also verifies that the
// generated path:
//
//   1. Contains adjacent cells only.
//   2. Moves in a valid cardinal direction.
//   3. Never crosses a discovered wall.
//
// This catches errors in BFS path reconstruction before the
// robot is allowed to execute the path.
// ============================================================

bool validateBfsPath(const std::vector<CellPosition>& path)
{
    if (path.empty())
    {
        std::cerr
            << "BFS ERROR: generated path is empty."
            << std::endl;

        return false;
    }


    bool passed = true;


    for (size_t i = 0; i + 1 < path.size(); ++i)
    {
        const CellPosition cur = path[i];
        const CellPosition next = path[i + 1];


        const int dx = next.x - cur.x;
        const int dy = next.y - cur.y;


        // ----------------------------------------------------
        // Check 1: cells must be adjacent
        // ----------------------------------------------------
        //
        // A valid Micromouse path moves exactly one cell
        // horizontally or vertically at every step.
        // ----------------------------------------------------

        if (std::abs(dx) + std::abs(dy) != 1)
        {
            std::cerr
                << "BFS ERROR: non-adjacent path segment: "
                << "("
                << cur.x
                << ","
                << cur.y
                << ") -> "
                << "("
                << next.x
                << ","
                << next.y
                << ")"
                << std::endl;

            passed = false;
            continue;
        }


        // ----------------------------------------------------
        // Check 2: determine movement direction
        // ----------------------------------------------------

        Direction dir;

        if (dx == 1)
            dir = EAST;
        else if (dx == -1)
            dir = WEST;
        else if (dy == 1)
            dir = NORTH;
        else
            dir = SOUTH;


        // ----------------------------------------------------
        // Check 3: path must not cross a discovered wall
        // ----------------------------------------------------

        if (discoveredMaze.hasWall(cur, dir))
        {
            std::cerr
                << "BFS ERROR: path crosses wall at "
                << "("
                << cur.x
                << ","
                << cur.y
                << ") "
                << directionToString(dir)
                << " -> "
                << "("
                << next.x
                << ","
                << next.y
                << ")"
                << std::endl;

            passed = false;
        }
    }


    return passed;
}


// ============================================================
// BFS Test
// ============================================================

bool runBfsTest(
    const CellPosition& start,
    const CellPosition& goal
)
{
    std::cerr
        << "\n========== BFS TEST ==========\n";


    std::cerr
        << "Start: ("
        << start.x
        << ","
        << start.y
        << ")\n";


    std::cerr
        << "Goal: ("
        << goal.x
        << ","
        << goal.y
        << ")\n";


    // Reset only the robot's physical/software state.
    //
    // discoveredMaze is intentionally preserved because BFS
    // must find its path using the map discovered by DFS.
    resetRobotForBFS(start);


    // Find the shortest path through the discovered maze.
    bfs.solve(
        start,
        goal
    );


    const std::vector<CellPosition>& path =
        bfs.getPath();


    // Highlight the calculated path in the simulator.
    // This provides a visual way to verify the BFS result.
    for (const CellPosition& pos : path)
    {
        api.setColor(
            pos.x,
            pos.y,
            'G'
        );
    }


    std::cerr
        << "Path length: "
        << path.size()
        << std::endl;


    bool passed = true;


    // --------------------------------------------------------
    // Check that BFS produced a path
    // --------------------------------------------------------

    if (path.empty())
    {
        std::cerr
            << "BFS ERROR: no path generated."
            << std::endl;

        passed = false;
    }
    else
    {
        // ----------------------------------------------------
        // Check that the path starts and ends correctly.
        // ----------------------------------------------------

        if (path.front() != start)
        {
            std::cerr
                << "BFS ERROR: path does not start at "
                << "("
                << start.x
                << ","
                << start.y
                << ")"
                << std::endl;

            passed = false;
        }


        if (path.back() != goal)
        {
            std::cerr
                << "BFS ERROR: path does not end at "
                << "("
                << goal.x
                << ","
                << goal.y
                << ")"
                << std::endl;

            passed = false;
        }


        // ----------------------------------------------------
        // Validate path geometry and wall constraints.
        // ----------------------------------------------------

        if (!validateBfsPath(path))
            passed = false;
    }


    // --------------------------------------------------------
    // Execute the generated path
    // --------------------------------------------------------
    //
    // Only execute when BFS generated a non-empty path.
    // The path has already been validated above.
    // --------------------------------------------------------

    if (!path.empty())
        bfs.executePath();


    // --------------------------------------------------------
    // Check final robot position
    // --------------------------------------------------------
    //
    // This verifies the complete chain:
    //
    //      BFS -> path -> navigation -> robot position
    //
    // A correct path alone does not guarantee that the
    // navigation layer executed it correctly.
    // --------------------------------------------------------

    if (robotPos != goal)
    {
        std::cerr
            << "BFS ERROR: robot did not reach goal. "
            << "Expected=("
            << goal.x
            << ","
            << goal.y
            << ") Actual=("
            << robotPos.x
            << ","
            << robotPos.y
            << ")"
            << std::endl;

        passed = false;
    }


    if (passed)
        std::cerr << "BFS PASSED\n";
    else
        std::cerr << "BFS FAILED\n";


    return passed;
}


// ============================================================
// main
// ============================================================
//
// The tests intentionally run sequentially:
//
//      1. Load original maze
//      2. DFS explores the maze
//      3. Validate discovered maze
//      4. Reset robot state
//      5. BFS finds shortest path using discovered maze
//      6. Validate BFS path
//      7. Execute path
//      8. Verify that the robot reaches the goal
// ============================================================

int main()
{
    Maze originalMaze;

    // --------------------------------------------------------
    // Load the ground-truth maze used to validate DFS.
    // --------------------------------------------------------

    if (!loadOriginalMaze(MAZE_PATH,originalMaze) )
        return 1;
    printMaze("ORIGINAL", originalMaze);

    // --------------------------------------------------------
    // Test 1: DFS exploration
    // --------------------------------------------------------
    //
    // DFS starts from the simulator's current robot state and
    // builds discoveredMaze using only sensor information.
    // --------------------------------------------------------

    bool dfsPassed =
        runDfsTest(
            originalMaze
        );

    // --------------------------------------------------------
    // Test 2: BFS on the DFS-discovered maze
    // --------------------------------------------------------
    //
    // BFS does not use the original maze directly.
    // It must operate on discoveredMaze, just like a real
    // robot would operate on its learned map.
    // --------------------------------------------------------

    const CellPosition start = {0, 0};
    const CellPosition goal  = {7, 7};

    bool bfsPassed = runBfsTest(start, goal);

    // --------------------------------------------------------
    // Overall result
    // --------------------------------------------------------
    //
    // The complete test passes only when both stages pass.
    // --------------------------------------------------------

    bool allPassed = dfsPassed && bfsPassed;

    std::cerr
        << "\n================================\n"
        << "OVERALL RESULT: "
        << (allPassed ? "PASSED" : "FAILED")
        << "\n================================\n";

    return allPassed ? 0 : 1;
}
