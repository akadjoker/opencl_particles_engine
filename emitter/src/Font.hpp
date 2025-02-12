#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>
#include <stdarg.h>  
#include <vector>
#include <string>
#include "Shader.hpp"
#include "stb_truetype.h"

struct Character
{
    glm::vec2 uvOffset;
    glm::vec2 uvSize;
    glm::vec2 size;
    glm::vec2 bearing;
    float advance;
};

class FontRenderer {
private:
    GLuint VAO, VBO, textureAtlas;
    Shader* shader;
    std::map<char, Character> characters;
    int atlasWidth, atlasHeight;
    glm::mat4 projection;

    void generateFontAtlas(const std::string& fontPath, int fontSize);

public:
    FontRenderer(const std::string& fontPath, int fontSize);
    ~FontRenderer();

    void setSize(int width, int height);

    void release();
    void print(const std::string& text, float x, float y, float scale,
               glm::vec3 color);
    void printf(float x, float y, float scale, glm::vec3 color,
                const char* format, ...);
};
