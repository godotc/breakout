

#include "shader.h"
#include "gl_macros.h"
#include <cmath>
#include <iostream>
#include <type_traits>

Shader &Shader::Use()
{
    GL_CALL(glUseProgram(this->ID));
    return *this;
}

void Shader::Compile(const char *vertexSource, const char *fragmentSource, const char *geometrySource)
{
    unsigned int sVertex, sFragment, gShader;

    // vertex Shader
    GL_CALL(sVertex = glCreateShader(GL_VERTEX_SHADER));
    GL_CALL(glShaderSource(sVertex, 1, &vertexSource, nullptr));
    GL_CALL(glCompileShader(sVertex));
    checkCompileErrors(sVertex, "VERTEX");


    // fragment Shader
    GL_CALL(sFragment = glCreateShader(GL_FRAGMENT_SHADER));
    GL_CALL(glShaderSource(sFragment, 1, &fragmentSource, nullptr));
    GL_CALL(glCompileShader(sFragment));
    checkCompileErrors(sFragment, "FRAGMENT");

    // if geometry shader source code is given, also compile geometry shader
    if (geometrySource != nullptr)
    {
        GL_CALL(gShader = glCreateShader(GL_GEOMETRY_SHADER));
        GL_CALL(glShaderSource(gShader, 1, &geometrySource, nullptr));
        GL_CALL(glCompileShader(gShader));
        checkCompileErrors(gShader, "GEOMETRY");
    }

    // shader program
    this->ID = glCreateProgram();
    GL_CALL(glAttachShader(this->ID, sVertex));
    GL_CALL(glAttachShader(this->ID, sFragment));
    if (geometrySource != nullptr)
        GL_CALL(glAttachShader(this->ID, gShader));

    GL_CALL(glLinkProgram(this->ID));
    checkCompileErrors(this->ID, "PROGRAM");

    // delete the shaders as they're linked into our program now and no longer necessary
    GL_CALL(glDeleteShader(sVertex));
    GL_CALL(glDeleteShader(sFragment));
    if (geometrySource != nullptr)
        GL_CALL(glDeleteShader(gShader));
}

void Shader::SetFloat(const char *name, float value, bool useShader)
{
    if (useShader)
        this->Use();
    GL_CALL(glUniform1f((glGetUniformLocation(this->ID, name)), value));
}
void Shader::SetInteger(const char *name, int value, bool useShader)
{
    if (useShader)
        this->Use();
    GL_CALL(glUniform1i(glGetUniformLocation(this->ID, name), value));
}
void Shader::SetVector2f(const char *name, float x, float y, bool useShader)
{
    if (useShader)
        this->Use();
    GL_CALL(glUniform2f(glGetUniformLocation(this->ID, name), x, y));
}
void Shader::SetVector2f(const char *name, const glm::vec2 &value, bool useShader)
{
    if (useShader)
        this->Use();
    GL_CALL(glUniform2f(glGetUniformLocation(this->ID, name), value.x, value.y));
}
void Shader::SetVector3f(const char *name, float x, float y, float z, bool useShader)
{
    if (useShader)
        this->Use();
    GL_CALL(glUniform3f(glGetUniformLocation(this->ID, name), x, y, z));
}
void Shader::SetVector3f(const char *name, const glm::vec3 &value, bool useShader)
{
    if (useShader)
        this->Use();
    GL_CALL(glUniform3f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z));
}
void Shader::SetVector4f(const char *name, float x, float y, float z, float w, bool useShader)
{
    if (useShader)
        this->Use();
    GL_CALL(glUniform4f(glGetUniformLocation(this->ID, name), x, y, z, w));
}
void Shader::SetVector4f(const char *name, const glm::vec4 &value, bool useShader)
{
    if (useShader)
        this->Use();
    GL_CALL(glUniform4f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z, value.w));
}
void Shader::SetMatrix4(const char *name, const glm::mat4 &matrix, bool useShader)
{
    if (useShader)
        this->Use();
    GL_CALL(glUniformMatrix4fv(glGetUniformLocation(this->ID, name), 1, false, glm::value_ptr(matrix)));
}


void Shader::checkCompileErrors(unsigned int object, const std::string &type)
{
    int  success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        GL_CALL(glGetShaderiv(object, GL_COMPILE_STATUS, &success));
        if (!success)
        {
            GL_CALL(glGetShaderInfoLog(object, 1024, nullptr, infoLog));
            std::cerr << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- "
                      << std::endl;
        }
    }
    else
    {
        GL_CALL(glGetProgramiv(object, GL_LINK_STATUS, &success));
        if (!success)
        {
            GL_CALL(glGetProgramInfoLog(object, 1024, nullptr, infoLog));
            std::cerr << "| ERROR::Shader: Link-time error: Type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- "
                      << std::endl;
        }
    }
}
