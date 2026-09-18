#include <string>
#include <algorithm>

#include "EMPlayer.h"

EM_BOOL EMPlayer::EMAudioCallback(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData){
    return static_cast<EMPlayer*>(userData)->audioCallback(numInputs, inputs, numOutputs, outputs, numParams, params);
}

EMPlayer::EMPlayer() {
    m_ringBuffer.init(44100 * 2 * sizeof(int16_t));
    m_isPlaying = true;
}

EMPlayer::~EMPlayer() {
   
}

void EMPlayer::OnAudioNodeCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData) {
    if (!success) {
        return;
    }
    
    int outputChannelCounts[1] = { 2 };
    EmscriptenAudioWorkletNodeCreateOptions options = {};
    options.numberOfInputs = 0;
    options.numberOfOutputs = 1;
    options.outputChannelCounts = outputChannelCounts;

    EMSCRIPTEN_WEBAUDIO_T workletNode = emscripten_create_wasm_audio_worklet_node(
        audioContext, 
        "audio_processor", 
        &options, 
        EMPlayer::EMAudioCallback,
        userData
    );

    emscripten_audio_node_connect(workletNode, audioContext, 0, 0);
}

void EMPlayer::OnAudioThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData) {

    if (!success) {
        return;
    }

    WebAudioWorkletProcessorCreateOptions opts = {};
    opts.name = "audio_processor";
  
    emscripten_create_wasm_audio_worklet_processor_async(
        audioContext, 
        &opts, 
        OnAudioNodeCreated, 
        userData
    );
}

EM_BOOL EMPlayer::OnCanvasClick(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
    EMSCRIPTEN_WEBAUDIO_T context = (EMSCRIPTEN_WEBAUDIO_T)userData;
    if (emscripten_audio_context_state(context) != AUDIO_CONTEXT_STATE_RUNNING) {
        emscripten_resume_audio_context_sync(context);
    }
    return EM_FALSE;
}

bool EMPlayer::init() {
    EmscriptenWebAudioCreateAttributes attr = {};
    attr.sampleRate = 44100;

    m_audioContext = emscripten_create_audio_context(&attr);
    emscripten_set_click_callback("canvas", (void*)m_audioContext, EM_FALSE, EMPlayer::OnCanvasClick);

    static uint8_t wasmAudioWorkletStack[1024 * 64];
    emscripten_start_wasm_audio_worklet_thread_async(
        m_audioContext, 
        wasmAudioWorkletStack, 
        sizeof(wasmAudioWorkletStack), 
        EMPlayer::OnAudioThreadInitialized, 
        this
    );
    return true;
}

void EMPlayer::resume() {
    m_isPlaying = true;
    if (m_audioContext) {
        if (emscripten_audio_context_state(m_audioContext) == AUDIO_CONTEXT_STATE_SUSPENDED) {
            emscripten_resume_audio_context_sync(m_audioContext);
        }
    }
}

void EMPlayer::pause() {
    m_isPlaying = false;     
}

void EMPlayer::flush() {
    m_accumulator.clear();
    m_ringBuffer.clear(); 
}

void EMPlayer::enqueueData(const std::vector<float>& pcmData) {
    if (emscripten_audio_context_state(m_audioContext) != AUDIO_CONTEXT_STATE_RUNNING) {
        return; 
    }

    if (!pcmData.empty()) {
        m_accumulator.insert(m_accumulator.end(), pcmData.begin(), pcmData.end());
    }

    if (m_accumulator.size() < 4096) {
        return;
    }

    size_t availableWrite = m_ringBuffer.getAvailableWrite();
    if (availableWrite > 0 && !m_accumulator.empty()) {
        size_t toWrite = std::min(availableWrite, m_accumulator.size());
        m_ringBuffer.write(m_accumulator.data(), toWrite);
        m_accumulator.erase(m_accumulator.begin(), m_accumulator.begin() + toWrite);
    }
    resume();
}

EM_BOOL EMPlayer::audioCallback(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params){
    if (numOutputs < 1) return EM_TRUE;

    int numChannels = outputs[0].numberOfChannels;
    int numFrames = outputs[0].samplesPerChannel;
    int totalSamplesNeeded = numFrames * numChannels;

     if (!m_isPlaying) {
        std::fill_n(outputs[0].data, totalSamplesNeeded, 0.0f);
        return EM_TRUE;
    }
    
    float interleavedBuffer[1024] = { 0.0f }; 
    if (totalSamplesNeeded > 1024) return EM_TRUE;

    size_t samplesRead = m_ringBuffer.read(interleavedBuffer, totalSamplesNeeded);
    
    if (samplesRead < totalSamplesNeeded) {
        std::fill_n(interleavedBuffer + samplesRead, totalSamplesNeeded - samplesRead, 0.0f);
    }

    m_softwareMixer.mixAudio(interleavedBuffer, static_cast<int32_t>(totalSamplesNeeded));

    for (int c = 0; c < numChannels; ++c) {
        for (int f = 0; f < numFrames; ++f) {
            int interleavedIndex = f * numChannels + c;
            int planarIndex = c * numFrames + f;        
            outputs[0].data[planarIndex] = interleavedBuffer[interleavedIndex];
        }
    }

    return EM_TRUE; 
}

void EMPlayer::setVolume(float volume){
    m_softwareMixer.setVolume(volume);
}

float EMPlayer::getVolume() {
    return m_softwareMixer.getVolume();
}