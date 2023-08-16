#include "sdl_backend.h"

#include <algorithm>
#include <m_log/log.h>
#include <optional>
#include <thread>

#include "SDL_log.h"
#include "SDL_mixer.h"
#include "SDL_stdinc.h"


void channel_complete_callback(int chan)
{
    Mix_Chunk *done_chunk = Mix_GetChunk(chan);
    LOG_DEBUG("{} is done,  Channel pointer is {} ", chan, (void *)done_chunk);
    GetChannelStates()[chan] = false;
}

void SDL_Chunk::flip_sample()
{
    Uint16 format;
    int    channels;
    Mix_QuerySpec(nullptr, &format, &channels);

    int incr = (format & 0xFF) * channels;

    Uint8 *start = wave->abuf;
    Uint8 *end   = start + wave->alen;

    auto iterate = [](auto &head, auto &tail) {
        auto tmp = *head;
        *head    = *tail;
        *tail    = tmp;
        ++head;
        --tail;
    };

    switch (incr)
    {
    case 8:
        for (int i = wave->alen / 2; i >= 0; --i) {
            iterate(start, end);
        }
        break;
    case 16:
    {
        Uint16 *head = reinterpret_cast<Uint16 *>(start);
        Uint16 *tail = reinterpret_cast<Uint16 *>(end);
        for (int i = wave->alen / 2; i >= 0; i -= 2) {
            iterate(head, tail);
        }
        break;
    }
    case 32:
    {
        Uint16 *head = reinterpret_cast<Uint16 *>(start);
        Uint16 *tail = reinterpret_cast<Uint16 *>(end);
        for (int i = wave->alen / 2; i >= 0; i -= 4) {
            iterate(head, tail);
        }
        break;
    }
    default:
        LOG_ERROR("Unhandleed format in sample flipping incr: {}", incr);
        return;
    }
    bReversed = !bReversed;
}

void SDL_Chunk::LoadWave(const char *path, bool bReverseSample /*= false*/)
{
    wave = Mix_LoadWAV(path);
    if (wave == nullptr) {
        LOG_ERROR("Faile to load {}", path);
        bValid = false;
        return;
    }
    if (bReverseSample) {
        flip_sample();
    }

    bValid = true;
}

// ------------------------------------ Begin SDL_Player

SDL_Player::SDL_Player()
{
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        SDL_Log("Failed to initialize SDL: %s\n", SDL_GetError());
    }

    // open audio device
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,
                      MIX_DEFAULT_FORMAT,
                      MIX_DEFAULT_CHANNELS,
                      4096) < 0) {
        SDL_Log("Open audio :%s\n", SDL_GetError());
        return;
    }

    ShowInfo();

    bAudioOpen = true;


    PostPrecessThread = std::thread([&] {
        while (bAudioOpen) {
            if (IsPlaying()) {
                PostEffects();
                SDL_Delay(1);
            }
        }
    });


    // callback when a wave end
    Mix_ChannelFinished(channel_complete_callback);
}

void SDL_Player::ShowInfo()
{
    int    audio_volume;
    int    audio_rate;
    int    audio_channels;
    Uint16 audio_format;

    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);

    SDL_Log("Open audio at %d Hz %d bit%s %s",
            audio_rate,
            (audio_format & 0xFF),
            (SDL_AUDIO_ISFLOAT(audio_format) ? " (float)" : ""),
            (audio_channels > 2)   ? "surround"
            : (audio_channels > 1) ? "stereo"
                                   : "mono");

    putchar('\n');

    reporte_version();
    report_decoders();
}

SDL_Chunk *const SDL_Player::GetWavePtr(std::string sound_name)
{
    if (auto chunk = m_Chunks.find(sound_name); chunk != m_Chunks.end()) {
        return &chunk->second;
    }

    LOG_ERROR("Not such sound {}", sound_name);
    return nullptr;
}
void SDL_Player::LoadWave(const char *path, std::string sound_name, bool bReverseSample /*= false*/)
{
    SDL_Chunk chunk(path);
    if (chunk.bValid) {
        m_Chunks[sound_name] = chunk;
    }
}

int SDL_Player::consume_one_channel()
{
    auto &ChannelStates = GetChannelStates();


    int free_channel = -1;

    for (int i = 0; i < ChannelStates.size(); ++i)
    {
        if (!ChannelStates[i]) {
            free_channel = i;
            break;
        }
    }
    if (free_channel == -1) {
        LOG_WARN("No free channel avaliable");
        LOG_DEBUG("Total channel size: {}", ChannelStates.size());
    }
    else {
        ChannelStates[free_channel] = true;
    }

    return free_channel;
}

void SDL_Player::Play(const std::string &sound_name, int volume /*=128*/, bool bLoop /*= false*/, bool bReversed /*= false*/)
{
    int free_channel = consume_one_channel();
    if (free_channel == -1) {
        LOG_WARN("Did not have valid channel for play {}", sound_name);
        return;
    }

    if (m_Chunks.find(sound_name) == m_Chunks.end())
    {
        LOG_WARN("Not such sound [{}]", sound_name);
        return;
    }

    SDL_Chunk &chunk = m_Chunks[sound_name];
    chunk.UpdateSampleDirection(bReversed);

    // TODO: change the state by `chunk`'s property
    Mix_Volume(free_channel, volume);

    LOG_DEBUG("Play sound [{}] ", sound_name);
    Mix_PlayChannel(free_channel, m_Chunks[sound_name].wave, bLoop);
}



void SDL_Player::do_position_update()
{
    static Sint16 distance             = 1;
    static Sint8  distance_incur       = 1;
    static Sint16 angle                = 0;
    static Sint8  angle_incur          = 1;
    static int    position_ok          = 1;
    static Uint32 next_position_update = 0;

    if ((position_ok) && (SDL_GetTicks() >= next_position_update))
    {
        position_ok = Mix_SetPosition(0, angle, (Uint8)distance);
        if (!position_ok) {
            LOG_WARN("Reason: [%s].\n", Mix_GetError());
        }

        if (angle == 0) {
            LOG_DEBUG("Due north; now rotating clockwise...\n");
            angle_incur = 1;
        }

        else if (angle == 360) {
            LOG_DEBUG("Due north; now rotating counter-clockwise...\n");
            angle_incur = -1;
        }

        distance += distance_incur;
        if (distance < 0) {
            distance       = 0;
            distance_incur = 3;
            LOG_DEBUG("Distance is very, very near. Stepping away by threes...\n");
        }
        else if (distance > 255) {
            distance       = 255;
            distance_incur = -3;
            LOG_DEBUG("Distance is very, very far. Stepping towards by threes...\n");
        }

        angle += angle_incur;
        next_position_update = SDL_GetTicks() + 30;
    }
}

void SDL_Player::do_distance_update(void)
{
    static Uint8  distance             = 1;
    static Sint8  distance_incr        = 1;
    static int    distance_ok          = 1;
    static Uint32 next_distance_update = 0;

    if (distance_ok && (SDL_GetTicks() >= next_distance_update))
    {
        distance_ok = Mix_SetDistance(0, distance);

        if (!distance_ok) {
            LOG_WARN(" Mix distance failed! | Reason: [{}].", Mix_GetError());
        }

        if (distance == 0) {
            LOG_DEBUG("Distance at nearest point.");
            distance_incr *= -1;
        }
        else if (distance == 255) {
            LOG_DEBUG("Distance at furthest point.");
            distance_incr *= -1;
        }

        distance += distance_incr;
        next_distance_update = SDL_GetTicks() + 15;
    }
}

void SDL_Player::do_panning_update()
{
    static int    panning_ok          = 1;
    static Uint32 next_panning_updtae = 0;
    static Uint8  left_vol            = 128,
                 right_vol            = 128;
    static Sint8 left_incr            = -1,
                 right_incr           = 1;

    if (panning_ok && (SDL_GetTicks() >= next_panning_updtae))
    {
        panning_ok = Mix_SetPanning(0, left_vol, right_vol);

        if (!panning_ok)
        {
            LOG_WARN("mix_panning failed! | {}", SDL_GetError());
        }

        if ((left_vol == 0) || (left_vol == 255)) {
            if (left_vol == 255) {
                LOG_DEBUG("All the way in the left speaker");
            }
            left_incr *= -1;
        }
        if ((right_vol == 255) || (right_vol == 0)) {
            if (right_vol == 255) {
                LOG_DEBUG("All the way in the right speaker.\n");
            }
            right_incr *= -1;
        }

        left_vol += left_incr;
        right_vol += right_incr;
        next_panning_updtae = SDL_GetTicks() + 10;
    }
}


static void output_versions(const char *libname, const SDL_version *compiled,
                            const SDL_version *linked)
{
    SDL_Log("This program was compiled against %s %d.%d.%d,\n"
            " and is dynamically linked to %d.%d.%d.\n",
            libname,
            compiled->major, compiled->minor, compiled->patch,
            linked->major, linked->minor, linked->patch);
}

bool SDL_Player::IsPlaying()
{
#define check_all 0
#if check_all
    int num_playing_chans = Mix_Playing(-1);
    return num_playing_chans > 0;
#else // check_any
    for (const int chan : GetChannelStates()) {
        if (Mix_Playing(chan))
            return true;
    }
    return false;
#endif
#undef check_all
}

void SDL_Player::reporte_version()
{
    SDL_version compiled, linked;

    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    output_versions("SDL_mixter", &compiled, &linked);
}

void SDL_Player::report_decoders()
{
    SDL_Log("Supported decoders...\n");

    int total = Mix_GetNumChunkDecoders();
    for (int i = 0; i < total; i++) {
        SDL_Log(" - chunk decoder: %s\n", Mix_GetChunkDecoder(i));
    }

    total = Mix_GetNumMusicDecoders();
    for (int i = 0; i < total; i++) {
        SDL_Log(" - music decoder: %s\n", Mix_GetMusicDecoder(i));
    }
}
