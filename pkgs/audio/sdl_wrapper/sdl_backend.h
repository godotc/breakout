
#include "log.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include <property.h>



static std::vector<bool> &GetChannelStates()
{
    using bOccurpying = bool;
    static std::vector<bOccurpying> ChannelStates(Mix_AllocateChannels(-1), false);
    return ChannelStates;
}

void SDLCALL channel_complete_callback(int chan);


class SDL_Chunk
{

  public:
    SDL_Chunk() {}


    SDL_Chunk(const char *path) { LoadWave(path); }

    void LoadWave(const char *path, bool bReverseSample = false);

    void Clear()
    {
        Mix_FreeChunk(wave);
        wave = nullptr;
    }


    ~SDL_Chunk()
    {
    }

    // operator Mix_Chunk *() const
    // {
    //     return wave;
    // }

    void UpdateSampleDirection(bool bReversed)
    {
        if (bReversed != this->bReversed) {
            flip_sample();
        }
    }

  private:
    void flip_sample();

  public:
    Mix_Chunk *wave      = {nullptr};
    bool       bValid    = {false};
    bool       bReversed = {false};
};


class SDL_Player
{

  public:
    // static std::shared_ptr<SDL_Player> &Get()
    static SDL_Player *Get()
    {
        static SDL_Player *player = new SDL_Player;
        return player;
    }

    SDL_Player();

    ~SDL_Player()
    {
        LOG_WARN("SDL_Player is destructing!");
        CleanUp();
    }

    void LoadWave(const char *path, std::string sound_name, bool bReverseSample = false);
    void Play(const std::string &sound_name, int volume = 128, bool bLoop = false, bool bReversed = false);

    SDL_Chunk *const GetWavePtr(std::string sound_name);

    // Pause of a channel will be seen as playing
    bool IsPlaying();

    void PostEffects(int chan)
    {
        // TODO: Post effects for every single channel
        if (GetbPanning())
            do_panning_update();
        if (GetbChangingDistance())
            do_distance_update();
        if (GetbChangingDistance())
            do_position_update();
    }

    void ShowInfo();



  private:
    void flip_sample(Mix_Chunk *wave);

    void do_position_update();
    void do_distance_update();
    void do_panning_update();


    /**
     * @brief  Auto find and occur a valid channel
     *
     * @return int  the chnnael id, -1 if not found
     */
    int consume_one_channel();


    void CleanUp(/*int exit_code*/)
    {

        for (auto &&[_, chunk] : m_Chunks) {
            chunk.Clear();
        }

        if (bAudioOpen) {
            Mix_CloseAudio();
            bAudioOpen = false;
        }
        SDL_Quit();
        PostPrecessThread.join();
        // exit(exit_code);
    }

  private:
    void report_decoders();
    void reporte_version();

  public:

    bool bAudioOpen = false;

    // Now We use these as glboal effect
    // sround
    PROPERTY(bool, bPanning, false);
    // far to near...
    PROPERTY(bool, bChangingDistance, false);
    // left to right
    PROPERTY(bool, bChaningLocation, false);


  private:
    // max is 128
    std::unordered_map<std::string, SDL_Chunk> m_Chunks;

    std::thread PostPrecessThread;
};