#include "input.hpp"
#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

namespace {
struct DragState {
    bool isDragging = false;
    double pressX = 0.0;
    double pressY = 0.0;
};

float pendingScrollY = 0.0f;

void scroll_callback(GLFWwindow*, double, double yoffset) {
    pendingScrollY += static_cast<float>(yoffset);
}

Vec cursorToWorld(GLFWwindow* window, double x, double y, Num scaleX, Num scaleY) {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    if (width <= 0 || height <= 0) {
        return Vec(0.0, 0.0, 0.0);
    }

    const Num ndcX = (2.0 * x / static_cast<Num>(width)) - 1.0;
    const Num ndcY = 1.0 - (2.0 * y / static_cast<Num>(height));
    return Vec(ndcX * scaleX, ndcY * scaleY, 0.0);
}

glm::vec3 cameraForward(const Camera& camera) {
    const float yawRad = glm::radians(camera.yaw);
    const float pitchRad = glm::radians(camera.pitch);
    const glm::vec3 f(
        std::cos(yawRad) * std::cos(pitchRad),
        std::sin(pitchRad),
        std::sin(yawRad) * std::cos(pitchRad)
    );
    return glm::normalize(f);
}


bool rayPlaneIntersection(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDir,
    const glm::vec3& planePoint,
    const glm::vec3& planeNormal,
    glm::vec3& outPoint
) {
    const float denom = glm::dot(rayDir, planeNormal);
    if (std::abs(denom) < 1e-6f) {
        return false;
    }
    const float t = glm::dot(planePoint - rayOrigin, planeNormal) / denom;
    if (t < 0.0f) {
        return false;
    }
    outPoint = rayOrigin + rayDir * t;
    return true;
}

bool cursorToWorldOnSimPlane(GLFWwindow* window, double x, double y, const Camera& camera, Num scaleX, Num scaleY, Vec& worldPoint) {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    if (width <= 0 || height <= 0) {
        return false;
    }

    const glm::vec3 forward = cameraForward(camera);
    const glm::vec3 eye = camera.target - forward * camera.distance;

    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    const glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.01f, 100.0f);
    const glm::mat4 view = glm::lookAt(eye, camera.target, glm::vec3(0.0f, 1.0f, 0.0f));

    const float ndcX = (2.0f * static_cast<float>(x) / static_cast<float>(width)) - 1.0f;
    const float ndcY = 1.0f - (2.0f * static_cast<float>(y) / static_cast<float>(height));

    const glm::mat4 invVP = glm::inverse(projection * view);
    glm::vec4 nearPoint = invVP * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 farPoint = invVP * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
    nearPoint /= nearPoint.w;
    farPoint /= farPoint.w;

    const glm::vec3 rayOrigin = glm::vec3(nearPoint);
    const glm::vec3 rayDir = glm::normalize(glm::vec3(farPoint - nearPoint));

    // Spawn on a camera-facing plane through the orbit target so placement is not locked to z = 0.
    glm::vec3 hitPoint;
    const bool hit = rayPlaneIntersection(
        rayOrigin,
        rayDir,
        camera.target,
        forward,
        hitPoint
    );
    if (!hit) {
        return false;
    }

    worldPoint = Vec(
        static_cast<Num>(hitPoint.x) * scaleX,
        static_cast<Num>(hitPoint.y) * scaleY,
        static_cast<Num>(hitPoint.z)
    );
    return true;
}
}

void processInput(GLFWwindow *window, std::vector<Body>& bodies, Num scaleX, Num scaleY, Camera& camera, float deltaTime) {
    static DragState dragState;
    static bool rightMouseActive = false;
    static double lastLookX = 0.0;
    static double lastLookY = 0.0;
    static bool scrollCallbackSet = false;

    if (!scrollCallbackSet) {
        glfwSetScrollCallback(window, scroll_callback);
        scrollCallbackSet = true;
    }

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    const glm::vec3 forward = cameraForward(camera);
    const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    const glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
    const glm::vec3 up = glm::normalize(glm::cross(right, forward));
    const float panAmount = camera.panSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.target += up * panAmount;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.target -= up * panAmount;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.target -= right * panAmount;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.target += right * panAmount;

    if (pendingScrollY != 0.0f) {
        camera.distance -= pendingScrollY * camera.zoomSensitivity;
        camera.distance = std::clamp(camera.distance, camera.minDistance, camera.maxDistance);
        pendingScrollY = 0.0f;
    }

    const int leftState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    const int rightState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    double cursorX = 0.0;
    double cursorY = 0.0;
    glfwGetCursorPos(window, &cursorX, &cursorY);

    if (rightState == GLFW_PRESS) {
        if (!rightMouseActive) {
            rightMouseActive = true;
            lastLookX = cursorX;
            lastLookY = cursorY;
        }

        const float dx = static_cast<float>(cursorX - lastLookX);
        const float dy = static_cast<float>(cursorY - lastLookY);
        camera.yaw += dx * camera.lookSensitivity;
        camera.pitch -= dy * camera.lookSensitivity;
        camera.pitch = std::clamp(camera.pitch, -89.0f, 89.0f);
        lastLookX = cursorX;
        lastLookY = cursorY;
    } else {
        rightMouseActive = false;
    }

    if (leftState == GLFW_PRESS && !dragState.isDragging) {
        dragState.isDragging = true;
        dragState.pressX = cursorX;
        dragState.pressY = cursorY;
    } else if (leftState == GLFW_RELEASE && dragState.isDragging) {
        Vec pressPos;
        Vec releasePos;
        const bool pressHit = cursorToWorldOnSimPlane(window, dragState.pressX, dragState.pressY, camera, scaleX, scaleY, pressPos);
        const bool releaseHit = cursorToWorldOnSimPlane(window, cursorX, cursorY, camera, scaleX, scaleY, releasePos);

        if (!pressHit || !releaseHit) {
            pressPos = cursorToWorld(window, dragState.pressX, dragState.pressY, scaleX, scaleY);
            releasePos = cursorToWorld(window, cursorX, cursorY, scaleX, scaleY);
        }

        const Vec dragVector = releasePos - pressPos;

        constexpr Num NEW_BODY_MASS = 1e-4; //around a third of earth's mass; arbitrary choice
        constexpr Num VELOCITY_SCALE = 4.0;
        const Vec launchVelocity = dragVector * VELOCITY_SCALE;

        bodies.emplace_back(NEW_BODY_MASS, releasePos, launchVelocity);
        dragState.isDragging = false;
    }
}