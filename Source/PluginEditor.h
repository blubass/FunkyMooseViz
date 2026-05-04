#pragma once

#include "Components/ElchComponent.h"
#include "Components/MeterComponent.h"
#include "Components/SpectrumComponent.h"
#include "Components/WaveformComponent.h"
#include "Components/LoudnessHistoryComponent.h"
#include "Components/MultibandCorrelationComponent.h"
#include "PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <deque>

class UweVizAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   public juce::Timer,
                                   public juce::FileDragAndDropTarget {
public:
  explicit UweVizAudioProcessorEditor(UweVizAudioProcessor &);
  ~UweVizAudioProcessorEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  void timerCallback() override;
  
  bool isInterestedInFileDrag (const juce::StringArray& files) override;
  void filesDropped (const juce::StringArray& files, int x, int y) override;
  void updateModeButtons();
  void updateWeightingButton();
  void updateCrestButton();
  void updateCalmSkinButton();
  void updateReferenceButton();
  void updateAverageButton();
  void updateMaskingButton();
  void updateSmoothingButton();
  void updateMarkersButton();
  void updateGridButton();
  void updateZoomButton();
  void updateSchemeButton();
  void updateToneButton();
  void updateSCButton();
  void updateClashButton();
  void updatePlayButton();
  void takeSnapshot();
  void updateMooseModeStyling();

  UweVizAudioProcessor &audioProcessor;

  SpectrumComponent spectrumComponent;
  WaveformComponent waveformComponent;
  LoudnessHistoryComponent loudnessHistoryComponent;
  MultibandCorrelationComponent multibandCorrelationComponent;
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

  std::deque<float> freqHistory;
  float smoothedFreq = 0.0f;
  float confidence = 0.0f;

  juce::TextButton lrButton{"L/R"};
  juce::TextButton msButton{"M/S"};
  juce::TextButton waterfallButton{"WATERFALL"};
  juce::TextButton freezeButton{"FREEZE"};
  juce::TextButton rangeButton{"90 dB"};
  juce::TextButton tiltButton{"TILT OFF"};
  juce::TextButton crestButton{"CREST"};
  juce::TextButton schemeButton{"SCHEME"};
  juce::TextButton referenceButton{"REF"};
  juce::TextButton averageButton{"AVG"};
  juce::TextButton maskingButton{"MASK"};
  juce::TextButton smoothingButton{"SMOOTH"};
  juce::TextButton markersButton{"MARKERS"};
  juce::TextButton gridButton{"GRID"};
  juce::TextButton zoomButton{"ZOOM"};
  juce::TextButton toneButton{"TONE"};
  juce::TextButton scButton{"SIDECHAIN"};
  juce::TextButton clashButton{"CLASH"};
  juce::TextButton playButton{"PLAY"};
  juce::TextButton stopFileButton{"STOP"};
  juce::TextButton snapshotButton{"SNAPSHOT"};
  juce::ToggleButton analyzeOnlyButton{"MONITOR OUT"};

  SpectrumComponent::DisplayMode displayMode =
      SpectrumComponent::DisplayMode::LR;
  SpectrumComponent::WeightingMode weightingMode =
      SpectrumComponent::WeightingMode::Flat;
  bool crestModeEnabled = false;
  bool averageTraceEnabled = false;
  bool maskingVisualizationEnabled = false;
  bool sidechainEnabled = false;
  bool clashEnabled = false;
  bool frequencyMarkersEnabled = true;
  bool noteGridEnabled = false;
  int zoomMode = 0; // 0: Full, 1: Low, 2: Mid, 3: High
  SpectrumComponent::ColorScheme colorScheme = SpectrumComponent::ColorScheme::Funky;
  SpectrumComponent::SmoothingMode smoothingMode = SpectrumComponent::SmoothingMode::Standard;
  bool frozen = false;
  uint32_t lastConsumedFrame = 0;

  std::unique_ptr<juce::FileChooser> fileChooser;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UweVizAudioProcessorEditor)
};
