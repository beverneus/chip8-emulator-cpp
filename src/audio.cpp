#include "audio.hpp"

Beep::Beep() : 
    stream(nullptr),
    current_sin_sample(0)
    {
    if(!SDL_Init(SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    }

    spec.channels = 1;
    spec.format = SDL_AUDIO_F32;
    spec.freq = 48000;

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!stream) (SDL_Log("Couldn't create audio stream: %s", SDL_GetError()));

    SDL_ResumeAudioStreamDevice(stream);
}

void Beep::play() {
    static constexpr int freq = 800;
    float samples[static_cast<int>(static_cast<float>(spec.freq) / 60)];
    for (size_t i = 0; i < SDL_arraysize(samples); i++) {
        double phase = static_cast<double>(current_sin_sample) * freq / spec.freq;
        samples[i] = 0.5f * SDL_sinf(phase * 2 * SDL_PI_F);
        current_sin_sample++;
    }

    current_sin_sample %= spec.freq;

    SDL_PutAudioStreamData(stream, samples, sizeof(samples));
}
