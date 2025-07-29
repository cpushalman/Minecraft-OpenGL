#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <filesystem>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define TILE_SIZE (1.0f / 6.0f)
#define TILE_U(col) ((col) * TILE_SIZE)
#define TILE_V(row) ((row) * TILE_SIZE)
// === Global Camera Variables ===
float playerYVelocity = 0.0f;
const float gravity = -9.8f;
const float jumpStrength = 5.0f;
bool isGrounded = false;

const int CHUNK_SIZE = 16;

struct Chunk {
    int blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    Chunk() {
        for (int x = 0; x < CHUNK_SIZE; ++x)
            for (int y = 0; y < CHUNK_SIZE; ++y)
                for (int z = 0; z < CHUNK_SIZE; ++z)
                    blocks[x][y][z] = (y < CHUNK_SIZE / 2) ? 1 : 0;  // simple terrain
    }
};

glm::vec3 cameraPos = glm::vec3(0.0f, 8.0f, 3.0f); // 1 block above terrain

glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::ivec3 blockPos = glm::floor(cameraPos);
std::vector<glm::vec3> chunkPositions;


float yaw = -90.0f;  // Start facing negative Z
float pitch = 0.0f;
float lastX = 800.0f / 2.0;  // Assuming 800x600 window
float lastY = 600.0f / 2.0;
bool firstMouse = true;
int getHeight(int x, int z) {
    return (int)(5.0f * sin(0.1f * x) * cos(0.1f * z)) + 8;  // wavy hills
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
void getTileUV(int col, int row, float& uMin, float& vMin, float& uMax, float& vMax) {
    uMin = col * TILE_SIZE;
    vMin = row * TILE_SIZE;
    uMax = uMin + TILE_SIZE;
    vMax = vMin + TILE_SIZE;
}
void processInput(GLFWwindow* window, glm::vec3& cameraPos, glm::vec3& cameraFront, glm::vec3& cameraUp, float deltaTime) {
    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && isGrounded) {
        playerYVelocity = jumpStrength;
        isGrounded = false;
    }


    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}
void generateChunk(Chunk& chunk, int chunkX, int chunkY, int chunkZ) {
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            int worldX = chunkX * CHUNK_SIZE + x;
            int worldZ = chunkZ * CHUNK_SIZE + z;
            int height = getHeight(worldX, worldZ);  // world height at (x,z)

            for (int y = 0; y < CHUNK_SIZE; ++y) {
                int worldY = chunkY * CHUNK_SIZE + y;

                if (worldY <= height) {
                    chunk.blocks[x][y][z] = 1; // Block is filled
                }
                else {
                    chunk.blocks[x][y][z] = 0; // Air
                }
            }
        }
    }
}
void renderChunk(const Chunk& chunk, const glm::vec3& chunkPos, unsigned int shaderProgram, unsigned int mvpLoc, const glm::mat4& view, const glm::mat4& projection) {
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                if (chunk.blocks[x][y][z] == 1) {
                    glm::vec3 blockWorldPos = chunkPos + glm::vec3(x, y, z);
                    glm::mat4 model = glm::translate(glm::mat4(1.0f), blockWorldPos);
                    glm::mat4 mvp = projection * view * model;
                    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
                }
            }
        }
    }
}

bool isBlockSolid(const std::vector<Chunk>& chunks, const std::vector<glm::vec3>& chunkPositions, const glm::vec3& worldPos)
{
    int x = (int)floor(worldPos.x);
    int y = (int)floor(worldPos.y);
    int z = (int)floor(worldPos.z);

    int chunkX = floor(x / CHUNK_SIZE);
    int chunkY = floor(y / CHUNK_SIZE);
    int chunkZ = floor(z / CHUNK_SIZE);

    int localX = x % CHUNK_SIZE;
    int localY = y % CHUNK_SIZE;
    int localZ = z % CHUNK_SIZE;

    // Adjust negative mods
    if (localX < 0) localX += CHUNK_SIZE;
    if (localY < 0) localY += CHUNK_SIZE;
    if (localZ < 0) localZ += CHUNK_SIZE;

    for (size_t i = 0; i < chunks.size(); ++i) {
        glm::vec3 pos = chunkPositions[i];
        if ((int)(pos.x / CHUNK_SIZE) == chunkX &&
            (int)(pos.y / CHUNK_SIZE) == chunkY &&
            (int)(pos.z / CHUNK_SIZE) == chunkZ) {

            return chunks[i].blocks[localX][localY][localZ] == 1;
        }
    }
    return false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static float sensitivity = 0.1f;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed: y ranges bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Clamp pitch
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Update camera front vector
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}


int main() {
    std::vector<Chunk> chunks;
    std::vector<glm::vec3> chunkPositions;

    for (int x = -1; x <= 1; ++x) {
        for (int z = -1; z <= 1; ++z) {
            Chunk chunk;
            generateChunk(chunk, x, 0, z); // Use your perlin-based terrain
            chunks.push_back(chunk);
            chunkPositions.emplace_back(x * CHUNK_SIZE, 0, z * CHUNK_SIZE);

        }
    }



    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    const char* texturePath = "textures/atlas.png";
    std::ifstream file(texturePath);
    if (!file.good()) {
        std::cerr << "File not found: " << texturePath << std::endl;
    }
  
    // Vertex data
   // UVs for each face
    float top_uMin, top_vMin, top_uMax, top_vMax;
    float bottom_uMin, bottom_vMin, bottom_uMax, bottom_vMax;
    float front_uMin, front_vMin, front_uMax, front_vMax;
    float back_uMin, back_vMin, back_uMax, back_vMax;
    float left_uMin, left_vMin, left_uMax, left_vMax;
    float right_uMin, right_vMin, right_uMax, right_vMax;

    getTileUV(1, 1, front_uMin, front_vMin, front_uMax, front_vMax);  // Stone
    getTileUV(2, 1, back_uMin, back_vMin, back_uMax, back_vMax);      // Cobble
    getTileUV(3, 1, left_uMin, left_vMin, left_uMax, left_vMax);      // Wood
    getTileUV(4, 1, right_uMin, right_vMin, right_uMax, right_vMax);  // Brick

    getTileUV(0, 0, top_uMin, top_vMin, top_uMax, top_vMax);     // Grass top
    getTileUV(0, 1, bottom_uMin, bottom_vMin, bottom_uMax, bottom_vMax); // Dirt
   

    float vertices[] = {
        // Front face (Stone)
        -0.5f, -0.5f,  0.5f,  front_uMin, front_vMax,
         0.5f, -0.5f,  0.5f,  front_uMax, front_vMax,
         0.5f,  0.5f,  0.5f,  front_uMax, front_vMin,
        -0.5f,  0.5f,  0.5f,  front_uMin, front_vMin,

        // Back face (Cobble)
        -0.5f, -0.5f, -0.5f,  back_uMax, back_vMax,
         0.5f, -0.5f, -0.5f,  back_uMin, back_vMax,
         0.5f,  0.5f, -0.5f,  back_uMin, back_vMin,
        -0.5f,  0.5f, -0.5f,  back_uMax, back_vMin,

        // Left face (Wood)
        -0.5f, -0.5f, -0.5f,  left_uMin, left_vMax,
        -0.5f, -0.5f,  0.5f,  left_uMax, left_vMax,
        -0.5f,  0.5f,  0.5f,  left_uMax, left_vMin,
        -0.5f,  0.5f, -0.5f,  left_uMin, left_vMin,

        // Right face (Brick)
         0.5f, -0.5f, -0.5f,  right_uMax, right_vMax,
         0.5f, -0.5f,  0.5f,  right_uMin, right_vMax,
         0.5f,  0.5f,  0.5f,  right_uMin, right_vMin,
         0.5f,  0.5f, -0.5f,  right_uMax, right_vMin,

         // Top face (Grass Top)
         -0.5f,  0.5f, -0.5f,  top_uMin, top_vMin,
          0.5f,  0.5f, -0.5f,  top_uMax, top_vMin,
          0.5f,  0.5f,  0.5f,  top_uMax, top_vMax,
         -0.5f,  0.5f,  0.5f,  top_uMin, top_vMax,

         // Bottom face (Dirt)
         -0.5f, -0.5f, -0.5f,  bottom_uMin, bottom_vMax,
          0.5f, -0.5f, -0.5f,  bottom_uMax, bottom_vMax,
          0.5f, -0.5f,  0.5f,  bottom_uMax, bottom_vMin,
         -0.5f, -0.5f,  0.5f,  bottom_uMin, bottom_vMin
    };

    unsigned int indices[] = {
        0, 1, 2,  2, 3, 0,        // front
        4, 5, 6,  6, 7, 4,        // back
        8, 9,10, 10,11, 8,        // left
       12,13,14, 14,15,12,        // right
       16,17,18, 18,19,16,        // top
       20,21,22, 22,23,20         // bottom
    };




    // Vertex Shader source
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "out vec2 TexCoord;\n"
        "uniform mat4 mvp;\n"
        "void main() {\n"
        "   gl_Position = mvp * vec4(aPos, 1.0);\n"
        "   TexCoord = aTexCoord;\n"
        "}\0";

    // Fragment Shader source
    const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D ourTexture;\n"
        "void main() {\n"
        "   FragColor = texture(ourTexture, TexCoord);\n"
        "}\n";

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Set OpenGL version to 3.3 core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
   
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Load OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Set viewport and callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glViewport(0, 0, 800, 600);

    // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Create shader program
    unsigned int shaderProgram = glCreateProgram(); // Create BEFORE attaching
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up VAO and VBO
    unsigned int VBO, VAO,EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    // Define vertex layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set texture wrapping/filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image using stb_image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);  // optional, but usually needed
    unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
            GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture: " << stbi_failure_reason() << "\n";
    }
    stbi_image_free(data);
    glEnable(GL_DEPTH_TEST);
    std::vector<glm::vec3> cubePositions = {
      { 0.0f,  0.0f,  0.0f},
      { 2.0f,  5.0f, -15.0f},
      {-1.5f, -2.2f, -2.5f},
      {-3.8f, -2.0f, -12.3f},
      { 2.4f, -0.4f, -3.5f},
      {-1.7f,  3.0f, -7.5f},
      { 1.3f, -2.0f, -2.5f},
      { 1.5f,  2.0f, -2.5f},
      { 1.5f,  0.2f, -1.5f},
      {-1.3f,  1.0f, -1.5f}
    };

    // Render loop
    while (!glfwWindowShouldClose(window)) {

        glClearColor(0.52f, 0.80f, 0.92f, 1.0f);  // nice daytime blue

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float time = glfwGetTime(); // ✅ Define time first
        float currentFrame = glfwGetTime();


        // Setup common view and projection matrices
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, cameraPos, cameraFront, cameraUp, deltaTime);
        // Apply gravity
       playerYVelocity += gravity * deltaTime;
cameraPos.y += playerYVelocity * deltaTime;

// Correct feet offset
glm::vec3 feet = cameraPos + glm::vec3(0.0f, -1.8f, 0.0f);  // exact foot point

if (isBlockSolid(chunks, chunkPositions, feet) && playerYVelocity < 0.0f) {
    // Land on block: snap exactly to top of block
    cameraPos.y = floor(cameraPos.y - 1.8f) + 2.8f;  // 1.8 offset + block height (1)
    playerYVelocity = 0.0f;
    isGrounded = true;
} else {
    isGrounded = false;
}



     

      

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            800.0f / 600.0f, 0.1f, 100.0f);

        unsigned int mvpLoc = glGetUniformLocation(shaderProgram, "mvp");

        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);

        for (size_t i = 0; i < chunks.size(); ++i) {
            renderChunk(chunks[i], chunkPositions[i], shaderProgram, mvpLoc, view, projection);
        }


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
