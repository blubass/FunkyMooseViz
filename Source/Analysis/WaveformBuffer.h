#pragma once
#include <vector>
#include <atomic>

class WaveformBuffer
{
public:
    void prepare (int size);
    
    // Audio Thread: Pure write, no locks, no allocations
    void pushStereoSamples (const float* left, const float* right, int numSamples);
    
    // GUI Thread: Linearizes and copies data into output vectors
    void getStereoWaveform (std::vector<float>& left, std::vector<float>& right) const;

private:
    std::vector<float> bufferL;
    std::vector<float> bufferR;
    
    std::atomic<int> writeIndex { 0 };
    std::atomic<bool> hasWrapped { false };
};
