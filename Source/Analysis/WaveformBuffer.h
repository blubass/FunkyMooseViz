#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>

class WaveformBuffer
{
public:
    void prepare (int size);
    void pushStereoSamples (const float* left, const float* right, int numSamples);
    void getStereoWaveform (std::vector<float>& left, std::vector<float>& right) const;

private:
    std::vector<float> bufferL;
    std::vector<float> bufferR;
    int writeIndex = 0;
    bool hasWrapped = false;
};
