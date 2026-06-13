#include "EffectsEngine.h"

#include "synth_constants.h"

namespace vfx {

using vial::poly_float;
using vial::Reverb;
using vial::StereoDelay;
namespace constants = vial::constants;

EffectsEngine::EffectsEngine() {
    for (int i = 0; i < (int) kParams.size(); ++i)
        indexOf_[kParams[i].id] = i;
    for (const auto& p : kParams)
        raw_[indexOf_.at (p.id)] = p.defaultValue;

    reverb_ = new Reverb();
    int delaySize = (int) (4.0 * vial::kDefaultSampleRate); // generous; resized in prepare
    delay_ = new StereoDelay (delaySize);

    for (int i = 1; i < Reverb::kNumInputs; ++i)
        reverb_->plug (&reverbIn_[i], i);
    for (int i = 1; i < StereoDelay::kNumInputs; ++i)
        delay_->plug (&delayIn_[i], i);
}

EffectsEngine::~EffectsEngine() {
    delete reverb_;
    delete delay_;
}

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
float EffectsEngine::tempoSyncFrequency (const char* prefix) const {
    const std::string p = prefix;
    int sync = (int) std::round (raw ((p + "_sync").c_str()));
    int tempoIndex = (int) std::round (raw ((p + "_tempo").c_str()));
    float freeFreq = scaled ((p + "_frequency").c_str()); // 2^raw

    tempoIndex = std::max (0, std::min (tempoIndex, constants::kNumSyncedFrequencyRatios - 1));
    float ratio = constants::kSyncedFrequencyRatios[tempoIndex];
    float bps = (float) (bpm_ / 60.0);
    float tempoValue = ratio * bps;

    switch (sync) {
        case 1: return tempoValue;             // Tempo
        case 2: return tempoValue * (2.0f / 3.0f); // Dotted
        case 3: return tempoValue * (3.0f / 2.0f); // Triplet
        case 0:
        default: return freeFreq;              // Seconds/Frequency
    }
}

void EffectsEngine::process (float* left, float* right, int numSamples) {
    const int kBlock = vial::kMaxBufferSize;
    for (int offset = 0; offset < numSamples; offset += kBlock) {
        int n = std::min (kBlock, numSamples - offset);

        poly_float* io = ioBuffer_.buffer;
        for (int s = 0; s < n; ++s)
            io[s] = poly_float (left[offset + s], right[offset + s]);

        // The processBlock helper reads/writes ioBuffer_ and chains effects.
        const poly_float* result = nullptr;
        {
            const poly_float* src = io;

            // --- Chorus ---
            bool chorusOn = raw ("chorus_on") >= 0.5f;
            if (chorusOn) {
                chorus_.setFrequencyHz (tempoSyncFrequency ("chorus"));
                chorus_.setDelay1Seconds (scaled ("chorus_delay_1"));
                chorus_.setDelay2Seconds (scaled ("chorus_delay_2"));
                chorus_.setModDepth (raw ("chorus_mod_depth"));
                chorus_.setDryWet (raw ("chorus_dry_wet"));
                chorus_.setVoices ((int) std::round (raw ("chorus_voices")));
                chorus_.setFeedback (raw ("chorus_feedback"));
                chorus_.setCutoffMidi (raw ("chorus_cutoff"));
                chorus_.setSpread (raw ("chorus_spread"));
                chorus_.process (src, n);
                src = chorus_.output();
            } else if (chorusWasOn_) {
                chorus_.hardReset();
            }
            chorusWasOn_ = chorusOn;

            // --- Delay ---
            bool delayOn = raw ("delay_on") >= 0.5f;
            if (delayOn) {
                float freq = tempoSyncFrequency ("delay");
                delayIn_[StereoDelay::kWet].buffer[0]          = raw ("delay_dry_wet");
                delayIn_[StereoDelay::kFrequency].buffer[0]    = freq;
                delayIn_[StereoDelay::kFrequencyAux].buffer[0] = freq;
                delayIn_[StereoDelay::kFeedback].buffer[0]     = raw ("delay_feedback");
                delayIn_[StereoDelay::kStyle].buffer[0]        = scaled ("delay_style");
                delayIn_[StereoDelay::kFilterCutoff].buffer[0] = raw ("delay_filter_cutoff");
                delayIn_[StereoDelay::kFilterSpread].buffer[0] = raw ("delay_filter_spread");
                delay_->processWithInput (src, n);
                src = delay_->output()->buffer;
            } else if (delayWasOn_) {
                delay_->hardReset();
            }
            delayWasOn_ = delayOn;

            // --- Reverb ---
            bool reverbOn = raw ("reverb_on") >= 0.5f;
            if (reverbOn) {
                reverbIn_[Reverb::kWet].buffer[0]            = raw ("reverb_dry_wet");
                reverbIn_[Reverb::kDecayTime].buffer[0]      = scaled ("reverb_decay_time");
                reverbIn_[Reverb::kPreLowCutoff].buffer[0]   = raw ("reverb_pre_low_cutoff");
                reverbIn_[Reverb::kPreHighCutoff].buffer[0]  = raw ("reverb_pre_high_cutoff");
                reverbIn_[Reverb::kLowCutoff].buffer[0]      = raw ("reverb_low_shelf_cutoff");
                reverbIn_[Reverb::kLowGain].buffer[0]        = raw ("reverb_low_shelf_gain");
                reverbIn_[Reverb::kHighCutoff].buffer[0]     = raw ("reverb_high_shelf_cutoff");
                reverbIn_[Reverb::kHighGain].buffer[0]       = raw ("reverb_high_shelf_gain");
                reverbIn_[Reverb::kChorusAmount].buffer[0]   = scaled ("reverb_chorus_amount");
                reverbIn_[Reverb::kChorusFrequency].buffer[0]= scaled ("reverb_chorus_frequency");
                reverbIn_[Reverb::kSize].buffer[0]           = raw ("reverb_size");
                reverbIn_[Reverb::kDelay].buffer[0]          = raw ("reverb_delay");
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
