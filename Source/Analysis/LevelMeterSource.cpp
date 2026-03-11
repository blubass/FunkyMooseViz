#include "LevelMeterSource.h"

void LevelMeterSource::prepare(double sampleRate, int maxSamplesPerBlock) {
  currentSampleRate = sampleRate;
  peakL = peakR = holdL = holdR = rmsL = rmsR = loudness = 0.0f;

  // 4x oversampling for True Peak (ISP) detection
  truePeakOversampler = std::make_unique<juce::dsp::Oversampling<float>>(
      2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true,
      false);
  truePeakOversampler->initProcessing(static_cast<size_t>(maxSamplesPerBlock));
}

void LevelMeterSource::processBlock(juce::AudioBuffer<float> &buffer) {
  processBlock(static_cast<const juce::AudioBuffer<float> &>(buffer));

  // Determine True Peak (ISP) via oversampling
  if (truePeakOversampler) {
    juce::dsp::AudioBlock<const float> block(buffer);
    auto oversampledBlock = truePeakOversampler->processSamplesUp(block);

    float ispL = 0.0f;
    float ispR = 0.0f;

    if (oversampledBlock.getNumChannels() > 0) {
      for (int i = 0; i < (int)oversampledBlock.getNumSamples(); ++i)
        ispL = juce::jmax(ispL, std::abs(oversampledBlock.getSample(0, i)));
    }

    if (oversampledBlock.getNumChannels() > 1) {
      for (int i = 0; i < (int)oversampledBlock.getNumSamples(); ++i)
        ispR = juce::jmax(ispR, std::abs(oversampledBlock.getSample(1, i)));
    }

    // Update peaks with intercepted sample peaks
    peakL = juce::jmax(peakL, ispL);
    peakR = juce::jmax(peakR, ispR);
    holdL = juce::jmax(holdL, ispL);
    holdR = juce::jmax(holdR, ispR);
  }

  // Calculate a simplified "Perceived Loudness" (Weighted RMS)
  float totalRms = (rmsL + rmsR) * 0.5f;
  // Weighting: slightly prioritize mids/highs for "funkiness" perception
  loudness = loudness + 0.15f * (totalRms - loudness);
}

void LevelMeterSource::processBlock(const juce::AudioBuffer<float> &buffer) {
  const int numSamples = buffer.getNumSamples();
  const int numChannels = buffer.getNumChannels();

  float newPeakL = 0.0f;
  float newPeakR = 0.0f;
  float newRmsL = 0.0f;
  float newRmsR = 0.0f;

  if (numChannels > 0) {
    newPeakL = buffer.getMagnitude(0, 0, numSamples);
    double sum = 0.0;
    const float *data = buffer.getReadPointer(0);
    for (int i = 0; i < numSamples; ++i)
      sum += data[i] * data[i];
    newRmsL = std::sqrt((float)(sum / juce::jmax(1, numSamples)));
  }

  if (numChannels > 1) {
    newPeakR = buffer.getMagnitude(1, 0, numSamples);
    double sum = 0.0;
    const float *data = buffer.getReadPointer(1);
    for (int i = 0; i < numSamples; ++i)
      sum += data[i] * data[i];
    newRmsR = std::sqrt((float)(sum / juce::jmax(1, numSamples)));

    double sumLL = 0.0, sumRR = 0.0, sumLR = 0.0;
    const float *leftData = buffer.getReadPointer(0);
    for (int i = 0; i < numSamples; ++i) {
      float l = leftData[i];
      float r = data[i];
      sumLL += l * l;
      sumRR += r * r;
      sumLR += l * r;
    }

    float currentCorr = 1.0f;
    if (sumLL > 0.0 && sumRR > 0.0)
      currentCorr = (float)(sumLR / std::sqrt(sumLL * sumRR));

    correlation = correlation + 0.05f * (currentCorr - correlation);
  } else {
    newPeakR = newPeakL;
    newRmsR = newRmsL;
    correlation = 1.0f;
  }

  peakL = juce::jmax(peakL, newPeakL);
  peakR = juce::jmax(peakR, newPeakR);
  holdL = juce::jmax(holdL, newPeakL);
  holdR = juce::jmax(holdR, newPeakR);
  rmsL = rmsL + rmsSmoothing * (newRmsL - rmsL);
  rmsR = rmsR + rmsSmoothing * (newRmsR - rmsR);
}

void LevelMeterSource::decay() {
  peakL = juce::jmax(0.0f, peakL - peakFalloff);
  peakR = juce::jmax(0.0f, peakR - peakFalloff);
  holdL = juce::jmax(0.0f, holdL - holdFalloff);
  holdR = juce::jmax(0.0f, holdR - holdFalloff);
}
