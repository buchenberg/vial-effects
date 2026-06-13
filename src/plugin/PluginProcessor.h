/* Vial Effects — AudioProcessor.
 *
 * Stereo effect plugin hosting Chorus -> Delay -> Reverb extracted from vial.
 * Parameters live in an APVTS built from vfx::kParams; the web UI binds to them
 * via JUCE 8 WebSliderRelays (see PluginEditor).
 */
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "dsp/EffectsEngine.h"
#include "dsp/EffectParameters.h"

class VialEffectsProcessor final : public juce::AudioProcessor {
public:
    VialEffectsProcessor();
    ~VialEffectsProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Vial Effects"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    vfx::EffectsEngine engine;
    std::array<std::atomic<float>*, vfx::kParams.size()> paramPtrs {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VialEffectsProcessor)
};
