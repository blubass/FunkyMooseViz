#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>

class WaveformComponent : public juce::Component {
public:
  void setWaveform(const std::vector<float> &left, const std::vector<float> &right);
  void setFrozen(bool shouldBeFrozen);
  void paint(juce::Graphics &g) override;

private:
  std::vector<float> waveformL;
  std::vector<float> waveformR;
  bool frozen = false;
};
