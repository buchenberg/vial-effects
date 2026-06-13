# Vial Effects

The chorus, delay, and reverb effects from the [vial](../vial) synthesiser,
extracted into a standalone audio effect plugin with an **entirely web-based UI**.

The DSP is vendored from vial unchanged, so the effects sound identical to the
synth. The user interface is a React app rendered in a JUCE 8
`WebBrowserComponent`, styled after vial's effects rack (three stacked panels:
Chorus, Delay, Reverb).

> **Status:** Builds and runs (VST3 + Standalone on Windows; AU on macOS). The
> DSP, plugin, WebView UI, and tests are all in place. See
> [CHANGELOG.md](CHANGELOG.md) and the screenshot at
> [docs/standalone-screenshot.png](docs/standalone-screenshot.png).

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

Prerequisites: CMake ≥ 3.22, a C++17 toolchain (MSVC on Windows), Node 18+, and
the **WebView2 SDK** (Windows). The build reuses the JUCE tree from the sibling
`vial` checkout (`../vial/JUCE`).

```bash
# 1. Build the web UI (produces ui/dist/index.html, embedded as binary data)
cd ui && npm install && npm run build && cd ..

# 2. Configure + build the plugin (Windows: run inside a VS dev shell / vcvars)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DJUCE_WEBVIEW2_PACKAGE_LOCATION=<dir containing Microsoft.Web.WebView2.*>
cmake --build build
```

On Windows the WebView2 NuGet package must be extracted into a
`Microsoft.Web.WebView2.<version>/` subfolder under the path passed to
`JUCE_WEBVIEW2_PACKAGE_LOCATION`. The editor explicitly selects the WebView2
backend (the JUCE default on Windows is the legacy IE backend, which the resource
provider does not support).

### Tests

```bash
ctest --test-dir build          # native DSP tests (also: build/.../VialEffectsTests.exe)
cd ui && npm test && npm run e2e # Vitest component tests + Playwright e2e/visual
```

## Licence & attribution

Derived from vial, which is itself based on Matt Tytel's **Vital**. The vendored
DSP retains its original GPLv3 headers, so this project is distributed under the
**GNU General Public License v3** — see [LICENSE](LICENSE).
