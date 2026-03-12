#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>

class FFTProcessor
{
public:
    FFTProcessor();

    void prepare (double sampleRate, int samplesPerBlock);
    void pushSamples (const float* samples, int numSamples);
    bool getMagnitudes (std::vector<float>& result);
    bool getLinearMagnitudes (std::vector<float>& result);
    
    bool isNewDataAvailable() const noexcept { return fftReady; }
    void clearNewDataFlag() noexcept          { fftReady = false; }

    double getSampleRate() const noexcept { return currentSampleRate; }
    int getFFTSize() const noexcept       { return fftSize; }

private:
    static constexpr int fftOrder = 12; // Increased for better bass resolution (4096 bins)
    static constexpr int fftSize  = 1 << fftOrder;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    std::array<float, fftSize> fifo {};
    std::array<float, fftSize * 2> fftData {};

    int fifoIndex = 0;
    bool fftReady = false;
    double currentSampleRate = 44100.0;
};
