#pragma once
#include <string>

class Maze;

enum LogLevel
{
    OFF,
    NORMAL,
    DEBUG
};

class Logger
{
    LogLevel level = LogLevel::NORMAL;

public:
    Logger();
    void setLevel(LogLevel level);

    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void debug(const std::string& message);

    void printMaze(Maze& maze);
};

extern Logger logger;
