#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "cache.h"
#include <vector>
#include <string>

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();
    void render(const Cache& cache);
    bool shouldClose() const;

private:
    GLFWwindow* window;
    GLuint shaderProgram;
    GLuint VAO, VBO;
    int width, height;
    std::vector<float> previousColors;
    std::vector<float> targetColors;

    void initShaders();
    void initGeometry();
    void drawCache(const std::vector<int>& cacheState, float hitRate);
    void drawText(const std::string& text, float x, float y, float scale, const float color[3]);
    void interpolateColors(const std::vector<int>& cacheState);
    void drawRectangle(float x, float y, float width, float height, const float color[3]);
    void setRGBFromHue(float hue, float& r, float& g, float& b);
};
