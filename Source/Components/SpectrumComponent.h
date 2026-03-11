#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>
#include <deque>

class SpectrumComponent : public juce::Component
{
public:
    enum class DisplayMode
    {
        LR,
        MS
    };

    SpectrumComponent();

    void setMagnitudesLeft (const std::vector<float>& newMagnitudes);
    void setMagnitudesRight (const std::vector<float>& newMagnitudes);
    void setAnalysisInfo (double newSampleRate, int newFFTSize);
    void setDisplayMode (DisplayMode newMode);
    void setFrozen (bool shouldBeFrozen);
    void resetPeakTrace();
    void mouseDoubleClick (const juce::MouseEvent&) override;

    void paint (juce::Graphics& g) override;

private:
    float frequencyToX (float frequency, float left, float width) const;
    float binToFrequency (int binIndex) const;
    void smoothInto (const std::vector<float>& input, std::vector<float>& target);
    void updatePeakTrace (std::vector<float>& trace, const std::vector<float>& source);

    std::vector<float> smoothedLeft;
    std::vector<float> smoothedRight;
    std::vector<float> peakTraceLeft;
    std::vector<float> peakTraceRight;

    // Spectrogram - Waterfall
    static constexpr int maxSpectrogramFrames = 64;
    std::deque<std::vector<float>> spectrogramFrames;

    double sampleRate = 44100.0;
    int fftSize = 2048;
    DisplayMode displayMode = DisplayMode::LR;
    bool frozen = false;

    static constexpr float minFrequency = 20.0f;
    static constexpr float maxFrequency = 20000.0f;
};
