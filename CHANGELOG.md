# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.4.0] - 2026-03-18
### Added
- New **Spectrum Analyzer** features (Waterfall mode).
- **Musical Pitch Detection** integration.
- **Stereo Correlation Meter** (Vectorscope).
- **Master Analysis Frame Counter** for synchronized GUI updates.
- Refactored **WaveformBuffer** for real-time safety.
- New professional **README** with banner and badges.

### Optimized
- **Waterfall Rendering Performance**: Implemented logarithmic bin lookup table to eliminate per-pixel `log10` and `pow` calculations in the GUI thread.

### Changed
- Improved GUI performance and responsiveness.
- Updated project structure for better CMake integration.

## [0.3.0] - Prior to March 2026
### Added
- Initial core visualization modules (Spectrum, Waveform).
- Basic JUCE plugin architecture.
- CMake build system support.
