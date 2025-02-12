#include "Camera.hpp"
#include <glm/gtc/type_ptr.hpp>

Camera::Camera(int screenWidth, int screenHeight, glm::vec3 startPosition)
    // : screenWidth(screenWidth), screenHeight(screenHeight),
    //   position(startPosition), front(glm::vec3(0.0f, 0.0f, -1.0f)),
    //   up(glm::vec3(0.0f, 1.0f, 0.0f)), yaw(-90.0f), pitch(0.0f), speed(5.0f),
    //   sensitivity(0.1f)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    this->position = startPosition;
    this->front = glm::vec3(0.0f, 0.0f, -1.0f);
    this->up = glm::vec3(0.0f, 1.0f, 0.0f);
    this->yaw = -90.0f;
    this->pitch = 0.0f;
    this->speed = 5.0f;
    this->sensitivity = 0.1f;

}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float fov, float near, float far) const
{
    glViewport(0, 0, screenWidth, screenHeight);
    return glm::perspective(
        glm::radians(fov), (float)screenWidth / (float)screenHeight, near, far);
}

void Camera::setSize(int width, int height) 
{
    screenWidth = width;
    screenHeight = height;
    
}

void Camera::processKeyboard(GLFWwindow* window, float deltaTime)
{
    float velocity = speed * deltaTime;
    glm::vec3 right = glm::normalize(glm::cross(front, up));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        position += front * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        position -= front * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        position -= right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        position += right * velocity;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void Camera::processMouse(float xoffset, float yoffset)
{
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);
}

glm::vec3 Camera::screenToWorldRay(float mouseX, float mouseY) const
{
    // Converter coordenadas do rato para Normalized Device Coordinates (NDC)
    glm::vec4 rayClip(
        (2.0f * mouseX) / screenWidth - 1.0f, // X Normalizado
        1.0f - (2.0f * mouseY) / screenHeight, // Y Normalizado (inverter Y)
        -1.0f, // Cursor na frente da câmara
        1.0f);


    glm::vec4 rayEye = glm::inverse(getProjectionMatrix()) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::vec3 rayWorld = glm::vec3(glm::inverse(getViewMatrix()) * rayEye);
    rayWorld = glm::normalize(rayWorld); 

    return rayWorld;
}
