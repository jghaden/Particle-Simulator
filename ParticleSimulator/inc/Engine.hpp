/**
  ******************************************************************************
  * @file    Engine.hpp
  * @author  Josh Haden
  * @version V0.0.1
  * @date    18 JAN 2025
  * @brief   Header for Engine.cpp
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ENGINE_HPP
#define __ENGINE_HPP

/* Includes ------------------------------------------------------------------*/

#include "PCH.hpp"

#include "Font.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 1024;

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* Class definition --------------------------------------------------------- */

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

    int window_width;
    int window_height;
public:

    Simulation* simulation;
    GLFWwindow* window;

    GLFWwindow* InitOpenGL();
    int InitFreeType();

    std::map<GLchar, Character> characters[NUMBER_OF_FONTS];

    std::string ReadShaderFile(const char* filePath);
    GLuint CompileShader(GLenum shaderType, const char* shaderSource);
    GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);
    void LoadAllShaders();
    GLuint GetShader(const std::string& key);

    int GetWindowWidth() const;
    int GetWindowHeight() const;
    void SetWindowSize(int width, int height);

    void SetupParticleBuffers(GLuint& VAO, GLuint& VBO_positions, GLuint& VBO_colors, size_t maxParticles);
    void SetupTextBuffers(GLuint& VAO, GLuint& VBO_positions);

    void UpdateParticleBuffers(const std::vector<Particle>& particles);

    void RenderCircle(float x, float y, float radius, float outlineThickness, glm::vec4 fillColor, glm::vec4 outlineColor);
    void RenderParticles(GLuint VAO, size_t particleCount);
    void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat pointSize, FONT_T font, glm::vec3 color);

    void SetSimulation(Simulation* simulation);

    int Init();
    void Run();
};



#endif /* __ENGINE_HPP */

/************************END OF FILE************************/
