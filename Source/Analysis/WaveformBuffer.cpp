#include "WaveformBuffer.h"

void WaveformBuffer::prepare (int size)
{
    bufferL.assign ((size_t) size, 0.0f);
    bufferR.assign ((size_t) size, 0.0f);
    writeIndex = 0;
    hasWrapped = false;
}

void WaveformBuffer::pushStereoSamples (const float* left, const float* right, int numSamples)
{
    if (bufferL.empty() || left == nullptr)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        bufferL[(size_t) writeIndex] = left[i];
        bufferR[(size_t) writeIndex] = (right != nullptr) ? right[i] : left[i];
        ++writeIndex;

        if (writeIndex >= (int) bufferL.size())
        {
            writeIndex = 0;
            hasWrapped = true;
        }
    }
}

void WaveformBuffer::getStereoWaveform (std::vector<float>& leftOut, std::vector<float>& rightOut) const
{
    leftOut.clear();
    rightOut.clear();

    if (bufferL.empty())
        return;

    const size_t size = bufferL.size();
    leftOut.reserve (size);
    rightOut.reserve (size);

    if (! hasWrapped)
    {
        leftOut.insert (leftOut.end(), bufferL.begin(), bufferL.begin() + writeIndex);
        rightOut.insert (rightOut.end(), bufferR.begin(), bufferR.begin() + writeIndex);
        return;
    }

    leftOut.insert (leftOut.end(), bufferL.begin() + writeIndex, bufferL.end());
    leftOut.insert (leftOut.end(), bufferL.begin(), bufferL.begin() + writeIndex);
    
    rightOut.insert (rightOut.end(), bufferR.begin() + writeIndex, bufferR.end());
    rightOut.insert (rightOut.end(), bufferR.begin(), bufferR.begin() + writeIndex);
}
