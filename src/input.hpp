#include <GLFW/glfw3.h>
#include <vector>
#include <glm/vec3.hpp>
#include "sim.hpp"

struct Camera {
	glm::vec3 target;
	float yaw;
	float pitch;
	float distance;
	float panSpeed;
	float lookSensitivity;
	float zoomSensitivity;
	float minDistance;
	float maxDistance;
};

void processInput(GLFWwindow *window, std::vector<Body>& bodies, Num scaleX, Num scaleY, Camera& camera, float deltaTime);