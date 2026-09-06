# Micromouse

A software project for a Micromouse robot, focused on navigation, maze exploration, path finding, motion control, simulation, and testing.

The project is designed in layers so that the algorithms are separated from the low-level hardware drivers. The `Robot` class acts as the main coordinator: it creates and initializes the required objects, passes their dependencies, coordinates the workflow, and provides a simple interface to `main`.

---

## Architecture

The project is organized into several layers:

### Drivers

Low-level components that provide access to hardware functionality without depending on the rest of the project.

- `DRV8833` — controls the motors.

- `Encoder` — provides wheel speed and travelled distance.

- `VL53L0X` — external sensor library used by `WallSensors`.

- `Logger` — shared logging facility used throughout the project.


### Core Components

- `WallSensors` — reads the three wall-detection sensors.

- `Maze` — represents and manages the maze map, including walls, visited cells, and other cell information.

- `Navigation` — controls the robot's movement and hides the low-level motion-control details from the algorithms.


`Navigation` uses the encoders, motor driver, wall sensors, and PID controllers to control the robot.

### Control

The robot uses a **cascade PID control system**. Each motor has an inner PID loop for speed control, while an outer PID loop uses the motor controllers to keep the robot centered while moving.

### Algorithms

- `DFS` — explores the maze and updates the discovered maze.

- `BFS` — analyzes the discovered maze, finds the shortest path, and executes it through `Navigation`.


### Robot

`Robot` is the highest-level software layer. It creates and initializes the required objects, connects their dependencies, coordinates the algorithms and components, and exposes a simple interface to `main`.

A separate **Class Diagram** is provided to show the detailed relationships between the classes.

---

## Robot Workflow

The main workflow of the robot is:

```text
main
  ↓
Robot
  ↓
DFS
  ↓
Maze Exploration
  ↓
Discovered Maze
  ↓
BFS
  ↓
Shortest Path
  ↓
Navigation
  ↓
Robot Movement
```

`DFS` is responsible only for exploration, while `BFS` uses the resulting maze map to find and execute the shortest path.

---

## Project Structure

```text
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
│   └── WallSensors/
│       ├── WallSensors.cpp
│       └── WallSensors.h
│
├── mms_tests/
│   ├── API.cpp
│   ├── API.h
│   └── test.cpp
│
├── src/
│   ├── main.cpp
│   ├── Robot.cpp
│   └── Robot.h
│
└── test/
    ├── test_Maze/
    │   └── test_Maze.cpp
    └── test_PID/
        └── test_PID.cpp
```

---

## Testing

The project uses **Unity** for unit testing.

To run all available tests:

```bash
pio test -e native
```

The current tests cover components such as:

- `Maze`

- `PID`


---

## Simulation

The project includes a Micromouse simulator test environment under `mms_tests`.

The simulator provides an `API` that allows the robot algorithms to interact with a virtual maze.

### Build the simulator

From the `mms_tests` directory:

```bash
g++ API.cpp test.cpp ../lib/Maze/Maze.cpp ../lib/DFS/DFS.cpp ../lib/BFS/BFS.cpp -I../include -I../lib/Maze -I../lib/DFS -I../lib/BFS -static-libgcc -static-libstdc++ -o test.exe
```

Then run:

```bash
test.exe
```

The simulator is used to test maze exploration and path-finding algorithms without running them on the physical robot.