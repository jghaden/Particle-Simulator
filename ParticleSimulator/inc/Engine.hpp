#pragma once

#include "PCH.hpp"

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 1024;

class Simulation;
class Particle;

struct Character
{
    GLuint TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    GLuint Advance;
};

class Engine
{
private:
    static void WindowResizeCallback(GLFWwindow* window, int width, int height);
    static void MousePositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
public:
    Simulation* simulation;

    GLFWwindow* InitOpenGL();
    int InitFreeType();

    std::map<GLchar, Character> characters;

    std::string ReadShaderFile(const char* filePath);
    GLuint CompileShader(GLenum shaderType, const char* shaderSource);
    GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);
    void LoadAllShaders();
    GLuint GetShader(const std::string& key);

    void SetupParticleBuffers(GLuint& VAO, GLuint& VBO_positions, GLuint& VBO_colors, size_t maxParticles);
    void SetupTextBuffers(GLuint& VAO, GLuint& VBO_positions);

    void UpdateParticleBuffers(const std::vector<Particle>& particles);

    void RenderParticles(GLuint VAO, size_t particleCount);
    void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

    void SetSimulation(Simulation* simulation);

    int Init();
};
