#pragma once

#include "../resource_manager/shader.h"
#include "../resource_manager/texture.h"


class PostProcessor
{
  public:
    PostProcessor(Shader shader, unsigned int w, unsigned int h);

  public:
    void BeginRender();
    void EndRender();

    void Render(float time);

  private:
    void
    initRenderData();

  public:
    Shader       PostProcessingShader;
    Texture2D    Texture;
    unsigned int m_Width, m_Height;
    bool         bConfuse, bChaos, bShake;


  private:

    unsigned int
        VAO,
        MSFBO, // Multisampled FBO
        FBO,   // blitting MS color-buffer to texture
        RBO;   // used for multisampled color buffer
};
