/**
  ******************************************************************************
  * @file    Engine.hpp
  * @author  Josh Haden
  * @version V0.1.0
  * @date    19 JAN 2025
  * @brief   Header for Engine.cpp
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __ENGINE_HPP
#define __ENGINE_HPP

/* Includes ----------------------------------------------------------------- */

#include "PCH.hpp"

#include "Font.hpp"

/* Exported types ----------------------------------------------------------- */

typedef struct
{
    GLuint TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    GLuint Advance;
} CHARACTER_T;

typedef std::string                           SHADER_SOURCE_T;
typedef std::chrono::duration<float>          DURATION_T;
typedef std::chrono::system_clock::time_point TIME_POINT_T;
typedef std::map<GLchar, CHARACTER_T>         CHARACTERS_T;
typedef std::map<std::string, GLuint>         SHADERS_T;


/* Exported constants ------------------------------------------------------- */

const int WINDOW_WIDTH  = 1024;     // Initial width of window
const int WINDOW_HEIGHT = 1024;     // Initial height of window

/* Exported macro ----------------------------------------------------------- */
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
/* Forward declarations ----------------------------------------------------- */

class Particle;
class Simulation;

/* Class definition --------------------------------------------------------- */

class Engine
{
public:
    /* Public member variables -------------------------------------------------- */
    /* Public member functions -------------------------------------------------- */

    Engine();
    ~Engine();

    int Init();
    int InitFreeType();
    void InitParticleBuffers(GLuint& VAO, GLuint& VBO_positions, GLuint& VBO_colors, size_t maxParticles);
    void InitTextBuffers(GLuint& VAO, GLuint& VBO_positions);
    GLFWwindow* InitOpenGL();

    void LoadAllShaders();
    void RenderCircle(float x, float y, float radius, float outlineThickness, glm::vec4 fillColor, glm::vec4 outlineColor);
    void RenderParticles(GLuint VAO, size_t particleCount);
    void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat pointSize, FONT_T font, glm::vec3 color);
    void Run();    
    void UpdateParticleBuffers(const std::vector<Particle>& particles);
    GLuint CompileShader(GLenum shaderType, const char* shaderSource);
    GLuint LinkShaders(const char* vertex_file_path, const char* fragment_file_path);
    SHADER_SOURCE_T ReadShaderFile(const char* filePath) const;

    /* Getters ------------------------------------------------------------------ */

    size_t GetWindowWidth() const;
    size_t GetWindowHeight() const;
    GLuint GetShader(const std::string& key) const;
    Simulation* GetSimulation() const;
    GLFWwindow* GetGLFWWindow() const;

    /* Setters ------------------------------------------------------------------ */

    void SetSimulation(Simulation* simulation);
    void SetWindowSize(int width, int height);
private:
    /* Private member variables ------------------------------------------------- */

    size_t       window_width             = WINDOW_WIDTH;
    size_t       window_height            = WINDOW_HEIGHT;
    CHARACTERS_T fonts[NUMBER_OF_FONTS];
    Simulation*  simulation               = nullptr;
    GLFWwindow*  glfwWindow               = nullptr;

    /* Private member functions ------------------------------------------------- */

    static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void MousePositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void WindowResizeCallback(GLFWwindow* window, int width, int height);
    int GetPixelSizeFromPointSize(float pointSize, float dpi) const;

    /* Getters ------------------------------------------------------------------ */
    /* Setters ------------------------------------------------------------------ */
};



#endif /* __ENGINE_HPP */

/******************************** END OF FILE *********************************/
