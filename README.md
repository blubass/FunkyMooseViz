


![Funky Moose Viz](docs/banner.png)

# Funky Moose Viz

**Real-time audio visualization tools for JUCE plugins and audio apps.**

![JUCE](https://img.shields.io/badge/JUCE-framework-orange)
![C++](https://img.shields.io/badge/C++-17-blue)
![OpenGL](https://img.shields.io/badge/OpenGL-rendering-green)
![FFT](https://img.shields.io/badge/FFT-audio%20analysis-purple)
![Platform](https://img.shields.io/badge/macOS-Windows-black)
![Moose Powered](https://img.shields.io/badge/Moose%20Powered-Funky-darkgreen?style=for-the-badge)

## Features

• **Spectrum analyzer** - High-resolution real-time frequency analysis  
• **Audio visualization components** - Modular and reusable UI elements  
• **Real-time FFT processing** - Low-latency windowed FFT analysis  
• **Modern JUCE GUI** - Custom rendering with a vintage/boutique feel  
• **Plugin / Standalone architecture** - Use as a VST3/AU plugin or independent app

---

### 🎛 Detailed Capabilities
- **Real-time Spectrum Analyzer**
- **Waveform display**
- **Vector Scope** for stereo visualization
- **Musical Pitch Detection**
- **Vintage / boutique inspired UI**
- **Lightweight JUCE architecture**
- **Cross-platform builds**

**Supported plugin formats:**
- **AU** (macOS)
- **VST3** (macOS / Windows)
- **Standalone application**

---

### 🎯 Typical Use Cases

#### Bass Monitoring
Check fundamental frequencies, overtone balance and pitch stability.

#### Stereo Image Control
Visualize stereo width and phase coherence with the vectorscope.

#### Mix Bus Insight
Understand spectral balance and low-end behaviour at a glance.

#### Sound Design
Track harmonic movement and transient structure in real time.

---

### 🖥 Screenshot
![Funky Moose Viz Screenshot](screenshot.png)

---

### 🛠 Build From Source

**Requirements:**
- **JUCE 8** (Latest stable)
- **CMake 3.20+**
- **Compiler:** Xcode (macOS), MSVC 2022 (Windows), or GCC/Clang (Linux)

#### Quick Start (CLI)
```bash
# 1. Clone the repository
git clone https://github.com/blubass/FunkyMooseViz.git
cd FunkyMooseViz

# 2. Configure CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. Build the project
cmake --build build --config Release
```

The resulting binaries (VST3, AU, Standalone) will be located in the `build/FunkyMooseViz_artefacts` directory.

---

### 🏷 Repository Topics
If you are managing this repo on GitHub, consider adding these topics for better discoverability:
`juce` • `audio-visualization` • `fft` • `spectrum-analyzer` • `audio-plugin` • `dsp` • `opengl` • `cmake` • `audio-software`

---

### ⚠️ Notes
- Pitch detection works best with monophonic signals.
- GUI performance may vary on older systems.
- Further refinements are planned.

---

### 🚀 Roadmap
Future improvements may include:
- improved pitch detection stability
- refined spectrum visualization
- GUI performance optimizations
- additional user customization

---

### 🧠 Built With
- JUCE Framework
- Modern C++
- CMake build system

---

## 🇩🇪 Deutsch

### Funky Moose Viz
**Echtzeit-Audio-Visualisierungstools für JUCE-Plugins und Audio-Apps.**

Funky Moose Viz ist ein mit JUCE entwickeltes Audio-Plugin, das Spektrumanalyse, Wellenform-Anzeige, Vektorskop und musikalische Pitch-Erkennung in einer gemeinsamen Oberfläche kombiniert.

---

### Funktionen
- Echtzeit Spektrumanalyse
- Wellenform-Anzeige
- Vektorskop zur Stereo-Visualisierung
- Pitch-Erkennung
- Vintage-inspirierte Benutzeroberfläche
- Schlanke JUCE-Architektur

---

### 🧠 Philosophie
> Klang wird nicht nur gehört. Er wird gesehen, gespürt und verstanden. Funky Moose Viz verbindet diese Ebenen.

---

### Lizenz
MIT License

---
*Copyright (c) 2026 blubass*
