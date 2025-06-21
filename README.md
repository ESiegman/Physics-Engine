# Physics-Engine

This is a C++ physics simulation engine that allows for the real-time simulation of a customizable number of objects in either a 2D or 3D environment. It features a graphical user interface (GUI) built with ImGui, specifically utilizing its docking and viewports features for a flexible layout. The engine uses a spatial grid for efficient collision detection and multithreading for collision resolution.

## Features

* **2D and 3D Simulation**: Toggle between 2D and 3D physics environments.
* **Configurable Parameters**: Adjust gravity, bounciness, object count, world dimensions, and more via the in-application GUI.
* **Efficient Collision Detection**: Utilizes a spatial grid to optimize collision checks between objects.
* **Multithreaded Physics**: Collision resolution is parallelized across multiple threads for improved performance.
* **ImGui-based GUI**: Interactive controls for simulation management and visualization.
* **OpenGL Rendering**: Uses OpenGL for rendering the simulation scene.

## Setup

This project uses `xmake` as its build system.

### NixOS Environment

If you are on NixOS, you can enter a development shell with all the necessary dependencies using the `nix-shell` command from the project's root directory:

```bash
nix-shell
```
This will set up the environment with glfw, opengl, glm, and glew.

### Other Environments

For non-NixOS systems, you will need to install xmake and the required dependencies manually:

1) Install xmake: Follow the instructions on the xmake official website.

2) Install Dependencies: Ensure you have the following libraries installed on your system:
  - GLFW 
  - OpenGL development libraries
  - GLM 
  - GLEW
  - ImGUI (specifically the docking branch, which most obtained from its Github page).
  
  On Ubuntu/Debian, you might install them using:
  ```Bash
  sudo apt-get update
  sudo apt-get install libglfw3-dev libglm-dev libglew-dev libopengl-dev
  ```
  For other operating systems, please refer to their respective package managers or build from source for GLFW, GLM, and GLEW.

## Compiling

Once xmake and the dependencies are set up, navigate to the project's root directory and run the following command to compile:
```Bash
xmake
```

This will build the Physics_Engine binary.

## Running

After successful compilation, the executable will be located in the bin/ directory. You can run the simulation using:
```Bash
xmake run
```

Alternatively, you can directly execute the binary:
```Bash
./bin/Physics_Engine
```

## How to Use

The application will launch with a simulation window and an ImGui-based GUI.

### GUI Controls

-  Settings Window: This panel allows you to modify various simulation parameters such as:
  - Number of Objects
  - Gravity
  - Bounciness (Coefficient of Restitution)
  - Vertical Damping
  - Toggle 3D mode
  - World Dimensions (Width, Height, Depth)
  - Physics Iterations and Fixed Delta Time
  - Default Object Properties (Radius, Mass, Min/Max Start Velocity)
  - Spatial Grid Settings (Cell Size)
  - Camera Settings (Movement Speed, Mouse Sensitivity, FOV)
  - You can also Restart Simulation or Open Camera Controls from here.

### Camera Controls (in 3D mode)

When in 3D mode, you can control the camera as follows:

- W: Move Forward
- S: Move Backward
- A: Move Left
- D: Move Right
- Space: Move Up
- Left Shift: Move Down
- Mouse Movement: Look Around (when not locked)
- Left Control: Toggle Camera Lock (locks/unlocks mouse look)
- ESC: Close Window / Exit Application

