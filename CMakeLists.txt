cmake_minimum_required(VERSION 3.21)

project(FunkyMooseViz VERSION 0.5.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================
# JUCE
# ==============================

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    find_package(JUCE CONFIG REQUIRED)
endif()

# ==============================
# Binary Assets
# ==============================

juce_add_binary_data(FunkyMooseVizAssets
    SOURCES
        Source/Assets/image.png
)

# ==============================
# Plugin Definition
# ==============================

juce_add_plugin(FunkyMooseViz
    COMPANY_NAME "Uwe Felchle"
    PRODUCT_NAME "Funky Moose Viz"

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE

    COPY_PLUGIN_AFTER_BUILD TRUE

    PLUGIN_MANUFACTURER_CODE UweF
    PLUGIN_CODE FkVz
    BUNDLE_ID "at.uwefelchle.funkymooseviz"

    FORMATS AU VST3 Standalone

    ICON_BIG Source/Assets/image.png  # Changed icon path to actual asset - Icon image must exist here

    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Funky Moose Viz benötigt Audioeingang für die Anzeige."
)

# ==============================
# Source Files
# ==============================

# Core module source files
set(FunkyMooseViz_SOURCES
    # Core
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.h

    # Analysis
    Source/Analysis/FFTProcessor.cpp
    Source/Analysis/FFTProcessor.h
    Source/Analysis/WaveformBuffer.cpp
    Source/Analysis/WaveformBuffer.h
    Source/Analysis/LevelMeterSource.cpp
    Source/Analysis/LevelMeterSource.h

    # Components
    Source/Components/SpectrumComponent.cpp
    Source/Components/SpectrumComponent.h
    Source/Components/WaveformComponent.cpp
    Source/Components/WaveformComponent.h
    Source/Components/MeterComponent.cpp
    Source/Components/MeterComponent.h
    Source/Components/ElchComponent.cpp
    Source/Components/ElchComponent.h
)

target_sources(FunkyMooseViz PRIVATE ${FunkyMooseViz_SOURCES})

target_include_directories(FunkyMooseViz PRIVATE Source)  # Added to include Source directory headers

# Organisiert die Dateien in der IDE (Xcode/VS) in der echten Ordnerstruktur
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source" PREFIX "Source" FILES ${FunkyMooseViz_SOURCES})

# ==============================
# Compiler Definitions
# ==============================

target_compile_definitions(FunkyMooseViz PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# ==============================
# Linking
# ==============================

target_link_libraries(FunkyMooseViz PRIVATE
    FunkyMooseVizAssets
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# ==============================

