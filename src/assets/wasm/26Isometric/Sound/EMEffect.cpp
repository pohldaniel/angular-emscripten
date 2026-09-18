#include <algorithm>

#include "EMEffect.h"

EM_BOOL EMEffect::EMAudioCallback(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData){
    return static_cast<EMEffect*>(userData)->audioCallback(numInputs, inputs, numOutputs, outputs, numParams, params);
}

CacheLRU<std::string, EMEffect::CacheEntry> EMEffect::Cache;

EMEffect::EMEffect() {
    Cache.Init(5u);
}

EMEffect::~EMEffect() {
   Cache.Clear();
}

void EMEffect::OnAudioNodeCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData) {
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
        EMEffect::EMAudioCallback,
        userData
    );

    emscripten_audio_node_connect(workletNode, audioContext, 0, 0);
}

void EMEffect::OnAudioThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData) {

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

EM_BOOL EMEffect::OnCanvasClick(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
    EMSCRIPTEN_WEBAUDIO_T context = (EMSCRIPTEN_WEBAUDIO_T)userData;
    if (emscripten_audio_context_state(context) != AUDIO_CONTEXT_STATE_RUNNING) {
        emscripten_resume_audio_context_sync(context);
    }
    return EM_FALSE;
}

void EMEffect::init() {
    EmscriptenWebAudioCreateAttributes attr = {};
    attr.sampleRate = 44100;

    m_audioContext = emscripten_create_audio_context(&attr);
    emscripten_set_click_callback("canvas", (void*)m_audioContext, EM_FALSE, EMEffect::OnCanvasClick);

    static uint8_t wasmAudioWorkletStack[1024 * 64];
    emscripten_start_wasm_audio_worklet_thread_async(
        m_audioContext, 
        wasmAudioWorkletStack, 
        sizeof(wasmAudioWorkletStack), 
        EMEffect::OnAudioThreadInitialized, 
        this
    );
}

void EMEffect::play(const std::string& file) {
    const CacheEntry& entry = Cache.Get(file);
    if (entry.m_samples.empty()) 
        return;

    bool channelFound = false;
    for (auto& channel : m_softwareMixer.m_channels) {
        if (channel.status == 0) {
            channel.pcmData = &entry.m_samples;
            channel.progress = 0.0f;
            channel.status = 1;
            channelFound = true;
            break;
        }
    }

    if (!channelFound) {
        size_t maxProgress = 0;
        ActiveSound* oldestChannel = nullptr;
        for (auto& channel : m_softwareMixer.m_channels) {
            if (channel.progress > maxProgress) {
                maxProgress = channel.progress;
                oldestChannel = &channel;
            }
        }

        if (oldestChannel) {
            oldestChannel->pcmData = &entry.m_samples;
            oldestChannel->progress = 0.0f;
            oldestChannel->status = 1;
        }
    }

    resume();
}

EM_BOOL EMEffect::audioCallback(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params){
    if (numOutputs < 1) return EM_TRUE;

    int numChannels = outputs[0].numberOfChannels;
    int numFrames = outputs[0].samplesPerChannel;
    int totalSamplesNeeded = numFrames * numChannels;

    float interleavedBuffer[1024] = { 0.0f }; 
    if (totalSamplesNeeded > 1024) return EM_TRUE;

    std::fill_n(interleavedBuffer, totalSamplesNeeded, 0.0f);
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

void EMEffect::setVolume(float volume) {
     m_softwareMixer.setVolume(volume);
}

void EMEffect::resume() {
    if (m_audioContext) {
        if (emscripten_audio_context_state(m_audioContext) == AUDIO_CONTEXT_STATE_SUSPENDED) {
            emscripten_resume_audio_context_sync(m_audioContext);
        }
    }
}

SoftwareMixer& EMEffect::getMixer() {
    return m_softwareMixer;
}

EMEffect::CacheEntry::CacheEntry(const std::string& file) {
    AVFormatContext* formatCtx = nullptr;
    avformat_open_input(&formatCtx, file.c_str(), nullptr, nullptr);
    avformat_find_stream_info(formatCtx, nullptr);

    int streamIdx = -1;
    for (unsigned int i = 0; i < formatCtx->nb_streams; i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            streamIdx = i;
            break;
        }
    }

    const AVCodec* codec = avcodec_find_decoder(formatCtx->streams[streamIdx]->codecpar->codec_id);
    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx, formatCtx->streams[streamIdx]->codecpar);
    avcodec_open2(codecCtx, codec, nullptr);

    SwrContext* swr = swr_alloc();
    av_opt_set_chlayout(swr, "in_chlayout", &codecCtx->ch_layout, 0);
    av_opt_set_int(swr, "in_sample_rate", codecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", codecCtx->sample_fmt, 0);

    AVChannelLayout outLayout;
    av_channel_layout_default(&outLayout, 2);
    av_opt_set_chlayout(swr, "out_chlayout", &outLayout, 0);
    av_opt_set_int(swr, "out_sample_rate", 44100, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
    swr_init(swr);

    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();


    std::vector<float> pcmData;
    while (av_read_frame(formatCtx, packet) >= 0) {
        if (packet->stream_index == streamIdx) {
            int send_ret = avcodec_send_packet(codecCtx, packet);
            if (send_ret >= 0) {
                int ret = 0;
                while (true) {
                    ret = avcodec_receive_frame(codecCtx, frame);

                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0) {
                        break;
                    }

                    int outSamples = swr_get_out_samples(swr, frame->nb_samples);
                    if (outSamples <= 0) {
                        av_frame_unref(frame);
                        continue;
                    }

                    int maxSamplesNeeded = outSamples * 2;
                    size_t oldSize = pcmData.size();
                    pcmData.resize(oldSize + maxSamplesNeeded);
                    uint8_t* buffer = reinterpret_cast<uint8_t*>(pcmData.data() + oldSize);

                    int convertedSamples = swr_convert(swr, &buffer, outSamples, (const uint8_t**)frame->data, frame->nb_samples);
                    if (convertedSamples >= 0) {
                        int actualSamplesConverted = convertedSamples * 2;
                        pcmData.resize(oldSize + actualSamplesConverted);
                    }
                    else {
                        pcmData.resize(oldSize);
                    }
                    av_frame_unref(frame);
                }
            }
        }
        av_packet_unref(packet);
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
    swr_free(&swr);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);

    m_samples = std::move(pcmData);
    m_totalSamples = m_samples.size();
}

EMEffect::CacheEntry::~CacheEntry() {
    
}

EMEffect::CacheEntry::CacheEntry(CacheEntry&& other) noexcept : m_samples(std::move(other.m_samples)), m_totalSamples(other.m_totalSamples){
    other.m_totalSamples = 0u;
}

EMEffect::CacheEntry& EMEffect::CacheEntry::operator=(CacheEntry&& other) noexcept {
    if (this != &other) {
        m_samples = std::move(other.m_samples);
        m_totalSamples = other.m_totalSamples;
        other.m_totalSamples = 0u;
    }
    return *this;
}