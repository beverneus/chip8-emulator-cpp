#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <math.h>
#include <SDL3/SDL.h>

class Beep {
    SDL_AudioStream *stream;
    SDL_AudioSpec spec;
    char *wav_path;
    uint64_t current_sin_sample;
    public:
        Beep();
        void play();
};

#endif