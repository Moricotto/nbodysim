#include <iostream>
#include <cstdio>
#include "sim.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "render.hpp"
#include "input.hpp"

constexpr size_t STEPS = 365;
constexpr Num dt = 1.0/(365*10);

constexpr float SCALE_X = 2;
constexpr float SCALE_Y = 2;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
        "FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}\n";


int main() {
    Body earth(3e-6, Vec(1, 0, 0), Vec(0, 6.3, 0));
    Body mars(3.2e-7, Vec(1.52, 0, 0), Vec(0, 5.05, 0));
    Body sun(1, Vec(0, 0, 0), Vec(0, 0, 0));
    std::vector<Body> bodies = {earth, sun, mars};
    //SECTION: initialise window
    GLFWwindow* window = createWindow(800, 600, "N-Body Simulation");
    //SECTION: compile shaders
    Shader vertexShader(vertexShaderSource, GL_VERTEX_SHADER);
    Shader fragmentShader(fragShaderSource, GL_FRAGMENT_SHADER);
    unsigned int program = linkShaders(vertexShader.id, fragmentShader.id);
    //SECTION: Set up VAO
    float vertices[3 * bodies.size()];
    auto [VAO, VBO] = createVAO(vertices, 3 * bodies.size());
    while(!glfwWindowShouldClose(window)) {
        update(bodies, dt);
        for (size_t j = 0; j < bodies.size(); j++) {
            vertices[3 * j] = static_cast<float>(bodies[j].position.x / SCALE_X);
            vertices[3 * j + 1] = static_cast<float>(bodies[j].position.y / SCALE_Y);
            vertices[3 * j + 2] = static_cast<float>(bodies[j].position.z);
        }
        processInput(window);
        glUseProgram(program);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(bodies.size()));
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;

}
/// 
GLuint compileShader(GLenum shaderType, const char* shaderSrc){

    GLuint shader;
    shader = glCreateShader(shaderType);
		if (shader == 0){
			return 0;
		}
    glShaderSource(shader, 1, &shaderSrc, NULL);
    glCompileShader(shader);
    int  success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
				return 0;
    }
		return shader;
}
GLuint linkShaderToProgram(GLuint vertexShader, GLuint fragmentShader){
    GLuint shaderProgram;
		GLint success;
		char infoLog[512];
		
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
				return 0;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
		return shaderProgram;

}

GLFWwindow* initWindow(int width, int height) {
    //SECTION: initialise window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(width, height, "NBodySim", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
    }
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		return window;
}
