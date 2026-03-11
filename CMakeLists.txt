cmake_minimum_required(VERSION 3.22)

# ==============================================================================
# 1. Project Setup
# ==============================================================================
project(FunkyMooseViz VERSION 0.1.3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# 2. JUCE Framework Integration
# ==============================================================================
# If JUCE is available as a subdirectory (e.g. in CI or a local clone), use it.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    find_package(JUCE CONFIG REQUIRED)
endif()

# ==============================================================================
# 3. Binary Assets
# ==============================================================================
juce_add_binary_data(FunkyMooseVizAssets
    SOURCES
        Source/Assets/image.png
)

# ==============================================================================
# 4. Plugin Metadata
# ==============================================================================
juce_add_plugin(FunkyMooseViz
    COMPANY_NAME "Uwe Felchle"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE UweF
    PLUGIN_CODE FmVz
    BUNDLE_ID "at.uwefelchle.funkymooseviz"
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Funky Moose Viz"
    VERSION "0.1.3"  # Explicitly sync version with project level
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Funky Moose Viz benötigt Audioeingang für die Analyzer-Anzeige."
    ICON_BIG "Source/Assets/FunkyMoose.icns"
)

# ==============================================================================
# 5. Source Files
# ==============================================================================
target_sources(FunkyMooseViz PRIVATE
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

# ==============================================================================
# 6. Build Configuration
# ==============================================================================
target_compile_definitions(FunkyMooseViz PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(FunkyMooseViz PRIVATE
    FunkyMooseVizAssets
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
