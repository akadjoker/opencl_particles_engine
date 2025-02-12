#pragma once
#include <GLFW/glfw3.h>


class Timer {
private:
    double lastTime;
    double fpsUpdateTime;
    int frameCount;
    float deltaTime;
    float fps;

public:
    Timer()
        : lastTime(glfwGetTime()), fpsUpdateTime(0), frameCount(0),
          deltaTime(0), fps(0)
    {}

    void update()
    {
        double currentTime = glfwGetTime();
        deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        frameCount++;
        fpsUpdateTime += deltaTime;
        if (fpsUpdateTime >= 1.0)
        {
            fps = frameCount / fpsUpdateTime;
            frameCount = 0;
            fpsUpdateTime = 0.0;
        }
    }

    float getDeltaTime() const { return deltaTime; }
    float getFPS() const { return fps; }
};

