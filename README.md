# 🦌 Funky Moose Viz
**Funky Moose Viz is a boutique-style visual analysis plugin for bass, stereo image and musical pitch awareness.**

![Funky Moose Viz UI](screenshot.png)

## 🤘 Why it exists
Most visualizers are either too clinical or too bloated. **Funky Moose Viz** provides a focused, high-performance analysis tool that doesn't just measure audio—it visualizes its character. It's the perfect companion for the **Funky Moose Amp**, bringing the same metallic aesthetic and "funky" energy to your master bus or instrument tracks.

*Ein musikalischer Visualizer für Bass, Mix und Stereo-Bild – nicht klinisch, sondern inspirierend.*

## ✨ Key Features
- **Pro Spectrum Analyzer**: Stereo & Mid/Side modes with a logarithmic scale (20Hz - 20kHz).
- **Peak-Hold (Pro)**: Intelligent peak lines that stay at the maximum for a moment before decaying, helping you spot resonances.
- **Waterfall Spectrogram**: See the frequency history over time for better context.
- **Vector Scope (Goniometer)**: Precise phase and stereo image visualization.
- **Waveform Monitor**: Real-time signal monitoring.
- **High-End Pitch Detection**: Sub-bin accurate frequency detection with musical note translation.
- **Dynamic Moose API**: Our mascot pulses and glows in sync with your audio levels.
- **State Persistence**: Remembers your settings (M/S mode, Range, window size, etc.) between sessions.

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

## ⚠️ Known Issues / Limitations
- **Pitch Detection**: Works best on monophonic signals. Complex polyphonic material might cause jitter.
- **CPU Usage**: The high-refresh-rate vector scope can be taxing on older systems.
- **M/S Refinement**: We are still fine-tuning the balance of the Mid/Side visual weights.

## 🗺 Roadmap
- [ ] **Advanced Pitch Engine**: Transitioning to a hybrid time/frequency domain detector for better bass accuracy.
- [ ] **Custom Color Themes**: User-definable palettes for the spectrum and UI accents.
- [ ] **Preset Management**: Save your favorite analysis setups.
- [ ] **Optimization**: Moving path building to background threads for even smoother UI performance.

---
**Built with [JUCE](https://juce.com/).**  
*Copyright (c) 2026 blubass*
