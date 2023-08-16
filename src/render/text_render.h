#pragma once


#include "glm/fwd.hpp"
#include <map>

#include "../resource_manager/shader.h"

struct Character
{
    unsigned int TextureID;
    glm::ivec2   Size;
    glm::ivec2   Bearing; // Offset from baseline top-left of glyh
    unsigned int Advance; // Horizontal offset to advance to next glyh
};


class TextRender
{
  public:
    TextRender(unsigned int w, unsigned int h);
    void Load(std::string font, unsigned int font_size);
    void RenderText(std::string text, float x, float y, float scale = 1.f, glm::vec3 color = glm::vec3(1.f));

  public:
    std::map<char, Character> Characters;
    Shader                    TextShader;

  private:
    unsigned int VAO, VBO;
};