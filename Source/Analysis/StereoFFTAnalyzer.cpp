#include "StereoFFTAnalyzer.h"
#include <complex>

StereoFFTAnalyzer::StereoFFTAnalyzer()
    : fft(fftOrder), window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    fifoL.assign(fftSize, 0.0f);
    fifoR.assign(fftSize, 0.0f);
    workL.assign(fftSize * 2, 0.0f);
    workR.assign(fftSize * 2, 0.0f);
}

void StereoFFTAnalyzer::prepare(double sr, int) {
    sampleRate = sr;
    fifoIndex = 0;
    newData = false;
}

void StereoFFTAnalyzer::pushSamples(const float* left, const float* right, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        fifoL[(size_t)fifoIndex] = left[i];
        fifoR[(size_t)fifoIndex] = right[i];
        fifoIndex++;

        if (fifoIndex == fftSize) {
            std::fill(workL.begin(), workL.end(), 0.0f);
            std::fill(workR.begin(), workR.end(), 0.0f);
            std::copy(fifoL.begin(), fifoL.end(), workL.begin());
            std::copy(fifoR.begin(), fifoR.end(), workR.begin());

            window.multiplyWithWindowingTable(workL.data(), fftSize);
            window.multiplyWithWindowingTable(workR.data(), fftSize);

            fft.performRealOnlyForwardTransform(workL.data());
            fft.performRealOnlyForwardTransform(workR.data());

            int backIndex = 1 - activeFrame.load();
            auto& frame = frames[(size_t)backIndex];

            // JUCE FFT layout for real-to-complex:
            // [Re(0), Re(N/2), Re(1), Im(1), Re(2), Im(2), ..., Re(N/2-1), Im(N/2-1)]
            // Wait, for complex correlation we need Re/Im.
            
            for (int bin = 1; bin < numBins; ++bin) {
                float reL = workL[(size_t)bin * 2];
                float imL = workL[(size_t)bin * 2 + 1];
                float reR = workR[(size_t)bin * 2];
                float imR = workR[(size_t)bin * 2 + 1];

                float magLSq = reL * reL + imL * imL;
                float magRSq = reR * reR + imR * imR;
                
                // Dot product of complex vectors
                float dot = reL * reR + imL * imR;
                float magL = std::sqrt(magLSq);
                float magR = std::sqrt(magRSq);

                if (magL > 1e-9f && magR > 1e-9f) {
                    frame.correlation[(size_t)bin] = dot / (magL * magR);
                } else {
                    frame.correlation[(size_t)bin] = 1.0f;
                }
                
                frame.magnitudes[(size_t)bin] = (magL + magR) * 0.5f;
            }
            
            // Special case for Bin 0 (DC)
            float reL0 = workL[0];
            float reR0 = workR[0];
            frame.correlation[0] = (reL0 * reR0 >= 0) ? 1.0f : -1.0f;
            frame.magnitudes[0] = (std::abs(reL0) + std::abs(reR0)) * 0.5f;

            activeFrame = backIndex;
            newData = true;
            fifoIndex = 0;
        }
    }
}

bool StereoFFTAnalyzer::getResult(AnalysisResult& result) {
    if (!newData.exchange(false)) return false;

    auto& frame = frames[(size_t)activeFrame.load()];
    result.correlation.assign(frame.correlation.begin(), frame.correlation.end());
    result.magnitudes.assign(frame.magnitudes.begin(), frame.magnitudes.end());
    return true;
}
