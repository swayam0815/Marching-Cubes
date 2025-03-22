Marching Cubes with mesh

Overview
This program is an implementation of the marching cubes algorithm and allows users to create two different meshes.

Features
- Click and drag with mouse to move mesh around
- Use up and down key to zoom in or out

IMPORTANT: The program can take some time to load due to nature of calculations! This is dependent on your computer and the stepsize specified. It is currently set to 0.05 to get a nicely detailed mesh.

Requirements
- OpenGL
- GLFW
- GLM
- GLEW
- C++ Compiler (e.g., g++)

Compilation & Execution:

Compile with:
g++ -o assign5 assign5.cpp -lGL -lGLEW -lglfw

Run with:
./assign5

Dependencies:
Main.hpp

Output:
PLY file for generated mesh.