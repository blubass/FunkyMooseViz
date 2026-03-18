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
  addAndMakeVisible(meterLeft);
  addAndMakeVisible(meterRight);
  addAndMakeVisible(correlationMeter);
  addAndMakeVisible(loudnessMeter);

  addAndMakeVisible(pitchLabel);
  addAndMakeVisible(lrButton);
  addAndMakeVisible(msButton);
  addAndMakeVisible(waterfallButton);
  addAndMakeVisible(rangeButton);
  addAndMakeVisible(freezeButton);
  addAndMakeVisible(analyzeOnlyButton);

  // Sync state from processor
  displayMode = static_cast<SpectrumComponent::DisplayMode>(audioProcessor.getDisplayMode());
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
    spectrumComponent.setDisplayMode(displayMode);
    updateModeButtons();
    updateMooseModeStyling();
  };

  msButton.onClick = [this] {
    displayMode = SpectrumComponent::DisplayMode::MS;
    audioProcessor.setDisplayMode(1);
    spectrumComponent.setDisplayMode(displayMode);
    updateModeButtons();
    updateMooseModeStyling();
  };

  waterfallButton.onClick = [this] {
    displayMode = SpectrumComponent::DisplayMode::Waterfall;
    audioProcessor.setDisplayMode(2); // New mode index
    spectrumComponent.setDisplayMode(displayMode);
    updateModeButtons();
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

  spectrumComponent.setDisplayMode(displayMode);
  spectrumComponent.setFrozen(frozen);
  spectrumComponent.setDisplayRange((float)audioProcessor.getDisplayRange());
  waveformComponent.setFrozen(frozen);
  
  if (audioProcessor.getDisplayRange() == 60) {
      rangeButton.setButtonText("60 dB");
  } else {
      rangeButton.setButtonText("90 dB");
  }

  updateModeButtons();
  pitchLabel.setJustificationType(juce::Justification::centred);
  pitchLabel.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
  pitchLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(88, 174, 219));
  pitchLabel.setText("---", juce::dontSendNotification);

  updateMooseModeStyling();

  startTimerHz(30);
}

UweVizAudioProcessorEditor::~UweVizAudioProcessorEditor() {}

void UweVizAudioProcessorEditor::paint(juce::Graphics &g) {
  // Funky Moose Metallic Background
  g.setColour(juce::Colour::fromRGB(10, 10, 10));
  g.fillAll();

  auto area = getLocalBounds().toFloat();
  auto shell = area.reduced(8.0f);

  // Outer Shell Bevel
  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 8));
  g.drawRoundedRectangle(shell, 12.0f, 1.5f);
  g.setColour(juce::Colour::fromRGBA(0, 0, 0, 150));
  g.drawRoundedRectangle(shell.reduced(1.0f), 12.0f, 1.0f);

  // Subtle Cyan highlight at the top (Rim Light)
  g.setColour(juce::Colour::fromRGBA(88, 174, 219, 15));
  g.drawRoundedRectangle(shell.reduced(3.0f), 12.0f, 1.2f);

  // Header Panel - Premium Metal Look
  auto innerShell = shell.reduced(12.0f);
  auto headerPanel = innerShell.removeFromTop(88.0f);
  
  juce::ColourGradient headerGrad(juce::Colour::fromRGB(35, 38, 42), headerPanel.getX(), headerPanel.getY(),
                                  juce::Colour::fromRGB(15, 16, 18), headerPanel.getX(), headerPanel.getBottom(), false);
  g.setGradientFill(headerGrad);
  g.fillRoundedRectangle(headerPanel, 6.0f);
  
  // Header Rim & Inner Shadow
  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 15));
  g.drawRoundedRectangle(headerPanel, 6.0f, 0.8f);
  g.setColour(juce::Colour::fromRGBA(0, 0, 0, 180));
  g.drawRoundedRectangle(headerPanel.reduced(1.0f), 6.0f, 1.0f);

  // Draw the new pitch box background next to mode buttons
  auto pb = pitchLabel.getBounds().toFloat();
  
  // Glass effect for pitch box
  g.setColour(juce::Colour::fromRGBA(20, 25, 30, 180));
  g.fillRoundedRectangle(pb, 4.0f);
  
  juce::ColourGradient pbRim(juce::Colour::fromRGBA(88, 174, 219, 40), pb.getX(), pb.getY(),
                             juce::Colour::fromRGBA(88, 174, 219, 10), pb.getX(), pb.getBottom(), false);
  g.setGradientFill(pbRim);
  g.drawRoundedRectangle(pb, 4.0f, 1.0f);
  
  // Sub-glow for pitch box
  g.setColour(juce::Colour::fromRGBA(88, 174, 219, 15));
  g.fillRoundedRectangle(pb.reduced(2.0f), 4.0f);

  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 12));
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

  waveformComponent.setBounds(lower);

  area.removeFromBottom(14);
  spectrumComponent.setBounds(area);
}

void UweVizAudioProcessorEditor::timerCallback() {
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

void UweVizAudioProcessorEditor::updateMooseModeStyling() {
  if (displayMode == SpectrumComponent::DisplayMode::LR)
    elchComponent.setVizMode(ElchComponent::VizMode::LR);
  else
    elchComponent.setVizMode(ElchComponent::VizMode::MS);
}
