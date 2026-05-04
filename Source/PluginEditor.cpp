#include "PluginEditor.h"
#include "BinaryData.h"

UweVizAudioProcessorEditor::UweVizAudioProcessorEditor(UweVizAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setSize(p.lastUIWidth, p.lastUIHeight);
  setResizable(true, true);
  setResizeLimits(760, 520, 1700, 1100);
  addAndMakeVisible(titleLabel);
  titleLabel.setText("FUNKY MOOSE", juce::dontSendNotification);
  titleLabel.setJustificationType(juce::Justification::centredLeft);
  titleLabel.setFont(juce::FontOptions(26.0f).withStyle("Bold"));
  titleLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(88, 174, 219));

  addAndMakeVisible(subTitleLabel);
  subTitleLabel.setText("STEREO AUDIO ANALYZER", juce::dontSendNotification);
  subTitleLabel.setJustificationType(juce::Justification::centredLeft);
  subTitleLabel.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
  subTitleLabel.setColour(juce::Label::textColourId,
                          juce::Colour::fromRGB(200, 200, 200));

  addAndMakeVisible(elchComponent);
  auto img = juce::ImageFileFormat::loadFrom(
      BinaryData::image_png,
      BinaryData::image_pngSize);
  if (img.isValid())
    elchComponent.setElchImage(img);
  elchComponent.setBackgroundColor(juce::Colours::transparentBlack);

  addAndMakeVisible(spectrumComponent);
  addAndMakeVisible(waveformComponent);
  addAndMakeVisible(loudnessHistoryComponent);
  addAndMakeVisible(multibandCorrelationComponent);
  addAndMakeVisible(meterLeft);
  addAndMakeVisible(meterRight);
  addAndMakeVisible(correlationMeter);
  addAndMakeVisible(loudnessMeter);

  addAndMakeVisible(pitchLabel);
  smoothingMode = static_cast<SpectrumComponent::SmoothingMode>(audioProcessor.getSpectrumSmoothingMode());
  frequencyMarkersEnabled = audioProcessor.getSpectrumFrequencyMarkers();
  noteGridEnabled = audioProcessor.getSpectrumNoteGrid();
  colorScheme = static_cast<SpectrumComponent::ColorScheme>(audioProcessor.getSpectrumColorScheme());

  addAndMakeVisible(lrButton);
  addAndMakeVisible(msButton);
  addAndMakeVisible(waterfallButton);
  addAndMakeVisible(rangeButton);
  addAndMakeVisible(freezeButton);
  addAndMakeVisible(tiltButton);
  addAndMakeVisible(crestButton);
  addAndMakeVisible(schemeButton);
  addAndMakeVisible(referenceButton);
  addAndMakeVisible(averageButton);
  addAndMakeVisible(maskingButton);
  addAndMakeVisible(smoothingButton);
  addAndMakeVisible(markersButton);
  addAndMakeVisible(gridButton);
  addAndMakeVisible(zoomButton);
  addAndMakeVisible(toneButton);
  addAndMakeVisible(scButton);
  addAndMakeVisible(clashButton);
  addAndMakeVisible(playButton);
  addAndMakeVisible(stopFileButton);
  addAndMakeVisible(snapshotButton);
  addAndMakeVisible(analyzeOnlyButton);

  // Sync state from processor
  displayMode = static_cast<SpectrumComponent::DisplayMode>(audioProcessor.getDisplayMode());
  weightingMode = static_cast<SpectrumComponent::WeightingMode>(
      juce::jlimit(0, 2, audioProcessor.getSpectrumWeightingMode()));
  crestModeEnabled = audioProcessor.getSpectrumCrestMode();
  colorScheme = static_cast<SpectrumComponent::ColorScheme>(audioProcessor.getSpectrumColorScheme());
  averageTraceEnabled = audioProcessor.getSpectrumAverageTrace();
  frozen = audioProcessor.getFrozen();

  meterLeft.setLabel("LEFT");
  meterRight.setLabel("RIGHT");

  analyzeOnlyButton.setToggleState(!audioProcessor.getAnalyzeOnly(),
                                   juce::dontSendNotification);
  analyzeOnlyButton.setColour(juce::ToggleButton::textColourId,
                              juce::Colour::fromRGBA(230, 235, 240, 190));
  analyzeOnlyButton.onClick = [this] {
    audioProcessor.setAnalyzeOnly(!analyzeOnlyButton.getToggleState());
  };

  lrButton.onClick = [this] {
    displayMode = SpectrumComponent::DisplayMode::LR;
    audioProcessor.setDisplayMode(0);
    spectrumComponent.clearReferenceTrace();
    spectrumComponent.resetAverageTrace();
    spectrumComponent.setDisplayMode(displayMode);
    updateModeButtons();
    updateReferenceButton();
    updateMooseModeStyling();
  };

  msButton.onClick = [this] {
    displayMode = SpectrumComponent::DisplayMode::MS;
    audioProcessor.setDisplayMode(1);
    spectrumComponent.clearReferenceTrace();
    spectrumComponent.resetAverageTrace();
    spectrumComponent.setDisplayMode(displayMode);
    updateModeButtons();
    updateReferenceButton();
    updateMooseModeStyling();
  };

  waterfallButton.onClick = [this] {
    displayMode = SpectrumComponent::DisplayMode::Waterfall;
    audioProcessor.setDisplayMode(2); // New mode index
    spectrumComponent.clearReferenceTrace();
    spectrumComponent.resetAverageTrace();
    spectrumComponent.setDisplayMode(displayMode);
    updateModeButtons();
    updateReferenceButton();
    updateMooseModeStyling();
  };

  rangeButton.onClick = [this] {
    if (audioProcessor.getDisplayRange() == 90) {
        audioProcessor.setDisplayRange(60);
        rangeButton.setButtonText("60 dB");
        spectrumComponent.setDisplayRange(60.0f);
    } else {
        audioProcessor.setDisplayRange(90);
        rangeButton.setButtonText("90 dB");
        spectrumComponent.setDisplayRange(90.0f);
    }
    updateModeButtons();
  };

  freezeButton.onClick = [this] {
    frozen = !frozen;
    audioProcessor.setFrozen(frozen);
    spectrumComponent.setFrozen(frozen);
    waveformComponent.setFrozen(frozen);
    updateModeButtons();
  };

  tiltButton.onClick = [this] {
    const int nextMode = (static_cast<int>(weightingMode) + 1) % 6;
    weightingMode = static_cast<SpectrumComponent::WeightingMode>(nextMode);
    audioProcessor.setSpectrumWeightingMode(nextMode);
    spectrumComponent.setWeightingMode(weightingMode);
    updateWeightingButton();
  };

  crestButton.onClick = [this] {
    crestModeEnabled = !crestModeEnabled;
    audioProcessor.setSpectrumCrestMode(crestModeEnabled);
    spectrumComponent.setCrestModeEnabled(crestModeEnabled);
    updateCrestButton();
  };

  scButton.onClick = [this] {
      sidechainEnabled = !sidechainEnabled;
      spectrumComponent.setSidechainVisible(sidechainEnabled);
      updateSCButton();
  };

  clashButton.onClick = [this] {
      clashEnabled = !clashEnabled;
      spectrumComponent.setClashDetectionEnabled(clashEnabled);
      updateClashButton();
  };

  schemeButton.onClick = [this] {
      const int nextScheme = (static_cast<int>(colorScheme) + 1) % 4;
      colorScheme = static_cast<SpectrumComponent::ColorScheme>(nextScheme);
      audioProcessor.setSpectrumColorScheme(nextScheme);
      spectrumComponent.setColorScheme(colorScheme);
      updateSchemeButton();
  };

  referenceButton.onClick = [this] {
    if (spectrumComponent.hasReferenceTrace())
      spectrumComponent.clearReferenceTrace();
    else
      spectrumComponent.captureReferenceTrace();

    updateReferenceButton();
  };

  averageButton.onClick = [this] {
    averageTraceEnabled = !averageTraceEnabled;
    audioProcessor.setSpectrumAverageTrace(averageTraceEnabled);
    spectrumComponent.setAverageTraceEnabled(averageTraceEnabled);
    updateAverageButton();
  };

  maskingButton.onClick = [this] {
    maskingVisualizationEnabled = !maskingVisualizationEnabled;
    spectrumComponent.setMaskingVisualizationEnabled(maskingVisualizationEnabled);
    updateMaskingButton();
  };

  smoothingButton.onClick = [this] {
      const int nextMode = (static_cast<int>(smoothingMode) + 1) % 3;
      smoothingMode = static_cast<SpectrumComponent::SmoothingMode>(nextMode);
      audioProcessor.setSpectrumSmoothingMode(nextMode);
      spectrumComponent.setSmoothingMode(smoothingMode);
      updateSmoothingButton();
  };

  markersButton.onClick = [this] {
      frequencyMarkersEnabled = !frequencyMarkersEnabled;
      audioProcessor.setSpectrumFrequencyMarkers(frequencyMarkersEnabled);
      spectrumComponent.setFrequencyMarkersEnabled(frequencyMarkersEnabled);
      updateMarkersButton();
  };

  gridButton.onClick = [this] {
      noteGridEnabled = !noteGridEnabled;
      audioProcessor.setSpectrumNoteGrid(noteGridEnabled);
      spectrumComponent.setNoteGridEnabled(noteGridEnabled);
      updateGridButton();
  };

  zoomButton.onClick = [this] {
      zoomMode = (zoomMode + 1) % 4;
      switch (zoomMode) {
          case 0: spectrumComponent.setFrequencyRange(20.0f, 20000.0f); break;
          case 1: spectrumComponent.setFrequencyRange(20.0f, 500.0f); break;
          case 2: spectrumComponent.setFrequencyRange(500.0f, 5000.0f); break;
          case 3: spectrumComponent.setFrequencyRange(5000.0f, 20000.0f); break;
      }
      updateZoomButton();
  };

  toneButton.onClick = [this] {
      bool enabled = !audioProcessor.isToneGeneratorEnabled();
      audioProcessor.setToneGeneratorEnabled(enabled);
      updateToneButton();
  };

  playButton.onClick = [this] {
      if (audioProcessor.isFilePlaying()) audioProcessor.pauseFile();
      else audioProcessor.playFile();
      updatePlayButton();
  };

  stopFileButton.onClick = [this] {
      audioProcessor.stopFile();
      updatePlayButton();
  };

  snapshotButton.onClick = [this] {
      takeSnapshot();
  };

  spectrumComponent.setDisplayMode(displayMode);
  spectrumComponent.setFrozen(frozen);
  spectrumComponent.setDisplayRange((float)audioProcessor.getDisplayRange());
  spectrumComponent.setWeightingMode(weightingMode);
  spectrumComponent.setCrestModeEnabled(crestModeEnabled);
  spectrumComponent.setColorScheme(colorScheme);
  spectrumComponent.setAverageTraceEnabled(averageTraceEnabled);
  spectrumComponent.setSmoothingMode(smoothingMode);
  spectrumComponent.setFrequencyMarkersEnabled(frequencyMarkersEnabled);
  spectrumComponent.setNoteGridEnabled(noteGridEnabled);
  spectrumComponent.setMaskingVisualizationEnabled(maskingVisualizationEnabled);
  waveformComponent.setFrozen(frozen);
  
  if (audioProcessor.getDisplayRange() == 60) {
      rangeButton.setButtonText("60 dB");
  } else {
      rangeButton.setButtonText("90 dB");
  }

  updateModeButtons();
  updateWeightingButton();
  updateCrestButton();
  updateReferenceButton();
  updateAverageButton();
  updateMaskingButton();
  updateSmoothingButton();
  updateMarkersButton();
  updateGridButton();
  updateZoomButton();
  updateSchemeButton();
  updateToneButton();
  updateSCButton();
  updateClashButton();
  updatePlayButton();
  pitchLabel.setJustificationType(juce::Justification::centred);
  pitchLabel.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
  pitchLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(88, 174, 219));
  pitchLabel.setText("---", juce::dontSendNotification);

  updateMooseModeStyling();

  startTimerHz(30);
}

UweVizAudioProcessorEditor::~UweVizAudioProcessorEditor() {}

void UweVizAudioProcessorEditor::paint(juce::Graphics &g) {
  // TRON Cyberpunk Background
  g.setColour(juce::Colour::fromRGB(4, 5, 8)); // Ultra deep blue/black
  g.fillAll();

  auto area = getLocalBounds().toFloat();
  auto shell = area.reduced(8.0f);

  // Neon Grid lines background
  g.setColour(juce::Colour::fromRGBA(0, 255, 255, 5));
  for (float i = 0; i < area.getWidth(); i += 40.0f)
      g.drawVerticalLine((int)i, 0.0f, area.getHeight());
  for (float i = 0; i < area.getHeight(); i += 40.0f)
      g.drawHorizontalLine((int)i, 0.0f, area.getWidth());

  // Outer Shell Bevel
  g.setColour(juce::Colour::fromRGBA(0, 255, 255, 40));
  g.drawRoundedRectangle(shell, 4.0f, 1.5f);
  g.setColour(juce::Colour::fromRGBA(255, 0, 255, 20));
  g.drawRoundedRectangle(shell.reduced(2.0f), 4.0f, 1.0f);

  // Header Panel - Glassmorphism Neon Look
  auto innerShell = shell.reduced(12.0f);
  auto headerPanel = innerShell.removeFromTop(88.0f);
  
  juce::ColourGradient headerGrad(juce::Colour::fromRGBA(0, 255, 255, 15), headerPanel.getX(), headerPanel.getY(),
                                  juce::Colour::fromRGBA(255, 0, 255, 5), headerPanel.getRight(), headerPanel.getBottom(), false);
  g.setGradientFill(headerGrad);
  g.fillRoundedRectangle(headerPanel, 4.0f);
  
  // Header Rim
  g.setColour(juce::Colour::fromRGBA(0, 255, 255, 60));
  g.drawRoundedRectangle(headerPanel, 4.0f, 1.0f);

  // Draw the pitch box background next to mode buttons
  auto pb = pitchLabel.getBounds().toFloat();
  
  // Glass effect for pitch box
  g.setColour(juce::Colour::fromRGBA(0, 0, 0, 180));
  g.fillRoundedRectangle(pb, 2.0f);
  
  g.setColour(juce::Colour::fromRGBA(0, 255, 255, 100));
  g.drawRoundedRectangle(pb, 2.0f, 1.0f);
  
  // Sub-glow for pitch box
  g.setColour(juce::Colour::fromRGBA(0, 255, 255, 20));
  g.fillRoundedRectangle(pb.reduced(1.0f), 2.0f);

  g.setColour(juce::Colour::fromRGBA(255, 0, 255, 30));
  g.drawLine(headerPanel.getX() + 18.0f, headerPanel.getBottom() + 10.0f,
             headerPanel.getRight() - 18.0f, headerPanel.getBottom() + 10.0f,
             1.5f);
}

void UweVizAudioProcessorEditor::resized() {
  audioProcessor.lastUIWidth = getWidth();
  audioProcessor.lastUIHeight = getHeight();
  
  auto area = getLocalBounds().reduced(24);
  auto header = area.removeFromTop(88);

  // Logo Left (Square-ish)
  // Logo Left (Square-ish) -> Made it larger!
  elchComponent.setBounds(header.removeFromLeft(125).reduced(4));
  header.removeFromLeft(4);

  auto titleArea = header.removeFromLeft(300).reduced(0, 18);
  titleLabel.setBounds(titleArea.removeFromTop(30));
  subTitleLabel.setBounds(titleArea.removeFromTop(18));

  // Right Side: Combined Mode & Pitch Group
  auto rightGroup = header.removeFromRight(370).reduced(0, 15);
  
  auto pitchArea = rightGroup.removeFromLeft(80).reduced(0, 20);
  pitchLabel.setBounds(pitchArea);
  pitchLabel.setJustificationType(juce::Justification::centred);

  auto modeArea = rightGroup;
  lrButton.setBounds(modeArea.removeFromLeft(48).reduced(2));
  msButton.setBounds(modeArea.removeFromLeft(48).reduced(2));
  waterfallButton.setBounds(modeArea.removeFromLeft(96).reduced(2));
  modeArea.removeFromLeft(6);
  rangeButton.setBounds(modeArea.removeFromLeft(64).reduced(4));
  modeArea.removeFromLeft(6);
  freezeButton.setBounds(modeArea.removeFromLeft(96).reduced(4));

  area.removeFromTop(12);

  auto topControlRow = area.removeFromTop(28);
  analyzeOnlyButton.setBounds(topControlRow.removeFromLeft(160));
  topControlRow.removeFromLeft(10);
  tiltButton.setBounds(topControlRow.removeFromLeft(96));
  topControlRow.removeFromLeft(8);
  crestButton.setBounds(topControlRow.removeFromLeft(82));
  topControlRow.removeFromLeft(8);
  schemeButton.setBounds(topControlRow.removeFromLeft(78));
  topControlRow.removeFromLeft(8);
  referenceButton.setBounds(topControlRow.removeFromLeft(64));
  topControlRow.removeFromLeft(8);
  averageButton.setBounds(topControlRow.removeFromLeft(64));
  topControlRow.removeFromLeft(8);
  maskingButton.setBounds(topControlRow.removeFromLeft(64));
  topControlRow.removeFromLeft(8);
  smoothingButton.setBounds(topControlRow.removeFromLeft(80));
  topControlRow.removeFromLeft(8);
  markersButton.setBounds(topControlRow.removeFromLeft(80));
  topControlRow.removeFromLeft(8);
  gridButton.setBounds(topControlRow.removeFromLeft(64));
  topControlRow.removeFromLeft(8);
  zoomButton.setBounds(topControlRow.removeFromLeft(64));
  topControlRow.removeFromLeft(8);
  toneButton.setBounds(topControlRow.removeFromLeft(64));
  topControlRow.removeFromLeft(8);
  scButton.setBounds(topControlRow.removeFromLeft(82));
  topControlRow.removeFromLeft(8);
  clashButton.setBounds(topControlRow.removeFromLeft(64));
  topControlRow.removeFromLeft(8);
  playButton.setBounds(topControlRow.removeFromLeft(64));
  topControlRow.removeFromLeft(8);
  stopFileButton.setBounds(topControlRow.removeFromLeft(64));
  topControlRow.removeFromLeft(8);
  snapshotButton.setBounds(topControlRow.removeFromLeft(80));
  area.removeFromTop(8);

  auto lower = area.removeFromBottom(190);
  const int meterWidth = 62;

  auto meterArea = lower.removeFromRight(meterWidth * 2 + 10 + 44 + 10 + 44 + 10);
  meterLeft.setBounds(meterArea.removeFromLeft(meterWidth));
  meterArea.removeFromLeft(10);
  meterRight.setBounds(meterArea.removeFromLeft(meterWidth));
  meterArea.removeFromLeft(10);
  correlationMeter.setBounds(meterArea.removeFromLeft(44));
  meterArea.removeFromLeft(10);
  loudnessMeter.setBounds(meterArea.removeFromLeft(44));

  auto waveformArea = lower.removeFromRight(240);
  waveformComponent.setBounds(waveformArea);
  lower.removeFromRight(12);
  
  auto historyArea = lower.removeFromRight(200);
  loudnessHistoryComponent.setBounds(historyArea);
  lower.removeFromRight(12);
  
  multibandCorrelationComponent.setBounds(lower);

  area.removeFromBottom(14);
  spectrumComponent.setBounds(area);
}

void UweVizAudioProcessorEditor::timerCallback() {
  audioProcessor.getMeterSource().updateHistory();
  loudnessHistoryComponent.setHistory(audioProcessor.getMeterSource().getLoudnessHistory());

  StereoFFTAnalyzer::AnalysisResult stereoResult;
  if (audioProcessor.getStereoAnalyzer().getResult(stereoResult))
      multibandCorrelationComponent.setData(stereoResult.correlation);

  updatePlayButton();
  
  spectrumComponent.setAnalysisInfo(
      audioProcessor.getFFTProcessorLeft().getSampleRate(),
      audioProcessor.getFFTProcessorLeft().getFFTSize());

  const uint32_t currentFrame = audioProcessor.analysisFrameCounter.load(std::memory_order_acquire);

  // Update all analysis components when a new master frame is ready
  if (!frozen && currentFrame != lastConsumedFrame) {
      lastConsumedFrame = currentFrame;
      
      std::vector<float> magsA, magsB;
      
      if (displayMode == SpectrumComponent::DisplayMode::LR) {
          audioProcessor.getFFTProcessorLeft().getMagnitudes(magsA);
          spectrumComponent.setMagnitudesLeft(magsA);
          audioProcessor.getFFTProcessorRight().getMagnitudes(magsB);
          spectrumComponent.setMagnitudesRight(magsB);
      } else {
          audioProcessor.getFFTProcessorMid().getMagnitudes(magsA);
          spectrumComponent.setMagnitudesLeft(magsA);
          audioProcessor.getFFTProcessorSide().getMagnitudes(magsB);
          spectrumComponent.setMagnitudesRight(magsB);
      }

      // Fetch Sidechain data if active
      if (sidechainEnabled)
      {
          std::vector<float> scMagsL, scMagsR;
          audioProcessor.getFFTProcessorSCLeft().getMagnitudes(scMagsL);
          audioProcessor.getFFTProcessorSCRight().getMagnitudes(scMagsR);
          spectrumComponent.setSidechainData(scMagsL, scMagsR);
      }

      elchComponent.setSpectrum(magsA);

      std::vector<float> waveformL, waveformR;
      audioProcessor.getWaveformBuffer().getStereoWaveform(waveformL, waveformR);
      waveformComponent.setWaveform(waveformL, waveformR);

      // Pitch detection logic (HPS - Harmonic Product Spectrum)
      std::vector<float> spectrum;
      if (audioProcessor.getFFTProcessorLeft().getLinearMagnitudes(spectrum)) {
          if (spectrum.size() > 512) {
              const int hpsOrder = 4;
              const size_t hpsSize = (size_t)spectrum.size() / hpsOrder;
              std::vector<float> hps(hpsSize);
              
              float maxMag = 0.0f;
              for (auto m : spectrum) if (m > maxMag) maxMag = m;
              
              // Only process if signal is above noise floor
              if (maxMag > 1.0e-5f) {
                  // Initialize HPS with the first spectrum slice
                  for (size_t i = 0; i < hpsSize; ++i) 
                      hps[i] = spectrum[i] / maxMag; 

                  // Harmonic Product: multiply by harmonics 2, 3, 4
                  for (int h = 2; h <= hpsOrder; ++h) {
                      for (size_t i = 0; i < hpsSize; ++i) {
                          hps[i] *= (spectrum[i * (size_t)h] / maxMag);
                      }
                  }
                  
                  // Ignore DC and subsonic below 20Hz
                  const float sampleRate = (float)audioProcessor.getFFTProcessorLeft().getSampleRate();
                  const float binFreq = (sampleRate / 2.0f) / (float)spectrum.size();
                  const size_t minBin = (size_t)std::ceil(20.0f / binFreq);
                  
                  if (minBin < hpsSize) {
                      auto hpsStart = hps.begin() + (std::ptrdiff_t)minBin;
                      auto maxHpsIt = std::max_element(hpsStart, hps.end());
                      const size_t peakBin = (size_t)std::distance(hps.begin(), maxHpsIt);
                      float peakVal = *maxHpsIt;
                      
                      // Parabolic Interpolation for jitter reduction
                      float peakBinF = (float)peakBin;
                      if (peakBin > 0 && peakBin < hpsSize - 1) {
                          float y1 = hps[peakBin - 1];
                          float y2 = hps[peakBin];
                          float y3 = hps[peakBin + 1];
                          float denom = 2.0f * y2 - y1 - y3;
                          if (std::abs(denom) > 1e-12f) peakBinF += 0.5f * (y1 - y3) / denom;
                      }

                      // Confidence calculation
                      float sumHps = 0.0f;
                      for (size_t i = minBin; i < hpsSize; ++i) sumHps += hps[i];
                      float avgHps = sumHps / (float)(hpsSize - minBin);
                      confidence = (avgHps > 0) ? (peakVal / avgHps) : 0.0f;
                      
                      // Filter and display
                      if (confidence > 4.0f) { // Higher threshold for stability
                          float freq = peakBinF * binFreq;
                          
                          freqHistory.push_back(freq);
                          if (freqHistory.size() > 8) freqHistory.pop_front();
                          
                          // Outlier rejection (median filter approach)
                          std::vector<float> sortedFreqs(freqHistory.begin(), freqHistory.end());
                          std::sort(sortedFreqs.begin(), sortedFreqs.end());
                          smoothedFreq = sortedFreqs[sortedFreqs.size() / 2];
                          
                          targetNoteStr = UweVizAudioProcessor::frequencyToNote(smoothedFreq);
                      } else {
                          if (!freqHistory.empty()) freqHistory.pop_front();
                          targetNoteStr = "---";
                      }
                  }
              } else {
                  freqHistory.clear();
                  targetNoteStr = "---";
              }
          }
          currentNoteStr = targetNoteStr;
      }
  }

  pitchLabel.setText(currentNoteStr, juce::dontSendNotification);
  spectrumComponent.setDetectedNote(currentNoteStr, smoothedFreq);

  auto &meter = audioProcessor.getMeterSource();
  elchComponent.setVizSignal(meter.getRmsLeft() + meter.getRmsRight(), 
                             std::max(meter.getPeakLeft(), meter.getPeakRight()));

  float pulse = juce::jlimit(0.0f, 1.0f, (meter.getRmsLeft() + meter.getRmsRight()) * 2.0f);
  titleLabel.setColour(juce::Label::textColourId, 
                       juce::Colour::fromRGB(88, 174, 219).interpolatedWith(juce::Colours::white, pulse * 0.4f));

  meter.decay();

  meterLeft.setLevels(meter.getPeakLeft(), meter.getHoldLeft(),
                      meter.getRmsLeft());
  meterRight.setLevels(meter.getPeakRight(), meter.getHoldRight(),
                       meter.getRmsRight());
  correlationMeter.setCorrelation(meter.getCorrelation());
  loudnessMeter.setLoudness(meter.getLoudness());
}

void UweVizAudioProcessorEditor::updateModeButtons() {
  auto styleButton = [](juce::TextButton &b, bool active) {
    b.setColour(juce::TextButton::buttonColourId,
                active ? juce::Colour::fromRGB(150, 90, 40)
                       : juce::Colour::fromRGBA(10, 10, 10, 60));

    b.setColour(juce::TextButton::textColourOffId,
                active ? juce::Colours::white
                       : juce::Colour::fromRGBA(225, 215, 190, 160));

    b.setColour(juce::TextButton::buttonOnColourId,
                juce::Colour::fromRGB(180, 110, 50));
  };

  styleButton(lrButton, displayMode == SpectrumComponent::DisplayMode::LR);
  styleButton(msButton, displayMode == SpectrumComponent::DisplayMode::MS);
  styleButton(waterfallButton, displayMode == SpectrumComponent::DisplayMode::Waterfall);

  freezeButton.setColour(juce::TextButton::buttonColourId,
                         frozen ? juce::Colour::fromRGB(180, 50, 40)
                                : juce::Colour::fromRGBA(10, 10, 10, 60));

  freezeButton.setColour(juce::TextButton::textColourOffId,
                         frozen ? juce::Colours::white
                                : juce::Colour::fromRGBA(225, 215, 190, 160));
                                
  bool is60dB = audioProcessor.getDisplayRange() == 60;
  rangeButton.setColour(juce::TextButton::buttonColourId,
                         is60dB ? juce::Colour::fromRGBA(88, 174, 219, 140)
                                : juce::Colour::fromRGBA(10, 10, 10, 60));

  rangeButton.setColour(juce::TextButton::textColourOffId,
                         is60dB ? juce::Colours::white
                                : juce::Colour::fromRGBA(225, 215, 190, 160));
}

void UweVizAudioProcessorEditor::updateWeightingButton() {
  bool active = true;

  switch (weightingMode) {
  case SpectrumComponent::WeightingMode::Tilt3:
    tiltButton.setButtonText("TILT +3");
    break;
  case SpectrumComponent::WeightingMode::Tilt45:
    tiltButton.setButtonText("TILT +4.5");
    break;
  case SpectrumComponent::WeightingMode::A:
    tiltButton.setButtonText("A-WEIGHT");
    break;
  case SpectrumComponent::WeightingMode::C:
    tiltButton.setButtonText("C-WEIGHT");
    break;
  case SpectrumComponent::WeightingMode::D:
    tiltButton.setButtonText("D-WEIGHT");
    break;
  case SpectrumComponent::WeightingMode::Flat:
  default:
    active = false;
    tiltButton.setButtonText("FLAT");
    break;
  }

  tiltButton.setColour(juce::TextButton::buttonColourId,
                       active ? juce::Colour::fromRGBA(88, 174, 219, 140)
                              : juce::Colour::fromRGBA(10, 10, 10, 60));

  tiltButton.setColour(juce::TextButton::textColourOffId,
                       active ? juce::Colours::white
                              : juce::Colour::fromRGBA(225, 215, 190, 160));
}

void UweVizAudioProcessorEditor::updateCrestButton() {
  crestButton.setColour(juce::TextButton::buttonColourId,
                        crestModeEnabled
                            ? juce::Colour::fromRGBA(255, 205, 90, 130)
                            : juce::Colour::fromRGBA(10, 10, 10, 60));

  crestButton.setColour(juce::TextButton::textColourOffId,
                        crestModeEnabled
                            ? juce::Colours::white
                            : juce::Colour::fromRGBA(225, 215, 190, 160));
}

void UweVizAudioProcessorEditor::updateSchemeButton() {
    juce::String text = "FUNKY";
    juce::Colour color = juce::Colour::fromRGBA(170, 80, 255, 120);
    switch (colorScheme) {
        case SpectrumComponent::ColorScheme::Funky:    text = "FUNKY"; color = juce::Colour::fromRGBA(170, 80, 255, 120); break;
        case SpectrumComponent::ColorScheme::Calm:     text = "CALM"; color = juce::Colour::fromRGBA(140, 220, 235, 120); break;
        case SpectrumComponent::ColorScheme::Midnight: text = "MIDNIGHT"; color = juce::Colour::fromRGBA(40, 100, 255, 120); break;
        case SpectrumComponent::ColorScheme::Vintage:  text = "VINTAGE"; color = juce::Colour::fromRGBA(220, 180, 80, 120); break;
    }
    schemeButton.setButtonText(text);
    schemeButton.setColour(juce::TextButton::buttonColourId, color);
    schemeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
}

void UweVizAudioProcessorEditor::updateReferenceButton() {
  const bool active = spectrumComponent.hasReferenceTrace();

  referenceButton.setColour(juce::TextButton::buttonColourId,
                            active
                                ? juce::Colour::fromRGBA(210, 180, 245, 125)
                                : juce::Colour::fromRGBA(10, 10, 10, 60));

  referenceButton.setColour(juce::TextButton::textColourOffId,
                            active
                                ? juce::Colours::white
                                : juce::Colour::fromRGBA(225, 215, 190, 160));
}

void UweVizAudioProcessorEditor::updateAverageButton() {
  averageButton.setColour(juce::TextButton::buttonColourId,
                          averageTraceEnabled
                              ? juce::Colour::fromRGBA(235, 225, 190, 120)
                              : juce::Colour::fromRGBA(10, 10, 10, 60));

  averageButton.setColour(juce::TextButton::textColourOffId,
                          averageTraceEnabled
                              ? juce::Colours::white
                              : juce::Colour::fromRGBA(225, 215, 190, 160));
}

void UweVizAudioProcessorEditor::updateMaskingButton() {
  maskingButton.setColour(juce::TextButton::buttonColourId,
                          maskingVisualizationEnabled
                              ? juce::Colour::fromRGBA(255, 120, 120, 120)
                              : juce::Colour::fromRGBA(10, 10, 10, 60));

  maskingButton.setColour(juce::TextButton::textColourOffId,
                          maskingVisualizationEnabled
                              ? juce::Colours::white
                              : juce::Colour::fromRGBA(225, 215, 190, 160));
}

void UweVizAudioProcessorEditor::updateSmoothingButton() {
    juce::String text = "SMOOTH";
    switch (smoothingMode) {
        case SpectrumComponent::SmoothingMode::Standard: text = "STD SMOOTH"; break;
        case SpectrumComponent::SmoothingMode::RMS:      text = "RMS SMOOTH"; break;
        case SpectrumComponent::SmoothingMode::Peak:     text = "PEAK SMOOTH"; break;
    }
    smoothingButton.setButtonText(text);
    smoothingButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGBA(88, 174, 219, 120));
    smoothingButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
}

void UweVizAudioProcessorEditor::updateMarkersButton() {
    markersButton.setColour(juce::TextButton::buttonColourId,
                           frequencyMarkersEnabled
                               ? juce::Colour::fromRGBA(90, 220, 170, 120)
                               : juce::Colour::fromRGBA(10, 10, 10, 60));
    markersButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
}

void UweVizAudioProcessorEditor::updateGridButton() {
    gridButton.setColour(juce::TextButton::buttonColourId,
                        noteGridEnabled
                            ? juce::Colour::fromRGBA(140, 220, 235, 120)
                            : juce::Colour::fromRGBA(10, 10, 10, 60));
    gridButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
}

void UweVizAudioProcessorEditor::updateZoomButton() {
    juce::String text = "FULL";
    switch (zoomMode) {
        case 0: text = "FULL"; break;
        case 1: text = "LOW"; break;
        case 2: text = "MID"; break;
        case 3: text = "HIGH"; break;
    }
    zoomButton.setButtonText(text);
    zoomButton.setColour(juce::TextButton::buttonColourId, zoomMode != 0 ? juce::Colour::fromRGBA(255, 150, 70, 120) : juce::Colour::fromRGBA(10, 10, 10, 60));
    zoomButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
}

void UweVizAudioProcessorEditor::updateToneButton() {
    bool enabled = audioProcessor.isToneGeneratorEnabled();
    toneButton.setColour(juce::TextButton::buttonColourId,
                        enabled
                            ? juce::Colour::fromRGBA(255, 100, 100, 120)
                            : juce::Colour::fromRGBA(10, 10, 10, 60));
    toneButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
}

void UweVizAudioProcessorEditor::updateSCButton() {
    scButton.setColour(juce::TextButton::buttonColourId,
                       sidechainEnabled
                           ? juce::Colour::fromRGBA(220, 180, 80, 120) // Vintage Amber
                           : juce::Colour::fromRGBA(10, 10, 10, 60));
    scButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
}

void UweVizAudioProcessorEditor::updateClashButton() {
    clashButton.setColour(juce::TextButton::buttonColourId,
                         clashEnabled
                             ? juce::Colour::fromRGBA(255, 60, 60, 120) // Red
                             : juce::Colour::fromRGBA(10, 10, 10, 60));
    clashButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
}

void UweVizAudioProcessorEditor::updatePlayButton() {
    bool playing = audioProcessor.isFilePlaying();
    playButton.setButtonText(playing ? "PAUSE" : "PLAY");
    playButton.setColour(juce::TextButton::buttonColourId,
                        playing ? juce::Colour::fromRGBA(100, 255, 100, 120)
                                : juce::Colour::fromRGBA(10, 10, 10, 60));
    
    stopFileButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGBA(10, 10, 10, 60));
}

bool UweVizAudioProcessorEditor::isInterestedInFileDrag (const juce::StringArray& files) {
    for (auto f : files) {
        if (f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".mp3") || 
            f.endsWithIgnoreCase(".aif") || f.endsWithIgnoreCase(".aiff") ||
            f.endsWithIgnoreCase(".flac"))
            return true;
    }
    return false;
}

void UweVizAudioProcessorEditor::filesDropped (const juce::StringArray& files, int, int) {
    if (files.size() > 0) {
        audioProcessor.loadFile(juce::File(files[0]));
        audioProcessor.playFile();
    }
}

void UweVizAudioProcessorEditor::updateMooseModeStyling() {
  if (displayMode == SpectrumComponent::DisplayMode::LR)
    elchComponent.setVizMode(ElchComponent::VizMode::LR);
  else
    elchComponent.setVizMode(ElchComponent::VizMode::MS);
}

void UweVizAudioProcessorEditor::takeSnapshot() {
    fileChooser = std::make_unique<juce::FileChooser>("Save Snapshot", juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("FunkyMoose_Snapshot.png"), "*.png");
    
    auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting;
    
    fileChooser->launchAsync(chooserFlags, [this] (const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file != juce::File{}) {
            auto bounds = getLocalBounds();
            juce::Image image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
            juce::Graphics g(image);
            paintEntireComponent(g, true);
            
            juce::PNGImageFormat png;
            juce::FileOutputStream stream(file);
            if (stream.openedOk()) {
                png.writeImageToStream(image, stream);
            }
        }
    });
}
