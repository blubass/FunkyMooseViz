# Funky Moose Viz

A custom audio visualizer plugin designed to match the aesthetic of the **Funky Moose Amp**. Built with JUCE.

![Funky Moose Viz UI](screenshot.png)

## Features

- **Spectrum Analyzer**: Stereo/MS modes with logarithmic frequency scale and waterfall spectrogram.
- **Wellenform & Vector Scope**: Real-time waveform display and phase correlation (Goniometer).
- **Pitch Detection**: Dominant frequency display with note conversion (e.g., E1, A0).
- **Dynamic Elch (Moose)**: Animated logo that pulses and glows based on audio input levels.
- **Vintage Aesthetic**: Metallic beveled frames and a custom color palette inspired by boutique hardware.

## Installation

1. Clone the repository.
2. Open `CMakeLists.txt` in your IDE (VS Code, CLion) or use CMake directly.
3. Build the project.
4. The artifacts will be located in the `build/FunkyMooseViz_artefacts` directory.

## Releases (Mac & Windows)

This project uses **GitHub Actions** to automatically build the plugin for both macOS and Windows.
- To get the latest binaries, check the [Releases](https://github.com/blubass/FunkyMooseViz/releases) page.
- New releases are automatically generated whenever a version tag (e.g., `v0.1.0`) is pushed to the repository.

## License

Copyright (c) 2026 blubass
