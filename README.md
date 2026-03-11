# 🦌 Funky Moose Viz
**Boutique-style JUCE audio visualizer plugin for spectrum, waveform, vectorscope and pitch detection.**

![Funky Moose Viz UI](screenshot.png)

## 🤘 Why it exists
Most visualizers are either too clinical or too bloated. **Funky Moose Viz** was built to provide a focused, high-performance analysis tool that looks like high-end studio hardware. It's the perfect companion for the **Funky Moose Amp**, bringing the same metallic aesthetic and "funky" energy to your master bus or instrument tracks.

## ✨ Key Features
- **Pro Spectrum Analyzer**: Stereo & Mid/Side modes with a logarithmic scale (20Hz - 20kHz).
- **Peak-Hold (Pro)**: Intelligent peak lines that stay at the maximum for a moment before decaying, helping you spot resonances.
- **Waterfall Spectrogram**: See the frequency history over time for better context.
- **Vector Scope (Goniometer)**: Precise phase and stereo image visualization.
- **Waveform Monitor**: Real-time signal monitoring.
- **High-End Pitch Detection**: Sub-bin accurate frequency detection with musical note translation.
- **Dynamic Moose API**: Our mascot pulses and glows in sync with your audio levels.
- **State Persistence**: Remembers your settings (M/S mode, Range, etc.) between sessions.

## 🎧 Typical Use Cases
- **Check your Low-End**: Use the high-precision spectrum to ensure your Kick and Bass are sitting perfectly in the mix.
- **Stereo Image Control**: Use the Vector Scope and M/S mode to identify phase issues or excessive side-signal buildup.
- **Visual Confidence**: Get an immediate feel for the "punch" of your track through the synchronized animations and meters.

## 🛠 Installation
### For Users
1. Download the latest version from the **[Releases](https://github.com/blubass/FunkyMooseViz/releases)** page.
2. Move the `Funky Moose Viz.vst3` or `.component` file to your plugin folder:
   - **macOS (AU)**: `/Library/Audio/Plug-Ins/Components/`
   - **macOS (VST3)**: `/Library/Audio/Plug-Ins/VST3/`
   - **Windows (VST3)**: `C:\Program Files\Common Files\VST3\`
3. Restart your DAW and rescan plugins.

### For Developers
1. Clone the repo: `git clone https://github.com/blubass/FunkyMooseViz`
2. Open with VS Code (using CMake Tools) or CLion.
3. Build the project. The build system will automatically handle JUCE and assets.

## 📋 Current Formats
- **VST3** (64-bit)
- **Audio Unit (AU)**
- **Standalone Application**

## ⚠️ Known Limitations
- Pitch detection works best on monophonic signals (Bass, Vocals, Lead).
- Spectrogram frequency resolution is linked to the FFT size.

## 🗺 Roadmap
- [ ] Adjustable FFT sizes for higher resolution.
- [ ] Custom color themes.
- [ ] Preset Management for analysis setups.

---
**Built with [JUCE](https://juce.com/).**  
*Copyright (c) 2026 blubass*
