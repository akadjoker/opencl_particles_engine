#pragma once

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

class Shader {
private:
    GLuint programID;

    GLuint compileShader(GLenum type, const std::string& source);
    GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);

public:
    Shader(const std::string& vertexSource, const std::string& fragmentSource);
    ~Shader();

    void use() const;
    void setUniform(const std::string& name, const glm::mat4& matrix) const;
    void setUniform(const std::string& name, const glm::vec3& vector) const;
    void setUniform(const std::string& name, float value) const;
    GLuint getProgramID() const { return programID; }
    void release();
};

 