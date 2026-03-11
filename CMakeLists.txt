cmake_minimum_required(VERSION 3.15)

project(FunkyMooseViz VERSION 0.1.4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================================================
# JUCE Setup
# =========================================================

# If JUCE is available as a subdirectory (e.g. in CI or a local clone), use it.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    find_package(JUCE CONFIG REQUIRED)
endif()

# =========================================================
# Binary Assets
# =========================================================

juce_add_binary_data(FunkyMooseVizAssets
    SOURCES
        Source/Assets/image.png
)

# =========================================================
# Plugin Definition
# =========================================================

juce_add_plugin(FunkyMooseViz
    COMPANY_NAME "Uwe Felchle"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE

    PLUGIN_MANUFACTURER_CODE UweF
    PLUGIN_CODE FkVz
    BUNDLE_ID "at.uwefelchle.funkymooseviz"

    FORMATS AU VST3 Standalone

    PRODUCT_NAME "Funky Moose Viz"
    VERSION "0.1.4"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Funky Moose Viz benötigt Audioeingang für die Analyzer-Anzeige."
    ICON_BIG "Source/Assets/FunkyMoose.icns"
)

# =========================================================
# Source Files
# =========================================================

target_sources(FunkyMooseViz PRIVATE
    # Main Files
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp

    # Analysis Modules
    Source/Analysis/FFTProcessor.h
    Source/Analysis/FFTProcessor.cpp
    Source/Analysis/WaveformBuffer.h
    Source/Analysis/WaveformBuffer.cpp
    Source/Analysis/LevelMeterSource.h
    Source/Analysis/LevelMeterSource.cpp

    # UI Components
    Source/Components/SpectrumComponent.h
    Source/Components/SpectrumComponent.cpp
    Source/Components/WaveformComponent.h
    Source/Components/WaveformComponent.cpp
    Source/Components/MeterComponent.h
    Source/Components/MeterComponent.cpp
    Source/Components/ElchComponent.h
    Source/Components/ElchComponent.cpp
)

# =========================================================
# Compile Definitions
# =========================================================

target_compile_definitions(FunkyMooseViz PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# =========================================================
# Linking
# =========================================================

target_link_libraries(FunkyMooseViz PRIVATE
    FunkyMooseVizAssets
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
