#include "PluginEditor.h"
#include "BinaryData.h"

UweVizAudioProcessorEditor::UweVizAudioProcessorEditor(UweVizAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setSize(960, 620);
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
  addAndMakeVisible(rangeButton);
  addAndMakeVisible(freezeButton);
  addAndMakeVisible(analyzeOnlyButton);

  // Sync state from processor
  displayMode = static_cast<SpectrumComponent::DisplayMode>(audioProcessor.getDisplayMode());
  frozen = audioProcessor.getFrozen();

  meterLeft.setLabel("LEFT");
  meterRight.setLabel("RIGHT");

  analyzeOnlyButton.setToggleState(audioProcessor.getAnalyzeOnly(),
                                   juce::dontSendNotification);
  analyzeOnlyButton.setColour(juce::ToggleButton::textColourId,
                              juce::Colour::fromRGBA(230, 235, 240, 190));
  analyzeOnlyButton.onClick = [this] {
    audioProcessor.setAnalyzeOnly(analyzeOnlyButton.getToggleState());
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
  addAndMakeVisible(pitchLabel);
  pitchLabel.setJustificationType(juce::Justification::centred);
  pitchLabel.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
  pitchLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(88, 174, 219));
  pitchLabel.setText("---", juce::dontSendNotification);

  updateMooseModeStyling();

  startTimerHz(30);
}

UweVizAudioProcessorEditor::~UweVizAudioProcessorEditor() {}

void UweVizAudioProcessorEditor::paint(juce::Graphics &g) {
  const auto bounds = getLocalBounds().toFloat();

  // Funky Moose Metallic Background
  g.setColour(juce::Colour::fromRGB(10, 10, 10));
  g.fillAll();

  auto shell = bounds.reduced(8.0f);
  
  // Metallic Outer Frame (Beveled)
  juce::ColourGradient frameGrad(juce::Colour::fromRGB(80, 85, 90), shell.getX(), shell.getY(),
                                 juce::Colour::fromRGB(40, 42, 45), shell.getX(), shell.getBottom(), false);
  frameGrad.addColour(0.05f, juce::Colour::fromRGB(110, 115, 120)); // Highlight
  frameGrad.addColour(0.95f, juce::Colour::fromRGB(20, 22, 25));    // Shadow
  
  g.setGradientFill(frameGrad);
  g.drawRoundedRectangle(shell, 12.0f, 5.0f); // Thicker frame

  // Inner Dark Plate
  g.setColour(juce::Colour::fromRGB(18, 18, 18));
  g.fillRoundedRectangle(shell.reduced(2.5f), 10.0f);

  // Inner Glow/Accent
  g.setColour(juce::Colour::fromRGBA(88, 174, 219, 20));
  g.drawRoundedRectangle(shell.reduced(3.0f), 12.0f, 1.2f);

  auto headerPanel = shell.reduced(12.0f).removeFromTop(80.0f);
  
  // Header Style from Amp (Beveled)
  juce::ColourGradient headerGrad(juce::Colour::fromRGB(30, 30, 30), headerPanel.getX(), headerPanel.getY(),
                                  juce::Colour::fromRGB(10, 10, 10), headerPanel.getX(), headerPanel.getBottom(), false);
  g.setGradientFill(headerGrad);
  g.fillRoundedRectangle(headerPanel, 6.0f);
  
  // Bevel line
  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 35));
  g.drawRoundedRectangle(headerPanel, 6.0f, 1.2f);
  g.setColour(juce::Colour::fromRGBA(0, 0, 0, 80));
  g.drawRoundedRectangle(headerPanel.reduced(1.0f), 6.0f, 0.8f);

  // Draw the new pitch box background next to mode buttons
  auto pb = pitchLabel.getBounds().toFloat();
  g.setColour(juce::Colour::fromRGBA(10, 15, 20, 180));
  g.fillRoundedRectangle(pb, 4.0f);
  g.setColour(juce::Colour::fromRGBA(132, 238, 255, 60));
  g.drawRoundedRectangle(pb, 4.0f, 1.0f);

  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 12));
  g.drawLine(headerPanel.getX() + 18.0f, headerPanel.getBottom() + 10.0f,
             headerPanel.getRight() - 18.0f, headerPanel.getBottom() + 10.0f,
             1.5f);
}

void UweVizAudioProcessorEditor::resized() {
  auto area = getLocalBounds().reduced(24);
  auto header = area.removeFromTop(88);

  // Logo Left (Square-ish)
  elchComponent.setBounds(header.removeFromLeft(100).reduced(2));
  header.removeFromLeft(12);

  auto titleArea = header.removeFromLeft(300).reduced(0, 10);
  titleLabel.setBounds(titleArea.removeFromTop(30));
  subTitleLabel.setBounds(titleArea.removeFromTop(18));

  // Right Side: Combined Mode & Pitch Group
  auto rightGroup = header.removeFromRight(370);
  
  auto pitchArea = rightGroup.removeFromLeft(80).reduced(0, 20);
  pitchLabel.setBounds(pitchArea);
  pitchLabel.setJustificationType(juce::Justification::centred);

  auto modeArea = rightGroup;
  lrButton.setBounds(modeArea.removeFromLeft(56).reduced(4));
  msButton.setBounds(modeArea.removeFromLeft(56).reduced(4));
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

  std::vector<float> magsA;
  std::vector<float> magsB;

  if (displayMode == SpectrumComponent::DisplayMode::LR) {
    if (audioProcessor.getFFTProcessorLeft().getMagnitudes(magsA))
      spectrumComponent.setMagnitudesLeft(magsA);

    if (audioProcessor.getFFTProcessorRight().getMagnitudes(magsB))
      spectrumComponent.setMagnitudesRight(magsB);
  } else {
    if (audioProcessor.getFFTProcessorMid().getMagnitudes(magsA))
      spectrumComponent.setMagnitudesLeft(magsA);

    if (audioProcessor.getFFTProcessorSide().getMagnitudes(magsB))
      spectrumComponent.setMagnitudesRight(magsB);
  }

  std::vector<float> waveformL, waveformR;
  audioProcessor.getWaveformBuffer().getStereoWaveform(waveformL, waveformR);
  waveformComponent.setWaveform(waveformL, waveformR);

  // Pitch detection logic
  if (magsA.size() > 10) {
      // Find peak in a sensible range (ignore DC and very low rumble)
      int minBin = (int)std::ceil(20.0f * (float)magsA.size() / ((float)audioProcessor.getFFTProcessorLeft().getSampleRate() / 2.0f));
      auto searchStart = magsA.begin() + minBin;
      auto maxIt = std::max_element(searchStart, magsA.end());
      float maxDb = *maxIt;
      
      juce::String calcNote = "---";
      
      if (maxDb > -40.0f && !frozen) {
          int bin = (int)std::distance(magsA.begin(), maxIt);
          
          float binFreq = (float)bin * ((float)audioProcessor.getFFTProcessorLeft().getSampleRate() / 2.0f) / (float)magsA.size();
          float domFreq = binFreq;
          
          // Parabolic interpolation for sub-bin accuracy
          if (bin > 0 && bin < (int)magsA.size() - 1) {
              float g1 = juce::Decibels::decibelsToGain(magsA[(size_t)bin - 1]);
              float g2 = juce::Decibels::decibelsToGain(magsA[(size_t)bin]);
              float g3 = juce::Decibels::decibelsToGain(magsA[(size_t)bin + 1]);
              
              float denominator = (g1 - 2.0f * g2 + g3);
              if (std::abs(denominator) > 1e-6f) {
                  float p = 0.5f * (g1 - g3) / denominator;
                  domFreq = ((float)bin + p) * ((float)audioProcessor.getFFTProcessorLeft().getSampleRate() / 2.0f) / (float)magsA.size();
              }
          }
          
          calcNote = UweVizAudioProcessor::frequencyToNote(domFreq);
      }
      
      if (calcNote == targetNoteStr) {
          if (noteHoldCount < 10) noteHoldCount++;
          if (noteHoldCount >= 3) {
              currentNoteStr = targetNoteStr;
          }
      } else {
          targetNoteStr = calcNote;
          noteHoldCount = 0;
      }
  }

  pitchLabel.setText(currentNoteStr, juce::dontSendNotification);

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
