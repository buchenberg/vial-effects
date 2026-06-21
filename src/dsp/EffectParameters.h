/* Vial Effects — shared parameter specification.
 *
 * Single source of truth for every plugin parameter. Ranges, defaults and
 * value-scales are copied verbatim from vial's common/synth_parameters.cpp so
 * the extracted effects behave identically to the synth. Used by:
 *   - PluginProcessor   (to build the APVTS layout)
 *   - EffectsEngine     (to convert a raw parameter value into the value the
 *                        DSP Processor input expects)
 *   - the native tests  (to validate the scaling matches the JS UI)
 *
 * APVTS stores each parameter in its RAW [min,max] range (linear). The engine
 * value handed to a vial Processor input is the raw value passed through the
 * same scale operator that vial's SynthModule::createMonoModControl inserts
 * (see synthesis/framework/synth_module.cpp + operators.h cr:: scale ops).
 */
#pragma once

#include <array>
#include <cmath>
#include <string_view>

namespace vfx {

// Mirrors vial::ValueDetails::ValueScale (the subset the three effects use).
enum class Scale { Linear, Indexed, Quadratic, Exponential, SquareRoot };

struct ParamSpec {
    const char* id;            // APVTS id == vial parameter name
    float min;
    float max;
    float defaultValue;
    float postOffset;          // post_offset column
    float displayMultiply;     // display_multiply column (UI only)
    Scale scale;
    bool displayInvert;        // display_invert column (UI only; engine ignores)
    const char* unit;          // display_units (UI only)
    const char* displayName;   // human label
};

// ----------------------------------------------------------------------------
// Parameter table. Order is irrelevant; lookups are by id.
// Columns: id, min, max, default, postOffset, displayMultiply, scale, invert, unit, name
// ----------------------------------------------------------------------------
inline constexpr std::array<ParamSpec, 34> kParams { {
    // ---- Chorus ----
    { "chorus_on",        0.0f,   1.0f,    0.0f,     0.0f, 1.0f,   Scale::Indexed,     false, "",          "Chorus Switch" },
    { "chorus_dry_wet",   0.0f,   1.0f,    0.5f,     0.0f, 100.0f, Scale::Linear,      false, "%",         "Chorus Mix" },
    { "chorus_feedback", -0.95f,  0.95f,   0.4f,     0.0f, 100.0f, Scale::Linear,      false, "%",         "Chorus Feedback" },
    { "chorus_cutoff",    8.0f,   136.0f,  60.0f,    0.0f, 1.0f,   Scale::Linear,      false, "",          "Chorus Filter Cutoff" },
    { "chorus_spread",    0.0f,   1.0f,    1.0f,     0.0f, 1.0f,   Scale::Linear,      false, "",          "Chorus Filter Spread" },
    { "chorus_voices",    1.0f,   4.0f,    4.0f,     0.0f, 4.0f,   Scale::Indexed,     false, "",          "Chorus Voices" },
    { "chorus_frequency",-6.0f,   3.0f,   -3.0f,     0.0f, 1.0f,   Scale::Exponential, true,  " secs",     "Chorus Frequency" },
    { "chorus_sync",      0.0f,   3.0f,    1.0f,     0.0f, 1.0f,   Scale::Indexed,     false, "",          "Chorus Sync" },
    { "chorus_tempo",     0.0f,   10.0f,   4.0f,     0.0f, 1.0f,   Scale::Indexed,     false, "",          "Chorus Tempo" },
    { "chorus_mod_depth", 0.0f,   1.0f,    0.5f,     0.0f, 100.0f, Scale::Linear,      false, "%",         "Chorus Mod Depth" },
    { "chorus_delay_1",  -10.0f, -5.64386f,-9.0f,    0.0f, 1000.0f,Scale::Exponential, false, " ms",       "Chorus Delay 1" },
    { "chorus_delay_2",  -10.0f, -5.64386f,-7.0f,    0.0f, 1000.0f,Scale::Exponential, false, " ms",       "Chorus Delay 2" },

    // ---- Delay ----
    { "delay_on",         0.0f,   1.0f,    0.0f,     0.0f, 1.0f,   Scale::Indexed,     false, "",          "Delay Switch" },
    { "delay_dry_wet",    0.0f,   1.0f,    0.3334f,  0.0f, 100.0f, Scale::Linear,      false, "%",         "Delay Mix" },
    { "delay_feedback",  -1.0f,   1.0f,    0.5f,     0.0f, 100.0f, Scale::Linear,      false, "%",         "Delay Feedback" },
    { "delay_frequency", -2.0f,   9.0f,    2.0f,     0.0f, 1.0f,   Scale::Exponential, true,  " secs",     "Delay Frequency" },
    { "delay_style",      0.0f,   3.0f,    0.0f,     0.0f, 1.0f,   Scale::Indexed,     false, "",          "Delay Style" },
    { "delay_filter_cutoff",8.0f, 136.0f,  60.0f,    0.0f, 1.0f,   Scale::Linear,      false, "",          "Delay Filter Cutoff" },
    { "delay_filter_spread",0.0f, 1.0f,    1.0f,     0.0f, 1.0f,   Scale::Linear,      false, "",          "Delay Filter Spread" },
    { "delay_sync",       0.0f,   3.0f,    1.0f,     0.0f, 1.0f,   Scale::Indexed,     false, "",          "Delay Sync" },
    { "delay_tempo",      4.0f,   12.0f,   9.0f,     0.0f, 1.0f,   Scale::Indexed,     false, "",          "Delay Tempo" },

    // ---- Reverb ----
    { "reverb_on",            0.0f, 1.0f,   0.0f,     0.0f, 1.0f,   Scale::Indexed,     false, "",          "Reverb Switch" },
    { "reverb_pre_low_cutoff",0.0f, 128.0f, 0.0f,     0.0f, 1.0f,   Scale::Linear,      false, " semitones","Reverb Pre Low Cutoff" },
    { "reverb_pre_high_cutoff",0.0f,128.0f, 110.0f,   0.0f, 1.0f,   Scale::Linear,      false, " semitones","Reverb Pre High Cutoff" },
    { "reverb_low_shelf_cutoff",0.0f,128.0f,0.0f,     0.0f, 1.0f,   Scale::Linear,      false, " semitones","Reverb Low Cutoff" },
    { "reverb_low_shelf_gain",-6.0f, 0.0f,  0.0f,     0.0f, 1.0f,   Scale::Linear,      false, " dB",       "Reverb Low Gain" },
    { "reverb_high_shelf_cutoff",0.0f,128.0f,90.0f,   0.0f, 1.0f,   Scale::Linear,      false, " semitones","Reverb High Cutoff" },
    { "reverb_high_shelf_gain",-6.0f,0.0f, -1.0f,     0.0f, 1.0f,   Scale::Linear,      false, " dB",       "Reverb High Gain" },
    { "reverb_dry_wet",       0.0f, 1.0f,   0.25f,    0.0f, 100.0f, Scale::Linear,      false, "%",         "Reverb Mix" },
    { "reverb_delay",         0.0f, 0.3f,   0.0f,     0.0f, 1.0f,   Scale::Linear,      false, " secs",     "Reverb Delay" },
    { "reverb_decay_time",   -6.0f, 6.0f,   0.0f,     0.0f, 1.0f,   Scale::Exponential, false, " secs",     "Reverb Decay Time" },
    { "reverb_size",          0.0f, 1.0f,   0.5f,     0.0f, 100.0f, Scale::Linear,      false, "%",         "Reverb Size" },
    { "reverb_chorus_amount", 0.0f, 1.0f,   0.223607f,0.0f, 100.0f, Scale::Quadratic,   false, "%",         "Reverb Chorus Amount" },
    { "reverb_chorus_frequency",-8.0f,3.0f,-2.0f,     0.0f, 1.0f,   Scale::Exponential, false, " Hz",       "Reverb Chorus Frequency" },
} };

inline const ParamSpec* findParam (const char* id) {
    for (const auto& p : kParams)
        if (std::string_view (p.id) == id)
            return &p;
    return nullptr; // caller must check
}

// Small local clamp so this header has no JUCE dependency (usable in tests).
inline float clampf (float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// Convert a RAW parameter value (already in [min,max]) into the engine value
// the vial Processor input expects. Mirrors the cr:: scale operators in
// operators.h that SynthModule::createMonoModControl inserts.
inline float toEngineValue (const ParamSpec& s, float raw) {
    switch (s.scale) {
        case Scale::Indexed:
            return std::round (raw);
        case Scale::Quadratic: {
            float v = raw < 0.0f ? 0.0f : raw;          // cr::Square clamps >= 0
            return v * v + s.postOffset;                // cr::Quadratic adds offset (0 here)
        }
        case Scale::Exponential: {
            float v = clampf (raw, s.min, s.max);
            return std::pow (2.0f, v);                   // cr::ExponentialScale(scale=2). Engine ignores displayInvert.
        }
        case Scale::SquareRoot: {
            float v = raw < 0.0f ? 0.0f : raw;
            return std::sqrt (v) + s.postOffset;
        }
        case Scale::Linear:
        default:
            return raw;
    }
}

} // namespace vfx
