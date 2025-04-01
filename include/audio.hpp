#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <SDL3/SDL.h>

class Beep {
    SDL_AudioStream *stream;
    uint8_t *wav_data;
    uint32_t wav_data_len;
    SDL_AudioSpec spec;
    char *wav_path;

    public:
        Beep();
        void play();
};

#endif