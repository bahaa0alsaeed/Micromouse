

## Project Structure

```
Micromouse/
│
├── src/
|   ├── main.cpp 
|   ├── Robot.h
│   └── Robot.cpp
│
├── include/
│   └── Config.h
│
└── lib/
    │
    ├── DRV8833/
    │   ├── DRV8833.h
    │   └── DRV8833.cpp
    │
    ├── VL53L0X/
    │   ├── VL53L0X.h
    │   └── VL53L0X.cpp
    │
    ├── Encoder/
    │   ├── Encoder.h
    │   └── Encoder.cpp
    │
    ├── Logger/
    │   ├── Logger.h
    │   └── Logger.cpp
    │
    ├── WallSensors/
    │   ├── WallSensors.h
    │   └── WallSensors.cpp
    │
    ├── Navigation/
    │   ├── Navigation.h
    │   └── Navigation.cpp
    │
    ├── Maze/
    │   ├── Maze.h
    │   └── Maze.cpp
    │
    ├── PID/
    │   ├── PID.h
    │   └── PID.cpp
    │
    ├── DFS/
    │   ├── DFS.h
    │   └── DFS.cpp
    │
    └── BFS/
        ├── BFS.h
        └── BFS.cpp

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
