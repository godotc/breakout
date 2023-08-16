#pragma once

#include "glm/fwd.hpp"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct Particle
{
    glm::vec2 Position, Velocity;
    glm::vec4 Color;
    float     Life;

    Particle() : Position(0.f), Velocity(0.f), Color(1.f), Life(0.f) {}
};
