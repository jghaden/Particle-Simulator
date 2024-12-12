#include "PCH.hpp"

#include "Engine.hpp"
#include "Simulation.hpp"
#include "Particle.hpp"

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

//int Engine::InitFreeType()
//{
//    FT_Library ft;
//    if (FT_Init_FreeType(&ft))
//    {
//        std::cerr << "Could not init FreeType Library" << std::endl;
//        return -1;
//    }
//
//    // Load a font face
//    FT_Face face;
//    if (FT_New_Face(ft, "../data/fonts/Roboto/Roboto-Regular.ttf", 0, &face))
//    {
//        std::cerr << "Failed to load font" << std::endl;
//        return -1;
//    }
//    FT_Set_Pixel_Sizes(face, 0, 48);
//
//    // Load the first 128 characters of the ASCII set
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
//
//    for (GLubyte c = 0; c < 128; c++)
//    {
//        // Load character glyph
//        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
//        {
//            std::cerr << "Failed to load Glyph" << std::endl;
//            continue;
//        }
//
//        // Generate texture
//        GLuint texture;
//        glGenTextures(1, &texture);
//        glBindTexture(GL_TEXTURE_2D, texture);
//        glTexImage2D(
//            GL_TEXTURE_2D,
//            0,
//            GL_RED,
//            face->glyph->bitmap.width,
//            face->glyph->bitmap.rows,
//            0,
//            GL_RED,
//            GL_UNSIGNED_BYTE,
//            face->glyph->bitmap.buffer
//        );
//
//        // Set texture options
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//        //// Store character for later use
//        //this->characters.insert(std::pair<GLchar, Character>(c, {
//        //    texture,
//        //    glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
//        //    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
//        //    face->glyph->advance.x
//        //}));
//
//        // Now store character for later use
//        Character character = {
//            texture,
//            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
//            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
//            static_cast<GLuint>(face->glyph->advance.x)
//        };
//        this->characters.insert(std::pair<char, Character>(c, character));
//    }
//    glBindTexture(GL_TEXTURE_2D, 0);
//
//    // Destroy FreeType resources
//    FT_Done_Face(face);
//    FT_Done_FreeType(ft);
//}

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

void Engine::SetupBuffers(GLuint& VAO, GLuint& VBO_positions, GLuint& VBO_colors, const std::vector<Particle>* particles)
{
    std::vector<GLfloat> positions;
    std::vector<GLfloat> colors;

    for (const auto& p : *particles)
    {
        positions.push_back(p.position.x);
        positions.push_back(p.position.y);
        colors.push_back(p.color.r);
        colors.push_back(p.color.g);
        colors.push_back(p.color.b);
    }

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO_positions);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_DYNAMIC_DRAW);

    glGenBuffers(1, &VBO_colors);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_DYNAMIC_DRAW);

    /*glGenBuffers(1, &VBO_ages);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ages);
    glBufferData(GL_ARRAY_BUFFER, ages.size() * sizeof(GLfloat), ages.data(), GL_DYNAMIC_DRAW);*/

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    /*glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ages);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, nullptr);*/

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Engine::RenderParticles(GLuint VAO, size_t particleCount)
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, particleCount);
}

//void Engine::RenderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
//{
//    // Activate corresponding render state	
//    shader.use();
//    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
//    glActiveTexture(GL_TEXTURE0);
//    glBindVertexArray(VAO);
//
//    // Iterate through all characters
//    std::string::const_iterator c;
//    for (c = text.begin(); c != text.end(); c++)
//    {
//        Character ch = this->characters[*c];
//
//        GLfloat xpos = x + ch.Bearing.x * scale;
//        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
//
//        GLfloat w = ch.Size.x * scale;
//        GLfloat h = ch.Size.y * scale;
//        // Update VBO for each character
//        GLfloat vertices[6][4] = {
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos,     ypos,       0.0, 1.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//            { xpos + w, ypos + h,   1.0, 0.0 }
//        };
//        // Render glyph texture over quad
//        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
//        // Update content of VBO memory
//        glBindBuffer(GL_ARRAY_BUFFER, VBO);
//        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        // Render quad
//        glDrawArrays(GL_TRIANGLES, 0, 6);
//        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
//        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
//    }
//    glBindVertexArray(0);
//    glBindTexture(GL_TEXTURE_2D, 0);
//}

int cursor_state = -1;
int cursor_window_xpos = -1;
int cursor_window_ypos = -1;

void Engine::MousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    cursor_window_xpos = xpos;
    cursor_window_ypos = ypos;

    //std::cout << '(' << xpos << ", " << ypos << ')' << std::endl;
}

void Engine::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (action)
    {
        case GLFW_PRESS:
            switch (button)
            {
                case GLFW_MOUSE_BUTTON_LEFT:
                    cursor_state = GLFW_MOUSE_BUTTON_LEFT;
                    break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    cursor_state = GLFW_MOUSE_BUTTON_RIGHT;
                    break;
            }
            break;
        default:
            cursor_state = -1;
    }
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

        std::cout << "Brush size: " << e->simulation->particleBrushSize << std::endl;
    }
}

void Engine::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));

    if (e)
    {
        float massExp = 0;

        switch (action)
        {
            case GLFW_PRESS:
                switch (key)
                {
                    case GLFW_KEY_W: e->simulation->newParticleVelocity += 1.0; break;
                    case GLFW_KEY_S: e->simulation->newParticleVelocity -= 1.0; break;

                    case GLFW_KEY_0: massExp = 0; break;
                    case GLFW_KEY_1: massExp = 1; break;
                    case GLFW_KEY_2: massExp = 2; break;
                    case GLFW_KEY_3: massExp = 3; break;
                    case GLFW_KEY_4: massExp = 4; break;
                    case GLFW_KEY_5: massExp = 5; break;
                    case GLFW_KEY_6: massExp = 6; break;
                    case GLFW_KEY_7: massExp = 7; break;
                    case GLFW_KEY_8: massExp = 8; break;
                    case GLFW_KEY_9: massExp = 9; break;
                }

                e->simulation->newParticleMass = powf(10, massExp);

                std::cout << "Mass: " << e->simulation->newParticleMass << ", Velocity: " << e->simulation->newParticleVelocity << std::endl;
        }
    }
}

void Engine::SetSimulation(Simulation* simulation)
{
    this->simulation = simulation;
}

int Engine::Init()
{
    GLFWwindow* window = InitOpenGL();
    if (!window) return -1;

    //if (InitFreeType() != 0) return -1;

    glfwSetWindowUserPointer(window, this);
    glfwSetWindowSizeCallback(window, WindowResizeCallback);
    glfwSetCursorPosCallback(window, MousePositionCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, MouseScrollCallback);
    glfwSetKeyCallback(window, KeyboardCallback);

    GLuint shaderProgram = LoadShaders("../data/shaders/particle.vs", "../data/shaders/particle.fs");

    std::vector<Particle> particles;
    this->simulation->SetParticles(&particles);
    this->simulation->initializeParticles();

    GLuint VAO, VBO_positions, VBO_colors;
    SetupBuffers(VAO, VBO_positions, VBO_colors, &particles);

    glUseProgram(shaderProgram);

    glm::mat4 model = glm::mat4(1.0f);

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f),   // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f),   // Look at position
        glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
    );

    float zoom = 1.f;

    glm::mat4 projection = glm::ortho(
        -1.0f / zoom, 1.0f / zoom, // Left and right
        -1.0f / zoom, 1.0f / zoom, // Bottom and top
        0.1f, 100.0f   // Near and far planes
    );

    glm::mat4 mvp = projection * view * model;

    GLint mvpLoc = glGetUniformLocation(shaderProgram, "MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    GLint zoomLoc = glGetUniformLocation(shaderProgram, "zoom");
    glUniform1f(zoomLoc, zoom);

    while (!glfwWindowShouldClose(window))
    {
        if (cursor_state == GLFW_MOUSE_BUTTON_LEFT)
        {
            this->simulation->addParticles(particles, glm::vec2(cursor_window_xpos, cursor_window_ypos));
            SetupBuffers(VAO, VBO_positions, VBO_colors, &particles);
        }
        else if (cursor_state == GLFW_MOUSE_BUTTON_RIGHT)
        {
            this->simulation->removeParticle(particles, glm::vec2(cursor_window_xpos, cursor_window_ypos));
            SetupBuffers(VAO, VBO_positions, VBO_colors, &particles);
        }

        /*this->simulation->update();
        std::cout << "Simulation time: " << this->simulation->simulationTime << std::endl*/;
        this->simulation->updateParticles(particles);

        // Update VBOs
        std::vector<GLfloat> positions;
        std::vector<GLfloat> colors;
        for (const auto& p : particles)
        {
            positions.push_back(p.position.x);
            positions.push_back(p.position.y);
            colors.push_back(p.color.r);
            colors.push_back(p.color.g);
            colors.push_back(p.color.b);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
        glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(GLfloat), positions.data());

        glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
        glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(GLfloat), colors.data());

        glClear(GL_COLOR_BUFFER_BIT);

        RenderParticles(VAO, particles.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO_positions);
    glDeleteBuffers(1, &VBO_colors);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}