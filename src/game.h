#pragma once

#include "glm/fwd.hpp"
#include "level/game_level.h"

#include "obj/ball_object.h"
#include "obj/power_up.h"

#include "particle/particle_generator.h"

#include "render/sprite_render.h"
#include "render/text_render.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>



class PostProcessor;


enum class GameState
{
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
};

enum Direction
{
    UP,
    RIGHT,
    DOWN,
    LEFT
};


using OnObjectsCollied = std::function<void(GameObject *A, GameObject *B, std::tuple<bool, Direction, glm::vec2> *result)>;


inline const glm::vec2 PLAYER_SIZE{100.f, 20.f};
inline const float     PLAYER_VELOCITY{500.f};
inline const float     BALL_RADIUS = 12.5f;
inline const glm::vec2 INITIAL_BALL_VELOCITY(100.f, -350.f);


// struct CollisonCheckPair
// {
//     using Key   = std::pair<std::any &, std::any &>;
//     using Value = std::function<void(std::vector<std::any>)>;

//     void RegisterPair(Key *k, Value v)
//     {
//         this->pairs.insert_or_assign(k, v);
//     }

//     std::unordered_map<Key *, Value &> pairs;
// };

class Game
{
    using uint = unsigned int;

  public:
    Game(uint w, uint h);
    ~Game();
    void Init();

  public:
    void ProcessInput(float dt);
    void DoCollisions();
    void Update(float dt);
    void Render();

  private:
    void ResetLevel();
    void ResetPlayer();

    void ActivatePowerups(PowerUp &power_up);
    void SpawPowerUps(GameObject &block);
    void UpdatePowerups(float dt);

  private:
    void initCallback();
    void initShaders();
    void initTextures();
    void initLevels();
    void initAudios();

    void debugDraw();
    bool isOtherPoerUpActive(const std::string &type_name);

  private:
    using ColliedResult = std::tuple<bool, Direction, glm::vec2>;
    void onCollied_BallWithPaddle_Handler(BallObject *ball, GameObject *paddle, ColliedResult *result);
    void onCollied_BallWithBrick_Handler(BallObject *ball, GameObject *box, ColliedResult *result);
    void onCollied_PaddleWithPowerup_Handler(GameObject *paddle, GameObject *power_up, ColliedResult *result);

  public:

    GameState m_State;

    bool         m_keys[1024];
    bool         m_KeyProcessed[1024];
    unsigned int m_Width, m_Height;

    SpriteRender                       m_SpriteRnder;
    std::shared_ptr<GameObject>        m_Player;
    std::shared_ptr<BallObject>        m_Ball;
    std::shared_ptr<ParticleGenerator> m_Particles;
    std::shared_ptr<PostProcessor>     m_Effects;
    std::shared_ptr<TextRender>        m_TextRender;

    std::vector<GameLevel> m_Levels;
    size_t                 m_LevelIndex;

    std::vector<PowerUp> m_PowerUps;

  private: // about the gameoplay numerical value
    size_t m_PlayerLives = {3};

  private:
    float ShakeTime = 0.f;

    // CollisonCheckPair collisionCheckPairs;

  private:
    OnObjectsCollied onCollied_BallWithPaddle;
    OnObjectsCollied onCollied_BallWithBrick;
    OnObjectsCollied onCollied_PaddleWithPowerup;
};
