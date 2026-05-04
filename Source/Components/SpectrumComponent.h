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
        MS,
        Waterfall
    };

    enum class WeightingMode
    {
        Flat,
        Tilt3,
        Tilt45,
        A,    // A-Weighting (ISO 226, most common)
        C,    // C-Weighting (for loud SPL)
        D     // D-Weighting (aircraft noise)
    };

    enum class SmoothingMode
    {
        Standard,
        RMS,
        Peak
    };

    enum class ColorScheme
    {
        Funky,
        Calm,
        Midnight,
        Vintage
    };

    SpectrumComponent();

    void setMagnitudesLeft (const std::vector<float>& newMagnitudes);
    void setMagnitudesRight (const std::vector<float>& newMagnitudes);
    void setAnalysisInfo (double newSampleRate, int newFFTSize);
    void setDisplayMode (DisplayMode newMode);
    void setFrozen (bool shouldBeFrozen);
    void setDisplayRange (float newRangeDb);
    void setWeightingMode (WeightingMode newMode);
    void setSmoothingMode (SmoothingMode newMode);
    void setCrestModeEnabled (bool shouldBeEnabled);
    void setColorScheme (ColorScheme newScheme);
    void setFrequencyMarkersEnabled (bool shouldBeEnabled);
    void setNoteGridEnabled (bool shouldBeEnabled);
    void setFrequencyRange (float minFreq, float maxFreq);
    bool captureReferenceTrace();
    void clearReferenceTrace();
    bool hasReferenceTrace() const;
    void setAverageTraceEnabled (bool shouldBeEnabled);
    bool isAverageTraceEnabled() const;
    void resetAverageTrace();
    void setMaskingVisualizationEnabled (bool enabled) { maskingVisualizationEnabled = enabled; repaint(); }

    void setSidechainData (const std::vector<float>& left, const std::vector<float>& right);
    void setSidechainVisible (bool visible) { sidechainVisible = visible; repaint(); }
    bool isSidechainVisible() const { return sidechainVisible; }
    
    void setClashDetectionEnabled (bool enabled) { clashDetectionEnabled = enabled; repaint(); }
    bool isClashDetectionEnabled() const { return clashDetectionEnabled; }
    void setDetectedNote (const juce::String& note, float frequency);
    void resetPeakTrace();
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

    void resized() override;
    void paint (juce::Graphics& g) override;

private:
    float frequencyToX (float frequency, float left, float width) const;
    float binToFrequency (int binIndex) const;
    float frequencyToNormX (float frequency) const;
    void smoothInto (const std::vector<float>& input, std::vector<float>& target);
    void smoothIntoRMS (const std::vector<float>& input, std::vector<float>& target);
    void smoothIntoPeak (const std::vector<float>& input, std::vector<float>& target);
    void updateAverageTrace (const std::vector<float>& input, std::vector<float>& target);
    void updatePeakTrace (std::vector<float>& trace, const std::vector<float>& source);
    void drawFrequencyMarkers (juce::Graphics& g, juce::Rectangle<float> inner);
    void drawNoteGrid (juce::Graphics& g, juce::Rectangle<float> inner);

    struct RulerPoint
    {
        float xNorm = 0.0f;
        float yNorm = 0.0f;
    };

    juce::Rectangle<float> getSpectrumBounds() const;
    RulerPoint pointToRulerPoint (juce::Point<float> point) const;
    juce::Point<float> rulerPointToPosition (RulerPoint point) const;
    float rulerPointToFrequency (RulerPoint point) const;
    float rulerPointToDb (RulerPoint point) const;
    juce::String formatFrequency (float frequency) const;
    juce::String buildRulerTooltip() const;
    bool isAnalyzerFrozen() const;

    std::vector<float> smoothedLeft;
    std::vector<float> smoothedRight;
    std::vector<float> peakTraceLeft;
    std::vector<float> peakTraceRight;
    std::vector<float> referenceTraceLeft;
    std::vector<float> referenceTraceRight;
    bool referenceTraceVisible = false;
    std::vector<float> averageTraceLeft;
    std::vector<float> averageTraceRight;
    bool averageTraceEnabled = false;
    bool averageTraceVisible = false;

    // Auditory masking
    std::vector<float> maskingThresholdLeft;
    std::vector<float> maskingThresholdRight;
    bool maskingVisualizationEnabled = false;

    // Sidechain comparison
    std::vector<float> sidechainLeft;
    std::vector<float> sidechainRight;
    bool sidechainVisible = false;
    bool clashDetectionEnabled = false;

    // Spectrogram - Waterfall
    static constexpr int maxSpectrogramFrames = 64;
    std::deque<std::vector<float>> spectrogramFrames;

    std::vector<int> peakHoldLeft;
    std::vector<int> peakHoldRight;
    static constexpr int peakHoldTime = 40; // Approx 1.2s at 30fps

    double sampleRate = 44100.0;
    int fftSize = 2048;
    DisplayMode displayMode = DisplayMode::LR;
    WeightingMode weightingMode = WeightingMode::Flat;
    SmoothingMode smoothingMode = SmoothingMode::Standard;
    bool crestModeEnabled = false;
    ColorScheme colorScheme = ColorScheme::Funky;
    bool frequencyMarkersEnabled = true;
    bool noteGridEnabled = false;
    bool frozen = false;
    float displayRangeDb = 90.0f;

    // Pitch Detection
    juce::String currentNote = "--";
    float currentPitchFreq = 0.0f;
    float noteAlpha = 0.0f; // For fading the note display

    bool rulerVisible = false;
    bool rulerDragging = false;
    RulerPoint rulerStart;
    RulerPoint rulerEnd;

    bool inspectionActive = false;
    RulerPoint inspectionPoint;

    void updateLookupTable();
    std::vector<int> binLookupTable;
    bool needsLookupUpdate = true;

    void updateMaskingThreshold (const std::vector<float>& spectrum, std::vector<float>& maskingThreshold);

    float minFrequency = 20.0f;
    float maxFrequency = 20000.0f;
};
