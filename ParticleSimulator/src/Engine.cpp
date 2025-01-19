/**
  ******************************************************************************
  * @file    Engine.cpp
  * @author  Josh Haden
  * @version V0.0.1
  * @date    18 JAN 2025
  * @brief
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "PCH.hpp"

#include "Engine.hpp"
#include "Simulation.hpp"
#include "Particle.hpp"

/* Global Variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static bool                                  isClearParticles;
static bool                                  isCtrlMouseLeftClick;
static bool                                  isCtrlMouseLeftClickPrev;
static bool                                  isFrameStepping;
static bool                                  isKeyLeftAltPressed;
static bool                                  isKeyLeftCtrlPressed;
static bool                                  isKeyLeftShiftPressed;
static bool                                  isShowingUI;
static bool                                  isSimulationPaused;
static int                                   cursor_state;
static int                                   cursor_state_prev;
static int                                   particleMassExp;
static char                                  textBuffer[256];
static float                                 fElapsedTime;
static float                                 normalizedFaceHeight;
static double                                cursor_window_xpos;
static double                                cursor_window_ypos;
static std::chrono::duration<float>          elapsedTime;
static std::chrono::system_clock::time_point tp1;
static std::chrono::system_clock::time_point tp2;
static FT_Face                               face;
static FT_Library                            ft;
static GLuint                                VAO;
static GLuint                                VAO_text;
static GLuint                                VBO_colors;
static GLuint                                VBO_positions;
static GLuint                                VBO_text;
static glm::mat4                             projection;
static glm::mat4                             projectionText;
static std::map<std::string, GLuint>         shaders;

/* Private function prototypes -----------------------------------------------*/

int GetPixelSizeFromPointSize(float pointSize, float dpi = 96.0f);


/******************************************************************************/
/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/******************************************************************************/


// Initialize GLFW and create a window
GLFWwindow* Engine::InitOpenGL()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }

    // Set the required OpenGL version to 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 16);

    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    int screen_width = mode->width;
    int screen_height = mode->height;

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Particle Simulator", nullptr, nullptr);
    glfwSetWindowPos(window, (screen_width - WINDOW_WIDTH) / 2, (screen_height - WINDOW_HEIGHT) / 2);

    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return nullptr;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_PROGRAM_POINT_SIZE);

    return window;
}

void Engine::WindowResizeCallback(GLFWwindow* window, int width, int height)
{
    int left, top, right, bottom;
    glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);

    glViewport(0, 0, width, height);
}

int Engine::InitFreeType()
{   
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "Could not init FreeType Library" << std::endl;
        return -1;
    }

    // Load a font face
    if (FT_New_Face(ft, "../data/fonts/Roboto/Roboto-Light.ttf", 0, &face))
    {
        std::cerr << "Failed to load font" << std::endl;
        return -1;
    }

    float pointSize = 64.0f; // Desired font size in points
    int pixelHeight = GetPixelSizeFromPointSize(pointSize, 96.0f);
    FT_Set_Pixel_Sizes(face, 0, pixelHeight);

    normalizedFaceHeight = face->size->metrics.height / 64.0f; // Convert to float

    // Load the first 128 characters of the ASCII set
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "Failed to load Glyph" << std::endl;
            continue;
        }

        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //// Store character for later use
        this->characters.insert(std::pair<GLchar, Character>(c, {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x >> 6)
        }));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Destroy FreeType resources
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return 0;
}

// Helper function to read shader source code from a file
std::string Engine::ReadShaderFile(const char* filePath)
{
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open())
    {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    return shaderStream.str();
}

// Function to compile a shader and check for errors
GLuint Engine::CompileShader(GLenum shaderType, const char* shaderSource)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    GLint success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }

    return shader;
}

// Function to load, compile, and link shaders
GLuint Engine::LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
    // Read the vertex shader code from the file
    std::string vertexShaderCode = ReadShaderFile(vertex_file_path);
    if (vertexShaderCode.empty()) return 0;

    // Read the fragment shader code from the file
    std::string fragmentShaderCode = ReadShaderFile(fragment_file_path);
    if (fragmentShaderCode.empty()) return 0;

    // Compile the vertex shader
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderCode.c_str());
    if (!vertexShader) return 0;

    // Compile the fragment shader
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode.c_str());
    if (!fragmentShader) return 0;

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    GLint success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        glDeleteProgram(shaderProgram);
        return 0;
    }

    // Clean up shaders as they are no longer needed after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void Engine::LoadAllShaders()
{
    shaders["particle"] = LoadShaders("../data/shaders/particle.vs", "../data/shaders/particle.fs");
    shaders["text"]= LoadShaders("../data/shaders/text.vs", "../data/shaders/text.fs");
}

GLuint Engine::GetShader(const std::string& key)
{
    return shaders[key];
}

void Engine::SetupParticleBuffers(GLuint& VAO, GLuint& VBO_positions, GLuint& VBO_colors, size_t maxParticles)
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO_positions);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * 2 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &VBO_colors);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * 3 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
}


void Engine::SetupTextBuffers(GLuint& VAO, GLuint& VBO_positions)
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO_positions);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW); // 6 vertices, 4 attributes (x, y, u, v)

    // Position and texture coordinates
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0); // Position (x, y)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat))); // Texture coords (u, v)
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Engine::UpdateParticleBuffers(const std::vector<Particle>& particles)
{
    std::vector<GLfloat> positions;
    std::vector<GLfloat> colors;

    for (const auto& p : particles)
    {
        positions.push_back(static_cast<GLfloat>(p.position.x));
        positions.push_back(static_cast<GLfloat>(p.position.y));
        colors.push_back(p.color.r);
        colors.push_back(p.color.g);
        colors.push_back(p.color.b);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(GLfloat), positions.data());

    glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
    glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(GLfloat), colors.data());
}

void Engine::RenderParticles(GLuint VAO, size_t particleCount)
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, (GLsizei)particleCount);
}

void Engine::RenderText(std::string text, GLfloat x, GLfloat y, GLfloat pointSize, glm::vec3 color)
{
    GLuint shaderProgram = GetShader("text");
    glUseProgram(shaderProgram);

    // Set the text color
    GLint textColorLocation = glGetUniformLocation(shaderProgram, "textColor");
    glUniform3f(textColorLocation, color.x, color.y, color.z);

    // Set the projection matrix
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionText));

    GLint zoomLoc = glGetUniformLocation(shaderProgram, "zoom");
    glUniform1f(zoomLoc, 1.0f);

    glBindVertexArray(VAO_text);

    float scaleFactor = pointSize / normalizedFaceHeight;

    // Determine the baseline offset (largest Bearing.y in the text)
    GLfloat baselineOffset = 0.0f;
    for (const char& c : text)
    {
        Character ch = this->characters[c];
        baselineOffset = std::max(baselineOffset, (GLfloat)ch.Bearing.y);
    }

    // Iterate through all characters
    for (const char& c : text)
    {
        Character ch = this->characters[c];

        // Adjust position for the glyph relative to the baseline
        GLfloat xpos = x + ch.Bearing.x * scaleFactor;
        GLfloat ypos = y + (baselineOffset - ch.Bearing.y) * scaleFactor;

        GLfloat w = ch.Size.x * scaleFactor;
        GLfloat h = ch.Size.y * scaleFactor;

        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 1.0f }, // Bottom-left
            { xpos,     ypos,       0.0f, 0.0f }, // Top-left
            { xpos + w, ypos,       1.0f, 0.0f }, // Top-right

            { xpos,     ypos + h,   0.0f, 1.0f }, // Bottom-left
            { xpos + w, ypos,       1.0f, 0.0f }, // Top-right
            { xpos + w, ypos + h,   1.0f, 1.0f }  // Bottom-right
        };

        // Bind the glyph texture
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO_text);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        // Render the quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Advance to the next character
        x += ch.Advance * scaleFactor;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Engine::MousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    cursor_window_xpos = xpos;
    cursor_window_ypos = ypos;
}

void Engine::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (action)
    {
        case GLFW_RELEASE:
            switch (button)
            {
                case GLFW_MOUSE_BUTTON_LEFT:
                    isCtrlMouseLeftClick = false;
                default:
                    cursor_state = -1;
                    break;
            }
            break;
        case GLFW_PRESS:
            switch (button)
            {
                case GLFW_MOUSE_BUTTON_LEFT:
                    cursor_state = GLFW_MOUSE_BUTTON_LEFT;
                    isCtrlMouseLeftClick = isKeyLeftCtrlPressed ? true : false;
                    break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    cursor_state = GLFW_MOUSE_BUTTON_RIGHT;
                    break;
            }
            break;
        default:
            cursor_state = -1;
            break;
    }

    cursor_state_prev = cursor_state;
}

void Engine::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));

    if (e)
    {
        if (e->simulation->particleBrushSize < 100 && yoffset == 1)
        {
            e->simulation->particleBrushSize++;
        } else if (e->simulation->particleBrushSize > 1 && yoffset == -1)
        {
            e->simulation->particleBrushSize--;
        }
    }
}

void Engine::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));

    if (e)
    {
        switch (action)
        {
            case GLFW_RELEASE:
                switch (key)
                {
                    case GLFW_KEY_LEFT_CONTROL: isKeyLeftCtrlPressed = false; break;
                }
                break;
            case GLFW_PRESS:
                switch (key)
                {
                    
                    case GLFW_KEY_SPACE:
                    {
                        printf("Simulation %s\r\n", !isSimulationPaused ? "paused" : "resumed");
                        isSimulationPaused = !isSimulationPaused;
                        isFrameStepping = false;
                        break;
                    }

                    case GLFW_KEY_0: particleMassExp = (isKeyLeftCtrlPressed) ? 10 : 0; break;
                    case GLFW_KEY_1: particleMassExp = (isKeyLeftCtrlPressed) ? 11 : 1; break;
                    case GLFW_KEY_2: particleMassExp = (isKeyLeftCtrlPressed) ? 12 : 2; break;
                    case GLFW_KEY_3: particleMassExp = (isKeyLeftCtrlPressed) ? 13 : 3; break;
                    case GLFW_KEY_4: particleMassExp = (isKeyLeftCtrlPressed) ? 14 : 4; break;
                    case GLFW_KEY_5: particleMassExp = (isKeyLeftCtrlPressed) ? 15 : 5; break;
                    case GLFW_KEY_6: particleMassExp = (isKeyLeftCtrlPressed) ? 16 : 6; break;
                    case GLFW_KEY_7: particleMassExp = (isKeyLeftCtrlPressed) ? 17 : 7; break;
                    case GLFW_KEY_8: particleMassExp = (isKeyLeftCtrlPressed) ? 18 : 8; break;
                    case GLFW_KEY_9: particleMassExp = (isKeyLeftCtrlPressed) ? 19 : 9; break;                    

                    case GLFW_KEY_F: isFrameStepping = true; isSimulationPaused = true; e->simulation->updateParticles(); break;
                    case GLFW_KEY_R: isClearParticles = true; break;
                    case GLFW_KEY_S: e->simulation->newParticleVelocity -= (isKeyLeftCtrlPressed) ? 10 : 1.0; break;
                    case GLFW_KEY_W: e->simulation->newParticleVelocity += (isKeyLeftCtrlPressed) ? 10 : 1.0; break;

                    case GLFW_KEY_LEFT_BRACKET: e->simulation->particleBrushSize -= 10; break;
                    case GLFW_KEY_RIGHT_BRACKET: e->simulation->particleBrushSize += 10; break;

                    case GLFW_KEY_F1: isShowingUI = !isShowingUI; break;

                    case GLFW_KEY_ESCAPE: break;
                    case GLFW_KEY_LEFT_CONTROL: isKeyLeftCtrlPressed = true; break;

                    case GLFW_KEY_KP_0: particleMassExp = (isKeyLeftCtrlPressed) ? 30 : 20; break;
                    case GLFW_KEY_KP_1: particleMassExp = (isKeyLeftCtrlPressed) ? 31 : 21; break;
                    case GLFW_KEY_KP_2: particleMassExp = (isKeyLeftCtrlPressed) ? 32 : 22; break;
                    case GLFW_KEY_KP_3: particleMassExp = (isKeyLeftCtrlPressed) ? 33 : 23; break;
                    case GLFW_KEY_KP_4: particleMassExp = (isKeyLeftCtrlPressed) ? 34 : 24; break;
                    case GLFW_KEY_KP_5: particleMassExp = (isKeyLeftCtrlPressed) ? 35 : 25; break;
                    case GLFW_KEY_KP_6: particleMassExp = (isKeyLeftCtrlPressed) ? 36 : 26; break;
                    case GLFW_KEY_KP_7: particleMassExp = (isKeyLeftCtrlPressed) ? 37 : 27; break;
                    case GLFW_KEY_KP_8: particleMassExp = (isKeyLeftCtrlPressed) ? 38 : 28; break;
                    case GLFW_KEY_KP_9: particleMassExp = (isKeyLeftCtrlPressed) ? 39 : 29; break;
                }

                e->simulation->newParticleMass = powl(10, particleMassExp);

                printf("Particle [mass: %.1e, velocity: %.2lf]\r\n", e->simulation->newParticleMass, e->simulation->newParticleVelocity);
                break;
        }
    }
}

void Engine::SetSimulation(Simulation* simulation)
{
    this->simulation = simulation;
}

int Engine::Init()
{
    isKeyLeftAltPressed      = false;
    isKeyLeftCtrlPressed     = false;
    isKeyLeftShiftPressed    = false;
    isCtrlMouseLeftClick     = false;
    isCtrlMouseLeftClickPrev = false;
    isClearParticles         = false;
    isSimulationPaused       = false;
    isFrameStepping          = false;
    isShowingUI              = true;
    particleMassExp          = 8;
    cursor_state             = -1;
    cursor_state_prev        = -1;
    cursor_window_xpos       = -1;
    cursor_window_ypos       = -1;
    projection               = glm::ortho(
        -1.0f, 1.0f,
        -1.0f, 1.0f,
        0.1f, 100.0f 
    );
    projectionText = glm::ortho(
        0.0f, static_cast<GLfloat>(WINDOW_WIDTH),
        static_cast<GLfloat>(WINDOW_HEIGHT), 0.0f,
        -1.0f, 1.0f
    );

    window = InitOpenGL();
    if (!window) return -1;

    if (InitFreeType() != 0) return -1;

    glfwSetWindowUserPointer(window, this);
    glfwSetWindowSizeCallback(window, WindowResizeCallback);
    glfwSetCursorPosCallback(window, MousePositionCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, MouseScrollCallback);
    glfwSetKeyCallback(window, KeyboardCallback);

    LoadAllShaders();

    GLuint shaderParticle = GetShader("particle");
    GLuint shaderText = GetShader("text");

    SetupParticleBuffers(VAO, VBO_positions, VBO_colors, this->simulation->getMaxParticleCount());
    SetupTextBuffers(VAO_text, VBO_text);

    glUseProgram(shaderParticle);

    glm::mat4 model = glm::mat4(1.0f);

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f),   // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f),   // Look at position
        glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
    );

    float zoom = 1.f;

    glm::mat4 mvp = projection * view * model;

    GLint mvpLoc = glGetUniformLocation(shaderParticle, "MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    GLint zoomLoc = glGetUniformLocation(shaderParticle, "zoom");
    glUniform1f(zoomLoc, zoom);

    this->Run();

    glDeleteBuffers(1, &VBO_positions);
    glDeleteBuffers(1, &VBO_colors);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderParticle);
    glDeleteProgram(shaderText);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void Engine::Run()
{
    GLuint shaderParticle = GetShader("particle");

    std::vector<Particle> particles;
    this->simulation->SetParticles(&particles);
    this->simulation->initializeParticles();

    while (!glfwWindowShouldClose(window))
    {
        tp2 = std::chrono::system_clock::now();
        elapsedTime = tp2 - tp1;
        tp1 = tp2;
        fElapsedTime = elapsedTime.count();

        if (cursor_state == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (isCtrlMouseLeftClick)
            {
                if (!isCtrlMouseLeftClickPrev)
                {
                    this->simulation->addParticle(glm::vec2(cursor_window_xpos, cursor_window_ypos));
                }
            }
            else
            {
                this->simulation->addParticles(glm::vec2(cursor_window_xpos, cursor_window_ypos));
            }
        }
        else if (cursor_state == GLFW_MOUSE_BUTTON_RIGHT)
        {
            this->simulation->removeParticle(glm::vec2(cursor_window_xpos, cursor_window_ypos));
        }

        isCtrlMouseLeftClickPrev = isCtrlMouseLeftClick;

        if (isClearParticles)
        {
            isClearParticles = false;

            this->simulation->removeAllParticles();
        }

        if (!isSimulationPaused)
        {
            this->simulation->updateParticles();
        }

        UpdateParticleBuffers(particles);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        // Render particles
        glDisable(GL_BLEND);
        glUseProgram(shaderParticle);
        RenderParticles(VAO, particles.size());

        // Render text
        if (isShowingUI)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            std::string textParticles = "Particles: " + std::to_string(this->simulation->getParticleCount());
            RenderText(textParticles, 10.0f, 10.0f, 20.0f, glm::vec3(1.0f));

            std::string textFPS = "FPS: " + std::to_string((int)(1.0f / fElapsedTime));
            RenderText(textFPS, WINDOW_WIDTH - 70.0f, 10, 18.0f, glm::vec3(1.0f));
            
            sprintf_s(textBuffer, "Mass: %.0e kg", this->simulation->newParticleMass);
            RenderText(textBuffer, 10.0f, WINDOW_HEIGHT - 70.0f, 18.0f, glm::vec3(1.0f));

            sprintf_s(textBuffer, "Velocity: %.0lf m/s", this->simulation->newParticleVelocity);
            RenderText(textBuffer, 10.0f, WINDOW_HEIGHT - 50.0f, 18.0f, glm::vec3(1.0f));

            sprintf_s(textBuffer, "Brush size: %d", this->simulation->particleBrushSize);
            RenderText(textBuffer, 10.0f, WINDOW_HEIGHT - 30.0f, 18.0f, glm::vec3(1.0f));

            if (isSimulationPaused)
            {
                RenderText("| |", WINDOW_WIDTH - 30.0f, WINDOW_HEIGHT - 30.0f, 24.0f, glm::vec3(1.0f));
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}


/******************************************************************************/
/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/******************************************************************************/


int GetPixelSizeFromPointSize(float pointSize, float dpi)
{
    return static_cast<int>(pointSize * (dpi / 72.0f));
}



/************************END OF FILE************************/
