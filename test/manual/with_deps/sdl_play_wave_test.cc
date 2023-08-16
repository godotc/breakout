#include "log.h"
#include <audio/sdl_wrapper/sdl_backend.h>
#include <stdio.h>
#include <utility>


int main()
{
    auto Player = SDL_Player::Get();

    Player->LoadWave("../res/audio/breakout.mp3", "breakout");
    auto wave          = Player->GetWavePtr("breakout");
    // wave->audio_volume = 10;
    LOG_DEBUG("Complete loading");
    Player->Play("breakout");


    Player->LoadWave("../res/audio/solid.wav", "solid");
    bool        b = true;
    std::thread t([&] {
        while (b)
        {
            static int i = 0;
            if (++i % 12345) {
                Player->Play("solid");
            }
        };
    });

    getchar();
    b = false;
    t.join();


    return 0;
}
