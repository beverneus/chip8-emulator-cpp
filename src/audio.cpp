#include "audio.hpp"

Beep::Beep() :
 stream(nullptr), wav_data(nullptr), wav_data_len(0), wav_path(nullptr) {
    if(!SDL_Init(SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    }

    //Load WAV
    SDL_asprintf(&wav_path, "%s../assets/audio/beep.wav", SDL_GetBasePath());
    if (!SDL_LoadWAV(wav_path, &spec, &wav_data, &wav_data_len)) {
        SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
    }
    SDL_free(wav_path);

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!stream) (SDL_Log("Couldn't create audio stream: %s", SDL_GetError()));

    SDL_ResumeAudioStreamDevice(stream);
}

void Beep::play() {
    SDL_ClearAudioStream(stream);
    SDL_PutAudioStreamData(stream, wav_data, wav_data_len);
}