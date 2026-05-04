#include "SpectrumComponent.h"

SpectrumComponent::SpectrumComponent() {
}

void SpectrumComponent::setAnalysisInfo(double newSampleRate, int newFFTSize) {
  if (sampleRate != newSampleRate || fftSize != newFFTSize) {
    sampleRate = newSampleRate;
    fftSize = newFFTSize;
    needsLookupUpdate = true;
  }
}

void SpectrumComponent::resized() {
  needsLookupUpdate = true;
}

juce::Rectangle<float> SpectrumComponent::getSpectrumBounds() const {
  return getLocalBounds().toFloat().reduced(12.0f);
}

void SpectrumComponent::updateLookupTable() {
  auto inner = getSpectrumBounds();
  const int width = (int)inner.getWidth();
  if (width <= 0) return;

  binLookupTable.resize((size_t)width);
  const float minLog = std::log10(minFrequency);
  const float maxLog = std::log10(maxFrequency);

  for (int x = 0; x < width; ++x) {
    const float normX = (float)x / (float)width;
    const float freqLog = minLog + normX * (maxLog - minLog);
    const float freq = std::pow(10.0f, freqLog);
    binLookupTable[(size_t)x] = juce::jlimit(0, fftSize / 2, (int)(freq * fftSize / sampleRate));
  }
  needsLookupUpdate = false;
}

void SpectrumComponent::setDisplayMode(DisplayMode newMode) {
  displayMode = newMode;
  repaint();
}

void SpectrumComponent::setFrozen(bool shouldBeFrozen) {
  frozen = shouldBeFrozen;
  if (frozen)
    inspectionActive = false;
  repaint();
}

void SpectrumComponent::setDetectedNote(const juce::String &note,
                                        float frequency) {
  if (note != "---" && note != "--") {
    currentNote = note;
    currentPitchFreq = frequency;
    noteAlpha = juce::jmin(1.0f, noteAlpha + 0.15f);
  } else {
    noteAlpha = juce::jmax(0.0f, noteAlpha - 0.04f);
  }
}

void SpectrumComponent::setDisplayRange(float newRangeDb) {
  displayRangeDb = newRangeDb;
  repaint();
}

void SpectrumComponent::setSidechainData (const std::vector<float>& left, const std::vector<float>& right)
{
    if (sidechainLeft.size() != left.size()) sidechainLeft.resize(left.size(), -120.0f);
    if (sidechainRight.size() != right.size()) sidechainRight.resize(right.size(), -120.0f);
    
    std::copy(left.begin(), left.end(), sidechainLeft.begin());
    std::copy(right.begin(), right.end(), sidechainRight.begin());
    
    if (sidechainVisible) repaint();
}

void SpectrumComponent::setWeightingMode(WeightingMode newMode) {
  weightingMode = newMode;
  repaint();
}

void SpectrumComponent::setSmoothingMode(SmoothingMode newMode) {
  smoothingMode = newMode;
  repaint();
}

void SpectrumComponent::setCrestModeEnabled(bool shouldBeEnabled) {
  crestModeEnabled = shouldBeEnabled;
  repaint();
}

void SpectrumComponent::setColorScheme(ColorScheme newScheme) {
  colorScheme = newScheme;
  repaint();
}

void SpectrumComponent::setFrequencyMarkersEnabled(bool shouldBeEnabled) {
  frequencyMarkersEnabled = shouldBeEnabled;
  repaint();
}

void SpectrumComponent::setNoteGridEnabled(bool shouldBeEnabled) {
  noteGridEnabled = shouldBeEnabled;
  repaint();
}

void SpectrumComponent::setFrequencyRange(float minFreq, float maxFreq) {
    minFrequency = juce::jlimit(2.0f, 1000.0f, minFreq);
    maxFrequency = juce::jlimit(2000.0f, 24000.0f, maxFreq);
    needsLookupUpdate = true;
    repaint();
}

bool SpectrumComponent::captureReferenceTrace() {
  if (smoothedLeft.empty() && smoothedRight.empty())
    return false;

  referenceTraceLeft = smoothedLeft;
  referenceTraceRight = smoothedRight;
  referenceTraceVisible = true;
  repaint();
  return true;
}

void SpectrumComponent::clearReferenceTrace() {
  referenceTraceVisible = false;
  referenceTraceLeft.clear();
  referenceTraceRight.clear();
  repaint();
}

bool SpectrumComponent::hasReferenceTrace() const {
  return referenceTraceVisible;
}

void SpectrumComponent::setAverageTraceEnabled(bool shouldBeEnabled) {
  averageTraceEnabled = shouldBeEnabled;

  if (!averageTraceEnabled)
    resetAverageTrace();
  else
    repaint();
}

bool SpectrumComponent::isAverageTraceEnabled() const {
  return averageTraceEnabled;
}

void SpectrumComponent::resetAverageTrace() {
  averageTraceVisible = false;
  averageTraceLeft.clear();
  averageTraceRight.clear();
  repaint();
}


void SpectrumComponent::smoothInto(const std::vector<float> &input,
                                   std::vector<float> &target) {
  if (target.size() != input.size())
    target = input;

  float smoothing = 0.65f;

  switch (smoothingMode) {
  case SmoothingMode::RMS:
      smoothIntoRMS(input, target);
      return;
  case SmoothingMode::Peak:
      smoothIntoPeak(input, target);
      return;
  case SmoothingMode::Standard:
  default:
      break;
  }

  for (size_t i = 0; i < input.size(); ++i)
    target[i] = smoothing * target[i] + (1.0f - smoothing) * input[i];
}

void SpectrumComponent::smoothIntoRMS(const std::vector<float> &input,
                                      std::vector<float> &target) {
    if (target.size() != input.size())
        target = input;

    const float smoothing = 0.85f; // Slower for RMS
    for (size_t i = 0; i < input.size(); ++i) {
        // Convert dB to power, average, then back to dB
        float powerIn = std::pow(10.0f, input[i] / 10.0f);
        float powerTarget = std::pow(10.0f, target[i] / 10.0f);
        float smoothedPower = smoothing * powerTarget + (1.0f - smoothing) * powerIn;
        target[i] = 10.0f * std::log10(std::max(1e-10f, smoothedPower));
    }
}

void SpectrumComponent::smoothIntoPeak(const std::vector<float> &input,
                                       std::vector<float> &target) {
    if (target.size() != input.size())
        target = input;

    const float attack = 0.2f;   // Fast attack
    const float release = 0.95f; // Slow release
    
    for (size_t i = 0; i < input.size(); ++i) {
        float coeff = (input[i] > target[i]) ? attack : release;
        target[i] = coeff * target[i] + (1.0f - coeff) * input[i];
    }
}

void SpectrumComponent::updateAverageTrace(const std::vector<float> &input,
                                           std::vector<float> &target) {
  if (!averageTraceEnabled || input.empty())
    return;

  if (target.size() != input.size()) {
    target = input;
    averageTraceVisible = true;
    return;
  }

  constexpr float averaging = 0.965f;
  for (size_t i = 0; i < input.size(); ++i)
    target[i] = averaging * target[i] + (1.0f - averaging) * input[i];

  averageTraceVisible = true;
}

void SpectrumComponent::setMagnitudesLeft(
    const std::vector<float> &newMagnitudes) {
  if (isAnalyzerFrozen())
    return;

  smoothInto(newMagnitudes, smoothedLeft);
  updateAverageTrace(smoothedLeft, averageTraceLeft);

  // Calculate masking threshold if visualization is enabled
  if (maskingVisualizationEnabled)
    updateMaskingThreshold(smoothedLeft, maskingThresholdLeft);

  // Push to spectrogram (Mid or Left depending on mode)
  if (spectrogramFrames.size() >= maxSpectrogramFrames)
      spectrogramFrames.pop_front();
  
  spectrogramFrames.push_back(smoothedLeft);

  repaint();
}

void SpectrumComponent::setMagnitudesRight(
    const std::vector<float> &newMagnitudes) {
  if (isAnalyzerFrozen())
    return;

  smoothInto(newMagnitudes, smoothedRight);
  updateAverageTrace(smoothedRight, averageTraceRight);

  // Calculate masking threshold if visualization is enabled
  if (maskingVisualizationEnabled)
    updateMaskingThreshold(smoothedRight, maskingThresholdRight);

  repaint();
}

SpectrumComponent::RulerPoint
SpectrumComponent::pointToRulerPoint(juce::Point<float> point) const {
  auto inner = getSpectrumBounds();
  if (inner.getWidth() <= 0.0f || inner.getHeight() <= 0.0f)
    return {};

  return {
      juce::jlimit(0.0f, 1.0f, (point.x - inner.getX()) / inner.getWidth()),
      juce::jlimit(0.0f, 1.0f,
                   (inner.getBottom() - point.y) / inner.getHeight())};
}

juce::Point<float>
SpectrumComponent::rulerPointToPosition(RulerPoint point) const {
  auto inner = getSpectrumBounds();
  return {inner.getX() + point.xNorm * inner.getWidth(),
          inner.getBottom() - point.yNorm * inner.getHeight()};
}

float SpectrumComponent::rulerPointToFrequency(RulerPoint point) const {
  const float minLog = std::log10(minFrequency);
  const float maxLog = std::log10(maxFrequency);
  return std::pow(10.0f, minLog + point.xNorm * (maxLog - minLog));
}

float SpectrumComponent::rulerPointToDb(RulerPoint point) const {
  return -displayRangeDb + point.yNorm * displayRangeDb;
}

juce::String SpectrumComponent::formatFrequency(float frequency) const {
  if (frequency >= 1000.0f)
    return juce::String(frequency / 1000.0f, 2) + " kHz";

  return juce::String(frequency, frequency >= 100.0f ? 0 : 1) + " Hz";
}

juce::String SpectrumComponent::buildRulerTooltip() const {
  const float startFreq = rulerPointToFrequency(rulerStart);
  const float endFreq = rulerPointToFrequency(rulerEnd);
  const float startDb = rulerPointToDb(rulerStart);
  const float endDb = rulerPointToDb(rulerEnd);
  const float octaves = std::log2(endFreq / startFreq);
  const float deltaDb = endDb - startDb;

  return formatFrequency(startFreq) + " -> " + formatFrequency(endFreq) +
         " | " + juce::String(octaves, 2) + " oct | " +
         juce::String(deltaDb >= 0.0f ? "+" : "") +
         juce::String(deltaDb, 1) + " dB";
}

bool SpectrumComponent::isAnalyzerFrozen() const {
  return frozen || inspectionActive;
}

float SpectrumComponent::frequencyToNormX(float frequency) const {
  const float minLog = std::log10(minFrequency);
  const float maxLog = std::log10(maxFrequency);
  const float fLog = std::log10(juce::jlimit(minFrequency, maxFrequency, frequency));

  return (fLog - minLog) / (maxLog - minLog);
}

// Calculate A-Weighting according to IEC 61672-1:2003
static float calculateAWeightingDb(float f) {
  if (f < 1.0f) return -100.0f;
  
  const double f2 = (double)f * f;
  const double rA = (12194.0 * 12194.0 * f2 * f2) /
                    ((f2 + 20.6 * 20.6) * 
                     std::sqrt((f2 + 107.7 * 107.7) * (f2 + 737.9 * 737.9)) * 
                     (f2 + 12194.0 * 12194.0));
  
  return (float)(20.0 * std::log10(rA) + 2.0);
}

// Calculate C-Weighting according to IEC 61672-1:2003
static float calculateCWeightingDb(float f) {
  if (f < 1.0f) return -100.0f;
  
  const double f2 = (double)f * f;
  const double rC = (12194.0 * 12194.0 * f2) /
                    ((f2 + 20.6 * 20.6) * (f2 + 12194.0 * 12194.0));
  
  return (float)(20.0 * std::log10(rC) + 0.06);
}

// Calculate D-Weighting according to IEC 537
static float calculateDWeightingDb(float f) {
  if (f < 1.0f) return -100.0f;
  
  const double f2 = (double)f * f;
  const double fr = f;
  const double rD = (fr / (6.8966888496476e-5)) * 
                    std::sqrt(
                      (std::pow(1037918.48 - f2, 2) + (1080768.16 * f2)) / 
                      ((9837328.0 - f2) * (std::pow(11723776.0 - f2, 2) + (5037160.0 * f2)))
                    );
  
  // Normalization at 1kHz for D-weighting is often around -3.4dB relative to 0dB reference
  return (float)(20.0 * std::log10(rD));
}

static float calculateWeightingDb(float frequency,
                                  SpectrumComponent::WeightingMode mode) {
  const float clampedFreq = juce::jlimit(20.0f, 20000.0f, frequency);

  switch (mode) {
  case SpectrumComponent::WeightingMode::Tilt3:
    return 3.0f * std::log2(clampedFreq / 1000.0f);
  case SpectrumComponent::WeightingMode::Tilt45:
    return 4.5f * std::log2(clampedFreq / 1000.0f);
  case SpectrumComponent::WeightingMode::A:
    return calculateAWeightingDb(clampedFreq);
  case SpectrumComponent::WeightingMode::C:
    return calculateCWeightingDb(clampedFreq);
  case SpectrumComponent::WeightingMode::D:
    return calculateDWeightingDb(clampedFreq);
  case SpectrumComponent::WeightingMode::Flat:
  default:
    return 0.0f;
  }
}

static float sampleWeightedSpectrumDb(const std::vector<float> &mags,
                                      float frequency,
                                      double sampleRate,
                                      int fftSize,
                                      float rangeDb,
                                      SpectrumComponent::WeightingMode weightingMode) {
  if (mags.empty() || fftSize <= 0 || sampleRate <= 0.0)
    return -rangeDb;

  const int bin = juce::jlimit(
      0, (int)mags.size() - 1,
      (int)std::round(frequency * (float)fftSize / (float)sampleRate));

  float sum = 0.0f;
  int count = 0;

  for (int k = -2; k <= 2; ++k) {
    const int idx = bin + k;
    if (idx >= 0 && idx < (int)mags.size()) {
      sum += mags[(size_t)idx];
      ++count;
    }
  }

  float db = (count > 0 ? sum / (float)count : -rangeDb);
  db += calculateWeightingDb(frequency, weightingMode);
  return juce::jlimit(-rangeDb, 0.0f, db);
}

static juce::Path buildSpectrumPath(const std::vector<float> &mags,
                                    juce::Rectangle<float> inner,
                                    double sampleRate, int fftSize,
                                    float rangeDb,
                                    SpectrumComponent::WeightingMode weightingMode,
                                    float minFreq, float maxFreq) {
  juce::Path p;

  if (mags.empty() || fftSize <= 0 || sampleRate <= 0.0)
    return p;

  const int widthPixels = juce::jmax(2, (int)inner.getWidth());
  const float minLog = std::log10(minFreq);
  const float maxLog = std::log10(maxFreq);

  for (int xPixel = 0; xPixel < widthPixels; ++xPixel) {
    const float normX = (float)xPixel / (float)(widthPixels - 1);
    const float freqLog = minLog + normX * (maxLog - minLog);
    const float frequency = std::pow(10.0f, freqLog);

    float db = sampleWeightedSpectrumDb(mags, frequency, sampleRate, fftSize,
                                        rangeDb, weightingMode);

    const float normY = juce::jmap(db, -rangeDb, 0.0f, 0.0f, 1.0f);
    const float x = inner.getX() + normX * inner.getWidth();
    const float y = inner.getBottom() - normY * inner.getHeight();

    if (xPixel == 0)
      p.startNewSubPath(x, y);
    else
      p.lineTo(x, y);
  }

  return p;
}

static juce::Path buildSpectrumBandPath(
    const std::vector<float> &upperMags, const std::vector<float> &lowerMags,
    juce::Rectangle<float> inner, double sampleRate, int fftSize, float rangeDb,
    SpectrumComponent::WeightingMode weightingMode, float minFreq, float maxFreq) {
  juce::Path p;

  if (upperMags.empty() || lowerMags.empty() || fftSize <= 0 || sampleRate <= 0.0)
    return p;

  const int widthPixels = juce::jmax(2, (int)inner.getWidth());
  const float minLog = std::log10(minFreq);
  const float maxLog = std::log10(maxFreq);

  std::vector<juce::Point<float>> topPoints;
  std::vector<juce::Point<float>> bottomPoints;
  topPoints.reserve((size_t)widthPixels);
  bottomPoints.reserve((size_t)widthPixels);

  for (int xPixel = 0; xPixel < widthPixels; ++xPixel) {
    const float normX = (float)xPixel / (float)(widthPixels - 1);
    const float freqLog = minLog + normX * (maxLog - minLog);
    const float frequency = std::pow(10.0f, freqLog);

    const float lowerDb = sampleWeightedSpectrumDb(
        lowerMags, frequency, sampleRate, fftSize, rangeDb, weightingMode);
    float upperDb = sampleWeightedSpectrumDb(
        upperMags, frequency, sampleRate, fftSize, rangeDb, weightingMode);
    upperDb = juce::jmax(upperDb, lowerDb);

    if (upperDb - lowerDb < 1.0f)
      upperDb = lowerDb;

    const float x = inner.getX() + normX * inner.getWidth();
    const float topNormY = juce::jmap(upperDb, -rangeDb, 0.0f, 0.0f, 1.0f);
    const float bottomNormY = juce::jmap(lowerDb, -rangeDb, 0.0f, 0.0f, 1.0f);

    topPoints.push_back({x, inner.getBottom() - topNormY * inner.getHeight()});
    bottomPoints.push_back(
        {x, inner.getBottom() - bottomNormY * inner.getHeight()});
  }

  p.startNewSubPath(topPoints.front());
  for (size_t i = 1; i < topPoints.size(); ++i)
    p.lineTo(topPoints[i]);

  for (auto i = bottomPoints.rbegin(); i != bottomPoints.rend(); ++i)
    p.lineTo(*i);

  p.closeSubPath();
  return p;
}

static juce::Path buildReferenceDeltaBandPath(
    const std::vector<float> &currentMags,
    const std::vector<float> &referenceMags,
    juce::Rectangle<float> inner, double sampleRate, int fftSize, float rangeDb,
    SpectrumComponent::WeightingMode weightingMode, bool liveAboveReference,
    float minFreq, float maxFreq) {
  juce::Path p;

  if (currentMags.empty() || referenceMags.empty() || fftSize <= 0 ||
      sampleRate <= 0.0)
    return p;

  const int widthPixels = juce::jmax(2, (int)inner.getWidth());
  const float minLog = std::log10(minFreq);
  const float maxLog = std::log10(maxFreq);
  constexpr float minDeltaDb = 0.75f;

  std::vector<juce::Point<float>> topPoints;
  std::vector<juce::Point<float>> bottomPoints;
  topPoints.reserve((size_t)widthPixels);
  bottomPoints.reserve((size_t)widthPixels);

  for (int xPixel = 0; xPixel < widthPixels; ++xPixel) {
    const float normX = (float)xPixel / (float)(widthPixels - 1);
    const float freqLog = minLog + normX * (maxLog - minLog);
    const float frequency = std::pow(10.0f, freqLog);

    const float currentDb = sampleWeightedSpectrumDb(
        currentMags, frequency, sampleRate, fftSize, rangeDb, weightingMode);
    const float referenceDb = sampleWeightedSpectrumDb(
        referenceMags, frequency, sampleRate, fftSize, rangeDb, weightingMode);
    const float deltaDb = currentDb - referenceDb;
    const bool active = liveAboveReference ? (deltaDb > minDeltaDb)
                                           : (deltaDb < -minDeltaDb);

    const float topDb =
        active ? juce::jmax(currentDb, referenceDb) : referenceDb;
    const float bottomDb =
        active ? juce::jmin(currentDb, referenceDb) : referenceDb;

    const float x = inner.getX() + normX * inner.getWidth();
    const float topNormY = juce::jmap(topDb, -rangeDb, 0.0f, 0.0f, 1.0f);
    const float bottomNormY =
        juce::jmap(bottomDb, -rangeDb, 0.0f, 0.0f, 1.0f);

    topPoints.push_back({x, inner.getBottom() - topNormY * inner.getHeight()});
    bottomPoints.push_back(
        {x, inner.getBottom() - bottomNormY * inner.getHeight()});
  }

  p.startNewSubPath(topPoints.front());
  for (size_t i = 1; i < topPoints.size(); ++i)
    p.lineTo(topPoints[i]);

  for (auto i = bottomPoints.rbegin(); i != bottomPoints.rend(); ++i)
    p.lineTo(*i);

  p.closeSubPath();
  return p;
}

// Calculate auditory masking threshold based on psychoacoustic model
void SpectrumComponent::updateMaskingThreshold(const std::vector<float> &spectrum,
                                                std::vector<float> &maskingThreshold) {
  if (spectrum.empty())
    return;

  if (maskingThreshold.size() != spectrum.size())
    maskingThreshold.resize(spectrum.size());

  // Initialize to minimum threshold (threshold of hearing approximation)
  for (size_t i = 0; i < spectrum.size(); ++i) {
      float f = binToFrequency((int)i);
      // Threshold of hearing formula (approx)
      maskingThreshold[i] = 3.64 * std::pow(f/1000.0, -0.8) - 6.5 * std::exp(-0.6 * std::pow(f/1000.0 - 3.3, 2)) + 0.001 * std::pow(f/1000.0, 4) - 90.0f;
  }

  // Simplified spreading function
  for (size_t i = 0; i < spectrum.size(); ++i) {
    const float sourceMag = spectrum[i];
    if (sourceMag < -80.0f) continue;

    const float f_i = binToFrequency((int)i);
    const float bark_i = 13.0f * std::atan(0.00076f * f_i) + 3.5f * std::atan(std::pow(f_i / 7500.0f, 2));

    for (size_t j = 0; j < spectrum.size(); ++j) {
        if (i == j) continue;
        
        const float f_j = binToFrequency((int)j);
        const float bark_j = 13.0f * std::atan(0.00076f * f_j) + 3.5f * std::atan(std::pow(f_j / 7500.0f, 2));
        const float dz = bark_j - bark_i;
        
        float spreading;
        if (dz < 0) {
            spreading = 27.0f * dz;
        } else {
            spreading = (24.0f + 0.23f * std::min(f_i / 1000.0f, 2.0f) - 0.21f * sourceMag) * dz;
        }
        
        const float maskedDb = sourceMag - 15.0f + spreading; // 15dB offset for masking
        maskingThreshold[j] = std::max(maskingThreshold[j], maskedDb);
    }
  }
}

void SpectrumComponent::paint(juce::Graphics &g) {
  auto area = getLocalBounds().toFloat();
  
  juce::Colour accentBlue, accentPurple, accentGold, accentOrange, gridColour, labelColour, frameColorTop, frameColorBottom, panelColor;
  float glowAlpha, fillAlpha, lineThickness;
  bool useGlow = true;

  switch (colorScheme) {
  case ColorScheme::Calm:
      accentBlue = juce::Colour::fromRGB(140, 220, 235);
      accentPurple = juce::Colour::fromRGB(210, 180, 245);
      accentGold = juce::Colour::fromRGB(245, 210, 125);
      accentOrange = juce::Colour::fromRGB(235, 175, 110);
      gridColour = juce::Colour::fromRGBA(185, 205, 215, 34);
      labelColour = juce::Colour::fromRGBA(230, 236, 238, 118);
      frameColorTop = juce::Colour::fromRGB(38, 40, 41);
      frameColorBottom = juce::Colour::fromRGB(10, 12, 13);
      panelColor = juce::Colour::fromRGB(9, 11, 12);
      glowAlpha = 0.16f;
      fillAlpha = 0.18f;
      lineThickness = 1.25f;
      useGlow = false;
      break;
  case ColorScheme::Midnight:
      accentBlue = juce::Colour::fromRGB(40, 100, 255);
      accentPurple = juce::Colour::fromRGB(100, 40, 255);
      accentGold = juce::Colour::fromRGB(0, 255, 150);
      accentOrange = juce::Colour::fromRGB(255, 0, 100);
      gridColour = juce::Colour::fromRGBA(40, 60, 120, 40);
      labelColour = juce::Colour::fromRGBA(150, 180, 255, 120);
      frameColorTop = juce::Colour::fromRGB(10, 12, 20);
      frameColorBottom = juce::Colour::fromRGB(5, 5, 8);
      panelColor = juce::Colour::fromRGB(2, 3, 5);
      glowAlpha = 0.4f;
      fillAlpha = 0.35f;
      lineThickness = 1.5f;
      break;
  case ColorScheme::Vintage:
      accentBlue = juce::Colour::fromRGB(220, 180, 80);
      accentPurple = juce::Colour::fromRGB(180, 100, 40);
      accentGold = juce::Colour::fromRGB(255, 220, 150);
      accentOrange = juce::Colour::fromRGB(200, 120, 60);
      gridColour = juce::Colour::fromRGBA(150, 120, 80, 30);
      labelColour = juce::Colour::fromRGBA(220, 200, 160, 100);
      frameColorTop = juce::Colour::fromRGB(35, 25, 15);
      frameColorBottom = juce::Colour::fromRGB(15, 10, 5);
      panelColor = juce::Colour::fromRGB(12, 8, 4);
      glowAlpha = 0.25f;
      fillAlpha = 0.3f;
      lineThickness = 1.4f;
      break;
  case ColorScheme::Funky:
  default:
      accentBlue = juce::Colour::fromRGB(0, 255, 255); // Neon Cyan
      accentPurple = juce::Colour::fromRGB(255, 0, 255); // Neon Magenta
      accentGold = juce::Colour::fromRGB(255, 255, 0); // Neon Yellow
      accentOrange = juce::Colour::fromRGB(255, 128, 0); // Neon Orange
      gridColour = juce::Colour::fromRGBA(0, 255, 255, 40); // Brighter cyan grid
      labelColour = juce::Colour::fromRGBA(0, 255, 255, 200);
      frameColorTop = juce::Colour::fromRGB(2, 3, 5);
      frameColorBottom = juce::Colour::fromRGB(0, 0, 0);
      panelColor = juce::Colour::fromRGB(4, 5, 8);
      glowAlpha = 0.6f; // More glow
      fillAlpha = 0.2f; // Less fill, more outline
      lineThickness = 2.0f; // Thicker lines
      break;
  }

  // TRON Cyberpunk Frame
  auto frameBounds = area.reduced(1.0f);
  g.setColour(frameColorTop);
  g.fillRoundedRectangle(frameBounds, 2.0f);

  auto panelBounds = frameBounds.reduced(2.0f);
  g.setColour(panelColor);
  g.fillRoundedRectangle(panelBounds, 1.0f);
  
  // Neon Frame Bevel
  g.setColour(juce::Colour::fromRGBA(0, 255, 255, 60));
  g.drawRoundedRectangle(frameBounds, 2.0f, 1.5f);
  g.setColour(juce::Colour::fromRGBA(255, 0, 255, 40));
  g.drawRoundedRectangle(panelBounds, 1.0f, 1.0f);

  // Background Grid for the Panel
  g.setColour(juce::Colour::fromRGBA(0, 255, 255, 8));
  for (float i = panelBounds.getX(); i < panelBounds.getRight(); i += 20.0f)
      g.drawVerticalLine((int)i, panelBounds.getY(), panelBounds.getBottom());
  for (float i = panelBounds.getY(); i < panelBounds.getBottom(); i += 20.0f)
      g.drawHorizontalLine((int)i, panelBounds.getX(), panelBounds.getRight());
  
  auto inner = getSpectrumBounds();

  // --- SPECTROGRAM (WATERFALL) ---
  if (!spectrogramFrames.empty()) {
      if (needsLookupUpdate)
          updateLookupTable();

      const int numFrames = (int)spectrogramFrames.size();
      const float frameHeight = inner.getHeight() / (float)maxSpectrogramFrames;
      const bool isFullWaterfall = (displayMode == DisplayMode::Waterfall);
      const int step = isFullWaterfall ? 2 : 4;

      for (int i = 0; i < numFrames; ++i) {
          const auto& frame = spectrogramFrames[(size_t)i];
          const float y = inner.getBottom() - (float)(numFrames - i) * frameHeight;
          
          for (int x = 0; x < (int)inner.getWidth(); x += step) {
              const int bin = binLookupTable[(size_t)x];
              float weightedDb = frame[(size_t)bin]
                                  + calculateWeightingDb(binToFrequency(bin), weightingMode);
              weightedDb = juce::jlimit(-displayRangeDb, 0.0f, weightedDb);
              float val = juce::jmap(weightedDb, -displayRangeDb, 0.0f, 0.0f, 1.0f);

              if (val > 0.05f) {
                  juce::Colour c;
                  if (isFullWaterfall) {
                      // Heatmap color scheme: Blue -> Cyan -> Yellow -> White
                      if (val < 0.3f) c = juce::Colour::fromRGB(0, 0, 100).interpolatedWith(juce::Colours::blue, val / 0.3f);
                      else if (val < 0.6f) c = juce::Colours::blue.interpolatedWith(juce::Colours::cyan, (val - 0.3f) / 0.3f);
                      else if (val < 0.8f) c = juce::Colours::cyan.interpolatedWith(juce::Colours::yellow, (val - 0.6f) / 0.2f);
                      else c = juce::Colours::yellow.interpolatedWith(juce::Colours::white, (val - 0.8f) / 0.2f);
                      
                      g.setColour(c.withAlpha(0.8f));
                  } else {
                      c = accentBlue;
                      g.setColour(c.withAlpha(val * (colorScheme == ColorScheme::Calm ? 0.08f : 0.15f)));
                  }
                  g.fillRect(inner.getX() + (float)x, y, (float)step, frameHeight);

                  // Add a slight bloom/glow to the newest line (bottom line)
                  if (i == numFrames - 1) {
                      g.setColour(c.withAlpha(0.3f));
                      g.fillRect(inner.getX() + (float)x, y - 1.0f, (float)step, 3.0f);
                  }
              }
          }
      }
  }

  g.setFont(juce::FontOptions(11.0f));

  // Note Grid
  if (noteGridEnabled)
      drawNoteGrid(g, inner);

  // Grid
  g.setColour(gridColour);
  const float dashH[] = {4.0f, 4.0f};
  std::vector<float> gridLines = (displayRangeDb < 70.0f) 
                                 ? std::vector<float>{0.f, -10.f, -20.f, -30.f, -40.f, -50.f, -60.f}
                                 : std::vector<float>{0.f, -12.f, -24.f, -36.f, -48.f, -60.f, -72.f, -84.f};
  for (float db : gridLines) {
    const float norm = juce::jmap(db, -displayRangeDb, 0.f, 0.f, 1.f);
    const float y = inner.getBottom() - norm * inner.getHeight();

    g.drawDashedLine(juce::Line<float>(inner.getX(), y, inner.getRight(), y),
                      dashH, 2, 0.8f);

    g.setColour(gridColour);
    g.drawText(juce::String((int)db),
               juce::Rectangle<int>((int)inner.getX() + 4, (int)y - 8, 32, 12),
               juce::Justification::left, false);
    g.setColour(gridColour);
  }

  struct FreqMark {
    float hz;
    const char *label;
  };
  const FreqMark marks[] = {
      {20.0f, "20"},     {50.0f, "50"},    {100.0f, "100"}, {200.0f, "200"},
      {500.0f, "500"},   {1000.0f, "1k"},  {2000.0f, "2k"}, {5000.0f, "5k"},
      {10000.0f, "10k"}, {20000.0f, "20k"}};

  const float dashV[] = {3.0f, 5.0f};
  for (const auto &mark : marks) {
    const float x = inner.getX() + frequencyToNormX(mark.hz) * inner.getWidth();

    g.setColour(gridColour.withAlpha(0.2f));
    g.drawDashedLine(juce::Line<float>(x, inner.getY(), x, inner.getBottom()),
                      dashV, 2, 0.8f);

    g.setColour(labelColour);
    g.drawText(
        mark.label,
        juce::Rectangle<int>((int)x - 18, (int)inner.getBottom() - 14, 36, 12),
        juce::Justification::centred, false);
  }

  if (frequencyMarkersEnabled)
      drawFrequencyMarkers(g, inner);

  auto drawFilledPath = [&](const juce::Path &p, juce::Colour line,
                            juce::Colour glow, juce::Colour fillTop) {
    if (p.isEmpty())
      return;

    juce::Path fill = p;
    fill.lineTo(inner.getRight(), inner.getBottom());
    fill.lineTo(inner.getX(), inner.getBottom());
    fill.closeSubPath();

    juce::ColourGradient grad(fillTop.withAlpha(fillAlpha), inner.getCentreX(), inner.getY(),
                              fillTop.withAlpha(0.0f), inner.getCentreX(),
                              inner.getBottom(), false);

    g.setGradientFill(grad);
    g.fillPath(fill);

    // Dynamic Multi-Layer Glow
    if (useGlow) {
      g.setColour(glow.withAlpha(glowAlpha * 0.5f));
      g.strokePath(p, juce::PathStrokeType(6.0f));
      g.setColour(glow.withAlpha(glowAlpha));
      g.strokePath(p, juce::PathStrokeType(3.5f));
    } else {
      g.setColour(glow.withAlpha(glowAlpha));
      g.strokePath(p, juce::PathStrokeType(2.4f));
    }

    g.setColour(line);
    g.strokePath(p, juce::PathStrokeType(lineThickness));
    
    // Bright Core
    g.setColour(juce::Colours::white.withAlpha(fillAlpha * 0.5f));
    g.strokePath(p, juce::PathStrokeType(0.6f));
  };

  const auto leftPath =
      buildSpectrumPath(smoothedLeft, inner, sampleRate, fftSize, displayRangeDb,
                        weightingMode, minFrequency, maxFrequency);
  const auto rightPath =
      buildSpectrumPath(smoothedRight, inner, sampleRate, fftSize, displayRangeDb,
                        weightingMode, minFrequency, maxFrequency);

  // --- Draw Masking Visualization ---
  if (maskingVisualizationEnabled && !maskingThresholdLeft.empty()) {
    const auto maskingPath = buildSpectrumPath(maskingThresholdLeft, inner, sampleRate, fftSize,
                                               displayRangeDb, WeightingMode::Flat, minFrequency, maxFrequency);
    
    // Draw masking threshold as semi-transparent region
    g.setColour(juce::Colour::fromRGBA(255, 100, 100, 30)); // Semi-transparent red
    g.fillPath(maskingPath);
    g.setColour(juce::Colour::fromRGBA(255, 80, 80, 80));
    g.strokePath(maskingPath, juce::PathStrokeType(0.8f));
  }

  // Matches the Blue/Purple accents from the Amp
  if (displayMode == DisplayMode::Waterfall) {
      // Don't draw the spectrum path in waterfall mode, or draw it as a thin line
      g.setColour(juce::Colours::white.withAlpha(0.5f));
      g.strokePath(leftPath, juce::PathStrokeType(1.0f));
  } else if (displayMode == DisplayMode::LR) {
    drawFilledPath(leftPath, accentBlue, accentBlue, accentBlue);

    drawFilledPath(rightPath, accentPurple, accentPurple, accentPurple);
  } else {
    // M/S specific warmer colors
    drawFilledPath(leftPath, accentGold, accentGold, accentGold);

    drawFilledPath(rightPath, accentOrange, accentOrange, accentOrange);
  }

  if (referenceTraceVisible && displayMode != DisplayMode::Waterfall) {
    const auto referenceLeftPath =
        buildSpectrumPath(referenceTraceLeft, inner, sampleRate, fftSize,
                          displayRangeDb, weightingMode, minFrequency, maxFrequency);
    const auto referenceRightPath =
        buildSpectrumPath(referenceTraceRight, inner, sampleRate, fftSize,
                          displayRangeDb, weightingMode, minFrequency, maxFrequency);

    auto drawDeltaBand = [&](const juce::Path &band, juce::Colour colour) {
      if (band.isEmpty())
        return;

      g.setColour(colour.withAlpha(fillAlpha * 0.4f));
      g.fillPath(band);
    };

    const auto leftAboveReference =
        buildReferenceDeltaBandPath(smoothedLeft, referenceTraceLeft, inner,
                                    sampleRate, fftSize, displayRangeDb,
                                    weightingMode, true, minFrequency, maxFrequency);
    const auto leftBelowReference =
        buildReferenceDeltaBandPath(smoothedLeft, referenceTraceLeft, inner,
                                    sampleRate, fftSize, displayRangeDb,
                                    weightingMode, false, minFrequency, maxFrequency);
    const auto rightAboveReference =
        buildReferenceDeltaBandPath(smoothedRight, referenceTraceRight, inner,
                                    sampleRate, fftSize, displayRangeDb,
                                    weightingMode, true, minFrequency, maxFrequency);
    const auto rightBelowReference =
        buildReferenceDeltaBandPath(smoothedRight, referenceTraceRight, inner,
                                    sampleRate, fftSize, displayRangeDb,
                                    weightingMode, false, minFrequency, maxFrequency);

    drawDeltaBand(leftAboveReference, juce::Colour::fromRGB(90, 220, 170));
    drawDeltaBand(rightAboveReference, juce::Colour::fromRGB(90, 220, 170));
    drawDeltaBand(leftBelowReference, juce::Colour::fromRGB(245, 135, 155));
    drawDeltaBand(rightBelowReference, juce::Colour::fromRGB(245, 135, 155));

    auto drawReferencePath = [&](const juce::Path &p, juce::Colour colour) {
      if (p.isEmpty())
        return;

      g.setColour(juce::Colours::black.withAlpha(0.35f));
      g.strokePath(p, juce::PathStrokeType(2.6f));
      g.setColour(colour.withAlpha(0.6f));
      g.strokePath(p, juce::PathStrokeType(lineThickness * 0.8f));
    };

    if (displayMode == DisplayMode::LR) {
      drawReferencePath(referenceLeftPath, accentBlue);
      drawReferencePath(referenceRightPath, accentPurple);
    } else {
      drawReferencePath(referenceLeftPath, accentGold);
      drawReferencePath(referenceRightPath, accentOrange);
    }

    g.setColour(accentPurple.withAlpha(0.72f));
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.drawText("REF", juce::Rectangle<int>((int)inner.getRight() - 82,
                                           (int)inner.getY() + 8, 24, 12),
               juce::Justification::centred, false);
  }

  if (averageTraceVisible && displayMode != DisplayMode::Waterfall) {
    const auto averageLeftPath =
        buildSpectrumPath(averageTraceLeft, inner, sampleRate, fftSize,
                          displayRangeDb, weightingMode, minFrequency, maxFrequency);
    const auto averageRightPath =
        buildSpectrumPath(averageTraceRight, inner, sampleRate, fftSize,
                          displayRangeDb, weightingMode, minFrequency, maxFrequency);

    auto drawAveragePath = [&](const juce::Path &p, juce::Colour colour) {
      if (p.isEmpty())
        return;

      g.setColour(juce::Colours::black.withAlpha(0.4f));
      g.strokePath(p, juce::PathStrokeType(2.0f));
      g.setColour(colour.withAlpha(0.7f));
      g.strokePath(p, juce::PathStrokeType(0.9f));
    };

    const auto averageColour = labelColour;

    drawAveragePath(averageLeftPath, averageColour);
    drawAveragePath(averageRightPath, averageColour);

    g.setColour(averageColour.withAlpha(0.72f));
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.drawText("AVG", juce::Rectangle<int>((int)inner.getRight() - 116,
                                           (int)inner.getY() + 8, 28, 12),
               juce::Justification::centred, false);
  }

  // 6. Draw Sidechain Path (Amber/Gold)
  if (sidechainVisible && !sidechainLeft.empty()) {
    auto drawSidechainPath = [&](const std::vector<float> &data, juce::Colour colour) {
      juce::Path p = buildSpectrumPath(data, inner, sampleRate, fftSize, displayRangeDb,
                                       WeightingMode::Flat, minFrequency, maxFrequency);
      if (p.isEmpty()) return;

      g.setColour(juce::Colours::black.withAlpha(0.3f));
      g.strokePath(p, juce::PathStrokeType(2.4f));
      g.setColour(colour.withAlpha(0.75f));
      g.strokePath(p, juce::PathStrokeType(1.1f));
    };

    if (displayMode == DisplayMode::LR) {
      drawSidechainPath(sidechainLeft, accentGold);
      drawSidechainPath(sidechainRight, accentOrange);
    } else {
      drawSidechainPath(sidechainLeft, accentGold);
    }
  }



  // 6.5. Draw Sidechain Clash (Red Highlight)
  if (clashDetectionEnabled && sidechainVisible && !sidechainLeft.empty() && !smoothedLeft.empty()) {
      auto drawClashPath = [&](const std::vector<float>& main, const std::vector<float>& sc, juce::Colour colour) {
          std::vector<float> clash(main.size());
          bool hasClash = false;
          for (size_t i = 0; i < main.size(); ++i) {
              float m = main[i];
              float s = sc[i];
              // Clash is significant if both are above threshold and close to each other
              if (m > -70.0f && s > -70.0f) {
                  clash[i] = std::min(m, s);
                  hasClash = true;
              } else {
                  clash[i] = -120.0f;
              }
          }
          
          if (!hasClash) return;

          juce::Path p = buildSpectrumPath(clash, inner, sampleRate, fftSize, displayRangeDb,
                                           WeightingMode::Flat, this->minFrequency, this->maxFrequency);
          if (p.isEmpty()) return;

          p.lineTo(inner.getRight(), inner.getBottom());
          p.lineTo(inner.getX(), inner.getBottom());
          p.closeSubPath();

          juce::ColourGradient clashGrad(colour.withAlpha(0.4f), 0, inner.getY(),
                                         colour.withAlpha(0.0f), 0, inner.getBottom(), false);
          g.setGradientFill(clashGrad);
          g.fillPath(p);
          
          g.setColour(colour.withAlpha(0.6f));
          g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
          g.drawText("CLASH", inner.reduced(10), juce::Justification::topRight, false);
      };

      if (displayMode == DisplayMode::LR) {
          drawClashPath(smoothedLeft, sidechainLeft, juce::Colours::red);
          drawClashPath(smoothedRight, sidechainRight, juce::Colours::red);
      } else {
          drawClashPath(smoothedLeft, sidechainLeft, juce::Colours::red);
      }
  }

  // --- PEAK TRACES ---
  if (!isAnalyzerFrozen()) {
    if (peakTraceLeft.size() != smoothedLeft.size()) {
        peakTraceLeft = smoothedLeft;
        peakHoldLeft.assign(smoothedLeft.size(), 0);
    }
    if (peakTraceRight.size() != smoothedRight.size()) {
        peakTraceRight = smoothedRight;
        peakHoldRight.assign(smoothedRight.size(), 0);
    }

    for (size_t i = 0; i < (size_t)smoothedLeft.size(); ++i) {
        if (smoothedLeft[i] > peakTraceLeft[i]) {
            peakTraceLeft[i] = smoothedLeft[i];
            peakHoldLeft[i] = peakHoldTime;
        } else {
            if (peakHoldLeft[i] > 0) --peakHoldLeft[i];
            else peakTraceLeft[i] -= 0.35f; 
        }
    }

    for (size_t i = 0; i < (size_t)smoothedRight.size(); ++i) {
        if (smoothedRight[i] > peakTraceRight[i]) {
            peakTraceRight[i] = smoothedRight[i];
            peakHoldRight[i] = peakHoldTime;
        } else {
            if (peakHoldRight[i] > 0) --peakHoldRight[i];
            else peakTraceRight[i] -= 0.35f;
        }
    }
  }

  const auto leftPeakPath =
      buildSpectrumPath(peakTraceLeft, inner, sampleRate, fftSize, displayRangeDb,
                        weightingMode, minFrequency, maxFrequency);
  const auto rightPeakPath =
      buildSpectrumPath(peakTraceRight, inner, sampleRate, fftSize, displayRangeDb,
                        weightingMode, minFrequency, maxFrequency);

  auto drawCrestBand = [&](const juce::Path &band, juce::Colour colour) {
    if (band.isEmpty())
      return;

    g.setColour(colour.withAlpha(fillAlpha * 0.45f));
    g.fillPath(band);
    g.setColour(colour.withAlpha(0.32f));
    g.strokePath(band, juce::PathStrokeType(0.7f));
  };

  if (crestModeEnabled) {
    const auto leftCrestBand =
        buildSpectrumBandPath(peakTraceLeft, smoothedLeft, inner, sampleRate,
                              fftSize, displayRangeDb, weightingMode, minFrequency, maxFrequency);
    const auto rightCrestBand =
        buildSpectrumBandPath(peakTraceRight, smoothedRight, inner, sampleRate,
                              fftSize, displayRangeDb, weightingMode, minFrequency, maxFrequency);

    if (displayMode == DisplayMode::LR) {
      drawCrestBand(leftCrestBand, accentBlue);
      drawCrestBand(rightCrestBand, accentGold);
    } else {
      drawCrestBand(leftCrestBand, accentGold);
      drawCrestBand(rightCrestBand, accentOrange);
    }
  }

  auto drawPeakLine = [&](const juce::Path &p, juce::Colour c) {
    if (!p.isEmpty()) {
      g.setColour(c.withAlpha(0.85f));
      g.strokePath(p, juce::PathStrokeType(0.8f)); // Even thinner line
    }
  };

  if (displayMode == DisplayMode::LR) {
      drawPeakLine(leftPeakPath, accentBlue);
      drawPeakLine(rightPeakPath, accentGold);
  } else {
      drawPeakLine(leftPeakPath, accentGold);
      drawPeakLine(rightPeakPath, accentOrange);
  }

  g.setColour(labelColour);
  g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
  g.drawText("SPECTRUM", getLocalBounds().reduced(16, 10),
             juce::Justification::topLeft, false);

  if (frozen || inspectionActive) {
      g.setColour(accentBlue.withAlpha(0.82f));
      g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
      g.drawText(frozen ? "FROZEN" : "INSPECT", getLocalBounds().reduced(20, 10),
                 juce::Justification::topRight, false);
  } else if (crestModeEnabled) {
      g.setColour(accentGold.withAlpha(0.76f));
      g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
      g.drawText("CREST", getLocalBounds().reduced(20, 10),
                 juce::Justification::topRight, false);
  } else {
      g.setColour(labelColour);
      g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
      g.drawText("ACTIVE", getLocalBounds().reduced(20, 10),
                 juce::Justification::topRight, false);
  }

  g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));

  if (displayMode == DisplayMode::LR) {
    g.setColour(accentBlue.withAlpha(0.76f));
    g.drawText("L",
               juce::Rectangle<int>((int)inner.getRight() - 52,
                                    (int)inner.getY() + 8, 16, 12),
               juce::Justification::centred, false);

    g.setColour(accentGold.withAlpha(0.76f));
    g.drawText("R",
               juce::Rectangle<int>((int)inner.getRight() - 28,
                                    (int)inner.getY() + 8, 16, 12),
               juce::Justification::centred, false);
  } else {
    g.setColour(accentGold.withAlpha(0.76f));
    g.drawText("M",
               juce::Rectangle<int>((int)inner.getRight() - 52,
                                    (int)inner.getY() + 8, 16, 12),
               juce::Justification::centred, false);

    g.setColour(accentOrange.withAlpha(0.76f));
    g.drawText("S",
               juce::Rectangle<int>((int)inner.getRight() - 28,
                                    (int)inner.getY() + 8, 16, 12),
               juce::Justification::centred, false);
  }

  // --- RULER TAPE ---
  if (rulerVisible) {
    const auto start = rulerPointToPosition(rulerStart);
    const auto end = rulerPointToPosition(rulerEnd);
    const juce::Line<float> rulerLine(start, end);

    g.setColour(accentBlue.withAlpha(fillAlpha * 0.7f));
    g.drawLine(rulerLine, 5.0f);
    g.setColour(accentGold.withAlpha(0.82f));
    g.drawLine(rulerLine, 1.4f);

    auto drawHandle = [&](juce::Point<float> point) {
      g.setColour(juce::Colours::black.withAlpha(0.72f));
      g.fillEllipse(point.x - 4.0f, point.y - 4.0f, 8.0f, 8.0f);
      g.setColour(accentGold.withAlpha(0.9f));
      g.drawEllipse(point.x - 4.0f, point.y - 4.0f, 8.0f, 8.0f, 1.2f);
    };

    drawHandle(start);
    drawHandle(end);

    if (rulerLine.getLength() > 4.0f) {
      const juce::Point<float> midpoint((start.x + end.x) * 0.5f,
                                        (start.y + end.y) * 0.5f);
      juce::Rectangle<float> tooltipBounds(midpoint.x - 155.0f,
                                           midpoint.y - 34.0f, 310.0f, 22.0f);

      if (tooltipBounds.getY() < inner.getY() + 4.0f)
        tooltipBounds.setY(midpoint.y + 12.0f);

      tooltipBounds.setX(juce::jlimit(inner.getX() + 4.0f,
                                      inner.getRight() - tooltipBounds.getWidth() - 4.0f,
                                      tooltipBounds.getX()));
      tooltipBounds.setY(juce::jlimit(inner.getY() + 4.0f,
                                      inner.getBottom() - tooltipBounds.getHeight() - 4.0f,
                                      tooltipBounds.getY()));

      g.setColour(juce::Colours::black.withAlpha(0.78f));
      g.fillRoundedRectangle(tooltipBounds, 4.0f);
      g.setColour(accentGold.withAlpha(0.72f));
      g.drawRoundedRectangle(tooltipBounds, 4.0f, 1.0f);

      g.setColour(juce::Colours::white.withAlpha(0.92f));
      g.setFont(juce::FontOptions(11.0f));
      g.drawText(buildRulerTooltip(), tooltipBounds.toNearestInt(),
                 juce::Justification::centred, false);
    }
  }

  // --- MOUSE HOVER DATAPOINT (TOOLTIP) ---
  auto mousePos = inspectionActive ? rulerPointToPosition(inspectionPoint)
                                   : getMouseXYRelative().toFloat();
  if (inspectionActive || inner.contains(mousePos)) {
    float fLog =
        std::log10(20.0f) +
        ((float)(mousePos.x - inner.getX()) / (float)inner.getWidth()) *
            (std::log10(20000.0f) - std::log10(20.0f));
    float freqHz = std::pow(10.0f, fLog);

    float normY =
        (float)(inner.getBottom() - mousePos.y) / (float)inner.getHeight();
    float db = -displayRangeDb + normY * displayRangeDb;

    // Calculate note (A4 = 440 Hz = MIDI note 69)
    int midiNote = (int)std::round(69.0f + 12.0f * std::log2(freqHz / 440.0f));
    const char *noteNames[] = {"C",  "C#", "D",  "D#", "E",  "F",
                               "F#", "G",  "G#", "A",  "A#", "B"};
    int octave = (midiNote / 12) - 1;
    juce::String noteStr =
        (midiNote >= 0 && midiNote <= 127)
            ? juce::String(noteNames[midiNote % 12]) + juce::String(octave)
            : "--";

    juce::String tooltipText = juce::String(freqHz, 1) + " Hz | " + noteStr +
                               " | " + juce::String(db, 1) + " dB";
    bool hasReferenceDelta = false;

    if (referenceTraceVisible && displayMode != DisplayMode::Waterfall) {
      const float currentA = sampleWeightedSpectrumDb(
          smoothedLeft, freqHz, sampleRate, fftSize, displayRangeDb,
          weightingMode);
      const float referenceA = sampleWeightedSpectrumDb(
          referenceTraceLeft, freqHz, sampleRate, fftSize, displayRangeDb,
          weightingMode);
      const float currentB = sampleWeightedSpectrumDb(
          smoothedRight, freqHz, sampleRate, fftSize, displayRangeDb,
          weightingMode);
      const float referenceB = sampleWeightedSpectrumDb(
          referenceTraceRight, freqHz, sampleRate, fftSize, displayRangeDb,
          weightingMode);
      const auto labelA =
          displayMode == DisplayMode::LR ? juce::String("L") : juce::String("M");
      const auto labelB =
          displayMode == DisplayMode::LR ? juce::String("R") : juce::String("S");
      const auto formatDelta = [](float delta) {
        return juce::String(delta >= 0.0f ? "+" : "") + juce::String(delta, 1);
      };

      tooltipText += " | Ref " + labelA + " " +
                     formatDelta(currentA - referenceA) + " " + labelB + " " +
                     formatDelta(currentB - referenceB);
      hasReferenceDelta = true;
    }

    // Draw crosshair
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawVerticalLine((int)mousePos.x, inner.getY(), inner.getBottom());
    g.drawHorizontalLine((int)mousePos.y, inner.getX(), inner.getRight());

    // Draw text box
    juce::Rectangle<int> tooltipBounds(static_cast<int>(mousePos.x + 10.0f),
                                       static_cast<int>(mousePos.y - 20.0f),
                                       hasReferenceDelta ? 250 : 140, 20);
    if (tooltipBounds.getRight() > inner.getRight())
      tooltipBounds.setX(
          static_cast<int>(mousePos.x - (hasReferenceDelta ? 260.0f : 150.0f)));

    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.fillRoundedRectangle(tooltipBounds.toFloat(), 4.0f);

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(12.0f));
    g.drawText(tooltipText, tooltipBounds, juce::Justification::centred, false);
  }

  // Draw Mode Indicator
  g.setColour(juce::Colours::white.withAlpha(0.3f));
  g.setFont(juce::FontOptions(10.0f));
  juce::String modeStr = "L/R";
  if (displayMode == DisplayMode::MS) modeStr = "M/S";
  else if (displayMode == DisplayMode::Waterfall) modeStr = "WATERFALL";
  g.drawText(modeStr, inner.withTrimmedTop(inner.getHeight() - 15).withTrimmedLeft(inner.getWidth() - 60), juce::Justification::centredRight, false);

  // --- PREMIUM FINISH: GLASS & VIGNETTE ---
  g.setOrigin(0, 0);
  
  // 1. Subtle Vignette
  juce::ColourGradient vignette(juce::Colours::transparentBlack, inner.getCentreX(), inner.getCentreY(),
                                juce::Colours::black.withAlpha(0.2f), inner.getX(), inner.getY(), true);
  g.setGradientFill(vignette);
  g.fillRoundedRectangle(inner, 2.0f);

  // 2. Glass Reflection Shine (Diagonal)
  juce::Path glassPath;
  glassPath.addRoundedRectangle(inner.getX(), inner.getY(), inner.getWidth(), inner.getHeight(), 2.0f);
  g.reduceClipRegion(glassPath);
  
  juce::ColourGradient glassGrad(juce::Colours::white.withAlpha(fillAlpha * 0.15f), inner.getX(), inner.getY(),
                                 juce::Colours::transparentWhite, inner.getCentreX() + inner.getWidth() * 0.2f, inner.getCentreY() + inner.getHeight() * 0.2f, false);
  g.setGradientFill(glassGrad);
  g.fillPath(glassPath);

  // 3. Inner Shadow/Rim
  g.setColour(juce::Colours::black.withAlpha(0.4f));
  g.drawRoundedRectangle(inner, 2.0f, 1.2f);
}

void SpectrumComponent::resetPeakTrace() {
  std::fill(peakTraceLeft.begin(), peakTraceLeft.end(), -displayRangeDb);
  std::fill(peakTraceRight.begin(), peakTraceRight.end(), -displayRangeDb);
}

void SpectrumComponent::mouseDown(const juce::MouseEvent &event) {
  auto inner = getSpectrumBounds();
  if (!inner.contains(event.position))
    return;

  if (!event.mods.isAltDown())
  {
    inspectionPoint = pointToRulerPoint(event.position);
    inspectionActive = !frozen;
    repaint();
    return;
  }

  rulerStart = pointToRulerPoint(event.position);
  rulerEnd = rulerStart;
  rulerVisible = true;
  rulerDragging = true;
  repaint();
}

void SpectrumComponent::mouseDrag(const juce::MouseEvent &event) {
  if (rulerDragging)
  {
    rulerEnd = pointToRulerPoint(event.position);
    repaint();
    return;
  }

  if (inspectionActive)
  {
    inspectionPoint = pointToRulerPoint(event.position);
    repaint();
  }
}

void SpectrumComponent::mouseUp(const juce::MouseEvent &) {
  rulerDragging = false;
  if (inspectionActive)
  {
    inspectionActive = false;
    repaint();
  }
}

void SpectrumComponent::mouseDoubleClick(const juce::MouseEvent &) {
  inspectionActive = false;

  if (rulerVisible) {
    rulerVisible = false;
    rulerDragging = false;
    repaint();
    return;
  }

  resetPeakTrace();
}

float SpectrumComponent::frequencyToX(float frequency, float left,
                                      float width) const {
  const float minLog = std::log10(minFrequency);
  const float maxLog = std::log10(maxFrequency);
  const float fLog =
      std::log10(juce::jlimit(minFrequency, maxFrequency, frequency));

  float normX = (fLog - minLog) / (maxLog - minLog);
  return left + normX * width;
}

float SpectrumComponent::binToFrequency(int binIndex) const {
  return (float)binIndex * (float)sampleRate / (float)fftSize;
}

void SpectrumComponent::updatePeakTrace(std::vector<float> &trace,
                                        const std::vector<float> &source) {
  if (trace.size() != source.size()) {
    trace = source;
    return;
  }
  for (size_t i = 0; i < source.size(); ++i) {
    if (source[i] > trace[i])
      trace[i] = source[i];
  }
}

void SpectrumComponent::drawFrequencyMarkers(juce::Graphics& g, juce::Rectangle<float> inner) {
    if (smoothedLeft.empty()) return;

    const float threshold = -40.0f;
    const int minDistanceBins = 10;
    
    std::vector<int> peaks;
    for (int i = 1; i < (int)smoothedLeft.size() - 1; ++i) {
        if (smoothedLeft[i] > threshold && smoothedLeft[i] > smoothedLeft[i-1] && smoothedLeft[i] > smoothedLeft[i+1]) {
            if (peaks.empty() || i - peaks.back() > minDistanceBins) {
                peaks.push_back(i);
            }
        }
    }

    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(juce::FontOptions(10.0f));

    for (int bin : peaks) {
        float freq = binToFrequency(bin);
        float x = inner.getX() + frequencyToNormX(freq) * inner.getWidth();
        float y = inner.getBottom() - juce::jmap(smoothedLeft[bin], -displayRangeDb, 0.0f, 0.0f, 1.0f) * inner.getHeight();

        g.drawVerticalLine((int)x, y - 5, y + 5);
        g.drawText(formatFrequency(freq), (int)x - 20, (int)y - 15, 40, 10, juce::Justification::centred);
        
        // Harmonics
        for (int h = 2; h <= 4; ++h) {
            float hFreq = freq * h;
            if (hFreq > 20000.0f) break;
            float hX = inner.getX() + frequencyToNormX(hFreq) * inner.getWidth();
            g.setColour(juce::Colours::white.withAlpha(0.2f));
            g.drawVerticalLine((int)hX, inner.getY(), inner.getBottom());
        }
        g.setColour(juce::Colours::white.withAlpha(0.6f));
    }
}

void SpectrumComponent::drawNoteGrid(juce::Graphics& g, juce::Rectangle<float> inner) {
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    
    g.setColour(juce::Colour::fromRGBA(255, 255, 255, 15));
    for (int midi = 24; midi <= 108; midi += 12) { // Every C note from C1 to C8
        float freq = 440.0f * std::pow(2.0f, (midi - 69) / 12.0f);
        float x = inner.getX() + frequencyToNormX(freq) * inner.getWidth();
        
        if (x >= inner.getX() && x <= inner.getRight()) {
            g.drawVerticalLine((int)x, inner.getY(), inner.getBottom());
            g.drawText(noteNames[midi % 12] + juce::String(midi / 12 - 1), (int)x + 2, (int)inner.getBottom() - 25, 30, 10, juce::Justification::left);
        }
    }
}
