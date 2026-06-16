# Vial Effects

The chorus, delay, and reverb effects from the [vial](../vial) synthesiser,
extracted into a standalone audio effect plugin with an **entirely web-based UI**.

The DSP is vendored from vial unchanged, so the effects sound identical to the
synth. The user interface is a React app rendered in a JUCE 8
`WebBrowserComponent`, styled after vial's effects rack (three stacked panels:
Chorus, Delay, Reverb).

![Vial Effects UI](docs/Vial_Effects_UI.png)

> **Status:** Builds and runs (VST3 + Standalone on Windows; AU on macOS). The
> DSP, plugin, WebView UI, and tests are all in place. See
> [CHANGELOG.md](CHANGELOG.md).
>
> Known issue: in the standalone, integer parameters (chorus voices, the tempo
> selectors) display their value differently than the dev/test build — the
> WebView relay reports `AudioParameterInt` values in a way the UI reads back as
> the minimum. Float controls are correct. Follow-up needed.

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
the **WebView2 SDK** (Windows). JUCE 8 is cloned into `third_party/JUCE`.

```bash
# 1. Clone JUCE (one-time setup)
git clone --depth 1 https://github.com/juce-framework/JUCE.git third_party/JUCE

# 2. Build the web UI (produces ui/dist/index.html, embedded as binary data)
cd ui && npm install && npm run build && cd ..

# 3. Configure + build the plugin (Windows: run inside a VS dev shell / vcvars)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DJUCE_WEBVIEW2_PACKAGE_LOCATION=third_party/webview2
cmake --build build
```

On Windows the WebView2 NuGet package must be extracted into a
`Microsoft.Web.WebView2.<version>/` subfolder under the path passed to
`JUCE_WEBVIEW2_PACKAGE_LOCATION`. The default location is `third_party/webview2`.
The editor explicitly selects the WebView2
backend (the JUCE default on Windows is the legacy IE backend, which the resource
provider does not support).

### Tests

```bash
ctest --test-dir build          # native DSP tests (also: build/.../VialEffectsTests.exe)
cd ui && npm test && npm run e2e # Vitest component tests + Playwright e2e/visual
```

## Windows Installer

An [Inno Setup](https://jrsoftware.org/isinfo.php) script is included at
`installer/VialEffects.iss`. After building the plugin, run:

```bash
# Build the installer (requires Inno Setup 6+)
iscc /DConfiguration=Release installer\VialEffects.iss
#        ^^^^^^^^ change to Debug if you built the Debug configuration
```

The installer is also wired into CMake as an `installer` target. If Inno Setup
is detected at configure time, building the project also produces the installer:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release ...
cmake --build build              # builds VialEffects + the installer
```

The output `.exe` lands in `build/installer/`.

### Installer features

- **Component selection** — VST3 plugin only, standalone only, or both.
- Installs VST3 to the system-wide VST3 folder (`C:\Program Files\Common Files\VST3\`).
- Installs the standalone to `Program Files\buchenberg\Vial Effects\` with
  Start Menu shortcuts.
- Full uninstall support via Add/Remove Programs.

## macOS Installer

A flat `.pkg` installer is built automatically by CI. To build it locally:

```bash
# After building the plugin:
mkdir -p pkg_root/Library/Audio/Plug-Ins/VST3
mkdir -p pkg_root/Library/Audio/Plug-Ins/Components
mkdir -p pkg_root/Applications

cp -R "build/VialEffects_artefacts/Release/VST3/Vial Effects.vst3" \
  "pkg_root/Library/Audio/Plug-Ins/VST3/"
cp -R "build/VialEffects_artefacts/Release/AU/Vial Effects.component" \
  "pkg_root/Library/Audio/Plug-Ins/Components/"
cp -R "build/VialEffects_artefacts/Release/Standalone/Vial Effects.app" \
  "pkg_root/Applications/"

pkgbuild --root pkg_root \
  --identifier com.buchenberg.vialeffects \
  --version "0.1.0" \
  --install-location / \
  build/installer/VialEffects-0.1.0-macOS.pkg
```

The `.pkg` installs:
- VST3 → `/Library/Audio/Plug-Ins/VST3/Vial Effects.vst3`
- AU → `/Library/Audio/Plug-Ins/Components/Vial Effects.component`
- Standalone → `/Applications/Vial Effects.app`

## CI/CD

On every push to `main`, [GitHub Actions](.github/workflows/build-and-release.yml)
builds installers for all three platforms and creates a GitHub Release with the
artifacts. Pull requests trigger per-platform build checks (no release).

| Platform | Installer | Build Workflow |
|----------|-----------|----------------|
| Windows | `.exe` (Inno Setup) | `build-installer.yml` |
| macOS | `.pkg` (pkgbuild) | `build-installer-macos.yml` |
| Linux | `.tar.gz` | `build-installer-linux.yml` |
| **Release** | all three | `build-and-release.yml` |

## Licence & attribution

Derived from vial, which is itself based on Matt Tytel's **Vital**. The vendored
DSP retains its original GPLv3 headers, so this project is distributed under the
**GNU General Public License v3** — see [LICENSE](LICENSE).
