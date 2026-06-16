cmake_minimum_required(VERSION 3.22)
project(VialEffects VERSION 0.1.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE 8 tree lives in third_party/JUCE
set(VIAL_JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/JUCE" CACHE PATH "Path to JUCE")
add_subdirectory("${VIAL_JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")

# ----------------------------------------------------------------------------
# Shared source lists
# ----------------------------------------------------------------------------
# Vendored vial DSP (.cpp only; headers are found via include dirs).
set(VIAL_DSP_SOURCES
    src/dsp/vial/framework/utils.cpp
    src/dsp/vial/framework/processor.cpp
    src/dsp/vial/framework/processor_router.cpp
    src/dsp/vial/framework/feedback.cpp
    src/dsp/vial/framework/value.cpp
    src/dsp/vial/framework/operators.cpp
    src/dsp/vial/effects/reverb.cpp
    src/dsp/vial/effects/delay.cpp)

# Our DSP wrappers.
set(VFX_DSP_SOURCES
    src/dsp/Chorus.cpp
    src/dsp/EffectsEngine.cpp)

set(VFX_INCLUDE_DIRS
    src
    src/dsp
    src/dsp/vial/framework
    src/dsp/vial/effects
    src/dsp/vial/filters
    src/dsp/vial/lookups
    src/dsp/vial/common)

# ----------------------------------------------------------------------------
# Plugin target
# ----------------------------------------------------------------------------
juce_add_plugin(VialEffects
    PRODUCT_NAME "Vial Effects"
    COMPANY_NAME "buchenberg"
    BUNDLE_ID "com.buchenberg.vialeffects"
    PLUGIN_MANUFACTURER_CODE Bchb
    PLUGIN_CODE Vfx1
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    NEEDS_WEBVIEW2 TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS VST3 AU Standalone
    VST3_CATEGORIES Fx Modulation
    AU_MAIN_TYPE kAudioUnitType_Effect)

juce_generate_juce_header(VialEffects)

target_sources(VialEffects PRIVATE
    src/plugin/PluginProcessor.cpp
    src/plugin/PluginEditor.cpp
    ${VFX_DSP_SOURCES}
    ${VIAL_DSP_SOURCES})

target_include_directories(VialEffects PRIVATE ${VFX_INCLUDE_DIRS})

target_compile_definitions(VialEffects PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1)

# --- Web UI bundled as binary data ---------------------------------------
# Use the Vite build output if present; otherwise embed a placeholder page so
# the plugin still links before the UI has been built.
file(GLOB_RECURSE UI_DIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ui/dist/*")
if(NOT UI_DIST)
    set(_placeholder "${CMAKE_BINARY_DIR}/ui_placeholder/index.html")
    file(WRITE "${_placeholder}"
        "<!doctype html><html><body style=\"background:#1d1d24;color:#aaa;font-family:sans-serif\">\
<p style=\"padding:2rem\">Vial Effects UI not built yet. Run <code>npm install &amp;&amp; npm run build</code> in <code>ui/</code>.</p></body></html>")
    set(UI_DIST "${_placeholder}")
endif()

juce_add_binary_data(VialEffectsUI SOURCES ${UI_DIST})
set_target_properties(VialEffectsUI PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries(VialEffects PRIVATE
    VialEffectsUI
    juce::juce_audio_utils
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

# ----------------------------------------------------------------------------
# Native test target (DSP only — no GUI, no WebView, no binary data)
# ----------------------------------------------------------------------------
juce_add_console_app(VialEffectsTests PRODUCT_NAME "Vial Effects Tests")
juce_generate_juce_header(VialEffectsTests)

target_sources(VialEffectsTests PRIVATE
    tests/TestMain.cpp
    ${VFX_DSP_SOURCES}
    ${VIAL_DSP_SOURCES})

target_include_directories(VialEffectsTests PRIVATE ${VFX_INCLUDE_DIRS})

target_compile_definitions(VialEffectsTests PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_UNIT_TESTS=1)

target_link_libraries(VialEffectsTests PRIVATE
    juce::juce_audio_basics
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags)

enable_testing()
add_test(NAME VialEffectsTests COMMAND VialEffectsTests)

# ----------------------------------------------------------------------------
# Windows installer (WiX)
# ----------------------------------------------------------------------------
if(WIN32)
    find_program(CANDLE_EXECUTABLE candle
        HINTS "C:/Program Files (x86)/WiX Toolset v3.14/bin")

    find_program(LIGHT_EXECUTABLE light
        HINTS "C:/Program Files (x86)/WiX Toolset v3.14/bin")

    if(CANDLE_EXECUTABLE AND LIGHT_EXECUTABLE)
        set(WXS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/installer/VialEffects.wxs")
        add_custom_target(installer
            COMMENT "Building Windows installer with WiX..."
            COMMAND "${CANDLE_EXECUTABLE}" -arch x64 -ext WixUIExtension
                    -dConfiguration=$<CONFIG>
                    -dBuildDir="${CMAKE_BINARY_DIR}"
                    -o "${CMAKE_BINARY_DIR}/installer/"
                    "${WXS_FILE}"
            COMMAND "${LIGHT_EXECUTABLE}" -ext WixUIExtension -sval
                    -o "${CMAKE_BINARY_DIR}/installer/VialEffects-$<CONFIG>-win64.msi"
                    "${CMAKE_BINARY_DIR}/installer/VialEffects.wixobj"
            DEPENDS VialEffects
            SOURCES "${WXS_FILE}"
        )
        message(STATUS "WiX Toolset found: ${CANDLE_EXECUTABLE}")
    else()
        message(STATUS "WiX Toolset not found — installer target disabled.")
    endif()
endif()
