#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <atomic>

class LevelMeterSource {
public:
  void prepare(double sampleRate, int maxSamplesPerBlock);
  void processBlock(const juce::AudioBuffer<float> &buffer);
  void processBlock(juce::AudioBuffer<float>& buffer); // Mutable overload for true peak oversampling
  void decay();

  float getPeakLeft() const { return peakL; }
  float getPeakRight() const { return peakR; }
  float getHoldLeft() const { return holdL; }
  float getHoldRight() const { return holdR; }
  float getRmsLeft() const { return rmsL; }
  float getRmsRight() const { return rmsR; }
  float getLoudness() const { return loudness; }
  float getCorrelation() const { return correlation; }

private:
  std::atomic<float> peakL { 0.0f };
  std::atomic<float> peakR { 0.0f };
  std::atomic<float> holdL { 0.0f };
  std::atomic<float> holdR { 0.0f };
  std::atomic<float> rmsL { 0.0f };
  std::atomic<float> rmsR { 0.0f };
  std::atomic<float> loudness { 0.0f };
  std::atomic<float> correlation { 0.0f };

  double currentSampleRate = 44100.0;
  float peakFalloff = 0.03f;
  float holdFalloff = 0.008f;
  float rmsSmoothing = 0.18f;

  std::unique_ptr<juce::dsp::Oversampling<float>> truePeakOversampler;
};
