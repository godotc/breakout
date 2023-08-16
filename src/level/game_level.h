#include "../obj/game_object.h"
#include "../render/sprite_render.h"
#include <cstdint>
#include <vector>


class GameLevel
{
    using uint = unsigned int;

  public:
    GameLevel() = default;

  public:
    void Load(const char *file, uint level_width, uint level_height);
    void Draw(SpriteRender &render);
    void Reset(uint w, uint half_h);

    bool IsCompleted();

  public:
    std::vector<GameObject>                Bricks;
    std::vector<std::vector<unsigned int>> m_MapTileData;

  private:
    void init(std::vector<std::vector<uint>> tile_data, uint level_width, uint level_height);
};
