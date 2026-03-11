#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>

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
  float peakL = 0.0f;
  float peakR = 0.0f;
  float holdL = 0.0f;
  float holdR = 0.0f;
  float rmsL = 0.0f;
  float rmsR = 0.0f;
  float loudness = 0.0f;
  float correlation = 0.0f;

  double currentSampleRate = 44100.0;
  float peakFalloff = 0.03f;
  float holdFalloff = 0.008f;
  float rmsSmoothing = 0.18f;

  std::unique_ptr<juce::dsp::Oversampling<float>> truePeakOversampler;
};
