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
static FT_Face                               face_roboto_bold;
static FT_Face                               face_roboto_light;
static FT_Library                            ft;
static GLuint                                VAO_particles;
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
    LOG_INFO("Starting OpenGL");

    if (!glfwInit())
    {
        LOG_FATAL("Failed to initialize GLFW");
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

    LOG_INFO("Creating GLFW window");

    if (!window)
    {
        LOG_FATAL("Failed to create GLFW window");
        glfwTerminate();
        return nullptr;
    }

    LOG_SUCCESS("GLFW window created");

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    LOG_INFO("Starting GLEW");

    if (glewInit() != GLEW_OK)
    {
        LOG_FATAL("Failed to initialize GLEW");
        return nullptr;
    }

    LOG_SUCCESS("GLEW started");

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_PROGRAM_POINT_SIZE);

    LOG_SUCCESS("OpenGL started");

    return window;
}

void Engine::WindowResizeCallback(GLFWwindow* window, int width, int height)
{
    // Get the associated Engine instance
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));

    if (engine)
    {
        // Store width and height in the Engine instance
        engine->window_width = width;
        engine->window_height = height;

        // Update OpenGL viewport
        glViewport(0, 0, width, height);

        // Calculate the new aspect ratio
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

        // Update projection matrices
        if (aspectRatio > 1.0f)
        {
            // Wider window: extend horizontal range
            projection = glm::ortho(
                -aspectRatio, aspectRatio,
                -1.0f, 1.0f,
                0.1f, 100.0f
            );
        }
        else
        {
            // Taller window: extend vertical range
            projection = glm::ortho(
                -1.0f, 1.0f,
                -1.0f / aspectRatio, 1.0f / aspectRatio,
                0.1f, 100.0f
            );
        }

        projectionText = glm::ortho(
            0.0f, static_cast<float>(width),    // Text rendering in screen space
            static_cast<float>(height), 0.0f,  // Flip y-axis for top-left origin
            -1.0f, 1.0f
        );
    }
}

int Engine::InitFreeType()
{   
    LOG_INFO("Loading fonts");

    if (FT_Init_FreeType(&ft))
    {
        LOG_FATAL("Could not initialize FreeType library");
        return -1;
    }

    // Load a font face
    if (FT_New_Face(ft, "../data/fonts/Roboto/Roboto-Bold.ttf", 0, &face_roboto_bold))
    {
        LOG_FATAL("Failed to load font");
        return -1;
    }

    // Load a font face
    if (FT_New_Face(ft, "../data/fonts/Roboto/Roboto-Light.ttf", 0, &face_roboto_light))
    {
        LOG_FATAL("Failed to load font");
        return -1;
    }

    float pointSize = 64.0f; // Desired font size in points
    int pixelHeight = GetPixelSizeFromPointSize(pointSize, 96.0f);
    FT_Set_Pixel_Sizes(face_roboto_bold, 0, pixelHeight);
    FT_Set_Pixel_Sizes(face_roboto_light, 0, pixelHeight);

    normalizedFaceHeight = face_roboto_light->size->metrics.height / 64.0f; // Convert to float

    // Load the first 128 characters of the ASCII set
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    FT_Face fonts[2] = { face_roboto_bold, face_roboto_light };

    for (size_t i = 0; i < 2; i++)
    {
        for (GLubyte c = 0; c < 128; c++)
        {
            // Load character glyph
            if (FT_Load_Char(fonts[i], c, FT_LOAD_RENDER))
            {
                LOG_FATAL("Failed to load glyph");
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
                fonts[i]->glyph->bitmap.width,
                fonts[i]->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                fonts[i]->glyph->bitmap.buffer
            );

            // Set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            //// Store character for later use
            this->characters[i].insert(std::pair<GLchar, Character>(c, {
                texture,
                glm::ivec2(fonts[i]->glyph->bitmap.width, fonts[i]->glyph->bitmap.rows),
                glm::ivec2(fonts[i]->glyph->bitmap_left, fonts[i]->glyph->bitmap_top),
                static_cast<GLuint>(fonts[i]->glyph->advance.x >> 6)
                }));
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Destroy FreeType resources
    FT_Done_Face(face_roboto_bold);
    FT_Done_Face(face_roboto_light);
    FT_Done_FreeType(ft);

    LOG_SUCCESS("Fonts loaded");

    return 0;
}

// Helper function to read shader source code from a file
std::string Engine::ReadShaderFile(const char* filePath)
{
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open())
    {
        LOG_FATAL("Failed to open shader file: %s", filePath);
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
        LOG_FATAL("Shader compilation failed: %s", infoLog);
    }

    return shader;
}

// Function to load, compile, and link shaders
GLuint Engine::LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
    LOG_INFO("Compiling shaders: [%s, %s]", vertex_file_path, fragment_file_path);

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
        LOG_FATAL("Shader program linking failed: %s", infoLog);
        glDeleteProgram(shaderProgram);
        return 0;
    }

    // Clean up shaders as they are no longer needed after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    LOG_SUCCESS("Shaders compiled");

    return shaderProgram;
}

void Engine::LoadAllShaders()
{
    shaders["circle"] = LoadShaders("../data/shaders/circle.vs", "../data/shaders/circle.fs");
    shaders["particle"] = LoadShaders("../data/shaders/particle.vs", "../data/shaders/particle.fs");
    shaders["text"]= LoadShaders("../data/shaders/text.vs", "../data/shaders/text.fs");

    LOG_SUCCESS("Shaders loaded");
}

GLuint Engine::GetShader(const std::string& key)
{
    return shaders[key];
}

int Engine::GetWindowWidth() const
{
    return this->window_width;
}
int Engine::GetWindowHeight() const
{
    return this->window_height;
}

void Engine::SetWindowSize(int width, int height)
{
    this->window_width = width;
    this->window_height = height;
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

void Engine::RenderCircle(float x, float y, float radius, float outlineThickness, glm::vec4 fillColor, glm::vec4 outlineColor)
{
    GLuint shaderProgram = GetShader("circle");
    glUseProgram(shaderProgram);

    // Convert screen space (x, y) and radius to NDC
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float centerX = (x / width) * 2.0f - 1.0f;  // Convert x to NDC
    float centerY = 1.0f - (y / height) * 2.0f; // Convert y to NDC and flip y-axis
    float ndcRadius = radius / std::min(width, height) * 2.0f;
    float ndcOutlineThickness = outlineThickness / std::min(width, height) * 2.0f;

    // Pass uniforms
    glUniform2f(glGetUniformLocation(shaderProgram, "circleCenter"), centerX, centerY);
    glUniform1f(glGetUniformLocation(shaderProgram, "radius"), ndcRadius);
    glUniform1f(glGetUniformLocation(shaderProgram, "outlineThickness"), ndcOutlineThickness);
    glUniform4f(glGetUniformLocation(shaderProgram, "fillColor"), fillColor.r, fillColor.g, fillColor.b, fillColor.a);
    glUniform4f(glGetUniformLocation(shaderProgram, "outlineColor"), outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);

    // Render a full-screen quad
    float quadVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f,
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Cleanup
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}

void Engine::RenderParticles(GLuint VAO, size_t particleCount)
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, (GLsizei)particleCount);
}

void Engine::RenderText(std::string text, GLfloat x, GLfloat y, GLfloat pointSize, FONT_T font, glm::vec3 color)
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
        Character ch = this->characters[font][c];
        baselineOffset = std::max(baselineOffset, (GLfloat)ch.Bearing.y);
    }

    // Iterate through all characters
    for (const char& c : text)
    {
        Character ch = this->characters[font][c];

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
    LOG_INFO("Starting engine");

    this->window_width = WINDOW_WIDTH;
    this->window_height = WINDOW_HEIGHT;

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
        0.0f, static_cast<GLfloat>(this->GetWindowWidth()),
        static_cast<GLfloat>(this->GetWindowHeight()), 0.0f,
        -1.0f, 1.0f
    );

    window = InitOpenGL();
    if (!window) return -1;

    if (InitFreeType() != 0) return -1;

    glfwSetWindowUserPointer(window, this);
    glfwSetWindowSizeCallback(window, WindowResizeCallback);
    glfwSetCursorPosCallback(window, MousePositionCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, MouseScrollCallback);
    glfwSetKeyCallback(window, KeyboardCallback);

    LoadAllShaders();

    GLuint shaderParticle = GetShader("particle");
    GLuint shaderText = GetShader("text");

    SetupParticleBuffers(VAO_particles, VBO_positions, VBO_colors, this->simulation->getMaxParticleCount());
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

    glUniformMatrix4fv(glGetUniformLocation(shaderParticle, "MVP"), 1, GL_FALSE, glm::value_ptr(projection * view * model));
    glUniformMatrix4fv(glGetUniformLocation(shaderText, "projection"), 1, GL_FALSE, glm::value_ptr(projectionText));

    GLint mvpLoc = glGetUniformLocation(shaderParticle, "MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    this->Run();

    glDeleteBuffers(1, &VBO_positions);
    glDeleteBuffers(1, &VBO_colors);
    glDeleteBuffers(1, &VBO_text);
    glDeleteVertexArrays(1, &VAO_particles);
    glDeleteVertexArrays(1, &VAO_text);
    glDeleteProgram(shaderParticle);
    glDeleteProgram(shaderText);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void Engine::Run()
{
    LOG_SUCCESS("Engine running");

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
        RenderParticles(VAO_particles, particles.size());

        // Render text
        if (isShowingUI)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            RenderText("Particles:", 10.0f, 10.0f, 20.0f, FONT_T::RobotoBold, glm::vec3(1.0f));
            sprintf_s(textBuffer, "%lld", this->simulation->getParticleCount());
            RenderText(textBuffer, 90.0f, 10.0f, 20.0f, FONT_T::RobotoLight, glm::vec3(1.0f));

            RenderText("FPS:", this->GetWindowWidth() - 80.0f, 10, 18.0f, FONT_T::RobotoBold, glm::vec3(1.0f, 1.0, 0.0f));
            sprintf_s(textBuffer, "%ld", (int)(1.0f / fElapsedTime));
            RenderText(textBuffer, this->GetWindowWidth() - 40.0f, 10, 18.0f, FONT_T::RobotoLight, glm::vec3(1.0f, 1.0, 0.0f));
            
            RenderText("Mass:", 10.0f, this->GetWindowHeight() - 70.0f, 18.0f, FONT_T::RobotoBold, glm::vec3(0.61f, 0.85f, 0.9f));
            sprintf_s(textBuffer, "%.0e kg", this->simulation->newParticleMass);
            RenderText(textBuffer, 60.0f, this->GetWindowHeight() - 70.0f, 18.0f, FONT_T::RobotoLight, glm::vec3(0.61f, 0.85f, 0.9f));

            RenderText("Velocity:", 10.0f, this->GetWindowHeight() - 50.0f, 18.0f, FONT_T::RobotoBold, glm::vec3(0.61f, 0.85f, 0.9f));
            sprintf_s(textBuffer, "%.0lf m/s", this->simulation->newParticleVelocity);
            RenderText(textBuffer, 80.0f, this->GetWindowHeight() - 50.0f, 18.0f, FONT_T::RobotoLight, glm::vec3(0.61f, 0.85f, 0.9f));

            RenderText("Brush size:", 10.0f, this->GetWindowHeight() - 30.0f, 18.0f, FONT_T::RobotoBold, glm::vec3(0.61f, 0.85f, 0.9f));
            sprintf_s(textBuffer, "%d", this->simulation->particleBrushSize);
            RenderText(textBuffer, 95.0f, this->GetWindowHeight() - 30.0f, 18.0f, FONT_T::RobotoLight, glm::vec3(0.61f, 0.85f, 0.9f));

            if (isSimulationPaused)
            {
                RenderText("| |", this->GetWindowWidth() - 30.0f, this->GetWindowHeight() - 30.0f, 24.0f, FONT_T::RobotoLight, glm::vec3(1.0f));
            }

            RenderCircle(
                cursor_window_xpos,                         // x position in screen space
                cursor_window_ypos,                         // y position in screen space
                this->simulation->particleBrushSize * 3,    // Circle radius in pixels
                1.0f,                                       // Outline thickness in pixels
                glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),          // Fill color (transparent)
                glm::vec4(0.75f, 0.75f, 0.75f, 1.0f)        // Outline color (white)
            );
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
