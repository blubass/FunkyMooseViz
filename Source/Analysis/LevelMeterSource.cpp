#include "LevelMeterSource.h"

void LevelMeterSource::prepare(double sampleRate, int maxSamplesPerBlock) {
  currentSampleRate = sampleRate;
  peakL = peakR = holdL = holdR = rmsL = rmsR = loudness = 0.0f;

  // 4x oversampling for True Peak (ISP) detection
  truePeakOversampler = std::make_unique<juce::dsp::Oversampling<float>>(
      2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true,
      false);
  truePeakOversampler->initProcessing(static_cast<size_t>(maxSamplesPerBlock));
  
  // Initialize K-Weighting Filters
  auto stage1Coeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 1500.0, 0.707f, 1.584f); // +4dB
  auto stage2Coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 100.0, 0.5f); // RLB
  
  *kWeightStage1L.coefficients = *stage1Coeffs;
  *kWeightStage1R.coefficients = *stage1Coeffs;
  *kWeightStage2L.coefficients = *stage2Coeffs;
  *kWeightStage2R.coefficients = *stage2Coeffs;
  
  kWeightStage1L.reset(); kWeightStage1R.reset();
  kWeightStage2L.reset(); kWeightStage2R.reset();

  // Momentary LUFS 400ms window buffer init
  int momentarySamples = static_cast<int>(sampleRate * 0.4);
  momentaryBufferL.assign(static_cast<size_t>(momentarySamples), 0.0f);
  momentaryBufferR.assign(static_cast<size_t>(momentarySamples), 0.0f);
  momentaryWritePosition = 0;
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
    peakL = juce::jmax(peakL.load(), ispL);
    peakR = juce::jmax(peakR.load(), ispR);
    holdL = juce::jmax(holdL.load(), ispL);
    holdR = juce::jmax(holdR.load(), ispR);
  }
  
  // Calculate Actual BS.1770-4 Loudness (K-Weighted RMS)
  juce::AudioBuffer<float> kFilteredBuffer;
  kFilteredBuffer.makeCopyOf(buffer);
  
  const int numSamples = kFilteredBuffer.getNumSamples();
  const int numChannels = kFilteredBuffer.getNumChannels();
  
  if (numChannels > 0) {
      float* data = kFilteredBuffer.getWritePointer(0);
      for (int i = 0; i < numSamples; ++i) {
          data[i] = kWeightStage2L.processSample(kWeightStage1L.processSample(data[i]));
      }
  }
  if (numChannels > 1) {
      float* data = kFilteredBuffer.getWritePointer(1);
      for (int i = 0; i < numSamples; ++i) {
          data[i] = kWeightStage2R.processSample(kWeightStage1R.processSample(data[i]));
      }
  }
  
  // Add to 400ms sliding window ring buffer
  const int momentarySamples = static_cast<int>(momentaryBufferL.size());
  if (momentarySamples > 0) {
      int writePosL = momentaryWritePosition;
      int writePosR = momentaryWritePosition;
      
      if (numChannels > 0) {
          const float* data = kFilteredBuffer.getReadPointer(0);
          for (int i = 0; i < numSamples; ++i) {
              momentaryBufferL[static_cast<size_t>(writePosL)] = data[i] * data[i];
              writePosL = (writePosL + 1) % momentarySamples;
          }
      }
      if (numChannels > 1) {
          const float* data = kFilteredBuffer.getReadPointer(1);
          for (int i = 0; i < numSamples; ++i) {
              momentaryBufferR[static_cast<size_t>(writePosR)] = data[i] * data[i];
              writePosR = (writePosR + 1) % momentarySamples;
          }
      }
      if (numChannels > 0) momentaryWritePosition = writePosL;
  }
  
  // Calculate Mean Square over the full 400ms window
  float meanSquareL = 0.0f;
  float meanSquareR = 0.0f;
  
  if (momentarySamples > 0) {
      float sumL = 0.0f;
      for (float val : momentaryBufferL) sumL += val;
      meanSquareL = sumL / (float)momentarySamples;
      
      float sumR = 0.0f;
      for (float val : momentaryBufferR) sumR += val;
      meanSquareR = sumR / (float)momentarySamples;
  }
  
  // BS.1770-4 Loudness sum: Energies are summed (no average dividing by 2!)
  const float totalMeanSquare = (numChannels > 1) ? (meanSquareL + meanSquareR) : meanSquareL;
  
  // BS.1770-4 formula for Momentary Loudness 
  // LUFS = -0.69 + 10 * log10(Total Mean Square)
  float lufs = (totalMeanSquare > 0.0000001f) ? (-0.69f + 10.0f * std::log10(totalMeanSquare)) : -100.0f;
  
  // Map LUFS to 0..1 range for the meter display (-60 to 0 LUFS range)
  float lufsNorm = juce::jlimit(0.0f, 1.0f, juce::jmap(lufs, -60.0f, 0.0f, 0.0f, 1.0f));
  
  float currentLoudness = loudness.load();
  // Less artificial smoothing needed now since we have a true 400ms window, 
  // but a tiny bit prevents visual jitter from UI framerates
  loudness.store(currentLoudness + 0.35f * (lufsNorm - currentLoudness));
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

  peakL = juce::jmax(peakL.load(), newPeakL);
  peakR = juce::jmax(peakR.load(), newPeakR);
  holdL = juce::jmax(holdL.load(), newPeakL);
  holdR = juce::jmax(holdR.load(), newPeakR);
  rmsL = rmsL + rmsSmoothing * (newRmsL - rmsL);
  rmsR = rmsR + rmsSmoothing * (newRmsR - rmsR);
}

void LevelMeterSource::decay() {
  peakL = juce::jmax(0.0f, peakL - peakFalloff);
  peakR = juce::jmax(0.0f, peakR - peakFalloff);
  holdL = juce::jmax(0.0f, holdL - holdFalloff);
  holdR = juce::jmax(0.0f, holdR - holdFalloff);
}
