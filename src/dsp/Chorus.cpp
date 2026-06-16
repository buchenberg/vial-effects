#include "Chorus.h"

#include "futils.h"
#include "synth_constants.h"

namespace vfx {

using vial::poly_float;
using vial::MultiDelay;
namespace utils = vial::utils;
namespace futils = vial::futils;
namespace constants = vial::constants;

Chorus::Chorus() {
    int max_samples = (int) (kMaxChorusDelay * vial::kMaxSampleRate) + 1;

    wetOne_.buffer[0] = 1.0f;
    styleMono_.buffer[0] = (float) MultiDelay::kMono;

    for (int i = 0; i < kMaxDelayPairs; ++i) {
        delays_[i] = new MultiDelay (max_samples);
        delays_[i]->plug (&delayFrequency_[i], MultiDelay::kFrequency);
        delays_[i]->plug (&feedbackIn_,         MultiDelay::kFeedback);
        delays_[i]->plug (&wetOne_,             MultiDelay::kWet);
        delays_[i]->plug (&cutoffIn_,           MultiDelay::kFilterCutoff);
        delays_[i]->plug (&spreadIn_,           MultiDelay::kFilterSpread);
        delays_[i]->plug (&styleMono_,          MultiDelay::kStyle);
    }
}

Chorus::~Chorus() {
    for (int i = 0; i < kMaxDelayPairs; ++i)
        delete delays_[i];
}

void Chorus::setSampleRate (int sampleRate) {
    sampleRate_ = sampleRate;
    for (int i = 0; i < kMaxDelayPairs; ++i)
        delays_[i]->setSampleRate (sampleRate);
}

void Chorus::hardReset() {
    wet_ = 0.0f;
    dry_ = 0.0f;
    phase_ = 0.0f;
    lastNumVoices_ = 0;
    for (int i = 0; i < kMaxDelayPairs; ++i)
        delays_[i]->hardReset();
}

void Chorus::setFeedback (float f)   { feedbackIn_.buffer[0] = f; }
void Chorus::setCutoffMidi (float c) { cutoffIn_.buffer[0] = c; }
void Chorus::setSpread (float s)     { spreadIn_.buffer[0] = s; }

int Chorus::getNextNumVoicePairs() {
    int num_voice_pairs = voices_;
    for (int i = lastNumVoices_; i < num_voice_pairs; ++i)
        delays_[i]->reset (constants::kFullMask);
    lastNumVoices_ = num_voice_pairs;
    return num_voice_pairs;
}

// Mirrors vial::ChorusModule::processWithInput (chorus_module.cpp:92-156).
void Chorus::process (const poly_float* audio_in, int num_samples) {
    poly_float frequency = frequencyHz_;
    poly_float delta_phase = (frequency * static_cast<float>(num_samples)) * (1.0f / static_cast<float>(sampleRate_));
    phase_ = utils::mod (phase_ + delta_phase);

    poly_float* audio_out = output_.buffer;
    for (int s = 0; s < num_samples; ++s) {
        poly_float sample = audio_in[s] & constants::kFirstMask;
        audio_out[s] = sample + utils::swapVoices (sample);
    }

    int num_voices = getNextNumVoicePairs();

    poly_float delay1 = poly_float (delay1_);
    poly_float delay2 = poly_float (delay2_);
    poly_float delay_time = utils::maskLoad (delay2, delay1, constants::kFirstMask);
    poly_float average_delay = (delay_time + utils::swapVoices (delay_time)) * 0.5f;
    for (int i = 0; i < num_voices; ++i) {
        float pair_offset = i * 0.25f / num_voices;
        poly_float right_offset = (poly_float (0.25f) & constants::kRightMask);
        poly_float phase = phase_ + right_offset + (poly_float (0.5f) & ~constants::kFirstMask) + pair_offset;

        poly_float mod_depth = poly_float (modDepth_) * kMaxChorusModulation;
        poly_float mod = utils::sin (phase * vial::kPi * 2.0f) * 0.5f + 1.0f;
        float delay_t = 0.0f;
        if (i > 0)
            delay_t = i / (num_voices - 1.0f);
        poly_float delay = mod * mod_depth + utils::interpolate (delay_time, average_delay, delay_t);

        poly_float delay_frequency = poly_float (1.0f) / utils::max (0.00001f, delay);
        delayFrequency_[i].buffer[0] = delay_frequency;
        delays_[i]->processWithInput (audio_out, num_samples);
    }

    poly_float current_wet = wet_;
    poly_float current_dry = dry_;

    poly_float wet_value = utils::clamp (poly_float (dryWet_), 0.0f, 1.0f);
    wet_ = futils::equalPowerFade (wet_value);
    dry_ = futils::equalPowerFadeInverse (wet_value);

    float tick_increment = 1.0f / num_samples;
    poly_float delta_wet = (wet_ - current_wet) * tick_increment;
    poly_float delta_dry = (dry_ - current_dry) * tick_increment;

    utils::zeroBuffer (audio_out, num_samples);

    for (int i = 0; i < num_voices; ++i) {
        const poly_float* delay_out = delays_[i]->output()->buffer;
        for (int s = 0; s < num_samples; ++s) {
            poly_float sample_out = delay_out[s] * 0.5f;
            audio_out[s] += sample_out + utils::swapVoices (sample_out);
        }
    }

    for (int s = 0; s < num_samples; ++s) {
        current_dry += delta_dry;
        current_wet += delta_wet;
        audio_out[s] = current_dry * audio_in[s] + current_wet * audio_out[s];
    }
}

} // namespace vfx
