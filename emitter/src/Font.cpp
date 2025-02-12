#include "Font.hpp"
#include <iostream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

FontRenderer::FontRenderer(const std::string& fontPath, int fontSize)
{
    shader = new Shader(
        R"(
            #version 330 core
            layout(location = 0) in vec4 vertex;
            out vec2 TexCoords;
            uniform mat4 projection;
            void main() {
                gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
                TexCoords = vertex.zw;
            }
        )",
        R"(
            #version 330 core
            in vec2 TexCoords;
            out vec4 color;
            uniform sampler2D text;
            uniform vec3 textColor;
            void main() {
                vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
                color = vec4(textColor, 1.0) * sampled;
            }
        )");

    generateFontAtlas(fontPath, fontSize);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4 * 256, nullptr,
                 GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

FontRenderer::~FontRenderer()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &textureAtlas);
    delete shader;
}

void FontRenderer::setSize(int width, int height)
{
    //  projection = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
    projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f);
}

void FontRenderer::generateFontAtlas(const std::string& fontPath, int fontSize)
{
    std::vector<unsigned char> fontBuffer(1 << 20);
    FILE* fontFile = fopen(fontPath.c_str(), "rb");
    if (!fontFile)
    {
        std::cerr << "Erro ao carregar fonte: " << fontPath << std::endl;
        return;
    }
    size_t bytesRead = fread(fontBuffer.data(), 1, fontBuffer.size(), fontFile);
    fclose(fontFile);

    if (bytesRead == 0)
    {
        std::cerr << "Erro: arquivo da fonte está vazio" << std::endl;
        return;
    }

    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontBuffer.data(),
                        0)) // Removido stbtt_GetFontOffsetForIndex
    {
        std::cerr << "Erro ao inicializar fonte!" << std::endl;
        return;
    }

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    float scale =
        stbtt_ScaleForPixelHeight(&font, static_cast<float>(fontSize));

    int x = 0;
    int y = 0;
    int atlasWidth = 512;
    int atlasHeight = 512;
    int atlasSize = atlasWidth * atlasHeight;
    std::vector<unsigned char> atlas(atlasSize);
    int maxHeight = 0;

    for (unsigned char c = 32; c < 127; c++)
    {
        int width, height, xOffset, yOffset;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(
            &font, 0, scale, c, &width, &height, &xOffset, &yOffset);

        if (x + width >= atlasWidth)
        {
            x = 0;
            y += maxHeight + 1; // Adicionado padding vertical
            maxHeight = 0;
        }

        for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                if (y + row < atlasHeight && x + col < atlasWidth)
                { // Verificação de limites
                    atlas[(y + row) * atlasWidth + (x + col)] =
                        bitmap[row * width + col];
                }
            }
        }

        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&font, c, &advanceWidth, &leftSideBearing);
        float advance = advanceWidth * scale;

        characters[c] = { glm::vec2(x / static_cast<float>(atlasWidth),
                                    y / static_cast<float>(atlasHeight)),
                          glm::vec2(width / static_cast<float>(atlasWidth),
                                    height / static_cast<float>(atlasHeight)),
                          glm::vec2(width, height), glm::vec2(xOffset, yOffset),
                          advance };

        x += width + 1; // Adicionado padding horizontal
        maxHeight = std::max(maxHeight, height);
        stbtt_FreeBitmap(bitmap, nullptr);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT,
                  1); // Importante para texturas com GL_RED
    glGenTextures(1, &textureAtlas);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED,
                 GL_UNSIGNED_BYTE, atlas.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void FontRenderer::print(const std::string& text, float x, float y, float scale,
                         glm::vec3 color)
{
    shader->use();
    shader->setUniform("textColor", color);
    shader->setUniform("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);
    glBindVertexArray(VAO);

    std::vector<float> vertices;

    for (const char& c : text)
    {
        Character ch = characters[c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y + ch.bearing.y * scale;
        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        vertices.insert(vertices.end(),
                        { xpos,
                          ypos + h,
                          ch.uvOffset.x,
                          ch.uvOffset.y + ch.uvSize.y,
                          xpos,
                          ypos,
                          ch.uvOffset.x,
                          ch.uvOffset.y,
                          xpos + w,
                          ypos,
                          ch.uvOffset.x + ch.uvSize.x,
                          ch.uvOffset.y,

                          xpos,
                          ypos + h,
                          ch.uvOffset.x,
                          ch.uvOffset.y + ch.uvSize.y,
                          xpos + w,
                          ypos,
                          ch.uvOffset.x + ch.uvSize.x,
                          ch.uvOffset.y,
                          xpos + w,
                          ypos + h,
                          ch.uvOffset.x + ch.uvSize.x,
                          ch.uvOffset.y + ch.uvSize.y });

        x += ch.advance * scale;
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 4);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void FontRenderer::printf(float x, float y, float scale,glm::vec3 color,const char* format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format); 
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    print(std::string(buffer), x, y, scale, color);
}

void FontRenderer::release()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &textureAtlas);
    if (shader)
    {
        shader->release();
        delete shader;
        shader = nullptr;
    }
}
