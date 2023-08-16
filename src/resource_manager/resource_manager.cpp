
/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "resource_manager.h"

#include "shader.h"

#include "gl_macros.h"

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>

#include "log.h"
#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// Instantiate static variables
std::map<std::string, Texture2D> ResourceManager::Textures;
std::map<std::string, Shader>    ResourceManager::Shaders;


Shader ResourceManager::LoadShader(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile, std::string name)
{
    LOG_DEBUG("Loadding shader [{}]..", name);
    Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    return Shaders[name];
}

Shader ResourceManager::LoadShader(const char *UniversalFileName, const char *name)
{
    auto &&[vertSource, fragSource, gemoSource] = parseShaderFile(UniversalFileName);
    if (!vertSource.empty() && !fragSource.empty()) {
        Shader shader;
        shader.Compile(vertSource.c_str(), fragSource.c_str(), gemoSource.c_str());
        Shaders[name] = shader;
        return shader;
    }

    LOG_ERROR("Load shader from file {} failed!", UniversalFileName);
    return {};
}

Shader &ResourceManager::GetShader(const char *name)
{
    return Shaders[name];
}

Texture2D ResourceManager::LoadTexture(const char *file, std::string name)
{
    Textures[name] = loadTextureFromFile(file);
    return Textures[name];
}

Texture2D ResourceManager::GetTexture(std::string name)
{
#ifndef NDBUG
    if (Textures.find(name) == Textures.end())
        LOG_ERROR("Not such texture: {} ", name);
#endif
    return Textures[name];
}

Texture2D &ResourceManager::GetTextureRef(std::string name)
{
#ifndef NDBUG
    if (Textures.find(name) == Textures.end())
        LOG_ERROR("Not such texture: {} ", name);
#endif
    return Textures[name];
}

void ResourceManager::Clear()
{

    // (properly) delete all shaders
    for (const auto &iter : Shaders)
        GL_CALL(glDeleteProgram(iter.second.ID));
    // (properly) delete all textures
    for (const auto &iter : Textures)
        GL_CALL(glDeleteTextures(1, &iter.second.ID));
}

Shader ResourceManager::loadShaderFromFile(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    try
    {
        // open files
        std::ifstream     vertexShaderFile(vShaderFile);
        std::ifstream     fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;

        // read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();
        // convert stream into string
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        // if geometry shader path is present, also load a geometry shader
        if (gShaderFile != nullptr)
        {
            std::ifstream     geometryShaderFile(gShaderFile);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::exception e)
    {
        std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
    }

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();
    const char *gShaderCode = geometryCode.c_str();
    // 2. now create shader object from source code
    Shader shader;
    shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
    return shader;
}


ShaderProgramSource ResourceManager::parseShaderFile(const std::filesystem::path &UniversalFilePath)
{
    if (!UniversalFilePath.has_extension()) {
        LOG_WARN("Don't konw shader source file type");
        return {};
    }
#if !_WIN32
#warning fix the LOG macro
#warning compare issue
#endif
    auto ext = UniversalFilePath.filename().extension();
    if (ext.compare(".glsl")) {
        LOG_WARN("Not glsl file");
        return {};
    }


    std::ifstream stream(UniversalFilePath);

    enum class ShaderType
    {
        NONE     = -1,
        VERTEX   = 0,
        FRAGMENT = 1,
        GEOMETRY = 2,
    };

    std::string       line;
    std::stringstream ss[3];
    ShaderType        type = ShaderType::NONE;

    while (std::getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
            else if (line.find("geometry") != std::string::npos)
                type = ShaderType::GEOMETRY;
        }
        else
        {
            ss[(int)type] << line << "\n";
        }
    }
    return {ss[0].str(), ss[1].str(), ss[2].str()};
}

Texture2D ResourceManager::loadTextureFromFile(const char *file)
{
    // create texture object
    Texture2D texture;
    texture.Internal_Format = GL_RGBA;
    texture.Image_Format    = GL_RGBA;

    // load image
    int width, height, nrChannels;


    unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
    LOG_LOG("File: {} , W:H = {}:{}, channels: {}", file, width, height, nrChannels);
    if (!data) {
        LOG_ERROR("Load the texture from file {}, but get nullptr!", file);
        return texture;
    }

    if (nrChannels < 4) {
        texture.Internal_Format = GL_RGB;
        texture.Image_Format    = GL_RGB;
    }


    // now generate texture
    texture.Generate(width, height, data);
    // and finally free image data
    stbi_image_free(data);
    return texture;
}


// auto GetAudio() -> Audio *
// {
//     auto audio = new Audio;
//     return audio;
// }
// void ResourceManager::PlaySound(const char *name)
// {
//     GetAudio()->Play(name);
// }

// void ResourceManager::LoadSound(const char *file, std::string name)
// {
// #ifndef NDBUG
//     if (Sounds.find(name) == Sounds.end()) {
//         LOG_ERROR("Not such texture: {} ", name);
//         return;
//     }
// #endif
//     GetAudio()->Play(Sounds[name]);
// }