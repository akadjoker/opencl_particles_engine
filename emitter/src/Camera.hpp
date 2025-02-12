#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    float yaw, pitch;
    float speed, sensitivity;
    int screenWidth, screenHeight;

    Camera(int screenWidth, int screenHeight,
           glm::vec3 startPosition = glm::vec3(0.0f, 0.5f, 3.0f));

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float fov = 45.0f, float near = 0.1f,
                                  float far = 100.0f) const;
    void setSize(int width, int height);

    void processKeyboard(GLFWwindow* window, float deltaTime);
    void processMouse(float xoffset, float yoffset);
    glm::vec3 screenToWorldRay(float mouseX, float mouseY) const;
};

