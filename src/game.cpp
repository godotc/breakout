#include "game.h"
#include "GLFW/glfw3.h"
#include "SDL_mixer.h"
#include "fmt/core.h"
#include "glm/common.hpp"
#include "glm/geometric.hpp"
#include "obj/ball_object.h"
#include "obj/game_object.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <functional>
#include <gl_macros.h>

#include "post_processing/post_processor.h"


#include <glm/ext/matrix_clip_space.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/mat4x4.hpp>
#include <math.h>
#include <memory>
#include <mutex>
#include <stdlib.h>
#include <string_view>
#include <tuple>
#include <vector>

#include <fmt/format.h>
#include <log.h>


// TODO: more wrapper and intergrate into resource manager
#include "audio/sdl_wrapper/sdl_backend.h"


static auto sprite_shader   = "sprite";
static auto particle_shader = "particle";

template <class T>
static auto clamp(T val, T min, T max) -> T { std::max(min, std::min(max, val)); }


static Direction                              VectorDirection(glm::vec2 target);
static bool                                   CollisionCheck_2Rect_AABB(const GameObject &A, const GameObject &B);
static std::tuple<bool, Direction, glm::vec2> CollisionCheck_BallWithRect(const BallObject &A, const GameObject &B);

static bool ShouldSpawn(unsigned int change);

static void QuickPlaySoundEffect(const char *sound_name);



Game::Game(unsigned int width, unsigned int height) : m_State(GameState::GAME_ACTIVE), m_keys(), m_Width(width), m_Height(height)
{
    initCallback();
}

Game::~Game()
{
}

void Game::Init()
{
    LOG_LOG("W: {} | H: {}", m_Width, m_Height);

    initShaders();
    initTextures();
    initLevels();
    initAudios();


    // Player
    glm::vec2 player_pos = glm::vec2(m_Width / 2.f - PLAYER_SIZE.x / 2.f,
                                     m_Height - PLAYER_SIZE.y);
    m_Player             = std::make_shared<GameObject>(player_pos, PLAYER_SIZE,
                                            ResourceManager::GetTextureRef("paddle"));

    // Ball
    auto ball_pos = player_pos + glm::vec2(PLAYER_SIZE.x / 2.f - BALL_RADIUS,
                                           -BALL_RADIUS * 2.f);
    m_Ball        = std::make_shared<BallObject>(ball_pos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTextureRef("face"));

    // Particles
    m_Particles = std::make_shared<ParticleGenerator>(ResourceManager::GetShader(particle_shader),
                                                      ResourceManager::GetTexture("arch"),
                                                      500);


    // post processing
    m_Effects = std::shared_ptr<PostProcessor>(new PostProcessor(ResourceManager::GetShader("post_processing"),
                                                                 m_Width, m_Height));

    // text render
    m_TextRender = std::shared_ptr<TextRender>(new TextRender(m_Width, m_Height));
    m_TextRender->Load("../res/font/Cascadia.ttf", 24);


    GL_CHECK_HEALTH();
}

void Game::Update(float dt)
{
    // ball with collisions
    m_Ball->Move(dt, m_Width);
    DoCollisions();

    // particles
    m_Particles->Update(dt, *m_Ball, 2, glm::vec2(m_Ball->m_Radius / 2.f));

    // powerups
    UpdatePowerups(dt);

    // shader relates
    if (ShakeTime > 0.f) {
        ShakeTime -= dt;
        if (ShakeTime < 0.f)
            m_Effects->bShake = false;
    }

    // game logic
    if (m_Ball->m_Position.y >= m_Height) {
        --m_PlayerLives;

        if (m_PlayerLives == 0) {
            ResetLevel();
            m_State = GameState::GAME_MENU;
        }

        ResetPlayer();
    }

    if (m_State == GameState::GAME_ACTIVE) {
        if (m_Levels[m_LevelIndex].IsCompleted()) {
            ResetLevel();
            ResetPlayer();
            m_Effects->bChaos = true;
            m_State           = GameState::GAME_WIN;
        }
    }
    if (m_State == GameState::GAME_MENU) {
        // if (this->m_keys)
    }
    if (m_State == GameState::GAME_WIN) {
    }
}

void Game::ProcessInput(float dt)
{
    if (this->m_State == GameState::GAME_ACTIVE)
    {
        float velocity = PLAYER_VELOCITY * dt;

        if (this->m_keys[GLFW_KEY_A]) {
            if (m_Player->m_Position.x >= 0.f) {
                m_Player->m_Position.x -= velocity;

                // give a start force to ball
                if (m_Ball->bStuck) {
                    m_Ball->m_Position.x -= velocity;
                }
            }
        }
        if (this->m_keys[GLFW_KEY_D]) {
            if (m_Player->m_Position.x <= m_Width - m_Player->m_Size.x)
            {
                m_Player->m_Position.x += velocity;

                // give a start force to ball
                if (m_Ball->bStuck) {
                    m_Ball->m_Position.x += velocity;
                }
            }
        }

        if (m_keys[GLFW_KEY_SPACE]) {
            m_Ball->bStuck = false;
        }
    }

    auto begin_press = [this](int key) -> bool { return m_keys[key] && !m_KeyProcessed[key]; };
    auto press_down  = [this](int key) { m_KeyProcessed[key] = true; };

    if (m_State == GameState::GAME_MENU) {

        if (begin_press(GLFW_KEY_ENTER))
        {
            m_State = GameState::GAME_ACTIVE;
            press_down(GLFW_KEY_ENTER);
        }
        if (begin_press(GLFW_KEY_W)) {
            this->m_LevelIndex = (m_LevelIndex + 1) % m_Levels.size();
            press_down(GLFW_KEY_W);
        }
        if (begin_press(GLFW_KEY_S)) {
            if (this->m_LevelIndex > 0) {
                --m_LevelIndex;
            }
            else {
                m_LevelIndex = m_Levels.size() - 1;
            }
            press_down(GLFW_KEY_S);
        }
    }
    if (m_State == GameState::GAME_WIN) {
        if (m_keys[GLFW_KEY_ENTER]) {
            press_down(GLFW_KEY_ENTER);
            m_Effects->bChaos = false;
            m_State           = GameState::GAME_MENU;
        }
    }
}

void Game::Render()
{
    if (m_State == GameState::GAME_ACTIVE || m_State == GameState::GAME_MENU)
    {
        m_Effects->BeginRender();
        {
            m_SpriteRnder.DrawSprite(ResourceManager::GetTextureRef("background"),
                                     glm::vec2(0.f, 0.f),
                                     glm::vec2(m_Width, m_Height),
                                     0.f);
            m_Levels[m_LevelIndex].Draw(m_SpriteRnder);
            m_Player->Draw(m_SpriteRnder);
            // render the particles before the ball, because it use ONE as blend
            for (PowerUp &pu : m_PowerUps) {
                if (!pu.m_IsDestroyed) {
                    pu.Draw(m_SpriteRnder);
                }
            }
            m_Particles->Draw();
            m_Ball->Draw(m_SpriteRnder);
        }
        m_Effects->EndRender();
        m_Effects->Render(glfwGetTime());

        // Draw text
        m_TextRender->RenderText(fmt::format("Lives: {}", m_PlayerLives), 5.f, 5.f, 1.f);
    }

    if (m_State == GameState::GAME_MENU) {
        m_Effects->bChaos = true;
        m_TextRender->RenderText("Press Enter to start", m_Width / 3.f, m_Height / 2.f, 1.f);
        m_TextRender->RenderText("Press W/S to selct level", m_Width / 3.f, m_Height / 2.f + 30.f, 0.75f);
        m_TextRender->RenderText(fmt::format("Current Level: {}", m_LevelIndex + 1), m_Width / 3.f, m_Height / 2.f + 50.f, 0.75f);
    }

    if (m_State == GameState::GAME_WIN) {
        m_TextRender->RenderText("You WON!!!", m_Width / 2.5f, m_Height / 2.f - 30.f, 1.2f, {0, 1, 0});
        m_TextRender->RenderText("Press Enter/ESC to retry/quit", m_Width / 2.5f, m_Height / 2.f + 50.f, 1.f, {1, 0, 0});
    }



    debugDraw();
}

void Game::DoCollisions()
{
    // Ball with bricks
    for (auto &brick : m_Levels[m_LevelIndex].Bricks) {

        if (!brick.m_IsDestroyed)
        {
            auto &&result = CollisionCheck_BallWithRect(*m_Ball, brick);
            if (std::get<bool>(result)) {
                this->onCollied_BallWithBrick(m_Ball.get(), &brick, &result);
            }
        }
    }

    // Ball with player paddle
    auto &&result = CollisionCheck_BallWithRect(*m_Ball, *m_Player);
    if (std::get<bool>(result))
    {
        this->onCollied_BallWithPaddle(m_Ball.get(), m_Player.get(), &result);
    }


    //  Powerups that generated with player paddle
    for (auto &power_up : m_PowerUps)
    {
        if (!power_up.m_IsDestroyed)
        {
            if (power_up.m_Position.y >= m_Height) {
                power_up.m_IsDestroyed = true;
                continue;
            }
            if (CollisionCheck_2Rect_AABB(*m_Player, power_up))
            {
                this->onCollied_PaddleWithPowerup(m_Player.get(), &power_up, nullptr);
            }
        }
    }
}

void Game::ResetLevel()
{
    // I have preload this, will cause performance issue?
    m_Levels[m_LevelIndex].Reset(m_Width, m_Height / 2.f);

    m_PlayerLives = 3;
}

void Game::ResetPlayer()
{
    m_Player->m_Size     = PLAYER_SIZE;
    m_Player->m_Position = glm::vec2(m_Width / 2.f - m_Player->m_Size.x / 2.f,
                                     m_Height - PLAYER_SIZE.y);
    m_Ball->Reset(m_Player->m_Position + glm::vec2(PLAYER_SIZE.x / 2.f - BALL_RADIUS, -(BALL_RADIUS * 2.f)),
                  INITIAL_BALL_VELOCITY);
}



void Game::SpawPowerUps(GameObject &block)
{

    auto get_texture_name = [](auto name) { return "powerup_" + name; };

    static const auto
        tex_speed             = ResourceManager::GetTexture(get_texture_name(power_up::speed)),
        tex_stick             = ResourceManager::GetTexture(get_texture_name(power_up::sticky)),
        tex_pass_through      = ResourceManager::GetTexture(get_texture_name(power_up::pass_through)),
        tex_pad_size_increase = ResourceManager::GetTexture(get_texture_name(power_up::pad_increase)),
        tex_confuse           = ResourceManager::GetTexture(get_texture_name(power_up::confuse)),
        tex_chaos             = ResourceManager::GetTexture(get_texture_name(power_up::chaos));


    if (ShouldSpawn(100)) {
        m_PowerUps.push_back(PowerUp(power_up::speed, glm::vec3(0.5f, 0.5f, 1.0f),
                                     15.f, block.m_Position, tex_speed));
    };
    if (ShouldSpawn(75)) {
        m_PowerUps.push_back(PowerUp(power_up::sticky, glm::vec3(1.0f, 0.5f, 1.0f),
                                     20.f, block.m_Position, tex_stick));
    };
    if (ShouldSpawn(75)) {
        m_PowerUps.push_back(PowerUp(power_up::pass_through, glm::vec3(0.5f, 1.0f, 0.5f),
                                     10.f, block.m_Position, tex_pass_through));
    };
    if (ShouldSpawn(75)) {
        m_PowerUps.push_back(PowerUp(power_up::pad_increase, glm::vec3(1.0f, 0.6f, 0.4f),
                                     30.f, block.m_Position, tex_pad_size_increase));
    }
    if (ShouldSpawn(15)) {
        m_PowerUps.push_back(PowerUp(power_up::confuse, glm::vec3(1, 0.3, 0.3),
                                     5.f, block.m_Position, tex_confuse));
    };
    if (ShouldSpawn(15)) {
        m_PowerUps.push_back(PowerUp(power_up::chaos, glm::vec3(0.9, 0.25, 0.25),
                                     5.f, block.m_Position, tex_chaos));
    }
}

void Game::UpdatePowerups(float dt)
{
    for (PowerUp &power_up : m_PowerUps)
    {

        power_up.m_Position += power_up.m_Velocity * dt;


        if (!power_up.bActivated)
            continue;

        power_up.Duration -= dt;

        const auto type = power_up.Type;

        if (power_up.Duration < 0.f)
        {
            power_up.bActivated = false;

            auto one_more_powerup_exist = [&](auto &source_type, auto &expect_type) {
                return (source_type == expect_type) && isOtherPoerUpActive(expect_type);
            };

            if (type == power_up::sticky) {
                if (!isOtherPoerUpActive(type)) {
                    m_Ball->bSticky   = false;
                    m_Player->m_Color = glm::vec3(1.f);
                }
            }
            else if (type == power_up::pass_through) {
                if (!isOtherPoerUpActive(type)) {
                    m_Ball->bPassThrough = false;
                    m_Player->m_Color    = glm::vec3(1.f);
                }
            }
            else if (type == power_up::confuse) {
                if (!isOtherPoerUpActive(type)) {
                    m_Effects->bConfuse = false;
                }
            }
            else if (type == power_up::chaos) {
                if (!isOtherPoerUpActive(type)) {
                    LOG_DEBUG("{} | Duration < 0, but one more powerup activated", power_up::chaos);
                    m_Effects->bChaos = false;
                }
            }
        }
    }

    m_PowerUps.erase(std::remove_if(m_PowerUps.begin(), m_PowerUps.end(), [](const PowerUp &power_up) {
                         return power_up.m_IsDestroyed && !power_up.bActivated;
                     }),
                     m_PowerUps.end());
}

void Game::debugDraw()
{
}

void Game::ActivatePowerups(PowerUp &power_up)
{
    if (power_up.Type == power_up::speed)
    {
        m_Ball->m_Velocity *= 1.2;
    }
    else if (power_up.Type == power_up::sticky)
    {
        m_Ball->bSticky   = true;
        m_Player->m_Color = glm::vec3(1.0f, 0.5f, 1.0f);
    }
    else if (power_up.Type == power_up::pass_through)
    {
        m_Ball->bPassThrough = true;
        m_Ball->m_Color      = glm::vec3(1.0f, 0.5f, 0.5f);
    }
    else if (power_up.Type == power_up::pad_increase)
    {
        m_Player->m_Size.x += 50;
    }
    else if (power_up.Type == power_up::confuse)
    {
        if (!m_Effects->bChaos)
            m_Effects->bConfuse = true; // only activate if chaos wasn't already active
    }
    else if (power_up.Type == power_up::chaos)
    {
        if (!m_Effects->bConfuse)
            m_Effects->bChaos = true;
    }
}

void Game::initCallback()
{
    // using namespace std::placeholders;
    // Cannot bind on different function sig
    // this->onCollied_BallWithPaddle = std::bind(&Game::onCollied_BallWithPaddleHandler, this, _1, _2, _3);
    this->onCollied_BallWithPaddle = [&](GameObject *ball, GameObject *paddle, ColliedResult *result) {
        onCollied_BallWithPaddle_Handler(static_cast<BallObject *>(ball), paddle, result);
    };
    this->onCollied_BallWithBrick = [&](GameObject *ball, GameObject *paddle, ColliedResult *result) {
        onCollied_BallWithBrick_Handler(static_cast<BallObject *>(ball), paddle, result);
    };
    this->onCollied_PaddleWithPowerup = [&](GameObject *ball, GameObject *paddle, ColliedResult *result) {
        onCollied_PaddleWithPowerup_Handler(ball, paddle, result);
    };
}

void Game::initShaders()
{
    // loas shader
    ResourceManager::LoadShader("../res/shader/a.vert", "../res/shader/a.frag", nullptr, sprite_shader);
    ResourceManager::LoadShader("../res/shader/particle.vert", "../res/shader/particle.frag", nullptr, particle_shader);
    ResourceManager::LoadShader("../res/shader/post_processing.vert", "../res/shader/post_processing.frag", nullptr, "post_processing");
    ResourceManager::LoadShader("../res/shader/text_2d.vert", "../res/shader/text_2d.frag", nullptr, "text");

    // view projection to resolute the [-1,1]
    glm::mat4 projection = glm::ortho(0.0f, (float)this->m_Width,
                                      (float)this->m_Height, 0.0f,
                                      1.0f, -1.0f);

    // NOTICE: Must use this program first
    ResourceManager::GetShader(sprite_shader).Use();
    ResourceManager::GetShader(sprite_shader).SetMatrix4("projection", projection);
    // reset image
    ResourceManager::GetShader(sprite_shader).SetInteger("image", 0);
    ResourceManager::GetShader(sprite_shader).SetInteger("hasTexture", 1);

    m_SpriteRnder = SpriteRender(ResourceManager::GetShader(sprite_shader));

    ResourceManager::GetShader(particle_shader).Use().SetMatrix4("projection", projection);
    ResourceManager::GetShader(particle_shader).SetInteger("sprite", 0);
}

void Game::initTextures()
{ // load texture
    {
#if 1
        auto GetFileNameWithoutExtension =
            [](const std::string &path) {
                size_t slash_pos = path.find_last_of("/\\");
                size_t dot_pos   = path.find_last_of(".");

                // DEBUG("{}, {}, {}", path, slash_pos, dot_pos);
                auto filename = path.substr(slash_pos + 1, dot_pos - slash_pos - 1);
                // LOG("{}", filename);
                return filename;
            };

        std::vector<std::string> support_suffixs = {".jpg", ".png", ".bmp"};
        LOG_WARN("Load texture with '{}', '{}', '{}'", support_suffixs[0], support_suffixs[1], support_suffixs[2]);

        for (const auto texture : std::filesystem::directory_iterator("../res/texture/"))
        {
            const auto &file_path = texture.path().string();
            // LOG_DEBUG("Trying to load texture from '{}'", file_path);

            for (auto &suffx : support_suffixs)
            {
                if (file_path.ends_with(suffx))
                {
                    LOG_TRACE("Trying to load texture from '{}'", file_path);
                    const std::string texture_name = GetFileNameWithoutExtension(std::ref(file_path));

                    ResourceManager::LoadTexture(file_path.c_str(), texture_name);
                    break;
                }
            }
        }
#else
        ResourceManager::LoadTexture("../res/textures/arch.png", "arch");
        ResourceManager::LoadTexture("../res/textures/brick.bmp", "brick");
        ResourceManager::LoadTexture("../res/textures/block_solid.png", "block_solid");
        ResourceManager::LoadTexture("../res/textures/block.png", "block");
        ResourceManager::LoadTexture("../res/textures/background.jpg", "background");
#endif
    }
}

void Game::initLevels()
{
    // load Levels
    {
        size_t level_count = 4;
        m_Levels           = std::vector<GameLevel>(level_count);
        const char *level_names[] =
            {
                "0_standard",
                "1_a_few_small_gaps",
                "2_space_invader",
                "3_bounce_galore",
            };
        for (int i = 0; i < level_count; ++i)
        {
            m_Levels[i].Load(fmt::format("../res/level/{}", level_names[i]).c_str(),
                             m_Width,
                             m_Height / 2);
        }
        m_LevelIndex = 0;
    }
}

void Game::initAudios()
{
    auto AudioPlayer = SDL_Player::Get();

    auto GetFileNameWithoutExtension =
        [](const std::string &path) {
            size_t slash_pos = path.find_last_of("/\\");
            size_t dot_pos   = path.find_last_of(".");

            auto filename = path.substr(slash_pos + 1, dot_pos - slash_pos - 1);
            return filename;
        };

    std::vector<std::string> support_suffixs = {".mp3", ".wav"};
    LOG_WARN("Load audio that suffix with '{}', '{}'", support_suffixs[0], support_suffixs[1]);

    for (const auto audio : std::filesystem::directory_iterator("../res/audio/"))
    {
        const auto &file_path = audio.path().string();

        for (auto &suffx : support_suffixs)
        {
            if (file_path.ends_with(suffx))
            {
                LOG_TRACE("Trying to load audio from '{}'", file_path);
                const std::string audio_name = GetFileNameWithoutExtension(std::ref(file_path));

                AudioPlayer->LoadWave(file_path.c_str(), audio_name);
                break;
            }
        }
    }

    // play the backgound music
    AudioPlayer->Play("breakout", 128 / 3, true);
    // AudioPlayer->bChangingDistance = true;
    AudioPlayer->bChaningLocation = true;
    AudioPlayer->bPanning         = true;
}

void Game::onCollied_BallWithPaddle_Handler(BallObject *ball, GameObject *paddle, ColliedResult *result)
{
    if (!ball)
        return;

    // when not stuck with player
    if (!ball->bStuck)
        QuickPlaySoundEffect("pong");

    auto &[_, dir, diff] = *result;
    if (!ball->bStuck) {

        ball->bStuck = ball->bSticky;

        // if collided, we only need to caculate the hit point, and revert the dir vec
        float center_board = m_Player->m_Position.x + m_Player->m_Size.x / 2.f;
        float distance     = (ball->m_Position.x + ball->m_Radius) - center_board;
        float percentage   = distance / (m_Player->m_Size.x / 2.f);

        float strength     = 2.f;
        auto  old_velocity = ball->m_Velocity;
        ball->m_Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;

        // make the ball not stick when just in the paddle
        ball->m_Velocity.y = -1.f * abs(ball->m_Velocity.y);

        // reverse
        ball->m_Velocity = glm::normalize(ball->m_Velocity) * glm::length(old_velocity);
    }
}

void Game::onCollied_BallWithBrick_Handler(BallObject *ball, GameObject *box, ColliedResult *result)
{
    if (!ball || !box)
        return;

    if (!ball->bPassThrough)
        QuickPlaySoundEffect(box->m_IsSolid ? "solid" : "ping");

    // solid  will do shake
    if (box->m_IsSolid) {
        ShakeTime         = 0.05f;
        m_Effects->bShake = true;
    }
    // no-solid will destroy it
    else {
        box->m_IsDestroyed = true;
        this->SpawPowerUps(*box);
    }

    // check if in pass-throuth status, can pass the non-solid brick
    if (ball->bPassThrough && !box->m_IsSolid)
        return;


    auto &[_, dir_to_ball, diff] = *result;

    if (dir_to_ball == LEFT || dir_to_ball == RIGHT) {
        ball->m_Velocity.x = -ball->m_Velocity.x;
        float penetration  = ball->m_Radius - std::abs(diff.x);
        // reloacte
        if (dir_to_ball == LEFT)
            ball->m_Position.x += penetration;
        else
            ball->m_Position.x -= penetration;
    }
    else {
        ball->m_Velocity.y = -ball->m_Velocity.y;
        float penetration  = ball->m_Radius - std::abs(diff.y);
        // reloacte on the
        if (dir_to_ball == DOWN)
            ball->m_Position.y += penetration;
        else
            ball->m_Position.y -= penetration;
    }
}

void Game::onCollied_PaddleWithPowerup_Handler(GameObject *paddle, GameObject *power_up, ColliedResult *result)
{
    auto the_power_up = static_cast<PowerUp *>(power_up);
    if (!paddle || !the_power_up)
        return;

    QuickPlaySoundEffect("powerup");

    ActivatePowerups(*the_power_up);
    the_power_up->m_IsDestroyed = true;
    the_power_up->bActivated    = true;
}


bool Game::isOtherPoerUpActive(const std::string &type_name)
{
    for (const auto &pu : m_PowerUps) {
        if (pu.bActivated && pu.Type == type_name) {
            return true;
        }
    }
    return false;
}

// -------------------------



static bool CollisionCheck_2Rect_AABB(const GameObject &A, const GameObject &B)
{
    bool collisionX = A.m_Position.x + A.m_Size.x >= B.m_Position.x &&
                      B.m_Position.x + B.m_Size.x >= A.m_Position.x;

    bool collisionY = A.m_Position.y + A.m_Size.y >= B.m_Position.y &&
                      B.m_Position.y + B.m_Size.y >= A.m_Position.y;

    return collisionX && collisionY;
}

// from dot product get the projection of Vec to 4 diretions to get the longest vec,
// which is the best_match direction
static Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[4] = {
        { 0.f,  1.f},
        { 1.f,  0.f},
        { 0.f, -1.f},
        {-1.f,  0.f},
    };
    float        max        = 0.f;
    unsigned int best_match = -1;

    for (int i = 0; i < 4; ++i) {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max) {
            max        = dot_product;
            best_match = i;
        }
    }
    return static_cast<Direction>(best_match);
}

static std::tuple<bool, Direction, glm::vec2> CollisionCheck_BallWithRect(const BallObject &A, const GameObject &B)
{
    using glm::vec2;

    // get the center (from topleft or circle and rectangle)
    vec2 ball_center(A.m_Position + A.m_Radius);

    vec2       rect_center{};
    const vec2 rect_half_extens(B.m_Size.x / 2.f, B.m_Size.y / 2.f);
    rect_center = B.m_Position + rect_half_extens;

    vec2 diff = ball_center - rect_center;

    // it will let the x or y of vec from circle to rectangle
    // which beyond the half rectangle's L or W
    // clmaped to point to  the edge of rectangle (from rectangle center)
    // so it is the cloest point from rectangle to the circle
    vec2 clamped       = clamp(diff, -rect_half_extens, rect_half_extens);
    vec2 closeet_point = rect_center + clamped;

    // retrieve vec between center circle and closest point AABB
    //  and check if  len < radius
    diff = closeet_point - ball_center;

    if (glm::length(diff) <= A.m_Radius)
        return {true, VectorDirection(diff), diff};
    else
        return {
            false, UP, {0.f, 0.f}
        };
}

static bool ShouldSpawn(unsigned int change)
{
    return rand() % change == 0;
}

static void QuickPlaySoundEffect(const char *sound_name)
{
    auto player = SDL_Player::Get();
    player->Play(sound_name);
}
