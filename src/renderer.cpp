#include "renderer.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <ft2build.h>
#include FT_FREETYPE_H

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 color;
    void main() {
        FragColor = vec4(color, 1.0);
    }
)";

Renderer::Renderer(int w, int h) : width(w), height(h) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Cache Visualization", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return;
    }

    initShaders();
    initGeometry();
}

void Renderer::initShaders() {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Renderer::initGeometry() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void Renderer::render(const Cache& cache) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    interpolateColors(cache.getState());
    drawCache(cache.getState(), cache.getHitRate());

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Renderer::interpolateColors(const std::vector<int>& cacheState) {
    int cacheSize = cacheState.size();
    if (previousColors.size() != cacheSize * 3) {
        previousColors.resize(cacheSize * 3, 0.5f);
        targetColors.resize(cacheSize * 3, 0.5f);
    }

    for (int i = 0; i < cacheSize; ++i) {
        float hue = static_cast<float>(cacheState[i] % 360) / 360.0f;
        float r, g, b;
        if (cacheState[i] == -1) {
            r = g = b = 0.5f;  // Gray for empty slots
        } else {
            setRGBFromHue(hue, r, g, b);
        }

        int idx = i * 3;
        targetColors[idx] = r;
        targetColors[idx + 1] = g;
        targetColors[idx + 2] = b;

        previousColors[idx] += (targetColors[idx] - previousColors[idx]) * 0.1f;
        previousColors[idx + 1] += (targetColors[idx + 1] - previousColors[idx + 1]) * 0.1f;
        previousColors[idx + 2] += (targetColors[idx + 2] - previousColors[idx + 2]) * 0.1f;
    }
}

void Renderer::drawCache(const std::vector<int>& cacheState, float hitRate) {
    int cacheSize = cacheState.size();
    float rectWidth = 2.0f / cacheSize;
    float rectHeight = 0.5f;

    for (int i = 0; i < cacheSize; ++i) {
        float x = i * rectWidth - 1.0f;
        float color[3] = { previousColors[i * 3], previousColors[i * 3 + 1], previousColors[i * 3 + 2] };

        drawRectangle(x, -rectHeight, rectWidth, rectHeight, color);

        // Draw memory address as text
        std::stringstream ss;
        ss << cacheState[i];
        float textColor[] = { 1.0f, 1.0f, 1.0f };  // White color
        drawText(ss.str(), x + rectWidth / 2.0f, -rectHeight - 0.1f, 0.0005f, textColor);
    }

    // Draw cache statistics
    std::stringstream stats;
    stats << "Hit Rate: " << hitRate * 100.0f << "%";
    float statsColor[] = { 1.0f, 1.0f, 1.0f };  // White color
    drawText(stats.str(), -0.95f, 0.8f, 0.001f, statsColor);
}


void Renderer::drawRectangle(float x, float y, float width, float height, const float color[3]) {
    float vertices[] = {
        x, y,
        x + width, y,
        x + width, y + height,
        x, y + height
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glUseProgram(shaderProgram);
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), color[0], color[1], color[2]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void Renderer::drawText(const std::string& text, float x, float y, float scale, const float color[3]) {
    // Initialize FreeType library
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "Failed to initialize FreeType library" << std::endl;
        return;
    }

    // Load font face
    FT_Face face;
    if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face)) {
        std::cerr << "Failed to load font" << std::endl;
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    // Activate corresponding render state
    glUseProgram(shaderProgram);
    glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, color);

    // Disable depth test and enable blending
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render glyphs
    for (const char& c : text) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "Failed to load glyph" << std::endl;
            continue;
        }

        // Render character
        float xpos = x + face->glyph->bitmap_left * scale;
        float ypos = y - (face->glyph->bitmap.rows - face->glyph->bitmap_top) * scale;

        float w = face->glyph->bitmap.width * scale;
        float h = face->glyph->bitmap.rows * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Advance cursor to the start of the next character
        x += (face->glyph->advance.x >> 6) * scale;
        y += (face->glyph->advance.y >> 6) * scale;
    }

    // Cleanup FreeType
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Reset depth test and blending
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Renderer::setRGBFromHue(float hue, float& r, float& g, float& b) {
    int h = static_cast<int>(hue * 360.0f) / 60;
    float f = (hue * 360.0f / 60.0f) - h;
    float q = 1.0f - f;

    switch (h) {
    case 0: r = 1.0f; g = f; b = 0.0f; break;
    case 1: r = q; g = 1.0f; b = 0.0f; break;
    case 2: r = 0.0f; g = 1.0f; b = f; break;
    case 3: r = 0.0f; g = q; b = 1.0f; break;
    case 4: r = f; g = 0.0f; b = 1.0f; break;
    case 5: r = 1.0f; g = 0.0f; b = q; break;
    default: r = 1.0f; g = 1.0f; b = 1.0f; break;
    }
}

Renderer::~Renderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Renderer::shouldClose() const {
    return glfwWindowShouldClose(window);
}
