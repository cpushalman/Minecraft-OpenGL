# 🧱 Simple Minecraft Clone (C++ + OpenGL)

This project is a basic Minecraft-style voxel engine built using **C++**, **OpenGL**, **GLFW**, **GLAD**, and **GLM**. It features chunk-based terrain generation, player movement with gravity and jumping, basic collision detection, and textured cube rendering using a texture atlas.

---

## 🎮 Features Implemented

### ✅ Chunk-Based World
- Each chunk is a 16×16×16 3D array of blocks.
- Procedural terrain generated with a sinusoidal heightmap.

### ✅ Block Rendering
- Blocks are rendered as cubes using indexed vertex data (VBO + EBO).
- Texture atlas support using UV mapping for multiple block types (grass, dirt, stone, etc.).

### ✅ Camera & Movement
- FPS-style camera using `glm::lookAt`.
- Mouse-look support via GLFW cursor callback.
- WASD movement.
- Space to jump.

### ✅ Gravity & Collision
- Gravity is applied every frame.
- Player will fall until they hit the ground.
- Collision is calculated by checking the block below the player’s feet.
- Snaps camera to the top of the block on landing.

### ✅ Texture Support
- Textures loaded using `stb_image`.
- Mipmaps enabled for better visual quality at a distance.

---


https://github.com/user-attachments/assets/15c75f2c-413f-497f-ab31-c3ce414d99a8


---

## 🛠️ Dependencies

- [GLFW](https://www.glfw.org/) – window/context/input
- [GLAD](https://glad.dav1d.de/) – OpenGL function loader
- [GLM](https://github.com/g-truc/glm) – math library
- [stb_image](https://github.com/nothings/stb) – image loader

---

## 🚀 How to Run

1. Make sure you have a C++17-compatible compiler.
2. Install GLFW, GLAD, GLM, stb_image.
3. Link OpenGL libraries.
4. Build and run:
   ```sh
   g++ main.cpp -lglfw -lGL -ldl -lX11 -lpthread -lm -o minecraft
   ./minecraft
