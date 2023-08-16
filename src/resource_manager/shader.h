#pragma once

/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/

#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>



// General purpose shader object. Compiles from file, generates
// compile/link-time error messages and hosts several utility
// functions for easy management.
class Shader //: public utils::disable_copy_allow_move
{
  public:
    // state
    unsigned int ID;

    // constructor
    Shader() = default;
    ~Shader()
    {
        // LOG("Destruct of Shader: {}", this->ID);
        //  GL_CALL(glDeleteProgram(this->ID));
    }

    // sets the current shader as active
    Shader &Use();

  public:
    void Compile(const char *vertexSource, const char *fragmentSource, const char *geometrySource = nullptr); // note: geometry source code is optional

    void SetFloat(const char *name, float value, bool useShader = false);
    void SetInteger(const char *name, int value, bool useShader = false);

    void SetVector2f(const char *name, float x, float y, bool useShader = false);
    void SetVector2f(const char *name, const glm::vec2 &value, bool useShader = false);

    void SetVector3f(const char *name, float x, float y, float z, bool useShader = false);
    void SetVector3f(const char *name, const glm::vec3 &value, bool useShader = false);

    void SetVector4f(const char *name, float x, float y, float z, float w, bool useShader = false);
    void SetVector4f(const char *name, const glm::vec4 &value, bool useShader = false);

    void SetMatrix4(const char *name, const glm::mat4 &matrix, bool useShader = false);

  public:
    // checks if compilation or linking failed and if so, print the error logs
    void checkCompileErrors(unsigned int object, const std::string &type);
};
