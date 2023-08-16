#include "game_level.h"

#include "../obj/game_object.h"
#include "../resource_manager/resource_manager.h"
#include "log.h"
#include <cmath>
#include <fstream>
#include <glm/fwd.hpp>
#include <math.h>
#include <sstream>
#include <string>



void GameLevel::Load(const char *file, uint level_width, uint level_height)
{
    using std::vector;

    std::ifstream ifs(file);
    if (ifs.fail() || !ifs.is_open()) {
        LOG_ERROR("load level file {} failed", file);
        return;
    }

    this->Bricks.clear();

    uint        tile_code;
    std::string line;

    vector<vector<uint>> tile_data;

    while (std::getline(ifs, line))
    {
        std::vector<uint>  row;
        std::istringstream ss(line);

        // from string(raw hex) to unsigned int
        // and when reach a space will turn to next loop
        while (ss >> tile_code) {
            row.push_back(tile_code);
        }

        tile_data.push_back(row);
    }



    if (tile_data.size() > 0) {
        m_MapTileData = tile_data;
        init(tile_data, level_width, level_height);
    }
}

void GameLevel::Draw(SpriteRender &render)
{
    for (auto &brick : Bricks) {
        if (!brick.m_IsDestroyed) {
            brick.Draw(render);
            // LOG_DEBUG("brick's properties: size: {}, {},  location: {}, {}", brick.Size.x, brick.Size.y, brick.Position.x, brick.Position.y);
        }
    }
}

bool GameLevel::IsCompleted()
{
    for (auto &tile : this->Bricks) {
        if (!tile.m_IsSolid && !tile.m_IsDestroyed) {
            return false;
        }
    }
    return true;
}

void GameLevel::init(std::vector<std::vector<uint>> tile_data, uint level_width, uint level_height)
{
    using glm::vec2, glm::vec3;
    Bricks.clear();


    auto sizeX = level_width / (float)tile_data[0].size();
    auto sizeY = level_height / (float)tile_data.size();
    LOG_DEBUG("Every brick size x & Y: {} | {}", sizeX, sizeY);


    vec2 size(sizeX, sizeY);

    int posY = 0;
    int posX = 0;
    for (auto &row : tile_data)
    {
        posX = 0;
        for (auto ui : row)
        {
            vec3 color = vec3(1.f);
            uint value = ui;
            vec2 pos(posX, posY);
            bool bSolid = false;

            Texture2D texuture = ResourceManager::GetTexture("arch");

            if (value != 0) {
                if (value == 1) {
                    texuture = ResourceManager::GetTexture("block_solid");
                    color    = glm::vec3(0.8f, 0.8f, 0.7f);
                    bSolid   = true;
                }
                else {
                    texuture = ResourceManager::GetTexture("block");
                    if (value == 2) {
                        color = glm::vec3(0.2f, 0.6f, 1.0f);
                    }
                    else if (value == 3) {
                        color = glm::vec3(0.0f, 0.7f, 0.0f);
                    }
                    else if (value == 4) {
                        color = glm::vec3(0.8f, 0.8f, 0.4f);
                    }
                    else if (value == 5) {
                        color = glm::vec3(1.0f, 0.5f, 0.0f);
                    }
                }
                GameObject obj(pos, size, texuture, color, vec2(0, 0));
                obj.m_IsSolid = bSolid;
                this->Bricks.push_back(obj);
            }

            posX += sizeX;
        }
        posY += sizeY;
    }

    LOG_DEBUG("Finnal pos: {} | {}", posX, posY);
    // exit(1);
}

void GameLevel::Reset(uint level_width, uint level_height)
{
    init(m_MapTileData, level_width, level_height);
}