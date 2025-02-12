#include "Grid.hpp"
#include <iostream>


const std::string vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 position;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * view * vec4(position, 1.0);
    }
)";


const std::string fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(0.5, 0.5, 0.5, 1.0);
    }
)";

Grid::Grid(int size, float spacing): gridSize(size), spacing(spacing)
{
    shader = new Shader(vertexShaderSource, fragmentShaderSource);
    generateGrid();
}

Grid::~Grid() {}

void Grid::generateGrid()
{
    std::vector<glm::vec3> vertices;

    // eixo X
    for (int i = -gridSize; i <= gridSize; i++)
    {
        vertices.push_back(glm::vec3(i * spacing, 0.0f, -gridSize * spacing));
        vertices.push_back(glm::vec3(i * spacing, 0.0f, gridSize * spacing));
    }


    // eixo Z
    for (int i = -gridSize; i <= gridSize; i++)
    {
        vertices.push_back(glm::vec3(-gridSize * spacing, 0.0f, i * spacing));
        vertices.push_back(glm::vec3(gridSize * spacing, 0.0f, i * spacing));
    }


    count = vertices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
                 vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                          (void*)0);

    glBindVertexArray(0);
}

void Grid::draw(const glm::mat4& view, const glm::mat4& projection)
{
    shader->use();
    shader->setUniform("view", view);
    shader->setUniform("projection", projection);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, count);
    glBindVertexArray(0);
}

void Grid::release()
{
    if (VAO == 0) return;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    VAO = 0;
    VBO = 0;
    if (shader)
    {
        shader->release();
        delete shader;
        shader = nullptr;
    }
}
