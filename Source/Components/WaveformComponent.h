#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>

class WaveformComponent : public juce::Component {
public:
  enum class Mode {
      Waveform,
      Goniometer
  };

  void setWaveform(const std::vector<float> &left, const std::vector<float> &right);
  void setFrozen(bool shouldBeFrozen);
  void setMode(Mode newMode);
  void paint(juce::Graphics &g) override;

private:
  std::vector<float> waveformL;
  std::vector<float> waveformR;
  Mode mode = Mode::Waveform;
  bool frozen = false;
};
