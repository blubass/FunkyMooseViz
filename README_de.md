# Funky Moose Amp

![Funky Moose Amp](docs/banner.png)

**Moderner Bass-Amp für Groove, Klarheit, Punch und Charakter. Gebaut für musikalisches Spielgefühl, flexible Klangformung und eigenständige visuelle Präsenz.**

[![Stay Funky](https://img.shields.io/badge/Stay%20Funky-Moose%20Powered-darkgreen?style=for-the-badge)](#)
[![JUCE](https://img.shields.io/badge/JUCE-framework-orange)](#)
[![C++](https://img.shields.io/badge/C++-17-blue)](#)
[![VST3](https://img.shields.io/badge/VST3-plugin-green)](#)
[![AudioUnit](https://img.shields.io/badge/AU-macOS-lightgrey)](#)
[![Release](https://img.shields.io/github/v/release/blubass/FunkyMooseAmp)](https://github.com/blubass/FunkyMooseAmp/releases)

*Englische Version siehe [README.md](README.md)*

**Funky Moose Amp** ist ein klarer und druckvoller Bass-Amp für **Groove, Funk, Slap und moderne Basssounds**.  
Entwickelt mit **JUCE** und **C++**, verbindet das Plugin musikalische Klangformung mit direkter Ansprache, Charakter und einer eigenständigen visuellen Präsenz.

Kein generischer Amp-Simulator.  
Sondern ein musikalisches Werkzeug für Spielerinnen und Spieler, die **Klarheit, Dynamik und Ausdruck** suchen.

---

## Features

### Premium Bass DSP
- hochwertig abgestimmte Amp-Sektion mit großem Dynamikumfang
- Fokus auf **Klarheit, Punch und musikalische Tonformung**
- reagiert direkt und lebendig auf Spielweise und Anschlag

### Integrierter Tuner
- hochpräziser chromatischer Tuner
- direkt in die Oberfläche eingebaut
- schnelles Stimmen ohne externes Zusatztool

### MIDI Learn
- schnelle Zuweisung von Reglern und Schaltern
- ideal für Hardware-Controller und Live-Anwendungen

### 9 auswählbare Skins
- verschiedene Looks für unterschiedliche Stimmungen und Arbeitsweisen
- von klassisch bis modern

### ModFX
- **Octaver** für satte Sub- und Effektklänge
- **Envelope Filter** für dynamische, anschlagabhängige Filterbewegung
- **Phaser**
- **Chorus**

### Saturation & Color
- **Tube Mode** für harmonische Wärme und leichtes Anzerren
- **Slap Mode** für mehr Biss und frische Präsenz

### Interactive Visual Engine
- die Oberfläche reagiert auf Signalpegel und Kompression in Echtzeit
- der Moose ist nicht nur Deko, sondern Teil des Charakters

### DSP Safety
- Schutzmechanismen gegen problematische Peaks
- Soft-Clipping
- intelligente Noise- und NaN-Sicherheit

---
![Funky Moose Viz Screenshot](screenshot.png)

## Download

Die aktuelle stabile Version findest du auf der [Releases-Seite](https://github.com/blubass/FunkyMooseAmp/releases).

---

## Installation

### macOS
1. **Audio Unit:** `.component` nach `/Library/Audio/Plug-Ins/Components` kopieren  
2. **VST3:** `.vst3` nach `/Library/Audio/Plug-Ins/VST3` kopieren  
3. **Standalone:** `.app` in den Ordner `Programme` verschieben  

**Hinweis:**  
Da das Plugin aktuell nicht mit einem Apple-Developer-Zertifikat signiert ist, musst du es eventuell per Rechtsklick → **Öffnen** starten oder in **Datenschutz & Sicherheit** freigeben.

### Windows
1. **VST3:** `.vst3` nach `C:\Program Files\Common Files\VST3` kopieren

---

## Build from Source

### Voraussetzungen
- **CMake** 3.22 oder neuer
- **JUCE 8**
- ein **C++17**-fähiger Compiler wie Xcode, MSVC oder GCC

### Build-Schritte

```bash
# Repository klonen
git clone https://github.com/blubass/FunkyMooseAmp.git
cd FunkyMooseAmp

# Konfigurieren
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Bauen
cmake --build build --config Release
Lizenz

Dieses Projekt steht unter der MIT License.

Autor

Uwe Arthur Felchle
Musiker, Komponist und Entwickler
uwefelchle.at
