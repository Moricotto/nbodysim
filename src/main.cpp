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
    unsigned int vertex_shader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragment_shader = compileShader(fragShaderSource, GL_FRAGMENT_SHADER);
    unsigned int program = linkShaders(vertex_shader, fragment_shader);
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