#include "PluginProcessor.h"
#include "PluginEditor.h"

using vfx::kParams;
using vfx::ParamSpec;
using vfx::Scale;

namespace {
const juce::StringArray kSyncChoices { "Seconds", "Tempo", "Tempo Dotted", "Tempo Triplets" };
const juce::StringArray kDelayStyleChoices { "Mono", "Stereo", "Ping Pong", "Mid Ping Pong" };

bool isOnSwitch (const ParamSpec& p) {
    return juce::String (p.id).endsWith ("_on");
}
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout VialEffectsProcessor::createLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    for (const auto& p : kParams) {
        juce::ParameterID id { p.id, 1 };
        juce::String name (p.displayName);

        if (isOnSwitch (p)) {
            params.push_back (std::make_unique<juce::AudioParameterBool> (
                id, name, p.defaultValue >= 0.5f));
        } else if (juce::String (p.id) == "delay_style") {
            params.push_back (std::make_unique<juce::AudioParameterChoice> (
                id, name, kDelayStyleChoices, (int) p.defaultValue));
        } else if (juce::String (p.id).endsWith ("_sync")) {
            params.push_back (std::make_unique<juce::AudioParameterChoice> (
                id, name, kSyncChoices, (int) p.defaultValue));
        } else if (p.scale == Scale::Indexed) {
            // voices, tempo indices
            params.push_back (std::make_unique<juce::AudioParameterInt> (
                id, name, (int) p.min, (int) p.max, (int) p.defaultValue));
        } else {
            juce::NormalisableRange<float> range (p.min, p.max);
            params.push_back (std::make_unique<juce::AudioParameterFloat> (
                id, name, range, p.defaultValue));
        }
    }

    return { params.begin(), params.end() };
}

VialEffectsProcessor::VialEffectsProcessor()
    : AudioProcessor (BusesProperties()
          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createLayout()) {
    for (size_t i = 0; i < kParams.size(); ++i)
        paramPtrs[i] = apvts.getRawParameterValue (kParams[i].id);
}

bool VialEffectsProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const {
    const auto& mainOut = layouts.getMainOutputChannelSet();
    const auto& mainIn = layouts.getMainInputChannelSet();
    if (mainOut != juce::AudioChannelSet::stereo())
        return false;
    return mainIn == mainOut;
}

void VialEffectsProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
    engine.prepare ((int) sampleRate, samplesPerBlock);
}

void VialEffectsProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    for (auto ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    if (buffer.getNumChannels() < 2 || numSamples == 0)
        return;

    // Push current parameter snapshot to the engine.
    std::array<float, kParams.size()> raw;
    for (size_t i = 0; i < kParams.size(); ++i)
        raw[i] = paramPtrs[i]->load();
    engine.setRawParameters (raw);

    if (auto* ph = getPlayHead()) {
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm())
                engine.setBpm (*bpm);
    }

    engine.process (buffer.getWritePointer (0), buffer.getWritePointer (1), numSamples);
}

juce::AudioProcessorEditor* VialEffectsProcessor::createEditor() {
    return new VialEffectsEditor (*this);
}

void VialEffectsProcessor::getStateInformation (juce::MemoryBlock& destData) {
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void VialEffectsProcessor::setStateInformation (const void* data, int sizeInBytes) {
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new VialEffectsProcessor();
}
