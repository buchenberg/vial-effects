/* Vial Effects — DSP engine.
 *
 * Owns the three extracted vial effects (Reverb, StereoDelay, Chorus) and runs
 * them in series: input -> Chorus -> Delay -> Reverb -> output (matching the
 * panel order in the UI). Parameters arrive as RAW values (in each param's
 * [min,max]); the engine converts them to engine units (EffectParameters.h)
 * and writes them into each Processor's control-rate inputs each block.
 */
#pragma once

#include <array>
#include <unordered_map>
#include <string>

#include "EffectParameters.h"
#include "Chorus.h"
#include "reverb.h"
#include "delay.h"

namespace vfx {

class EffectsEngine {
public:
    EffectsEngine();
    ~EffectsEngine();

    void prepare (int sampleRate, int maxBlockSize);
    void reset();

    void setBpm (double bpm) { bpm_ = bpm > 0.0 ? bpm : 120.0; }

    // Copy the current raw parameter snapshot (indexed parallel to vfx::kParams).
    void setRawParameters (const std::array<float, kParams.size()>& raw) { raw_ = raw; }

    // Stereo, in place. numSamples may exceed the internal 128-sample block.
    void process (float* left, float* right, int numSamples);

private:
    float raw (const char* id) const { return raw_[indexOf_.at (id)]; }
    float scaled (const char* id) const { return toEngineValue (findParam (id), raw (id)); }
    float tempoSyncFrequency (const char* prefix) const;

    vial::Reverb* reverb_ = nullptr;
    vial::StereoDelay* delay_ = nullptr;
    Chorus chorus_;

    // Control-rate inputs (index == Processor input enum). Index 0 (kAudio) unused.
    vial::cr::Output reverbIn_[vial::Reverb::kNumInputs];
    vial::cr::Output delayIn_[vial::StereoDelay::kNumInputs];

    vial::Output ioBuffer_; // packed stereo block, also reused as passthrough

    std::unordered_map<std::string, int> indexOf_;
    std::array<float, kParams.size()> raw_ {};

    double bpm_ = 120.0;
    int sampleRate_ = 44100;

    bool chorusWasOn_ = false;
    bool delayWasOn_ = false;
    bool reverbWasOn_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectsEngine)
};

} // namespace vfx
