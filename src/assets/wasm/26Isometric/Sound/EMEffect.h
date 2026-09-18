#pragma once

#include <vector>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#include <emscripten/webaudio.h>

#include "ISoundEffect.h"
#include "SoftwareMixer.h"
#include "Cache.h"

class EMEffect : public ISoundEffect {

    struct CacheEntry {      
        CacheEntry(const std::string& file);
        ~CacheEntry();

        CacheEntry(const CacheEntry&) = delete;
        CacheEntry& operator=(const CacheEntry&) = delete;

        CacheEntry(CacheEntry&& other) noexcept;
        CacheEntry& operator=(CacheEntry&& other) noexcept;

        std::vector<float> m_samples;
        uint32_t m_totalSamples;
    };

public:

    EMEffect();
    ~EMEffect();

    void init() override;
    void play(const std::string& file) override;
    void resume();
    void setVolume(float volume);

    SoftwareMixer& getMixer();
   

private:

    EM_BOOL audioCallback(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params);
    SoftwareMixer m_softwareMixer;

    EMSCRIPTEN_WEBAUDIO_T m_audioContext;
    static EM_BOOL EMAudioCallback(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData);
    static void OnAudioNodeCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData);
    static void OnAudioThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData);
    static EM_BOOL OnCanvasClick(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
    static CacheLRU<std::string, EMEffect::CacheEntry> Cache;
};