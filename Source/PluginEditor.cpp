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
  subTitleLabel.setText("ANALYZER STRATEGY", juce::dontSendNotification);
  subTitleLabel.setJustificationType(juce::Justification::centredLeft);
  subTitleLabel.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
  subTitleLabel.setColour(juce::Label::textColourId,
                          juce::Colour::fromRGB(200, 200, 200));

  addAndMakeVisible(elchComponent);
  auto img = juce::ImageFileFormat::loadFrom(
      BinaryData::elch_vintage_clean_trans_png,
      BinaryData::elch_vintage_clean_trans_pngSize);
  if (img.isValid())
    elchComponent.setElchImage(img);
  elchComponent.setBackgroundColor(juce::Colours::transparentBlack);

  addAndMakeVisible(spectrumComponent);
  addAndMakeVisible(waveformComponent);
  addAndMakeVisible(meterLeft);
  addAndMakeVisible(meterRight);
  addAndMakeVisible(correlationMeter);
  addAndMakeVisible(loudnessMeter);

  addAndMakeVisible(lrButton);
  addAndMakeVisible(msButton);
  addAndMakeVisible(freezeButton);
  addAndMakeVisible(analyzeOnlyButton);

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
    spectrumComponent.setDisplayMode(displayMode);
    updateModeButtons();
    updateMooseModeStyling();
  };

  msButton.onClick = [this] {
    displayMode = SpectrumComponent::DisplayMode::MS;
    spectrumComponent.setDisplayMode(displayMode);
    updateModeButtons();
    updateMooseModeStyling();
  };

  freezeButton.onClick = [this] {
    frozen = !frozen;
    spectrumComponent.setFrozen(frozen);
    waveformComponent.setFrozen(frozen);
    updateModeButtons();
  };

  spectrumComponent.setDisplayMode(displayMode);
  spectrumComponent.setFrozen(frozen);
  waveformComponent.setFrozen(frozen);

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

  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 12));
  g.drawLine(headerPanel.getX() + 18.0f, headerPanel.getBottom() + 10.0f,
             headerPanel.getRight() - 18.0f, headerPanel.getBottom() + 10.0f,
             1.5f);

  if (frozen) {
    g.setColour(juce::Colour::fromRGBA(255, 220, 140, 160));
    g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
    g.drawText("Analyzer frozen",
               juce::Rectangle<int>(getWidth() - 200, 30, 140, 16),
               juce::Justification::centredRight, false);
  }
}

void UweVizAudioProcessorEditor::resized() {
  auto area = getLocalBounds().reduced(24);

  auto header = area.removeFromTop(88);

  auto leftHeader = header.removeFromLeft(420);
  elchComponent.setBounds(leftHeader.removeFromLeft(110).reduced(2));
  leftHeader.removeFromLeft(12);

  auto titleArea = leftHeader.reduced(0, 10);
  titleLabel.setBounds(titleArea.removeFromTop(30));
  subTitleLabel.setBounds(titleArea.removeFromTop(18));

  // Center pitch display
  pitchLabel.setBounds(header.getCentreX() - 50, header.getY() + 10, 100, 40);

  auto modeArea = header.removeFromRight(230);
  lrButton.setBounds(modeArea.removeFromLeft(58).reduced(4));
  msButton.setBounds(modeArea.removeFromLeft(58).reduced(4));
  modeArea.removeFromLeft(8);
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

  auto &meter = audioProcessor.getMeterSource();
  elchComponent.setVizSignal(meter.getRmsLeft() + meter.getRmsRight(), 
                             std::max(meter.getPeakLeft(), meter.getPeakRight()));
  meter.decay();

  meterLeft.setLevels(meter.getPeakLeft(), meter.getHoldLeft(),
                      meter.getRmsLeft());
  meterRight.setLevels(meter.getPeakRight(), meter.getHoldRight(),
                       meter.getRmsRight());
  correlationMeter.setCorrelation(meter.getCorrelation());
  loudnessMeter.setLoudness(meter.getLoudness());

  // Update Pitch Label
  float domFreq = audioProcessor.getDominantFrequency();
  if (domFreq > 25.0f) {
      pitchLabel.setText(UweVizAudioProcessor::frequencyToNote(domFreq), juce::dontSendNotification);
  } else {
      pitchLabel.setText("---", juce::dontSendNotification);
  }
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
}

void UweVizAudioProcessorEditor::updateMooseModeStyling() {
  if (displayMode == SpectrumComponent::DisplayMode::LR)
    elchComponent.setVizMode(ElchComponent::VizMode::LR);
  else
    elchComponent.setVizMode(ElchComponent::VizMode::MS);
}
