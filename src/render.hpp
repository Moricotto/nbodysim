
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <string>
#include <iostream>
#include <array>
#include <cstddef>

unsigned int compileShader(const char* source, int type);
unsigned int linkShaders(unsigned int vertexShader, unsigned int fragmentShader);
GLFWwindow* createWindow(unsigned int width, unsigned int height, const char* title);
std::array<unsigned int, 2> createVAO(float* vertices, std::size_t floatCount);