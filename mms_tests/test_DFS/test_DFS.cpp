#include "API.h"
#include "../../lib/Maze/Maze.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

Direction robotDir = NORTH;
CellPosition robotPos = {0,0};
CellPosition newPos = {0,0};
Maze maze;

constexpr int MAZE_WIDTH = 16;
constexpr int MAZE_HEIGHT = 16;

int dx[4] = {0, 1, 0, -1};
int dy[4] = {1, 0, -1, 0};

void log(const std::string& text)
{
    std::cerr << text << std::endl;
}

bool loadOriginalMaze(const std::string& path, Maze& originalMaze)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        log("ERROR: Cannot open maze file: " + path);
        return false;
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line))
    {
        lines.push_back(line);
    }

    file.close();

    const int expectedLines = MAZE_HEIGHT * 2 + 1;

    if (static_cast<int>(lines.size()) != expectedLines)
    {
        log(
            "ERROR: Invalid maze height. Expected " +
            std::to_string(expectedLines) +
            " lines, got " +
            std::to_string(lines.size())
        );

        return false;
    }

    const int expectedWidth = MAZE_WIDTH * 4 + 1;

    for (const auto& currentLine : lines)
    {
        if (static_cast<int>(currentLine.size()) != expectedWidth)
        {
            log(
                "ERROR: Invalid maze width. Expected " +
                std::to_string(expectedWidth) +
                ", got " +
                std::to_string(currentLine.size())
            );

            return false;
        }
    }

    /*
     * The text file is drawn from top to bottom.
     *
     * Our Maze coordinates are:
     *
     *        y+
     *        ^
     *        |
     *        |
     *        +------> x+
     *
     * Therefore the first cell row in the file
     * corresponds to y = HEIGHT - 1.
     */

    for (int fileRow = 0; fileRow < MAZE_HEIGHT; ++fileRow)
    {
        const int y = MAZE_HEIGHT - 1 - fileRow;

        /*
         * Horizontal walls are on:
         *
         * fileRow * 2
         *
         * Example:
         *
         * o---o---o
         *
         * The wall of cell (x,y) in the NORTH
         * direction is represented by "---".
         */

        const std::string& horizontal = lines[fileRow * 2];

        for (int x = 0; x < MAZE_WIDTH; ++x)
        {
            const int index = x * 4 + 1;

            bool wall =
                horizontal[index] == '-' &&
                horizontal[index + 1] == '-' &&
                horizontal[index + 2] == '-';

            if (wall)
            {
                originalMaze.setWall(
                    {x, y},
                    NORTH
                );
            }
        }

        /*
         * Vertical walls are on:
         *
         * fileRow * 2 + 1
         *
         * Example:
         *
         * |       |
         *
         * The '|' character at x * 4 represents
         * the WEST wall of the cell.
         */

        const std::string& vertical = lines[fileRow * 2 + 1];

        for (int x = 0; x < MAZE_WIDTH; ++x)
        {
            const int index = x * 4;

            bool wall = vertical[index] == '|';

            if (wall)
            {
                originalMaze.setWall(
                    {x, y},
                    WEST
                );
            }
        }

        /*
         * The last vertical position represents
         * the EAST wall of the last cell.
         */

        const int eastIndex = MAZE_WIDTH * 4;

        if (vertical[eastIndex] == '|')
        {
            originalMaze.setWall(
                {MAZE_WIDTH - 1, y},
                EAST
            );
        }
    }

    /*
     * The bottom horizontal line represents
     * the SOUTH walls of the bottom row.
     */

    const std::string& bottom = lines[MAZE_HEIGHT * 2];

    for (int x = 0; x < MAZE_WIDTH; ++x)
    {
        const int index = x * 4 + 1;

        bool wall =
            bottom[index] == '-' &&
            bottom[index + 1] == '-' &&
            bottom[index + 2] == '-';

        if (wall)
        {
            originalMaze.setWall(
                {x, 0},
                SOUTH
            );
        }
    }

    log("Original maze loaded successfully.");

    return true;
}

std::string mazeToString(const Maze& map)
{
    std::ostringstream output;

    /*
     * Top border
     */
    for (int x = 0; x < MAZE_WIDTH; ++x)
    {
        output << "o";

        CellPosition pos = {
            x,
            MAZE_HEIGHT - 1
        };

        if (map.hasWall(pos, NORTH))
            output << "---";
        else
            output << "   ";
    }

    output << "o\n";

    /*
     * Rows from top to bottom
     */
    for (int y = MAZE_HEIGHT - 1; y >= 0; --y)
    {
        /*
         * Vertical walls + cell contents
         */
        for (int x = 0; x < MAZE_WIDTH; ++x)
        {
            CellPosition pos = {x, y};

            if (map.hasWall(pos, WEST))
                output << "|";
            else
                output << " ";

            /*
             * Show visited cells as G.
             * Otherwise show empty cell.
             */
            if (map.isVisited(pos))
            {
                output << " G ";
            }
            else
            {
                output << "   ";
            }
        }

        /*
         * EAST wall
         */
        CellPosition last = {
            MAZE_WIDTH - 1,
            y
        };

        if (map.hasWall(last, EAST))
            output << "|";
        else
            output << " ";

        output << "\n";

        /*
         * Horizontal walls below this row
         */
        for (int x = 0; x < MAZE_WIDTH; ++x)
        {
            output << "o";

            CellPosition pos = {x, y};

            if (map.hasWall(pos, SOUTH))
                output << "---";
            else
                output << "   ";
        }

        output << "o\n";
    }

    return output.str();
}

void printDiscoveredMaze()
{
    log("");
    log("========================================");
    log("       DISCOVERED MAZE");
    log("========================================");

    std::cerr << mazeToString(maze) << std::endl;
}

bool saveMaze(
    const Maze& map,
    const std::string& path
)
{
    std::ofstream file(path);

    if (!file.is_open())
    {
        log("ERROR: Cannot create file: " + path);
        return false;
    }

    file << mazeToString(map);

    file.close();

    log("Discovered maze saved to: " + path);

    return true;
}

bool compareMazes(
    const Maze& original,
    const Maze& discovered
)
{
    bool identical = true;

    int mismatchCount = 0;
    int unknownCount = 0;
    int checkedCount = 0;

    log("");
    log("========================================");
    log("          MAZE COMPARISON");
    log("========================================");

    for (int x = 0; x < MAZE_WIDTH; ++x)
    {
        for (int y = 0; y < MAZE_HEIGHT; ++y)
        {
            CellPosition pos = {x, y};

            /*
             * Only NORTH and EAST are checked.
             * Each internal wall is therefore checked once.
             */
            for (Direction dir : {NORTH, EAST})
            {
                /*
                 * If DFS never observed this wall,
                 * we cannot compare it.
                 */
                if (!discovered.isKnown(pos, dir))
                {
                    ++unknownCount;
                    continue;
                }

                ++checkedCount;

                const bool originalWall =
                    original.hasWall(pos, dir);

                const bool discoveredWall =
                    discovered.hasWall(pos, dir);

                if (originalWall != discoveredWall)
                {
                    identical = false;
                    ++mismatchCount;

                    log(
                        "MISMATCH: (" +
                        std::to_string(x) +
                        "," +
                        std::to_string(y) +
                        ") DIR=" +
                        std::to_string(dir) +
                        " | ORIGINAL=" +
                        (originalWall ? "#" : ".") +
                        " DISCOVERED=" +
                        (discoveredWall ? "#" : ".")
                    );
                }
            }
        }
    }

    log("");

    log(
        "Checked walls: " +
        std::to_string(checkedCount)
    );

    log(
        "Unknown walls: " +
        std::to_string(unknownCount)
    );

    log(
        "Wall mismatches: " +
        std::to_string(mismatchCount)
    );

    if (identical)
    {
        log("RESULT: PASS");
        log("All observable walls match the original maze.");
    }
    else
    {
        log("RESULT: FAIL");
        log("At least one observable wall is incorrect.");
    }

    return identical;
}

void printOriginalMaze(const Maze& original)
{
    log("");
    log("========================================");
    log("          ORIGINAL MAZE");
    log("========================================");

    std::cerr << mazeToString(original) << std::endl;
}


void updateWalls()
{
    bool front = API::wallFront();
    bool right = API::wallRight();
    bool left = API::wallLeft();

    log("Sensors [F:" + std::string(front ? "#" : ".") +
        " R:" + std::string(right ? "#" : ".") +
        " L:" + std::string(left ? "#" : ".") + "]");

    Direction frontDir = robotDir;
    Direction rightDir = maze.rightOf(robotDir);
    Direction leftDir  = maze.leftOf(robotDir);
    Direction backDir  = maze.opposite(robotDir);

    if (front)
        maze.setWall(robotPos, frontDir);
    else
        maze.setOpen(robotPos, frontDir);

    if (right)
        maze.setWall(robotPos, rightDir);
    else
        maze.setOpen(robotPos, rightDir);

    if (left)
        maze.setWall(robotPos, leftDir);
    else
        maze.setOpen(robotPos, leftDir);

    /*
     * The rear direction is the path through which
     * the robot came, so it is guaranteed to be open.
     */
    maze.setOpen(robotPos, backDir);
}

void turnTO(Direction dir)
{
    if (robotDir == dir)
        return;

    if (maze.rightOf(robotDir) == dir)
    {
        log("Turn RIGHT");
        API::turnRight();
        robotDir = dir;
        return;
    }

    if (maze.leftOf(robotDir) == dir)
    {
        log("Turn LEFT");
        API::turnLeft();
        robotDir = dir;
        return;
    }

    if (maze.opposite(robotDir) == dir)
    {
        log("Turn 180");
        API::turnLeft();
        API::turnLeft();
        robotDir = dir;
        return;
    }
}

void explore()
{
    log("");
    log(">>> ENTER (" +
        std::to_string(robotPos.x) + "," +
        std::to_string(robotPos.y) +
        ") DIR=" + std::to_string(robotDir));

    maze.markVisited(robotPos);

    updateWalls();

    API::setColor(
        robotPos.x,
        robotPos.y,
        'G'
    );

    for (Direction dir : DIRECTIONS)
    {
        log("Check DIR=" + std::to_string(dir));

        if (!maze.isKnown(robotPos, dir))
        {
            log("  -> unknown");
            continue;
        }

        if (maze.hasWall(robotPos, dir))
        {
            log("  -> wall");
            continue;
        }

        newPos = maze.nextPosition(robotPos, dir);

        if (!maze.isValid(newPos))
        {
            log("  -> invalid");
            continue;
        }

        if (maze.isVisited(newPos))
        {
            log("  -> visited");
            continue;
        }

        log("  -> MOVE (" +
            std::to_string(newPos.x) + "," +
            std::to_string(newPos.y) + ")");

        turnTO(dir);

        API::moveForward();

        robotPos = newPos;

        explore();

        log("<<< RETURN to (" +
            std::to_string(robotPos.x) + "," +
            std::to_string(robotPos.y) + ")");

        Direction backDir = maze.opposite(dir);

        turnTO(backDir);

        robotPos = maze.nextPosition(robotPos, robotDir);

        API::moveForward();

        log("Back at (" +
            std::to_string(robotPos.x) + "," +
            std::to_string(robotPos.y) + ")");

        updateWalls();
    }

    log("<<< EXIT (" +
        std::to_string(robotPos.x) + "," +
        std::to_string(robotPos.y) + ")");
}

int main()
{
    log("=== START ===");

    log("Start position: (0,0)");
    log("Start direction: NORTH");

    Maze originalMaze;

    if (!loadOriginalMaze(
            "C:/Users/LEGION/Downloads/Micromouse/mazefiles-master/classic/alljapan-045-2024-exp-fin.txt",
            originalMaze))
    {
        return 1;
    }

    printOriginalMaze(originalMaze);

    explore();

    printDiscoveredMaze();

    saveMaze(
        maze,
        "discovered_maze.txt"
    );

    compareMazes(
        originalMaze,
        maze
    );

    log("=== END ===");

    return 0;
}
