/**
  ******************************************************************************
  * @file    Engine.cpp
  * @author  Josh Haden
  * @version V0.1.0
  * @date    19 JAN 2025
  * @brief   Handle render pipeline and user input
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ----------------------------------------------------------------- */

#include "PCH.hpp"

#include "Engine.hpp"
#include "Simulation.hpp"
#include "Particle.hpp"

/* Global variables --------------------------------------------------------- */
/* Private typedef ---------------------------------------------------------- */
/* Private define ----------------------------------------------------------- */
/* Private macro ------------------------------------------------------------ */
/* Private variables -------------------------------------------------------- */

static bool         isClearParticles;
static bool         isCtrlMouseLeftClick;
static bool         isCtrlMouseLeftClickPrev;
static bool         isFrameStepping;
static bool         isKeyLeftAltPressed;
static bool         isKeyLeftCtrlPressed;
static bool         isKeyLeftShiftPressed;
static bool         isShowingUI;
static bool         isSimulationPaused;
static int          cursorState;
static int          cursorStatePrev;
static int          particleMassExp;
static int          timeStepExp;
static char         textBuffer[256];
static float        fElapsedTime;
static float        normalizedFaceHeight;
static double       cursorWindowXPos;
static double       cursorWindowYPos;
static glm::dvec2   particleVelocity;
static DURATION_T   elapsedTime;
static TIME_POINT_T tp1;
static TIME_POINT_T tp2;
static FT_Face      faceRobotoBold;
static FT_Face      faceRobotoLight;
static FT_Library   ft;
static GLuint       VAOParticles;
static GLuint       VAOText;
static GLuint       VBOParticleColors;
static GLuint       VBOParticlePositions;
static GLuint       VBOText;
static glm::mat4    projectionParticles;
static glm::mat4    projectionText;
static SHADERS_T    shaders;

/* Private function prototypes ---------------------------------------------- */



/******************************************************************************/
/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/******************************************************************************/


/**
  * @brief  Default Engine constructor
  * @param  None
  * @retval None
  */
Engine::Engine() {}


/**
  * @brief  Engine destructor
  * @param  None
  * @retval None
  */
Engine::~Engine() {}


/**
  * @brief  Initialize engine variables and call related setup functions
  * @param  None
  * @retval int
  */
int Engine::Init()
{
    LOG_INFO("Starting engine");

    this->window_width = WINDOW_WIDTH;
    this->window_height = WINDOW_HEIGHT;

    isKeyLeftAltPressed = false;
    isKeyLeftCtrlPressed = false;
    isKeyLeftShiftPressed = false;
    isCtrlMouseLeftClick = false;
    isCtrlMouseLeftClickPrev = false;
    isClearParticles = false;
    isSimulationPaused = false;
    isFrameStepping = false;
    isShowingUI = true;
    particleMassExp = 8;
    timeStepExp = 3;
    cursorState = -1;
    cursorStatePrev = -1;
    cursorWindowXPos = -1;
    cursorWindowYPos = -1;
    particleVelocity = glm::dvec2(0.0);
    projectionParticles = glm::ortho(
        -1.0f, 1.0f,
        -1.0f, 1.0f,
        0.1f, 100.0f
    );
    projectionText = glm::ortho(
        0.0f, static_cast<GLfloat>(this->GetWindowWidth()),
        static_cast<GLfloat>(this->GetWindowHeight()), 0.0f,
        -1.0f, 1.0f
    );

    glfwWindow = InitOpenGL();
    if (!glfwWindow) return -1;

    if (InitFreeType() != 0) return -1;

    glfwSetWindowUserPointer(glfwWindow, this);

    glfwSetKeyCallback(glfwWindow, KeyboardCallback);
    glfwSetCursorPosCallback(glfwWindow, MousePositionCallback);
    glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetMouseButtonCallback(glfwWindow, MouseButtonCallback);
    glfwSetScrollCallback(glfwWindow, MouseScrollCallback);
    glfwSetWindowSizeCallback(glfwWindow, WindowResizeCallback);

    LoadAllShaders();

    GLuint shaderParticle = GetShader("particle");
    GLuint shaderText = GetShader("text");

    InitParticleBuffers(VAOParticles, VBOParticlePositions, VBOParticleColors, this->GetSimulation()->GetMaxParticleCount());
    InitTextBuffers(VAOText, VBOText);

    glUseProgram(shaderParticle);

    glm::mat4 model = glm::mat4(1.0f);

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f),   // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f),   // Look at position
        glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
    );

    float zoom = 1.f;

    glm::mat4 mvp = projectionParticles * view * model;

    glUniformMatrix4fv(glGetUniformLocation(shaderParticle, "MVP"), 1, GL_FALSE, glm::value_ptr(projectionParticles * view * model));
    glUniformMatrix4fv(glGetUniformLocation(shaderText, "projection"), 1, GL_FALSE, glm::value_ptr(projectionText));

    GLint mvpLoc = glGetUniformLocation(shaderParticle, "MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    this->Run();

    glDeleteBuffers(1, &VBOParticlePositions);
    glDeleteBuffers(1, &VBOParticleColors);
    glDeleteBuffers(1, &VBOText);
    glDeleteVertexArrays(1, &VAOParticles);
    glDeleteVertexArrays(1, &VAOText);
    glDeleteProgram(shaderParticle);
    glDeleteProgram(shaderText);

    glfwDestroyWindow(glfwWindow);
    glfwTerminate();

    return 0;
}


/**
  * @brief  Initialize FreeType library to load fonts
  * @param  None
  * @retval int
  */
int Engine::InitFreeType()
{
    LOG_INFO("Loading fonts");

    if (FT_Init_FreeType(&ft))
    {
        LOG_FATAL("Could not initialize FreeType library");
        return -1;
    }

    // Load a font face
    if (FT_New_Face(ft, "../data/fonts/Roboto/Roboto-Bold.ttf", 0, &faceRobotoBold))
    {
        LOG_FATAL("Failed to load font");
        return -1;
    }

    // Load a font face
    if (FT_New_Face(ft, "../data/fonts/Roboto/Roboto-Light.ttf", 0, &faceRobotoLight))
    {
        LOG_FATAL("Failed to load font");
        return -1;
    }

    float pointSize = 64.0f; // Desired font size in points
    int pixelHeight = GetPixelSizeFromPointSize(pointSize, 96.0f);
    FT_Set_Pixel_Sizes(faceRobotoBold, 0, pixelHeight);
    FT_Set_Pixel_Sizes(faceRobotoLight, 0, pixelHeight);

    normalizedFaceHeight = faceRobotoLight->size->metrics.height / 64.0f; // Convert to float

    // Load the first 128 characters of the ASCII set
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    FT_Face fonts[2] = { faceRobotoBold, faceRobotoLight };

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
            this->fonts[i].insert(std::pair<GLchar, CHARACTER_T>(c, {
                texture,
                glm::ivec2(fonts[i]->glyph->bitmap.width, fonts[i]->glyph->bitmap.rows),
                glm::ivec2(fonts[i]->glyph->bitmap_left, fonts[i]->glyph->bitmap_top),
                static_cast<GLuint>(fonts[i]->glyph->advance.x >> 6)
                }));
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Destroy FreeType resources
    FT_Done_Face(faceRobotoBold);
    FT_Done_Face(faceRobotoLight);
    FT_Done_FreeType(ft);

    LOG_SUCCESS("Fonts loaded");

    return 0;
}


/**
  * @brief  Initialize VAO and VBO buffers for rendering particles
  * @param  None
  * @retval None
  */
void Engine::InitParticleBuffers(GLuint& VAO, GLuint& VBO_positions, GLuint& VBO_colors, size_t maxParticles)
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


/**
  * @brief  Initialize VAO and VBO buffers for rendering text
  * @param  None
  * @retval None
  */
void Engine::InitTextBuffers(GLuint& VAO, GLuint& VBO_positions)
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


/**
  * @brief  Initialize OpenGL and create window
  * @param  None
  * @retval GLFWwindow*
  */
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

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

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


/**
  * @brief  Load shaders into Engine shader map
  * @param  None
  * @retval None
  */
void Engine::LoadAllShaders()
{
    shaders["circle"] = LinkShaders("../data/shaders/circle.vs", "../data/shaders/circle.fs");
    shaders["particle"] = LinkShaders("../data/shaders/particle.vs", "../data/shaders/particle.fs");
    shaders["text"] = LinkShaders("../data/shaders/text.vs", "../data/shaders/text.fs");

    LOG_SUCCESS("Shaders loaded");
}


/**
  * @brief  Render circle
  * @param  cx                  X screen coordinate for center origin
  * @param  cy                  Y screen coordinate for center origin
  * @param  radius              Radius in pixels
  * @param  outlineThickness    Circle inner thickness in pixels
  * @param  fillColor           RGBA color inside the circle
  * @param  outlineColor        RGBA color for circle outline
  * @retval None
  */
void Engine::RenderCircle(float cx, float cy, float radius, float outlineThickness, glm::vec4 fillColor, glm::vec4 outlineColor)
{
    GLuint shaderProgram = GetShader("circle");
    glUseProgram(shaderProgram);

    // Convert screen space (x, y) and radius to NDC
    float centerX = (cx / this->GetWindowWidth()) * 2.0f - 1.0f;    // Convert x to NDC
    float centerY = 1.0f - (cy / this->GetWindowHeight()) * 2.0f;   // Convert y to NDC and flip y-axis
    float ndcRadius = radius / std::min(this->GetWindowWidth(), this->GetWindowHeight()) * 2.0f;
    float ndcOutlineThickness = outlineThickness / std::min(this->GetWindowWidth(), this->GetWindowHeight()) * 2.0f;

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


/**
  * @brief  Render all particles
  * @param  VAO
  * @param  particleCount
  * @retval None
  */
void Engine::RenderParticles(GLuint VAO, size_t particleCount)
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, (GLsizei)particleCount);
}


/**
  * @brief  Render provided text
  * @param  text
  * @param  x           Screen space x coordinate
  * @param  y           Screen space y coordinate
  * @param  pointSize   Font point size
  * @param  font        Font type
  * @param  color       RGB color
  * @retval None
  */
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

    glBindVertexArray(VAOText);

    float scaleFactor = pointSize / normalizedFaceHeight;

    // Determine the baseline offset (largest Bearing.y in the text)
    GLfloat baselineOffset = 0.0f;
    for (const char& c : text)
    {
        CHARACTER_T ch = this->fonts[font][c];
        baselineOffset = std::max(baselineOffset, (GLfloat)ch.Bearing.y);
    }

    // Iterate through all characters
    for (const char& c : text)
    {
        CHARACTER_T ch = this->fonts[font][c];

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
        glBindBuffer(GL_ARRAY_BUFFER, VBOText);
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


/**
  * @brief  Main loop for the engine
  * @param  None
  * @retval None
  */
void Engine::Run()
{
    LOG_SUCCESS("Engine running");

    GLuint shaderParticle = GetShader("particle");

    std::vector<Particle> particles;
    this->GetSimulation()->SetParticles(&particles);
    this->GetSimulation()->InitTemplateParticles();

    while (!glfwWindowShouldClose(glfwWindow))
    {
        tp2 = std::chrono::system_clock::now();
        elapsedTime = tp2 - tp1;
        tp1 = tp2;
        fElapsedTime = elapsedTime.count();

        if (cursorState == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (isCtrlMouseLeftClick)
            {
                if (!isCtrlMouseLeftClickPrev)
                {
                    this->GetSimulation()->AddParticle(glm::vec2(cursorWindowXPos, cursorWindowYPos));
                }
            }
            else
            {
                this->GetSimulation()->AddParticles(glm::vec2(cursorWindowXPos, cursorWindowYPos));
            }
        }
        else if (cursorState == GLFW_MOUSE_BUTTON_RIGHT)
        {
            this->GetSimulation()->RemoveParticle(glm::vec2(cursorWindowXPos, cursorWindowYPos));
        }

        isCtrlMouseLeftClickPrev = isCtrlMouseLeftClick;

        if (isClearParticles)
        {
            isClearParticles = false;

            this->GetSimulation()->RemoveAllParticles();
        }

        if (!isSimulationPaused)
        {
            this->GetSimulation()->Update();
        }

        UpdateParticleBuffers(particles);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        // Render particles
        glDisable(GL_BLEND);
        glUseProgram(shaderParticle);
        RenderParticles(VAOParticles, particles.size());

        // Render text
        if (isShowingUI)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            RenderText("Particles:", 10.0f, 10.0f, 20.0f, FONT_T::RobotoBold, glm::vec3(1.0f));
            sprintf_s(textBuffer, "%lld", this->GetSimulation()->GetParticleCount());
            RenderText(textBuffer, 90.0f, 10.0f, 20.0f, FONT_T::RobotoLight, glm::vec3(1.0f));

            RenderText("Mass:", 10.0f, 30.0f, 20.0f, FONT_T::RobotoBold, glm::vec3(1.0f));
            sprintf_s(textBuffer, "%.2e kg", this->GetSimulation()->GetTotalMass());
            RenderText(textBuffer, 90.0f, 30.0f, 20.0f, FONT_T::RobotoLight, glm::vec3(1.0f));

            RenderText("Timestep:", this->GetWindowWidth() - 130.0f, 10, 18.0f, FONT_T::RobotoBold, glm::vec3(1.0f, 1.0, 0.0f));
            sprintf_s(textBuffer, "%.0e s", this->GetSimulation()->GetTimeStep());
            RenderText(textBuffer, this->GetWindowWidth() - 55.0f, 10, 18.0f, FONT_T::RobotoLight, glm::vec3(1.0f, 1.0, 0.0f));

            RenderText("FPS:", this->GetWindowWidth() - 93.0f, 30, 18.0f, FONT_T::RobotoBold, glm::vec3(1.0f, 1.0, 0.0f));
            sprintf_s(textBuffer, "%ld", (int)(1.0f / fElapsedTime));
            RenderText(textBuffer, this->GetWindowWidth() - 55.0f, 30, 18.0f, FONT_T::RobotoLight, glm::vec3(1.0f, 1.0, 0.0f));

            RenderText("Mass:", 10.0f, this->GetWindowHeight() - 70.0f, 18.0f, FONT_T::RobotoBold, glm::vec3(0.61f, 0.85f, 0.9f));
            sprintf_s(textBuffer, "%.0e kg", this->GetSimulation()->GetNewParticleMass());
            RenderText(textBuffer, 95.0f, this->GetWindowHeight() - 70.0f, 18.0f, FONT_T::RobotoLight, glm::vec3(0.61f, 0.85f, 0.9f));

            RenderText("Velocity:", 10.0f, this->GetWindowHeight() - 50.0f, 18.0f, FONT_T::RobotoBold, glm::vec3(0.61f, 0.85f, 0.9f));
            sprintf_s(textBuffer, "(%.0lf, %.0lf) m/s", this->GetSimulation()->GetNewParticleVelocity().x, this->GetSimulation()->GetNewParticleVelocity().y);
            RenderText(textBuffer, 95.0f, this->GetWindowHeight() - 50.0f, 18.0f, FONT_T::RobotoLight, glm::vec3(0.61f, 0.85f, 0.9f));

            RenderText("Brush size:", 10.0f, this->GetWindowHeight() - 30.0f, 18.0f, FONT_T::RobotoBold, glm::vec3(0.61f, 0.85f, 0.9f));
            sprintf_s(textBuffer, "%d", this->GetSimulation()->GetParticleBrushSize());
            RenderText(textBuffer, 95.0f, this->GetWindowHeight() - 30.0f, 18.0f, FONT_T::RobotoLight, glm::vec3(0.61f, 0.85f, 0.9f));

            if (isSimulationPaused)
            {
                RenderText("| |", this->GetWindowWidth() - 30.0f, this->GetWindowHeight() - 30.0f, 24.0f, FONT_T::RobotoLight, glm::vec3(1.0f));
            }

            // Render brush
            RenderCircle(
                static_cast<float>(cursorWindowXPos),                                   // x position in screen space
                static_cast<float>(cursorWindowYPos),                                   // y position in screen space
                static_cast<float>(this->GetSimulation()->GetParticleBrushSize() * 3),  // Circle radius in pixels
                1.0f,                                                                   // Outline thickness in pixels
                glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),                                      // Fill color (transparent)
                glm::vec4(0.75f, 0.75f, 0.75f, 1.0f)                                    // Outline color (white)
            );
        }        

        glfwSwapBuffers(glfwWindow);
        glfwPollEvents();
    }
}


/**
  * @brief  Update buffers for particles 
  * @param  particles
  * @retval None
  */
void Engine::UpdateParticleBuffers(const std::vector<Particle>& particles)
{
    std::vector<GLfloat> positions;
    std::vector<GLfloat> colors;

    for (const auto& p : particles)
    {
        positions.push_back(static_cast<GLfloat>(p.GetPosition().x));
        positions.push_back(static_cast<GLfloat>(p.GetPosition().y));
        colors.push_back(p.GetColor().r);
        colors.push_back(p.GetColor().g);
        colors.push_back(p.GetColor().b);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBOParticlePositions);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(GLfloat), positions.data());

    glBindBuffer(GL_ARRAY_BUFFER, VBOParticleColors);
    glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(GLfloat), colors.data());
}


/**
  * @brief  Compile shader from source
  * @param  shaderType      Fragment or vertex shader
  * @param  shaderSource    Plain text source code
  * @retval GLuint
  */
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


/**
  * @brief  Link vertex and fragment shaders
  * @param  vertexFilePath
  * @param  fragmentFilePath
  * @retval GLuint
  */
GLuint Engine::LinkShaders(const char* vertexFilePath, const char* fragmentFilePath)
{
    LOG_INFO("Compiling shaders: [%s, %s]", vertexFilePath, fragmentFilePath);

    // Read the vertex shader code from the file
    std::string vertexShaderCode = this->ReadShaderFile(vertexFilePath);
    if (vertexShaderCode.empty()) return 0;

    // Read the fragment shader code from the file
    std::string fragmentShaderCode = ReadShaderFile(fragmentFilePath);
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


/**
  * @brief  Read and return shader source code
  * @param  filePath            Relative file path to shader source file
  * @retval SHADER_SOURCE_T     Plain text shader source code
  */
SHADER_SOURCE_T Engine::ReadShaderFile(const char* filePath) const
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


/**
  * @brief  Get window width in pixels
  * @param  None
  * @retval size_t
  */
size_t Engine::GetWindowWidth() const
{
    return this->window_width;
}


/**
  * @brief  Get window height in pixels
  * @param  None
  * @retval size_t
  */
size_t Engine::GetWindowHeight() const
{
    return this->window_height;
}


/**
  * @brief  Get shader program
  * @param  key     Shader name
  * @retval size_t
  */
GLuint Engine::GetShader(const std::string& key) const
{
    return shaders[key];
}


/**
  * @brief  Get simulation attached to the engine
  * @param  None
  * @retval Simulation*
  */
Simulation* Engine::GetSimulation() const
{
    return this->simulation;
}


/**
  * @brief  Get GLFW window
  * @param  None
  * @retval GLFWwindow*
  */
GLFWwindow* Engine::GetGLFWWindow() const
{
    return this->glfwWindow;
}


/**
  * @brief  Set simulation attached to the engine
  * @param  simulation
  * @retval None
  */
void Engine::SetSimulation(Simulation* simulation)
{
    this->simulation = simulation;
}


/**
  * @brief  Set window size
  * @param  width
  * @param  height
  * @retval None
  */
void Engine::SetWindowSize(int width, int height)
{
    this->window_width = width;
    this->window_height = height;
}



/******************************************************************************/
/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/******************************************************************************/


/**
  * @brief  GLFW callback to handle keyboard events
  * @param  window
  * @param  key
  * @param  scancode
  * @param  action
  * @param  mods
  * @retval None
  */
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

                case GLFW_KEY_COMMA: timeStepExp += 1; break;
                case GLFW_KEY_PERIOD: timeStepExp -= 1; break;

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

                case GLFW_KEY_F: isFrameStepping = true; isSimulationPaused = true; e->GetSimulation()->UpdateParticles(); break;
                case GLFW_KEY_R: isClearParticles = true; break;

                case GLFW_KEY_W: particleVelocity.y += ((isKeyLeftCtrlPressed) ? 10 : 1); break;
                case GLFW_KEY_A: particleVelocity.x -= ((isKeyLeftCtrlPressed) ? 10 : 1); break;
                case GLFW_KEY_S: particleVelocity.y -= ((isKeyLeftCtrlPressed) ? 10 : 1); break;
                case GLFW_KEY_D: particleVelocity.x += ((isKeyLeftCtrlPressed) ? 10 : 1); break;

                case GLFW_KEY_LEFT_BRACKET: e->GetSimulation()->SetParticleBrushSize(e->GetSimulation()->GetParticleBrushSize() - ((isKeyLeftCtrlPressed) ? 10 : 1)); break;
                case GLFW_KEY_RIGHT_BRACKET: e->GetSimulation()->SetParticleBrushSize(e->GetSimulation()->GetParticleBrushSize() + ((isKeyLeftCtrlPressed) ? 10 : 1)); break;

                case GLFW_KEY_F1: isShowingUI = !isShowingUI; break;

                case GLFW_KEY_ESCAPE: Exit(0);  break;
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

            e->GetSimulation()->SetNewParticleMass(powl(10, particleMassExp));
            e->GetSimulation()->SetNewParticleVelocity(particleVelocity);
            e->GetSimulation()->SetTimeStep(powl(10, -timeStepExp));
            break;
        }
    }
}


/**
  * @brief  GLFW callback to handle mouse button events
  * @param  window
  * @param  button
  * @param  action
  * @param  mods
  * @retval None
  */
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
                cursorState = -1;
                break;
        }
        break;
    case GLFW_PRESS:
        switch (button)
        {
            case GLFW_MOUSE_BUTTON_LEFT:
                cursorState = GLFW_MOUSE_BUTTON_LEFT;
                isCtrlMouseLeftClick = isKeyLeftCtrlPressed ? true : false;
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                cursorState = GLFW_MOUSE_BUTTON_RIGHT;
                break;
        }
        break;
    default:
        cursorState = -1;
        break;
    }

    cursorStatePrev = cursorState;
}


/**
  * @brief  GLFW callback to handle mouse movement events
  * @param  window
  * @param  xPos
  * @param  yPos
  * @retval None
  */
void Engine::MousePositionCallback(GLFWwindow* window, double xPos, double yPos)
{
    cursorWindowXPos = xPos;
    cursorWindowYPos = yPos;
}


/**
  * @brief  GLFW callback to handle mouse scroll events
  * @param  window
  * @param  xOffset
  * @param  yOffset
  * @retval None
  */
void Engine::MouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));

    if (e)
    {
        int particleBrushSize = e->GetSimulation()->GetParticleBrushSize();

        if (yOffset == 1)
        {
            e->GetSimulation()->SetParticleBrushSize(++particleBrushSize);
        }
        else if (yOffset == -1)
        {
            e->GetSimulation()->SetParticleBrushSize(--particleBrushSize);
        }
    }
}


/**
  * @brief  GLFW callback to handle window resize events
  * @param  window
  * @param  width
  * @param  height
  * @retval None
  */
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
            projectionParticles = glm::ortho(
                -aspectRatio, aspectRatio,
                -1.0f, 1.0f,
                0.1f, 100.0f
            );
        }
        else
        {
            // Taller window: extend vertical range
            projectionParticles = glm::ortho(
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

/**
  * @brief  Convert point size to pixels
  * @param  pointSize
  * @param  dpi
  * @retval int
  */
int Engine::GetPixelSizeFromPointSize(float pointSize, float dpi) const
{
    return static_cast<int>(pointSize * (dpi / 72.0f));
}



/******************************** END OF FILE *********************************/
