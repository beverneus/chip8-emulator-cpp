#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <SDL3/SDL.h>

class Beep {
    SDL_AudioStream *stream;
    SDL_AudioSpec spec;
    int current_sin_sample;
    public:
        Beep();
        void play();
};

#endif