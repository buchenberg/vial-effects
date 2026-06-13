/* Vial Effects — WebView editor.
 *
 * Hosts a juce::WebBrowserComponent that loads the React/Vite UI (bundled as
 * BinaryData). Every APVTS parameter is bound to the JS frontend with a JUCE 8
 * relay + parameter attachment:
 *   - *_on            -> WebToggleButtonRelay
 *   - delay_style/*_sync -> WebComboBoxRelay
 *   - everything else -> WebSliderRelay
 */
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

class VialEffectsEditor final : public juce::AudioProcessorEditor {
public:
    explicit VialEffectsEditor (VialEffectsProcessor&);
    ~VialEffectsEditor() override;

    void resized() override;

private:
    std::optional<juce::WebBrowserComponent::Resource> getResource (const juce::String& url) const;

    VialEffectsProcessor& processorRef;

    std::vector<std::unique_ptr<juce::WebSliderRelay>> sliderRelays;
    std::vector<std::unique_ptr<juce::WebToggleButtonRelay>> toggleRelays;
    std::vector<std::unique_ptr<juce::WebComboBoxRelay>> comboRelays;

    std::vector<std::unique_ptr<juce::WebSliderParameterAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<juce::WebToggleButtonParameterAttachment>> toggleAttachments;
    std::vector<std::unique_ptr<juce::WebComboBoxParameterAttachment>> comboAttachments;

    std::unique_ptr<juce::WebBrowserComponent> webView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VialEffectsEditor)
};
