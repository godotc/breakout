/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#pragma once
#include "log.h"
#include <glad/glad.h>

class Texture2D
{
    using uint = unsigned int;

  public:
    // constructor (sets default texture modes)
    Texture2D();
    ~Texture2D()
    {
        // LOG("Destruct of texture2D: {}", this->ID);
        // glDeleteTextures(1, &this->ID);
    }

    // generates texture from image data
    void Generate(const uint width, const uint height, const unsigned char *data);
    // binds the texture as the current active GL_TEXTURE_2D texture object
    void Bind() const;


  public:
    // holds the ID of the texture object, used for all texture operations to reference to this particular texture
    uint ID;

  public:
    // texture image dimensions
    uint Width{0}, Height{0}; // width and height of loaded image in pixels

    // texture Format
    uint Internal_Format{GL_RGB}; // format of texture object
    uint Image_Format{GL_RGB};    // format of loaded image

    // texture configuration
    uint Wrap_S{GL_REPEAT};     // wrapping mode on S axis
    uint Wrap_T{GL_REPEAT};     // wrapping mode on T axis
    uint Filter_Min{GL_LINEAR}; // filtering mode if texture pixels < screen pixels
    uint Filter_Max{GL_LINEAR}; // filtering mode if texture pixels > screen pixels
};
