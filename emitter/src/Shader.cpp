#include "Shader.hpp"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const std::string& vertexSource,
               const std::string& fragmentSource)
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    programID = linkProgram(vertexShader, fragmentShader);
}

Shader::~Shader() { release(); }

void Shader::use() const { glUseProgram(programID); }

GLuint Shader::compileShader(GLenum type, const std::string& source)
{
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Erro ao compilar shader: " << infoLog << std::endl;
    }

    return shader;
}

GLuint Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Erro ao ligar programa de shaders: " << infoLog
                  << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void Shader::setUniform(const std::string& name, const glm::mat4& matrix) const
{
    GLint location = glGetUniformLocation(programID, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::setUniform(const std::string& name, const glm::vec3& vector) const
{
    GLint location = glGetUniformLocation(programID, name.c_str());
    glUniform3fv(location, 1, glm::value_ptr(vector));
}

void Shader::setUniform(const std::string& name, float value) const
{
    GLint location = glGetUniformLocation(programID, name.c_str());
    glUniform1f(location, value);
}

void Shader::release() 
{
    if (programID == 0) return;
    glDeleteProgram(programID); 
    programID = 0;
}
