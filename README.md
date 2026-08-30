

## Project Structure

```
Micromouse/
│
├── include/
│   ├── Config.h
│   ├── Directions.h
│   ├── INavigation.h
│   ├── IWallSensors.h
│   └── RobotState.h
│
├── lib/
│   ├── BFS/
│   │   ├── BFS.cpp
│   │   └── BFS.h
│   ├── DFS/
│   │   ├── DFS.cpp
│   │   └── DFS.h
│   ├── DRV8833/
│   │   ├── DRV8833.cpp
│   │   └── DRV8833.h
│   ├── Encoder/
│   │   ├── Encoder.cpp
│   │   └── Encoder.h
│   ├── Logger/
│   │   ├── Logger.cpp
│   │   └── Logger.h
│   ├── Maze/
│   │   ├── Maze.cpp
│   │   └── Maze.h
│   ├── Navigation/
│   │   ├── Navigation.cpp
│   │   └── Navigation.h
│   ├── PID/
│   │   ├── PID.cpp
│   │   └── PID.h
│   ├── VL53L0X/
│   │   ├── VL53L0X.cpp
│   │   └── VL53L0X.h
│   └── WallSensors/
│       ├── WallSensors.cpp
│       └── WallSensors.h
│
├── mms_tests/
│   └── test_DFS/
│       ├── API.cpp
│       ├── API.h
│       └── test_DFS.cpp
│
├── src/
│   ├── main.cpp
│   ├── Robot.cpp
│   └── Robot.h
│
├── test/
│   ├── test_Maze/
│   │   └── test_Maze.cpp
│   └── test_PID/
│       └── test_PID.cpp
│
├── .gitignore
├── platformio.ini
└── README.md

```

---

## Project Layers

```
Drivers
 ├── DRV8833
 ├── VL53L0X
 ├── Encoder
 └── Logger

Robot
 ├── WallSensors
 ├── Navigation
 └── Maze

Control
 └── PID

Algorithms
 ├── DFS
 └── BFS
```
