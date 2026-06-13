# Changelog

All notable changes to this project are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **DSP core** extracted from the [vial](../vial) synth, preserving the exact sound:
  - `Reverb` and `Delay` effect Processors plus their minimal `poly_float` SIMD
    framework, vendored under `src/dsp/vial/` (organized into
    `framework/`, `effects/`, `filters/`, `lookups/`, `common/`).
  - `Chorus` — standalone re-implementation of vial's `ChorusModule` multi-voice
    modulation over real `MultiDelay`s.
  - `EffectsEngine` — chains Chorus → Delay → Reverb, applies vial's value
    scaling and tempo-sync, and drives each effect's control inputs.
- **Parameter spec** (`EffectParameters.h`) — single source of truth for all 34
  parameters (ranges, defaults, scales) copied from vial's `synth_parameters.cpp`.
- **Plugin** (`VialEffectsProcessor`) — stereo VST3/AU/Standalone effect with an
  `AudioProcessorValueTreeState` parameter tree and host-tempo support.
- **Web UI editor** (`VialEffectsEditor`) — JUCE 8 `WebBrowserComponent` (WebView2
  backend) host with per-parameter relays (`WebSliderRelay` /
  `WebToggleButtonRelay` / `WebComboBoxRelay`) bound to the APVTS, serving the
  bundled front-end via a BinaryData resource provider.
- **React + Vite web UI** matching the vial effects rack: three colour-coded
  panels (Chorus/Delay/Reverb), SVG knobs with arc indicators, value/tempo boxes,
  delay-mode combo, power dots, and param-reactive canvas visualizers. Bundled as
  a single self-contained `index.html`.
- **CMake build**: `VialEffects` plugin (VST3/AU/Standalone, `NEEDS_WEBVIEW2`) and
  a `VialEffectsTests` console app; reuses the sibling `vial/JUCE` tree.
- **Automated tests**: native JUCE `UnitTest` (parameter scaling + engine smoke:
  finite/bounded output, exact bypass, tail decay) and web UI tests (11 Vitest
  component/param tests + 3 Playwright e2e/visual tests with a committed baseline).
- Verified end-to-end: builds clean, all tests pass, and the Standalone renders
  the WebView UI with correct parameter values
  ([docs/standalone-screenshot.png](docs/standalone-screenshot.png)).

### Notes
- On Windows the editor explicitly selects the WebView2 backend; JUCE's default
  there is the legacy IE backend, whose navigation does not invoke the resource
  provider.
- `synth_constants.h` gained an explicit `#include "value.h"` so the vendored
  header is self-contained.

[Unreleased]: https://example.com/vial-effects/tree/master
