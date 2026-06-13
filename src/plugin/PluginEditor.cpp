#include "PluginEditor.h"
#include "BinaryData.h"

using vfx::kParams;

namespace {
juce::String mimeForExtension (const juce::String& path) {
    if (path.endsWith (".html")) return "text/html";
    if (path.endsWith (".js") || path.endsWith (".mjs")) return "text/javascript";
    if (path.endsWith (".css")) return "text/css";
    if (path.endsWith (".json") || path.endsWith (".map")) return "application/json";
    if (path.endsWith (".svg")) return "image/svg+xml";
    if (path.endsWith (".png")) return "image/png";
    if (path.endsWith (".woff2")) return "font/woff2";
    return "application/octet-stream";
}

bool isOn (const vfx::ParamSpec& p)    { return juce::String (p.id).endsWith ("_on"); }
bool isCombo (const vfx::ParamSpec& p) { return juce::String (p.id) == "delay_style" || juce::String (p.id).endsWith ("_sync"); }
} // namespace

VialEffectsEditor::VialEffectsEditor (VialEffectsProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p) {
    using namespace juce;

    auto options = WebBrowserComponent::Options{}
                       .withBackend (WebBrowserComponent::Options::Backend::webview2)
                       .withWinWebView2Options (
                           WebBrowserComponent::Options::WinWebView2{}
                               .withUserDataFolder (juce::File::getSpecialLocation (juce::File::tempDirectory)
                                                        .getChildFile ("VialEffectsWebView")))
                       .withNativeIntegrationEnabled()
                       .withResourceProvider ([this] (const auto& url) { return getResource (url); });

    // Create a relay per parameter and register it with the WebView options.
    for (const auto& spec : kParams) {
        if (isOn (spec)) {
            auto relay = std::make_unique<WebToggleButtonRelay> (spec.id);
            options = options.withOptionsFrom (*relay);
            toggleRelays.push_back (std::move (relay));
        } else if (isCombo (spec)) {
            auto relay = std::make_unique<WebComboBoxRelay> (spec.id);
            options = options.withOptionsFrom (*relay);
            comboRelays.push_back (std::move (relay));
        } else {
            auto relay = std::make_unique<WebSliderRelay> (spec.id);
            options = options.withOptionsFrom (*relay);
            sliderRelays.push_back (std::move (relay));
        }
    }

    webView = std::make_unique<WebBrowserComponent> (std::move (options));
    addAndMakeVisible (*webView);

    // Attach each relay to its APVTS parameter.
    size_t toggleIdx = 0, comboIdx = 0, sliderIdx = 0;
    for (const auto& spec : kParams) {
        auto* param = processorRef.apvts.getParameter (spec.id);
        if (param == nullptr)
            continue;

        if (isOn (spec)) {
            toggleAttachments.push_back (std::make_unique<WebToggleButtonParameterAttachment> (
                *param, *toggleRelays[toggleIdx++], processorRef.apvts.undoManager));
        } else if (isCombo (spec)) {
            comboAttachments.push_back (std::make_unique<WebComboBoxParameterAttachment> (
                *param, *comboRelays[comboIdx++], processorRef.apvts.undoManager));
        } else {
            sliderAttachments.push_back (std::make_unique<WebSliderParameterAttachment> (
                *param, *sliderRelays[sliderIdx++], processorRef.apvts.undoManager));
        }
    }

    webView->goToURL (WebBrowserComponent::getResourceProviderRoot());

    setResizable (true, true);
    setResizeLimits (640, 480, 1920, 1500);
    setSize (1000, 780);
}

VialEffectsEditor::~VialEffectsEditor() = default;

void VialEffectsEditor::resized() {
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

std::optional<juce::WebBrowserComponent::Resource>
VialEffectsEditor::getResource (const juce::String& url) const {
    auto path = url;
    if (path.startsWith ("/"))
        path = path.substring (1);
    if (path.isEmpty())
        path = "index.html";

    juce::String filename = path.fromLastOccurrenceOf ("/", false, false);
    if (filename.isEmpty())
        filename = "index.html";

    for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {
        const char* resourceName = BinaryData::namedResourceList[i];
        if (juce::String (BinaryData::getNamedResourceOriginalFilename (resourceName)) == filename) {
            int dataSize = 0;
            const char* data = BinaryData::getNamedResource (resourceName, dataSize);
            std::vector<std::byte> bytes (reinterpret_cast<const std::byte*> (data),
                                          reinterpret_cast<const std::byte*> (data) + dataSize);
            return juce::WebBrowserComponent::Resource { std::move (bytes), mimeForExtension (filename) };
        }
    }
    return std::nullopt;
}
