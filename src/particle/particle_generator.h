#pragma once

#include "../obj/game_object.h"
#include "./particle.h"
#include "glm/fwd.hpp"
#include <vector>



class ParticleGenerator
{
  public:
    ParticleGenerator(Shader shader, Texture2D texture, size_t amount);


  public:
    void Update(float dt, GameObject &object, size_t nun_new_particles, glm::vec2 offset = glm::vec2(0.f, 0.f));
    void Draw();

  private:
    void init();
    int  first_unused_particle();
    void respawn_particle(Particle &particle, GameObject &object, glm::vec2 offset);

  private:
    std::vector<Particle> m_Particles;

    size_t    m_Amount = 500;
    Shader    particle_shader;
    Texture2D particle_texture;
    GLuint    particle_VAO;
    size_t    last_used_particle = 0;
};
