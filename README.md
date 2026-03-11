# 🦌 Funky Moose Viz
**🫎 A musical visualizer for bass players, mix engineers and sound designers — built with character, not just code.**

Funky Moose Viz is a custom-built JUCE plugin designed to visually support musical decision making. It combines spectrum analysis, waveform display, vectorscope and pitch detection in a cohesive, character-driven interface.

Built to complement the **Funky Moose Amp** — but powerful enough to stand on its own.

---

## ✨ Why this exists
Most visualizers are clinical tools. **Funky Moose Viz is different.**

It is designed for musicians, bass players and mix engineers who want visual feedback that feels musical — not laboratory sterile. The goal is not just to measure sound. The goal is to **understand** it.

---

## 🎛 Features
- **Real-time Spectrum Analyzer**: Detailed frequency overview.
- **Waveform Display**: Transient and dynamic monitoring.
- **Vector Scope**: Stereo / Mid-Side aware phase visualization.
- **Musical Pitch Detection**: High-accuracy note identification.
- **Boutique-inspired UI**: Metallic, hardware-driven aesthetic.
- **Lightweight**: Optimized JUCE-based architecture.
- **Cross-Platform**: AU / VST3 / Standalone builds.

---

## 🎯 Typical Use Cases
### 🎸 Bass Monitoring
Check fundamental clarity, overtone balance and pitch stability.

### 🎚 Stereo Image Control
Visualize stereo width and phase coherence using the vectorscope.

### 🥁 Mix Bus Insight
Quick visual feedback for low-end energy distribution and overall spectral balance.

### 🧪 Sound Design
Track harmonic development and transient behaviour in real time.

---

## 🖥 Supported Formats
- **AU** (macOS)
- **VST3** (macOS / Windows)
- **Standalone Application**

---

## 📦 Installation
### macOS
1. Download the latest release.
2. Copy the `.component` to: `/Library/Audio/Plug-Ins/Components`
3. Copy the `.vst3` to: `/Library/Audio/Plug-Ins/VST3`
4. Restart your DAW.

### Windows
1. Download the Windows release.
2. Copy the `.vst3` to your VST3 folder (usually): `C:\Program Files\Common Files\VST3`
3. Rescan plugins in your DAW.

---

## 🛠 Build From Source
### Requirements
- CMake
- JUCE 8
- Xcode (macOS) or Visual Studio (Windows)

### Example
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

---

## ⚠️ Known Limitations
- Pitch detection works best on monophonic material.
- GUI performance may vary on older systems.
- Further refinement of Mid/Side visualization planned.

---

## 🚀 Roadmap
- [ ] Improved pitch detection stability.
- [ ] Waterfall spectrum refinement.
- [ ] Optimized GUI rendering.
- [ ] Extended user customization options.

---

## 🧠 Built With
- **JUCE Framework**
- **Modern C++**
- **CMake build system**

---

## 📜 License
MIT License

---

## 🎵 Philosophy
> Sound is not only heard. It is seen, felt and understood. Funky Moose Viz exists to make that connection clearer.

---
*Copyright (c) 2026 blubass*
