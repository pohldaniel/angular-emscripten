#pragma once

#include <memory>
#include "ISoundEffect.h"

#include "OpenALEffect.h"
#include "EMEffect.h"

class SoundEffect {

public:

    SoundEffect() = default;
    ~SoundEffect() = default;

    template <typename EffectImpl = EMEffect>
    void init() {
        auto audio = std::make_unique<EffectImpl>();
        init(std::move(audio));
    }

    template <class EffectImpl = EMEffect>
    EffectImpl* get() {
        return static_cast<EffectImpl*>(m_soundEffect.get());
    }

    void play(const std::string& file);

private:

    void init(std::unique_ptr<ISoundEffect> soundEffect = nullptr);
    std::unique_ptr<ISoundEffect> m_soundEffect = nullptr;
};