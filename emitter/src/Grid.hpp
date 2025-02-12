#ifndef GRID_H
#define GRID_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.hpp"

class Grid {
private:
    GLuint VAO, VBO;
    Shader* shader;
    int gridSize;
    float spacing;
    long count;

    void generateGrid();

public:
    Grid(int size = 10, float spacing = 1.0f);
    ~Grid();

    void draw(const glm::mat4& view, const glm::mat4& projection);
    void release();
};

#endif
