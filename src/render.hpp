
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <string>
#include <iostream>
#include <array>
#include <cstddef>

class Shader {
public:
    Shader(const char* source, int type) : source(source) {
        id = glCreateShader(type);
        glShaderSource(id, 1, &this->source, NULL);
        glCompileShader(id);
        int  success;
        char infoLog[512];
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if(!success) {
            glGetShaderInfoLog(id, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    const char* source;
    unsigned int id;
};

unsigned int linkShaders(unsigned int vertexShader, unsigned int fragmentShader);
GLFWwindow* createWindow(unsigned int width, unsigned int height, const char* title);
std::array<unsigned int, 2> createVAO(float* vertices, std::size_t floatCount);