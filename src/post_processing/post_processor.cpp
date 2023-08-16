#include "./post_processor.h"
#include "gl_macros.h"

PostProcessor::PostProcessor(Shader shader, unsigned int w, unsigned int h)
{
    PostProcessingShader = shader;
    m_Width              = w;
    m_Height             = h;
    bChaos               = false;
    bConfuse             = false;
    bShake               = false;

    glGenFramebuffers(1, &this->MSFBO);
    glGenFramebuffers(1, &this->FBO);
    glGenRenderbuffers(1, &this->RBO);

    // initialize renderbuffer storeage with a multisampled color buffer (don't need depth /stencil buffer)
    glBindFramebuffer(GL_FRAMEBUFFER, MSFBO);
    {
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        {
            // allocate  storage for render
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB, m_Width, m_Height);

            // attach MS render buffer objcet to framebuffer
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, RBO);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                LOG_ERROR("POSTPROCESSOR: Failed to initialize MSFBD");
            }
        }
    }

    // Initialze FBO and texture
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    {
        this->Texture.Generate(m_Width, m_Height, NULL);
        // attach texture to frambuffer as its color attachment
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->Texture.ID, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOG_ERROR("POSTPROCESSOR: Failed to initialize FBO");
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // initialize initial data and unifroms
    initRenderData();
    this->PostProcessingShader.Use().SetInteger("scene", 0);

    float offset        = 1.f / 300.f;
    float offsets[9][2] = {
        {-offset,  offset}, // top-left
        {   0.0f,  offset}, // top-center
        { offset,  offset}, // top-right
        {-offset,    0.0f}, // center-left
        {   0.0f,    0.0f}, // center-center
        { offset,    0.0f}, // center - right
        {-offset, -offset}, // bottom-left
        {   0.0f, -offset}, // bottom-center
        { offset, -offset}  // bottom-right
    };
    glUniform2fv(glGetUniformLocation(this->PostProcessingShader.ID, "offsets"), 9, (float *)offsets);

    int edge_kernel[9]{
        -1, -1, -1,
        -1, 8, -1,
        -1, -1, -1};
    glUniform1iv(glGetUniformLocation(PostProcessingShader.ID, "edge_kernel"), 9, edge_kernel);

    float blur_kernel[9]{
        1.f / 16.f, 2.f / 16.f, 1.f / 16.f,
        1.f / 16.f, 4.f / 16.f, 2.f / 16.f,
        1.f / 16.f, 2.f / 16.f, 1.f / 16.f};
    glUniform1fv(glGetUniformLocation(PostProcessingShader.ID, "blur_kernel"), 9, blur_kernel);

    GL_CHECK_HEALTH();
}

void PostProcessor::BeginRender()
{
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, MSFBO));
    GL_CALL(glClearColor(0, 0, 0, 1));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

void PostProcessor::EndRender()
{
    // resolve multisampled color-buffer object(MSFBO) into intermediate frame buffer object(FBO) to store as texture
    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, MSFBO));
    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO));
    // blit == tranfer == memcpy
    GL_CALL(glBlitFramebuffer(0, 0, m_Width, m_Height, 0, 0, m_Width, m_Height, GL_COLOR_BUFFER_BIT, GL_NEAREST));
    // bind both read and wirete frame buffer to default frame buffer (reset to 0)
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void PostProcessor::Render(float time)
{
    PostProcessingShader.Use();
    PostProcessingShader.SetFloat("time", time);
    PostProcessingShader.SetInteger("confuse", bConfuse);
    PostProcessingShader.SetInteger("chaos", bChaos);
    PostProcessingShader.SetInteger("shake", bShake);

    glActiveTexture(GL_TEXTURE0);
    Texture.Bind();

    glBindVertexArray(VAO);
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glBindVertexArray(0);
}

void PostProcessor::initRenderData()
{
    // configure VAO/VBO
    unsigned int VBO;
    float        vertices[] =
        {-1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, 1.0f, 1.0f, 1.0f,
         -1.0f, 1.0f, 0.0f, 1.0f,

         -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f, 1.0f, 1.0f, 1.0f};

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &VBO);


    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GL_CHECK_HEALTH();
}
