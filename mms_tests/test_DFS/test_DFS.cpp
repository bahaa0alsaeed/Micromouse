#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "API.h"
#include "DFS.h"
#include "Maze.h"
#include "Directions.h"

std::string MAZE_PATH = "PUT_MAZE_FILE.txt";
constexpr int MAZE_WIDTH = 16;
constexpr int MAZE_HEIGHT = 16;

// ============================================================
// Adapters: translate DFS calls into MMS API commands
// ============================================================

class MMSNavigation : public INavigation
{
    API& api;

public:
    explicit MMSNavigation(API& api) : api(api) {}

    void moveForward(float distance, float speed = -1) override
    {
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

    bool wallFront() override { return api.wallFront(); }
    bool wallRight() override { return api.wallRight(); }
    bool wallLeft()  override { return api.wallLeft();  }
};

Maze discoveredMaze;
API api;
MMSNavigation navigation(api);
MMSWallSensors sensors(api);
DFS dfs(discoveredMaze, navigation, sensors);

// ============================================================
// Helpers
// ============================================================

std::string directionToString(Direction dir)
{
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

bool loadOriginalMaze(const std::string& path, Maze& originalMaze)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "ERROR: cannot open maze file: " << path << std::endl;
        return false;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);
    file.close();

    const int expectedLines = MAZE_HEIGHT * 2 + 1;
    if (static_cast<int>(lines.size()) != expectedLines)
    {
        std::cerr << "ERROR: invalid maze height (expected " << expectedLines
                   << ", got " << lines.size() << ")" << std::endl;
        return false;
    }

    const int expectedWidth = MAZE_WIDTH * 4 + 1;
    for (const auto& currentLine : lines)
    {
        if (static_cast<int>(currentLine.size()) != expectedWidth)
        {
            std::cerr << "ERROR: invalid maze width (expected " << expectedWidth
                       << ", got " << currentLine.size() << ")" << std::endl;
            return false;
        }
    }

    // The file is drawn top to bottom, while our y+ points upward => flip the rows
    for (int fileRow = 0; fileRow < MAZE_HEIGHT; ++fileRow)
    {
        const int y = MAZE_HEIGHT - 1 - fileRow;

        // Horizontal walls (NORTH) are on line fileRow * 2
        const std::string& horizontal = lines[fileRow * 2];
        for (int x = 0; x < MAZE_WIDTH; ++x)
        {
            const int index = x * 4 + 1;
            bool wall = horizontal[index] == '-' &&
                        horizontal[index + 1] == '-' &&
                        horizontal[index + 2] == '-';
            if (wall)
                originalMaze.setWall({x, y}, NORTH);
        }

        // Vertical walls (WEST) are on line fileRow * 2 + 1
        const std::string& vertical = lines[fileRow * 2 + 1];
        for (int x = 0; x < MAZE_WIDTH; ++x)
        {
            if (vertical[x * 4] == '|')
                originalMaze.setWall({x, y}, WEST);
        }

        // EAST wall of the last cell in the row
        if (vertical[MAZE_WIDTH * 4] == '|')
            originalMaze.setWall({MAZE_WIDTH - 1, y}, EAST);
    }

    // Bottom line = SOUTH walls of the bottom row
    const std::string& bottom = lines[MAZE_HEIGHT * 2];
    for (int x = 0; x < MAZE_WIDTH; ++x)
    {
        const int index = x * 4 + 1;
        bool wall = bottom[index] == '-' &&
                    bottom[index + 1] == '-' &&
                    bottom[index + 2] == '-';
        if (wall)
            originalMaze.setWall({x, 0}, SOUTH);
    }

    return true;
}

// ============================================================
// Save the discovered maze (not printed to the console)
// ============================================================

std::string mazeToString(const Maze& map)
{
    std::ostringstream output;

    for (int x = 0; x < MAZE_WIDTH; ++x)
    {
        output << "o";
        output << (map.hasWall({x, MAZE_HEIGHT - 1}, NORTH) ? "---" : "   ");
    }
    output << "o\n";

    for (int y = MAZE_HEIGHT - 1; y >= 0; --y)
    {
        for (int x = 0; x < MAZE_WIDTH; ++x)
        {
            CellPosition pos = {x, y};
            output << (map.hasWall(pos, WEST) ? "|" : " ");
            output << (map.isVisited(pos) ? "   " : "   ");
        }
        output << (map.hasWall({MAZE_WIDTH - 1, y}, EAST) ? "|" : " ") << "\n";

        for (int x = 0; x < MAZE_WIDTH; ++x)
        {
            output << "o";
            output << (map.hasWall({x, y}, SOUTH) ? "---" : "   ");
        }
        output << "o\n";
    }

    return output.str();
}

void printMaze(const std::string& title, const Maze& map)
{
    std::cerr << "--- " << title << " ---\n" << mazeToString(map) << std::endl;
}

bool saveMaze(const Maze& map, const std::string& path)
{
    std::ofstream file(path);
    if (!file.is_open())
    {
        std::cerr << "ERROR: cannot create file: " << path << std::endl;
        return false;
    }

    file << mazeToString(map);
    return true;
}

// ============================================================
// Compare the discovered maze against the original one
// ============================================================

std::vector<std::vector<bool>> computeReachable(Maze original, CellPosition start)
{
    std::vector<std::vector<bool>> reachable(
        MAZE_WIDTH, std::vector<bool>(MAZE_HEIGHT, false));

    std::vector<CellPosition> stack = {start};
    reachable[start.x][start.y] = true;

    while (!stack.empty())
    {
        CellPosition pos = stack.back();
        stack.pop_back();

        for (Direction dir : {NORTH, EAST, SOUTH, WEST})
        {
            if (original.hasWall(pos, dir))
                continue;

            CellPosition next = nextPosition(pos, dir);
            if (!original.isValid(next))
                continue;

            if (reachable[next.x][next.y])
                continue;

            reachable[next.x][next.y] = true;
            stack.push_back(next);
        }
    }

    return reachable;
}

bool compareMazes(const Maze& original, const Maze& discovered)
{
    std::vector<std::vector<bool>> reachable = computeReachable(original, {0, 0});

    int mismatchCount = 0;
    int unknownCount = 0;

    for (int x = 0; x < MAZE_WIDTH; ++x)
    {
        for (int y = 0; y < MAZE_HEIGHT; ++y)
        {
            // Cells the robot could never reach are excluded entirely;
            // DFS never visiting them is expected, not a bug.
            if (!reachable[x][y])
                continue;

            CellPosition pos = {x, y};

            for (Direction dir : {NORTH, EAST})
            {
                if (!discovered.isKnown(pos, dir))
                {
                    ++unknownCount;
                    std::cerr << "UNKNOWN (" << x << "," << y << ") "
                              << directionToString(dir) << std::endl;
                    continue;
                }

                bool originalWall = original.hasWall(pos, dir);
                bool discoveredWall = discovered.hasWall(pos, dir);

                if (originalWall != discoveredWall)
                {
                    ++mismatchCount;
                    std::cerr << "MISMATCH (" << x << "," << y << ") "
                              << directionToString(dir)
                              << ": original=" << (originalWall ? "#" : ".")
                              << " discovered=" << (discoveredWall ? "#" : ".")
                              << std::endl;
                }
            }
        }
    }

    bool passed = (mismatchCount == 0) && (unknownCount == 0);

    std::cerr << "mismatches=" << mismatchCount
              << " unknown=" << unknownCount
              << " -> " << (passed ? "PASSED" : "FAILED") << std::endl;

    return passed;
}

// ============================================================
// main
// ============================================================

int main(int argc, char* argv[])
{

    Maze originalMaze;
    if (!loadOriginalMaze(MAZE_PATH, originalMaze))
        return 1;

    printMaze("ORIGINAL", originalMaze);

    dfs.explore();

    printMaze("DISCOVERED", discoveredMaze);
    saveMaze(discoveredMaze, "discovered_maze.txt");

    bool passed = compareMazes(originalMaze, discoveredMaze);

    return passed ? 0 : 1;
}