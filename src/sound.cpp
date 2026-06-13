#pragma once
#include <raylib.h>
#include <iostream>


struct SoundPool {
    Sound* voices;
    int next = 0;
    int voiceCount = 0;
    void Load(const char* path, int _voiceCount) {
        voiceCount = _voiceCount;
        voices = (Sound*)malloc(voiceCount*sizeof(Sound));
        for(int i=0; i<voiceCount; i++) voices[i] = LoadSound(path);
        next = 0;
    }
    void Unload() {
        for(int i=0; i<voiceCount; i++) UnloadSound(voices[i]);
        delete voices;
        voiceCount = 0;
    }
    void Play(float volume) {
        if(!voiceCount) return;
        int idx = -1;
        for(int i = 0;i < voiceCount;i++)
            if (!IsSoundPlaying(voices[i])) {
                idx = i;
                break;
            }
        if(idx < 0) idx = next;
        float pitch = GetRandomValue(95, 105) / 100.0f; // 0.95 -> 1.05
        SetSoundPitch(voices[idx], pitch);
        SetSoundVolume(voices[idx], volume);
        PlaySound(voices[idx]);
        next = (next+1)%voiceCount;
    }
};

namespace sound {
    static SoundPool boom;
    static SoundPool explosion;
    static SoundPool gun;
    static SoundPool damage;
    static SoundPool dead;
    static SoundPool ohm;
    static SoundPool moo;
    static SoundPool noise;
    static SoundPool notify;
    static Sound select;
    static Sound select2;
    static Music bg;
}