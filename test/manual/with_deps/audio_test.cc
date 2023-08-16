
#include "miniaudio_wrapper/wraper.h"
#include <assert.h>
#include <stdio.h>



int main()
{
    ma_result result;
    ma_engine engine;

    result = ma_engine_init(nullptr, &engine);
    assert(result == MA_SUCCESS);

    ma_engine_play_sound(&engine, "../res/audio/breakout.mp3", nullptr);

    getchar();

    ma_engine_uninit(&engine);

    return 0;
}