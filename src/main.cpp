#include <iostream>
#include <cstdio>
#include "sim.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "render.hpp"
#include "input.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "container/octree.hpp"

constexpr size_t STEPS = 365;
constexpr Num dt = 1.0/(365);

constexpr float SCALE_X = 2;
constexpr float SCALE_Y = 2;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 uView;\n"
    "uniform mat4 uProj;\n"
    "out float vViewDistance;\n"
    "void main()\n"
    "{\n"
    "   vec4 viewPos = uView * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "   gl_Position = uProj * viewPos;\n"
    "   vViewDistance = length(viewPos.xyz);\n"
    "   gl_PointSize = 8.0;\n"
    "}\0";

const char *fragShaderSource = "#version 330 core\n"
    "in float vViewDistance;\n"
    "uniform float uFadeNear;\n"
    "uniform float uFadeFar;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
        "vec2 p = gl_PointCoord - vec2(0.5, 0.5);\n"
        "float d = length(p);\n"
        "if (d > 0.5) discard;\n"
        "float edgeAlpha = 1.0 - smoothstep(0.42, 0.5, d);\n"
        "float distAlpha = 1.0 - smoothstep(uFadeNear, uFadeFar, vViewDistance);\n"
        "float alpha = edgeAlpha * clamp(distAlpha, 0.2, 1.0);\n"
        "FragColor = vec4(1.0, 1.0, 1.0, alpha);\n"
    "}\n";


int main() {
    Body earth(3e-6, Vec(1, 0, 0), Vec(0, 6.3, 0));
    Body mars(3.2e-7, Vec(1.52, 0, 0), Vec(0, 5.05, 0));
    Body sun(1, Vec(0, 0, 0), Vec(0, 0, 0));
    std::vector<Body> bodies = {};
		Octree<Body> tree(bodies);
    //SECTION: initialise window
    GLFWwindow* window = createWindow(800, 600, "N-Body Simulation");
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    Camera camera {
        glm::vec3(0.0f, 0.0f, 0.0f),
        -90.0f,
        0.0f,
        6.0f,
        3.0f,
        0.15f,
        0.8f,
        0.5f,
        50.0f
    };
    float previousTime = static_cast<float>(glfwGetTime());
    //SECTION: compile shaders
    unsigned int vertex_shader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragment_shader = compileShader(fragShaderSource, GL_FRAGMENT_SHADER);
    unsigned int program = linkShaders(vertex_shader, fragment_shader);
    int viewLoc = glGetUniformLocation(program, "uView");
    int projLoc = glGetUniformLocation(program, "uProj");
    int fadeNearLoc = glGetUniformLocation(program, "uFadeNear");
    int fadeFarLoc = glGetUniformLocation(program, "uFadeFar");
    //SECTION: Set up VAO
    std::vector<float> vertices(3 * bodies.size(), 0.0f);
    auto [VAO, VBO] = createVAO(vertices.data(), vertices.size());
    while(!glfwWindowShouldClose(window)) {
        const float currentTime = static_cast<float>(glfwGetTime());
        const float deltaTime = currentTime - previousTime;
        previousTime = currentTime;

        update(bodies, dt);
        processInput(window, bodies, SCALE_X, SCALE_Y, camera, deltaTime);

        const glm::vec3 viewDir(
            std::cos(glm::radians(camera.yaw)) * std::cos(glm::radians(camera.pitch)),
            std::sin(glm::radians(camera.pitch)),
            std::sin(glm::radians(camera.yaw)) * std::cos(glm::radians(camera.pitch))
        );
        const glm::vec3 eye = camera.target - glm::normalize(viewDir) * camera.distance;
        const glm::mat4 view = glm::lookAt(eye, camera.target, glm::vec3(0.0f, 1.0f, 0.0f));

        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        const float aspect = (height > 0) ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
        const glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.01f, 100.0f);

        vertices.resize(3 * bodies.size());
        for (size_t j = 0; j < bodies.size(); j++) {
            vertices[3 * j] = static_cast<float>(bodies[j].position.x / SCALE_X);
            vertices[3 * j + 1] = static_cast<float>(bodies[j].position.y / SCALE_Y);
            vertices[3 * j + 2] = static_cast<float>(bodies[j].position.z);
        }
        glUseProgram(program);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(fadeNearLoc, 4.0f);
        glUniform1f(fadeFarLoc, 20.0f);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data(), GL_DYNAMIC_DRAW);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(bodies.size()));
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
