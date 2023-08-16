
#include "ball_object.h"
#include "game_object.h"
#include "glm/detail/qualifier.hpp"
#include "glm/fwd.hpp"
#include <cmath>
#include <math.h>



BallObject::BallObject() : GameObject(), m_Radius(12.5f), bStuck(true), bSticky(false), bPassThrough(false) {}

BallObject::BallObject(glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite)
    : GameObject(pos, glm::vec2(radius * 2.0f, radius * 2.0f), sprite, glm::vec3(1.0f), velocity),
      m_Radius(radius), bStuck(true), bSticky(false), bPassThrough(false) {}

glm::vec2 BallObject::Move(float dt, int window_width)
{
    if (!bStuck) {
        m_Position += m_Velocity * dt;

        if (m_Position.x < 0.f) {
            m_Velocity.x = -m_Velocity.x;
            m_Position.x = 0.f;
        }
        else if (m_Position.x + m_Size.x >= window_width) {
            m_Velocity.x = -m_Velocity.x;
            m_Position.x = window_width - m_Size.x;
        }
        else if (m_Position.y < 0.f) {
            m_Velocity.y = -m_Velocity.y;
            m_Position.y = 0.f;
        }
    }
    return this->m_Position;
}

void BallObject::Reset(glm::vec2 pos, glm::vec2 velocity)
{
    m_Position   = pos;
    m_Velocity   = velocity;
    this->bStuck = true;
    bSticky      = false;
    bPassThrough = false;
}
