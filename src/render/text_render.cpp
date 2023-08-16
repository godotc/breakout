#include "text_render.h"
#include "../resource_manager/resource_manager.h"
#include "gl_macros.h"
#include "glm/glm.hpp"
#include "log.h"


#include <ft2build.h>
#include FT_FREETYPE_H

TextRender::TextRender(unsigned int w, unsigned int h)
{
    TextShader = ResourceManager::GetShader("text");
    TextShader.Use().SetMatrix4("projection", glm::ortho(0.f, (float)w, (float)h, 0.f));
    TextShader.SetInteger("tex", 0);

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);


    glBindVertexArray(VAO);
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

            glEnableVertexAttribArray(0); // layout 0
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
}

void TextRender::Load(std::string font_path, unsigned int font_size)
{
    this->Characters.clear();

    FT_Library ft;
    if (/*0 != */ FT_Init_FreeType(&ft)) {
        LOG_ERROR("FREETYPE: colud not init freetye library");
    }

    FT_Face face;
    if (/*0!=*/FT_New_Face(ft, font_path.c_str(), 0, &face)) {
        LOG_ERROR("FREETYPE: could not load font from {}", font_path);
    }

    FT_Set_Pixel_Sizes(face, 0, font_size);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // loop ASCII characters, preload/compile
    for (GLubyte c = 0; c < 128; c++)
    {
        LOG_DEBUG("FREETYPE: loading character of '{}'", c);
        if (/*0!=*/FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            LOG_DEBUG("FREETYPE: failed to load Glyph from '{}'", c);
            continue;
        }

        auto *glyph  = face->glyph;
        auto &bitmap = face->glyph->bitmap;
        LOG_DEBUG("FREETYPE: font size of '{}' is ({}, {})", c, bitmap.width, bitmap.rows);

        Texture2D texture;
        {
            texture.Image_Format    = GL_RED;
            texture.Internal_Format = GL_RED;
            texture.Wrap_S          = GL_CLAMP_TO_EDGE;
            texture.Wrap_T          = GL_CLAMP_TO_EDGE;
            texture.Filter_Max      = GL_LINEAR;
            texture.Filter_Min      = GL_LINEAR;
            texture.Generate(bitmap.width, bitmap.rows, bitmap.buffer);
        }

        Character ch = {
            texture.ID,
            glm::ivec2(bitmap.width, bitmap.rows),
            glm::ivec2(glyph->bitmap_left, glyph->bitmap_top),
            static_cast<unsigned int>(glyph->advance.x)};

        Characters.insert({c, ch});
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void TextRender::RenderText(std::string text, float x, float y, float scale /*= 1.f*/, glm::vec3 color /*= glm::vec3(1.f)*/)
{
    TextShader.Use();
    // TextShader.SetFloat("scale", scale);
    TextShader.SetVector3f("texColor", color);
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(this->VAO);
    {
        for (const char c : text) {
            auto ch = Characters[c];

            const float x_pos = x + ch.Bearing.x * scale;
            const float y_pos = y + (Characters['H'].Bearing.y - ch.Bearing.y) * scale; // Let the char align by bottom

            const float w = ch.Size.x * scale;
            const float h = ch.Size.y * scale;

            const float vertices[6][4] = {
                {    x_pos, y_pos + h, 0.f, 1.f},
                {x_pos + w,     y_pos, 1.f, 0.f},
                {    x_pos,     y_pos, 0.f, 0.f},

                {    x_pos, y_pos + h, 0.f, 1.f},
                {x_pos + w, y_pos + h, 1.f, 1.f},
                {x_pos + w,     y_pos, 1.f, 0.f},
            };

            glBindTexture(GL_TEXTURE_2D, ch.TextureID); //

            // update buffer
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += (ch.Advance >> 6) * scale; // bitshift by 6 times to get value in pixels (2^6=64)
        }
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    GL_CHECK_HEALTH();
}
