#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <vector>

class FFTProcessor
{
public:
    FFTProcessor();

    void prepare (double sampleRate, int samplesPerBlock);
    bool pushSamples (const float* samples, int numSamples);

    bool getMagnitudes (std::vector<float>& result);
    bool getLinearMagnitudes (std::vector<float>& result);

    bool isNewDataAvailable() const noexcept
    {
        return publishedFrameCounter.load(std::memory_order_acquire)
             != consumedFrameCounter.load(std::memory_order_acquire);
    }

    void clearNewDataFlag() noexcept
    {
        consumedFrameCounter.store(
            publishedFrameCounter.load(std::memory_order_acquire),
            std::memory_order_release);
    }

    double getSampleRate() const noexcept { return currentSampleRate; }
    int getFFTSize() const noexcept       { return fftSize; }

private:
    static constexpr int fftOrder = 12;
    static constexpr int fftSize  = 1 << fftOrder;
    static constexpr int numBins  = fftSize / 2;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    std::array<float, fftSize> fifo {};
    std::array<float, fftSize * 2> workingFFTData {};

    std::array<std::array<float, numBins>, 2> linearFrames {};
    std::atomic<int> publishedBufferIndex { 0 };
    std::atomic<uint32_t> publishedFrameCounter { 0 };
    std::atomic<uint32_t> consumedFrameCounter { 0 };

    int fifoIndex = 0;
    double currentSampleRate = 44100.0;
};
