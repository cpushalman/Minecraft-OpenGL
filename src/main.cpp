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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
void getTileUV(int col, int row, float& uMin, float& vMin, float& uMax, float& vMax) {
    uMin = col * TILE_SIZE;
    vMin = row * TILE_SIZE;
    uMax = uMin + TILE_SIZE;
    vMax = vMin + TILE_SIZE;
}

int main() {



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
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = glfwGetTime(); // ✅ Define time first

        // Setup common view and projection matrices
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            800.0f / 600.0f, 0.1f, 100.0f);

        unsigned int mvpLoc = glGetUniformLocation(shaderProgram, "mvp");

        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);

        for (unsigned int i = 0; i < cubePositions.size(); i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i + time * 25.0f;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

            glm::mat4 mvp = projection * view * model;
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
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
