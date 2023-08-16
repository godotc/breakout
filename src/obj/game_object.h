#pragma once


#include "../render/sprite_render.h"
#include <glm/ext/vector_float2.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>

class GameObject
{
    using vec2 = glm::vec2;
    using vec3 = glm::vec3;

  public:
    GameObject();
    GameObject(vec2 pos, vec2 size, Texture2D &sprite, vec3 color = vec3(1.f), vec2 velocity = vec2(0.f, 0.f));

  public:
    virtual void Draw(SpriteRender &render);

  public:
    vec2  m_Position, m_Size, m_Velocity;
    vec3  m_Color;
    float m_Rotation;
    bool  m_IsSolid;
    bool  m_IsDestroyed;

    Texture2D m_Sprite;
};
