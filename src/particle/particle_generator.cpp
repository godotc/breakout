#include "particle_generator.h"
#include "log.h"


ParticleGenerator::ParticleGenerator(Shader shader, Texture2D texture, size_t amount)
    : particle_shader(shader), particle_texture(texture), m_Amount(amount)
{
    init();
}

void ParticleGenerator::init()
{
    unsigned int VBO;

    float particle_quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f};

    glGenVertexArrays(1, &this->particle_VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(particle_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    {
        // fil mesh buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

        // set mesh attr
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0); // 2 posion 2 color
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    for (size_t i = 0; i < m_Amount; ++i)
    {
        m_Particles.push_back(Particle());
    }
}

void ParticleGenerator::Update(float dt, GameObject &object, size_t nun_new_particles, glm::vec2 offset)
{
    // add new particles
    for (size_t i = 0; i < nun_new_particles; ++i) {
        int unused_particle = first_unused_particle();
        respawn_particle(m_Particles[unused_particle], object, offset);
    }

    // update life and transf
    for (size_t i = 0; i < m_Amount; ++i) {
        auto &particle = m_Particles[i];
        particle.Life -= dt;
        if (particle.Life > 0.f) {
            particle.Position -= particle.Velocity * dt;
            particle.Color.a -= dt * 2.5f;
        }
    }
}

void ParticleGenerator::Draw()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    {
        particle_shader.Use();
        for (auto &particle : m_Particles) {
            if (particle.Life > 0.f)
            {
                particle_shader.SetVector2f("offset", particle.Position);
                particle_shader.SetVector4f("color", particle.Color);
                particle_texture.Bind();

                glBindVertexArray(particle_VAO);
                {
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }
                glBindVertexArray(0);
            }
            // LOG_DEBUG("Draw one particle at : {}, {} ", particle.Position.x, particle.Position.y);
        }
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


int ParticleGenerator::first_unused_particle()
{
    for (size_t i = last_used_particle; i < m_Amount; ++i) {
        if (m_Particles[i].Life <= 0.f) {
            last_used_particle = i;
            return i;
        }
    }

    for (size_t i = 0; i < m_Amount; ++i) {
        if (m_Particles[i].Life <= 0.f) {
            last_used_particle = i;
            return i;
        }
    }

    last_used_particle = 0;
    return 0;
}


void ParticleGenerator::respawn_particle(Particle &particle, GameObject &object, glm::vec2 offset)
{
    float random = ((rand() % 100) - 50) / 10.f;
    float color  = 0.5f + (rand() % 100 / 100.f);

    // LOG_DEBUG("color : {} ", color);

    particle.Position = object.m_Position + random + offset;
    particle.Color    = glm::vec4(sin(color), cos(color), tan(color), 1.f);
    particle.Life     = 1.f;
    particle.Velocity = object.m_Velocity * 0.1f;

    // LOG_DEBUG("Respawn one particle at {} {}", particle.Position.x, particle.Position.y);
}
