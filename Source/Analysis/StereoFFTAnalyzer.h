#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <atomic>

class StereoFFTAnalyzer
{
public:
    StereoFFTAnalyzer();
    void prepare (double sampleRate, int samplesPerBlock);
    void pushSamples (const float* left, const float* right, int numSamples);
    
    struct AnalysisResult {
        std::vector<float> correlation; // -1 to 1 per bin
        std::vector<float> magnitudes;  // Average magnitude per bin
    };
    
    bool getResult (AnalysisResult& result);

private:
    static constexpr int fftOrder = 11; // 2048
    static constexpr int fftSize  = 1 << fftOrder;
    static constexpr int numBins  = fftSize / 2;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    std::vector<float> fifoL, fifoR;
    int fifoIndex = 0;

    struct Frame {
        std::array<float, numBins> correlation;
        std::array<float, numBins> magnitudes;
    };

    std::array<Frame, 2> frames;
    std::atomic<int> activeFrame { 0 };
    std::atomic<bool> newData { false };

    std::vector<float> workL, workR;
    double sampleRate = 44100.0;
};
