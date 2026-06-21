#include "EffectsEngine.h"

#include "synth_constants.h"

namespace vfx {

using vial::poly_float;
using vial::Reverb;
using vial::StereoDelay;
namespace constants = vial::constants;

int EffectsEngine::indexOf (const char* id) const {
    for (int i = 0; i < (int) kParams.size(); ++i) {
        bool match = true;
        const char* a = kParams[i].id;
        const char* b = id;
        while (*a && *b) {
            if (*a != *b) { match = false; break; }
            ++a; ++b;
        }
        if (match && *a == '\0' && *b == '\0')
            return i;
    }
    jassertfalse; // unknown parameter id
    return 0;
}

EffectsEngine::EffectsEngine() {
    // Resolve all parameter indices once at construction time.
    idx_.chorusOn          = indexOf ("chorus_on");
    idx_.chorusDryWet      = indexOf ("chorus_dry_wet");
    idx_.chorusFeedback    = indexOf ("chorus_feedback");
    idx_.chorusCutoff      = indexOf ("chorus_cutoff");
    idx_.chorusSpread      = indexOf ("chorus_spread");
    idx_.chorusVoices      = indexOf ("chorus_voices");
    idx_.chorusFrequency   = indexOf ("chorus_frequency");
    idx_.chorusSync        = indexOf ("chorus_sync");
    idx_.chorusTempo       = indexOf ("chorus_tempo");
    idx_.chorusModDepth    = indexOf ("chorus_mod_depth");
    idx_.chorusDelay1      = indexOf ("chorus_delay_1");
    idx_.chorusDelay2      = indexOf ("chorus_delay_2");

    idx_.delayOn           = indexOf ("delay_on");
    idx_.delayDryWet       = indexOf ("delay_dry_wet");
    idx_.delayFeedback     = indexOf ("delay_feedback");
    idx_.delayFrequency    = indexOf ("delay_frequency");
    idx_.delayStyle        = indexOf ("delay_style");
    idx_.delayFilterCutoff = indexOf ("delay_filter_cutoff");
    idx_.delayFilterSpread = indexOf ("delay_filter_spread");
    idx_.delaySync         = indexOf ("delay_sync");
    idx_.delayTempo        = indexOf ("delay_tempo");

    idx_.reverbOn              = indexOf ("reverb_on");
    idx_.reverbDryWet          = indexOf ("reverb_dry_wet");
    idx_.reverbDecayTime       = indexOf ("reverb_decay_time");
    idx_.reverbPreLowCutoff    = indexOf ("reverb_pre_low_cutoff");
    idx_.reverbPreHighCutoff   = indexOf ("reverb_pre_high_cutoff");
    idx_.reverbLowCutoff       = indexOf ("reverb_low_shelf_cutoff");
    idx_.reverbLowGain         = indexOf ("reverb_low_shelf_gain");
    idx_.reverbHighCutoff      = indexOf ("reverb_high_shelf_cutoff");
    idx_.reverbHighGain        = indexOf ("reverb_high_shelf_gain");
    idx_.reverbChorusAmount    = indexOf ("reverb_chorus_amount");
    idx_.reverbChorusFrequency = indexOf ("reverb_chorus_frequency");
    idx_.reverbSize            = indexOf ("reverb_size");
    idx_.reverbDelay           = indexOf ("reverb_delay");

    for (const auto& p : kParams)
        raw_[indexOf (p.id)] = p.defaultValue;

    reverb_ = std::make_unique<Reverb>();
    // Delay is sized properly in prepare(); allocate a minimal buffer here.
    delay_ = std::make_unique<StereoDelay> (1);

    for (int i = 1; i < Reverb::kNumInputs; ++i)
        reverb_->plug (&reverbIn_[i], i);
    for (int i = 1; i < StereoDelay::kNumInputs; ++i)
        delay_->plug (&delayIn_[i], i);
}

EffectsEngine::~EffectsEngine() = default;

void EffectsEngine::prepare (int sampleRate, int /*maxBlockSize*/) {
    sampleRate_ = sampleRate;
    reverb_->setSampleRate (sampleRate);
    delay_->setSampleRate (sampleRate);
    delay_->setMaxSamples ((int) (4.0 * sampleRate));
    chorus_.setSampleRate (sampleRate);
    reset();
}

void EffectsEngine::reset() {
    reverb_->hardReset();
    delay_->hardReset();
    chorus_.hardReset();
    chorusWasOn_ = delayWasOn_ = reverbWasOn_ = false;
}

// Mirrors vial::TempoChooser::process (operators.cpp). Returns frequency in Hz.
// Specialised per-effect to avoid any string work on the audio thread.
float EffectsEngine::chorusTempoSyncFrequency() const {
    int sync = (int) std::round (raw (idx_.chorusSync));
    int tempoIndex = (int) std::round (raw (idx_.chorusTempo));
    float freeFreq = scaled (idx_.chorusFrequency);

    tempoIndex = std::max (0, std::min (tempoIndex, constants::kNumSyncedFrequencyRatios - 1));
    float ratio = constants::kSyncedFrequencyRatios[tempoIndex];
    float bps = (float) (bpm_ / 60.0);
    float tempoValue = ratio * bps;

    switch (sync) {
        case 1: return tempoValue;
        case 2: return tempoValue * (2.0f / 3.0f);
        case 3: return tempoValue * (3.0f / 2.0f);
        case 0:
        default: return freeFreq;
    }
}

float EffectsEngine::delayTempoSyncFrequency() const {
    int sync = (int) std::round (raw (idx_.delaySync));
    int tempoIndex = (int) std::round (raw (idx_.delayTempo));
    float freeFreq = scaled (idx_.delayFrequency);

    tempoIndex = std::max (0, std::min (tempoIndex, constants::kNumSyncedFrequencyRatios - 1));
    float ratio = constants::kSyncedFrequencyRatios[tempoIndex];
    float bps = (float) (bpm_ / 60.0);
    float tempoValue = ratio * bps;

    switch (sync) {
        case 1: return tempoValue;
        case 2: return tempoValue * (2.0f / 3.0f);
        case 3: return tempoValue * (3.0f / 2.0f);
        case 0:
        default: return freeFreq;
    }
}

void EffectsEngine::process (float* left, float* right, int numSamples) {
    const int kBlock = vial::kMaxBufferSize;
    for (int offset = 0; offset < numSamples; offset += kBlock) {
        int n = std::min (kBlock, numSamples - offset);

        poly_float* io = ioBuffer_.buffer;
        for (int s = 0; s < n; ++s)
            io[s] = poly_float (left[offset + s], right[offset + s]);

        const poly_float* result = nullptr;
        {
            const poly_float* src = io;

            // --- Chorus ---
            bool chorusOn = raw (idx_.chorusOn) >= 0.5f;
            if (chorusOn) {
                chorus_.setFrequencyHz (chorusTempoSyncFrequency());
                chorus_.setDelay1Seconds (scaled (idx_.chorusDelay1));
                chorus_.setDelay2Seconds (scaled (idx_.chorusDelay2));
                chorus_.setModDepth (raw (idx_.chorusModDepth));
                chorus_.setDryWet (raw (idx_.chorusDryWet));
                chorus_.setVoices ((int) std::round (raw (idx_.chorusVoices)));
                chorus_.setFeedback (raw (idx_.chorusFeedback));
                chorus_.setCutoffMidi (raw (idx_.chorusCutoff));
                chorus_.setSpread (raw (idx_.chorusSpread));
                chorus_.process (src, n);
                src = chorus_.output();
            } else if (chorusWasOn_) {
                chorus_.hardReset();
            }
            chorusWasOn_ = chorusOn;

            // --- Delay ---
            bool delayOn = raw (idx_.delayOn) >= 0.5f;
            if (delayOn) {
                float freq = delayTempoSyncFrequency();
                delayIn_[StereoDelay::kWet].buffer[0]          = raw (idx_.delayDryWet);
                delayIn_[StereoDelay::kFrequency].buffer[0]    = freq;
                delayIn_[StereoDelay::kFrequencyAux].buffer[0] = freq;
                delayIn_[StereoDelay::kFeedback].buffer[0]     = raw (idx_.delayFeedback);
                delayIn_[StereoDelay::kStyle].buffer[0]        = scaled (idx_.delayStyle);
                delayIn_[StereoDelay::kFilterCutoff].buffer[0] = raw (idx_.delayFilterCutoff);
                delayIn_[StereoDelay::kFilterSpread].buffer[0] = raw (idx_.delayFilterSpread);
                delay_->processWithInput (src, n);
                src = delay_->output()->buffer;
            } else if (delayWasOn_) {
                delay_->hardReset();
            }
            delayWasOn_ = delayOn;

            // --- Reverb ---
            bool reverbOn = raw (idx_.reverbOn) >= 0.5f;
            if (reverbOn) {
                reverbIn_[Reverb::kWet].buffer[0]            = raw (idx_.reverbDryWet);
                reverbIn_[Reverb::kDecayTime].buffer[0]      = scaled (idx_.reverbDecayTime);
                reverbIn_[Reverb::kPreLowCutoff].buffer[0]   = raw (idx_.reverbPreLowCutoff);
                reverbIn_[Reverb::kPreHighCutoff].buffer[0]  = raw (idx_.reverbPreHighCutoff);
                reverbIn_[Reverb::kLowCutoff].buffer[0]      = raw (idx_.reverbLowCutoff);
                reverbIn_[Reverb::kLowGain].buffer[0]        = raw (idx_.reverbLowGain);
                reverbIn_[Reverb::kHighCutoff].buffer[0]     = raw (idx_.reverbHighCutoff);
                reverbIn_[Reverb::kHighGain].buffer[0]       = raw (idx_.reverbHighGain);
                reverbIn_[Reverb::kChorusAmount].buffer[0]   = scaled (idx_.reverbChorusAmount);
                reverbIn_[Reverb::kChorusFrequency].buffer[0]= scaled (idx_.reverbChorusFrequency);
                reverbIn_[Reverb::kSize].buffer[0]           = raw (idx_.reverbSize);
                reverbIn_[Reverb::kDelay].buffer[0]          = raw (idx_.reverbDelay);
                reverb_->processWithInput (src, n);
                src = reverb_->output()->buffer;
            } else if (reverbWasOn_) {
                reverb_->hardReset();
            }
            reverbWasOn_ = reverbOn;

            result = src;
        }

        for (int s = 0; s < n; ++s) {
            left[offset + s]  = result[s][0];
            right[offset + s] = result[s][1];
        }
    }
}

} // namespace vfx
