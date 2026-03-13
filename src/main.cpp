#include <iostream>
#include <cstdio>
#include "sim.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

constexpr size_t STEPS = 20;
constexpr Num dt = 1.0/(STEPS*10);
constexpr Num TAU = 2 * 3.14159265358979323846;
constexpr Num TAU_SQ = TAU * TAU;

constexpr float SCALE_X = 2;
constexpr float SCALE_Y = 2;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
} 

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
}

GLFWwindow* initWindow(int, int);
GLuint compileShader(GLenum shaderType, const char* shaderSrc);
GLuint linkShaderToProgram(GLuint vertexShader, GLuint fragmentShader);

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
    Body earth(3e-6 * TAU_SQ, Vec(1, 0, 0), Vec(0, 6.3, 0));
    Body mars(3.2e-7 * TAU_SQ, Vec(1.52, 0, 0), Vec(0, 5.05, 0));
    Body sun(1.0 * TAU_SQ, Vec(0, 0, 0), Vec(0, 0, 0));
    std::vector<Body> bodies = {earth, sun, mars};
		GLFWwindow* window = initWindow(800, 600);
    //SECTION: compile shaders
    GLuint vertexShader, fragmentShader, shaderProgram;
		int success;

		vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragShaderSource);
		if (!vertexShader || !fragmentShader){ 
			return -1;
		}
		shaderProgram = linkShaderToProgram(vertexShader, fragmentShader);
		if (!shaderProgram){
			return -1;
		}
    //SECTION: Set up VAO
    float vertices[3 * bodies.size()];
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glPointSize(5);
    while(!glfwWindowShouldClose(window)) {
        update(bodies, dt);
        for (size_t j = 0; j < bodies.size(); j++) {
            vertices[3 * j] = static_cast<float>(bodies[j].position.x / SCALE_X);
            vertices[3 * j + 1] = static_cast<float>(bodies[j].position.y / SCALE_Y);
            vertices[3 * j + 2] = static_cast<float>(bodies[j].position.z);
        }
        processInput(window);
        glUseProgram(shaderProgram);
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
