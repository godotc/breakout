#pragma once

#include "game_object.h"
#include "glm/fwd.hpp"

class BallObject : public GameObject
{
  public:
    BallObject();
    BallObject(glm::vec2 pos, float Radius, glm::vec2 velocity, Texture2D sprite);

  public:
    glm::vec2 Move(float dt, int window_width);
    void      Reset(glm::vec2 pos, glm::vec2 vlocity);

  public:
    float m_Radius;
    bool  bStuck;
    bool  bSticky;
    bool  bPassThrough;
};
