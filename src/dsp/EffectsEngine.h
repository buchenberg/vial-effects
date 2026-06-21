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
#include <memory>

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

    void setRawParameters (const std::array<float, kParams.size()>& raw) { raw_ = raw; }

    void process (float* left, float* right, int numSamples);

private:
    // Pre-resolved indices into kParams / raw_ — resolved once in the constructor
    // so the audio thread never does string comparisons or hash lookups.
    struct Idx {
        int chorusOn, chorusDryWet, chorusFeedback, chorusCutoff, chorusSpread;
        int chorusVoices, chorusFrequency, chorusSync, chorusTempo, chorusModDepth;
        int chorusDelay1, chorusDelay2;
        int delayOn, delayDryWet, delayFeedback, delayFrequency;
        int delayStyle, delayFilterCutoff, delayFilterSpread, delaySync, delayTempo;
        int reverbOn, reverbDryWet, reverbDecayTime;
        int reverbPreLowCutoff, reverbPreHighCutoff;
        int reverbLowCutoff, reverbLowGain, reverbHighCutoff, reverbHighGain;
        int reverbChorusAmount, reverbChorusFrequency, reverbSize, reverbDelay;
    } idx_;

    int indexOf (const char* id) const;

    float raw (int i) const { return raw_[i]; }
    float scaled (int i) const { return toEngineValue (kParams[i], raw_[i]); }

    float chorusTempoSyncFrequency() const;
    float delayTempoSyncFrequency() const;

    std::unique_ptr<vial::Reverb> reverb_;
    std::unique_ptr<vial::StereoDelay> delay_;
    Chorus chorus_;

    vial::cr::Output reverbIn_[vial::Reverb::kNumInputs];
    vial::cr::Output delayIn_[vial::StereoDelay::kNumInputs];

    vial::Output ioBuffer_;

    std::array<float, kParams.size()> raw_ {};

    double bpm_ = 120.0;
    int sampleRate_ = 44100;

    bool chorusWasOn_ = false;
    bool delayWasOn_ = false;
    bool reverbWasOn_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectsEngine)
};

} // namespace vfx
