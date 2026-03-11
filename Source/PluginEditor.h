#pragma once

#include "Components/ElchComponent.h"
#include "Components/MeterComponent.h"
#include "Components/SpectrumComponent.h"
#include "Components/WaveformComponent.h"
#include "PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>

class UweVizAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   private juce::Timer {
public:
  explicit UweVizAudioProcessorEditor(UweVizAudioProcessor &);
  ~UweVizAudioProcessorEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  void timerCallback() override;
  void updateModeButtons();
  void updateMooseModeStyling();

  UweVizAudioProcessor &audioProcessor;

  SpectrumComponent spectrumComponent;
  WaveformComponent waveformComponent;
  MeterComponent meterLeft;
  MeterComponent meterRight;
  CorrelationMeterComponent correlationMeter;
  LoudnessMeterComponent loudnessMeter;
  ElchComponent elchComponent;

  juce::Label titleLabel;
  juce::Label subTitleLabel;
  juce::Label pitchLabel;

  juce::String currentNoteStr = "---";
  juce::String targetNoteStr = "---";
  int noteHoldCount = 0;

  juce::TextButton lrButton{"L/R"};
  juce::TextButton msButton{"M/S"};
  juce::TextButton freezeButton{"FREEZE"};
  juce::TextButton rangeButton{"90 dB"};
  juce::ToggleButton analyzeOnlyButton{"MONITOR OUT"};

  SpectrumComponent::DisplayMode displayMode =
      SpectrumComponent::DisplayMode::LR;
  bool frozen = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UweVizAudioProcessorEditor)
};
