#pragma once

#include <vector>
#include <emscripten/webaudio.h>

#include "IAudioOutput.h"
#include "AudioRingBuffer.h"
#include "SoftwareMixer.h"

class EMPlayer : public IAudioOutput{
public:
    EMPlayer();
    ~EMPlayer();

    bool init() override;
    void enqueueData(const std::vector<float>& pcmData) override;
    void pause() override;
    void resume() override;

    void setVolume(float volume) override;
    float getVolume() override;

private:

    void flush() override;
    EM_BOOL audioCallback(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params);

    AudioRingBuffer m_ringBuffer;
    std::vector<float> m_accumulator;
    SoftwareMixer m_softwareMixer;
    bool m_isPlaying;

    EMSCRIPTEN_WEBAUDIO_T m_audioContext;
    static EM_BOOL EMAudioCallback(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData);
    static void OnAudioNodeCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData);
    static void OnAudioThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData);
};