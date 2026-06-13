# Vial Effects

The chorus, delay, and reverb effects from the [vial](../vial) synthesiser,
extracted into a standalone audio effect plugin with an **entirely web-based UI**.

The DSP is vendored from vial unchanged, so the effects sound identical to the
synth. The user interface is a React app rendered in a JUCE 8
`WebBrowserComponent`, styled after vial's effects rack (three stacked panels:
Chorus, Delay, Reverb).

> **Status: work in progress.** The C++ (DSP + plugin + WebView editor) is
> written; the React UI, CMake build, and tests are not yet in place, so the
> project does not build yet. See [CHANGELOG.md](CHANGELOG.md).

## Architecture

```text
audio in ─▶ Chorus ─▶ Delay ─▶ Reverb ─▶ audio out
                 ▲        ▲         ▲
            APVTS params (scaled to engine units, tempo-synced)
                 ▲
        WebView UI  ◀── relays/attachments ──▶  AudioProcessorValueTreeState
```

- **`src/dsp/vial/`** — DSP vendored from vial (`Reverb`, `Delay`, and the
  `poly_float` Processor framework they need). Grouped as `framework/`,
  `effects/`, `filters/`, `lookups/`, `common/`.
- **`src/dsp/`** — `Chorus`, `EffectsEngine`, and `EffectParameters.h`
  (the shared parameter table, mirroring vial's `synth_parameters.cpp`).
- **`src/plugin/`** — `VialEffectsProcessor` (APVTS, audio) and
  `VialEffectsEditor` (WebView, parameter relays).
- **`ui/`** — React + Vite front-end *(pending)*.

## Building

*(Pending — CMakeLists not yet written.)* The plan is a standard JUCE 8
`juce_add_plugin` target (`VST3 AU Standalone`, `NEEDS_WEBVIEW2`) referencing the
JUCE tree from the sibling `vial` checkout, with the Vite build bundled as
binary data.

## Licence & attribution

Derived from vial, which is itself based on Matt Tytel's **Vital**. The vendored
DSP retains its original GPLv3 headers, so this project is distributed under the
**GNU General Public License v3** — see [LICENSE](LICENSE).
