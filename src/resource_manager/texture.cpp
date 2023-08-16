/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "texture.h"
#include "gl_macros.h"
#include "log.h"


Texture2D::Texture2D()
{
    GL_CALL(glGenTextures(1, &this->ID));
}

void Texture2D::Generate(const unsigned int width, const unsigned int height, const unsigned char *data)
{
    this->Width  = width;
    this->Height = height;


    // create Texture
    GL_CALL(glBindTexture(GL_TEXTURE_2D, this->ID));

    // LOG_DEBUG("Data is: {}", (char *)data);
    // Load a texture from .bmp not a nullptr but, error on this line
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, width, height, 0, this->Image_Format, GL_UNSIGNED_BYTE, data));

    // set Texture wrap and filter modes
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max));

    // unbind texture
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture2D::Bind() const
{
    GL_CALL(glBindTexture(GL_TEXTURE_2D, this->ID));
}
