/* Vial Effects — Chorus.
 *
 * vial's chorus has no standalone Processor: the multi-voice modulation lives
 * in synthesis/modules/chorus_module.cpp, wrapped in SynthModule machinery.
 * This class reproduces that logic verbatim (see chorus_module.cpp::
 * processWithInput) but owns its MultiDelay Processors directly and is driven
 * by plain setters from EffectsEngine instead of the modulation graph.
 */
#pragma once

#include <array>
#include <memory>

#include "delay.h"
#include "memory.h"

namespace vfx {

class Chorus {
public:
    static constexpr float kMaxChorusModulation = 0.03f; // ChorusModule::kMaxChorusModulation
    static constexpr float kMaxChorusDelay = 0.08f;      // ChorusModule::kMaxChorusDelay
    static constexpr int   kMaxDelayPairs = 4;           // ChorusModule::kMaxDelayPairs

    Chorus();
    ~Chorus();

    void setSampleRate (int sampleRate);
    void hardReset();

    // Per-block control values (already converted to engine units by the engine).
    void setFrequencyHz (float hz)       { frequencyHz_ = hz; }
    void setDelay1Seconds (float s)      { delay1_ = s; }
    void setDelay2Seconds (float s)      { delay2_ = s; }
    void setModDepth (float d)           { modDepth_ = d; }
    void setDryWet (float w)             { dryWet_ = w; }
    void setVoices (int v)               { voices_ = v; }
    void setFeedback (float f);
    void setCutoffMidi (float c);
    void setSpread (float s);

    void process (const vial::poly_float* audio_in, int num_samples);
    const vial::poly_float* output() const { return output_.buffer; }

private:
    int getNextNumVoicePairs();

    std::array<std::unique_ptr<vial::MultiDelay>, kMaxDelayPairs> delays_;

    // Control-rate inputs plugged into each MultiDelay.
    vial::cr::Output delayFrequency_[kMaxDelayPairs];
    vial::cr::Output feedbackIn_;
    vial::cr::Output cutoffIn_;
    vial::cr::Output spreadIn_;
    vial::cr::Output wetOne_;     // delays always fully wet
    vial::cr::Output styleMono_;  // MultiDelay::kMono

    vial::Output output_;         // chorus mix output (kMaxBufferSize)

    float frequencyHz_ = 1.0f;
    float delay1_ = 0.002f;
    float delay2_ = 0.008f;
    float modDepth_ = 0.5f;
    float dryWet_ = 0.5f;
    int   voices_ = 4;

    vial::poly_float phase_ { 0.0f };
    vial::poly_float wet_ { 0.0f };
    vial::poly_float dry_ { 0.0f };
    int lastNumVoices_ = 0;
    int sampleRate_ = 44100;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Chorus)
};

} // namespace vfx
