#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>

class MeterComponent : public juce::Component {
public:
  void setLevels(float newPeak, float newHold, float newRms);
  void setLabel(juce::String newLabel);
  void paint(juce::Graphics &g) override;
  static float gainToMeter01(float value);

private:

  float peak = 0.0f;
  float hold = 0.0f;
  float rms = 0.0f;
  juce::String label = "M";
};

class LoudnessMeterComponent : public juce::Component {
public:
  void setLoudness(float newLoudness);
  void paint(juce::Graphics &g) override;

private:
  float loudness = 0.0f;
};

class CorrelationMeterComponent : public juce::Component {
public:
  void setCorrelation(float newCorrelation);
  void paint(juce::Graphics &g) override;

private:
  float correlation = 1.0f;
};
