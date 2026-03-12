#include "WaveformBuffer.h"
#include <algorithm>

void WaveformBuffer::prepare (int size)
{
    bufferL.assign ((size_t) size, 0.0f);
    bufferR.assign ((size_t) size, 0.0f);
    writeIndex.store (0, std::memory_order_release);
    hasWrapped.store (false, std::memory_order_release);
}

void WaveformBuffer::pushStereoSamples (const float* left, const float* right, int numSamples)
{
    if (bufferL.empty() || left == nullptr)
        return;

    const int size = (int) bufferL.size();
    int localWriteIndex = writeIndex.load (std::memory_order_relaxed);

    for (int i = 0; i < numSamples; ++i)
    {
        bufferL[(size_t) localWriteIndex] = left[i];
        bufferR[(size_t) localWriteIndex] = (right != nullptr) ? right[i] : left[i];
        
        if (++localWriteIndex >= size)
        {
            localWriteIndex = 0;
            hasWrapped.store (true, std::memory_order_release);
        }
    }
    
    writeIndex.store (localWriteIndex, std::memory_order_release);
}

void WaveformBuffer::getStereoWaveform (std::vector<float>& leftOut, std::vector<float>& rightOut) const
{
    if (bufferL.empty())
        return;

    const int currentIdx = writeIndex.load (std::memory_order_acquire);
    const bool wrapped = hasWrapped.load (std::memory_order_acquire);
    const size_t size = bufferL.size();

    leftOut.assign (size, 0.0f);
    rightOut.assign (size, 0.0f);

    if (! wrapped)
    {
        // Copy only up to what we have
        std::copy (bufferL.begin(), bufferL.begin() + currentIdx, leftOut.begin());
        std::copy (bufferR.begin(), bufferR.begin() + currentIdx, rightOut.begin());
    }
    else
    {
        // Linearity: Part 1 is from currentIdx to end, Part 2 is from 0 to currentIdx
        const size_t part2Size = (size_t) currentIdx;
        const size_t part1Size = size - part2Size;

        // Copy Part 1 (Older data)
        std::copy (bufferL.begin() + currentIdx, bufferL.end(), leftOut.begin());
        std::copy (bufferR.begin() + currentIdx, bufferR.end(), rightOut.begin());

        // Copy Part 2 (Newer data)
        std::copy (bufferL.begin(), bufferL.begin() + currentIdx, leftOut.begin() + (std::ptrdiff_t)part1Size);
        std::copy (bufferR.begin(), bufferR.begin() + currentIdx, rightOut.begin() + (std::ptrdiff_t)part1Size);
    }
}
