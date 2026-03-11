#include "FFTProcessor.h"

FFTProcessor::FFTProcessor()
    : fft (fftOrder),
      window (fftSize, juce::dsp::WindowingFunction<float>::hann)
{
}

void FFTProcessor::prepare (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    fifoIndex = 0;
    fftReady = false;
    std::fill (fifo.begin(), fifo.end(), 0.0f);
    std::fill (fftData.begin(), fftData.end(), 0.0f);
}

void FFTProcessor::pushSamples (const float* samples, int numSamples)
{
    if (samples == nullptr)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        fifo[(size_t) fifoIndex++] = samples[i];

        if (fifoIndex == fftSize)
        {
            std::fill (fftData.begin(), fftData.end(), 0.0f);
            std::copy (fifo.begin(), fifo.end(), fftData.begin());
            window.multiplyWithWindowingTable (fftData.data(), fftSize);
            fft.performFrequencyOnlyForwardTransform (fftData.data());
            fftReady = true;
            fifoIndex = 0;
        }
    }
}

bool FFTProcessor::getMagnitudes (std::vector<float>& result)
{
    if (! fftReady)
        return false;

    result.resize (fftSize / 2);

    for (size_t i = 0; i < result.size(); ++i)
    {
        const float mag = fftData[i] / (float) fftSize;
        result[i] = juce::Decibels::gainToDecibels (juce::jmax (mag, 1.0e-9f), -120.0f);
    }

    fftReady = false;
    return true;
}
