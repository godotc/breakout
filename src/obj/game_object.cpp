#include "game_object.h"



GameObject::GameObject()
    : m_Position(0.0f, 0.0f), m_Size(1.0f, 1.0f), m_Velocity(0.0f), m_Color(1.0f), m_Rotation(0.0f), m_IsSolid(false), m_IsDestroyed(false), m_Sprite() {}

GameObject::GameObject(vec2 pos, vec2 size, Texture2D &sprite, vec3 color, vec2 velocity)
    : m_Position(pos), m_Size(size), m_Velocity(velocity), m_Color(color), m_Rotation(0.0f), m_IsSolid(false), m_IsDestroyed(false), m_Sprite(sprite) {}

void GameObject::Draw(SpriteRender &render)
{
    render.DrawSprite(m_Sprite, m_Position, m_Size, m_Rotation, m_Color);
}
