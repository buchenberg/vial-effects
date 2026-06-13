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
- **Web UI editor** (`VialEffectsEditor`) — JUCE 8 `WebBrowserComponent` host with
  per-parameter relays (`WebSliderRelay` / `WebToggleButtonRelay` /
  `WebComboBoxRelay`) bound to the APVTS, serving the bundled front-end.

### Pending
- React + Vite web UI matching the vial effects panel layout.
- CMake build wiring (DSP sources, UI binary data, WebView2).
- Automated tests: Vitest + Playwright (UI) and JUCE `UnitTest` (DSP).
- First successful build and verification.

[Unreleased]: https://example.com/vial-effects/tree/master
