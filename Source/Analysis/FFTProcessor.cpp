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

    std::fill (fifo.begin(), fifo.end(), 0.0f);
    std::fill (workingFFTData.begin(), workingFFTData.end(), 0.0f);

    for (auto& frame : linearFrames)
        std::fill (frame.begin(), frame.end(), 0.0f);

    publishedBufferIndex.store (0, std::memory_order_release);
    publishedFrameCounter.store (0, std::memory_order_release);
    consumedFrameCounter.store (0, std::memory_order_release);
}

bool FFTProcessor::pushSamples (const float* samples, int numSamples)
{
    if (samples == nullptr)
        return false;

    bool framePublished = false;

    for (int i = 0; i < numSamples; ++i)
    {
        fifo[(size_t) fifoIndex++] = samples[i];

        if (fifoIndex == fftSize)
        {
            std::fill (workingFFTData.begin(), workingFFTData.end(), 0.0f);
            std::copy (fifo.begin(), fifo.end(), workingFFTData.begin());

            window.multiplyWithWindowingTable (workingFFTData.data(), fftSize);
            fft.performFrequencyOnlyForwardTransform (workingFFTData.data());

            // Get the current published index and write to the OTHER buffer
            const int currentFront = publishedBufferIndex.load(std::memory_order_acquire);
            const int backIndex = 1 - currentFront;
            auto& backFrame = linearFrames[(size_t) backIndex];

            // Normalize and copy results to the back buffer
            for (int bin = 0; bin < numBins; ++bin)
                backFrame[(size_t) bin] = workingFFTData[(size_t) bin] / (float) fftSize;

            // Atomic swap: publish the back buffer
            publishedBufferIndex.store (backIndex, std::memory_order_release);
            publishedFrameCounter.fetch_add (1, std::memory_order_acq_rel);

            fifoIndex = 0;
            framePublished = true;
        }
    }

    return framePublished;
}

bool FFTProcessor::getLinearMagnitudes (std::vector<float>& result)
{
    const auto published = publishedFrameCounter.load(std::memory_order_acquire);
    const auto consumed  = consumedFrameCounter.load(std::memory_order_acquire);

    // If no new data since last clearNewDataFlag()
    if (published == consumed)
        return false;

    const int frontIndex = publishedBufferIndex.load(std::memory_order_acquire);
    const auto& frame = linearFrames[(size_t) frontIndex];

    result.assign (frame.begin(), frame.end());
    return true;
}

bool FFTProcessor::getMagnitudes (std::vector<float>& result)
{
    const auto published = publishedFrameCounter.load(std::memory_order_acquire);
    const auto consumed  = consumedFrameCounter.load(std::memory_order_acquire);

    if (published == consumed)
        return false;

    const int frontIndex = publishedBufferIndex.load(std::memory_order_acquire);
    const auto& frame = linearFrames[(size_t) frontIndex];

    result.resize (frame.size());
    for (size_t i = 0; i < frame.size(); ++i)
    {
        // Convert to dB using the data from the front buffer
        result[i] = juce::Decibels::gainToDecibels (juce::jmax (frame[i], 1.0e-9f), -120.0f);
    }

    return true;
}
