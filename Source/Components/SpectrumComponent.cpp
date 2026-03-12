#include "SpectrumComponent.h"

SpectrumComponent::SpectrumComponent() {
}

void SpectrumComponent::setAnalysisInfo(double newSampleRate, int newFFTSize) {
  sampleRate = newSampleRate;
  fftSize = newFFTSize;
}

void SpectrumComponent::setDisplayMode(DisplayMode newMode) {
  displayMode = newMode;
  repaint();
}

void SpectrumComponent::setFrozen(bool shouldBeFrozen) {
  frozen = shouldBeFrozen;
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

void SpectrumComponent::smoothInto(const std::vector<float> &input,
                                   std::vector<float> &target) {
  if (target.size() != input.size())
    target = input;

  const float smoothing = 0.65f; // Faster reaction (was 0.92)

  for (size_t i = 0; i < input.size(); ++i)
    target[i] = smoothing * target[i] + (1.0f - smoothing) * input[i];
}

void SpectrumComponent::setMagnitudesLeft(
    const std::vector<float> &newMagnitudes) {
  if (frozen)
    return;

  smoothInto(newMagnitudes, smoothedLeft);

  // Push to spectrogram (Mid or Left depending on mode)
  if (spectrogramFrames.size() >= maxSpectrogramFrames)
      spectrogramFrames.pop_front();
  
  spectrogramFrames.push_back(smoothedLeft);

  repaint();
}

void SpectrumComponent::setMagnitudesRight(
    const std::vector<float> &newMagnitudes) {
  if (frozen)
    return;

  smoothInto(newMagnitudes, smoothedRight);
  repaint();
}

static float frequencyToNormX(float frequency) {
  const float minFreq = 20.0f;
  const float maxFreq = 20000.0f;

  const float minLog = std::log10(minFreq);
  const float maxLog = std::log10(maxFreq);
  const float fLog = std::log10(juce::jlimit(minFreq, maxFreq, frequency));

  return (fLog - minLog) / (maxLog - minLog);
}

static juce::Path buildSpectrumPath(const std::vector<float> &mags,
                                    juce::Rectangle<float> inner,
                                    double sampleRate, int fftSize, float rangeDb) {
  juce::Path p;

  if (mags.empty() || fftSize <= 0 || sampleRate <= 0.0)
    return p;

  const int widthPixels = juce::jmax(2, (int)inner.getWidth());
  const float minFreq = 20.0f;
  const float maxFreq = 20000.0f;
  const float minLog = std::log10(minFreq);
  const float maxLog = std::log10(maxFreq);

  for (int xPixel = 0; xPixel < widthPixels; ++xPixel) {
    const float normX = (float)xPixel / (float)(widthPixels - 1);
    const float freqLog = minLog + normX * (maxLog - minLog);
    const float frequency = std::pow(10.0f, freqLog);

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
    db = juce::jlimit(-rangeDb, 0.0f, db);

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

void SpectrumComponent::paint(juce::Graphics &g) {
  auto area = getLocalBounds().toFloat();

  // Draw Amp-Style Hardware Frame
  auto frameBounds = area.reduced(1.0f);
  juce::ColourGradient frameGradOut(juce::Colour::fromRGB(45, 50, 55), frameBounds.getX(), frameBounds.getY(),
                                    juce::Colour::fromRGB(15, 15, 18), frameBounds.getX(), frameBounds.getBottom(), false);
  g.setGradientFill(frameGradOut);
  g.fillRoundedRectangle(frameBounds, 4.0f);

  auto panelBounds = frameBounds.reduced(3.0f);
  g.setColour(juce::Colour::fromRGB(15, 15, 15));
  g.fillRoundedRectangle(panelBounds, 2.0f);
  g.setColour(juce::Colour::fromRGBA(0, 0, 0, 180));
  g.drawRoundedRectangle(panelBounds.reduced(1.0f), 2.0f, 1.5f);

  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 25));
  g.drawRoundedRectangle(frameBounds.reduced(1.0f).withTrimmedBottom(2.0f), 4.0f, 1.0f);
  g.setColour(juce::Colour::fromRGBA(0, 0, 0, 150));
  g.drawRoundedRectangle(frameBounds.reduced(0.5f).withTrimmedTop(2.0f), 4.0f, 1.0f);

  auto drawScrew = [&](float cx, float cy) {
      g.setColour(juce::Colour::fromRGBA(0, 0, 0, 200));
      g.fillEllipse(cx - 3.5f, cy - 3.5f, 7.0f, 7.0f);
      g.setColour(juce::Colour::fromRGB(16, 16, 18));
      g.fillEllipse(cx - 3.0f, cy - 3.0f, 6.0f, 6.0f);
      g.setColour(juce::Colour::fromRGBA(255, 255, 255, 40));
      g.drawEllipse(cx - 3.0f, cy - 3.0f, 6.0f, 6.0f, 1.0f);
      g.setColour(juce::Colour::fromRGBA(255, 255, 255, 50));
      g.drawLine(cx - 1.5f, cy - 1.5f, cx + 1.5f, cy + 1.5f, 1.5f);
      g.setColour(juce::Colour::fromRGBA(0, 0, 0, 200));
      g.drawLine(cx - 1.0f, cy - 1.5f, cx + 2.0f, cy + 1.5f, 1.0f);
  };
  float offset = 7.0f;
  drawScrew(area.getX() + offset, area.getY() + offset);
  drawScrew(area.getRight() - offset, area.getY() + offset);
  drawScrew(area.getX() + offset, area.getBottom() - offset);
  drawScrew(area.getRight() - offset, area.getBottom() - offset);
  
  auto inner = area.reduced(12.0f);

  // --- SPECTROGRAM (WATERFALL) ---
  if (!spectrogramFrames.empty()) {
      const int numFrames = (int)spectrogramFrames.size();
      const float frameHeight = inner.getHeight() / (float)maxSpectrogramFrames;
      const bool isFullWaterfall = (displayMode == DisplayMode::Waterfall);
      
      for (int i = 0; i < numFrames; ++i) {
          const auto& frame = spectrogramFrames[(size_t)i];
          const float y = inner.getBottom() - (float)(numFrames - i) * frameHeight;
          
          const int step = isFullWaterfall ? 2 : 4;
          for (int x = 0; x < (int)inner.getWidth(); x += step) {
              const float normX = (float)x / inner.getWidth();
              const float freqLog = std::log10(20.0f) + normX * (std::log10(20000.0f) - std::log10(20.0f));
              const float freq = std::pow(10.0f, freqLog);
              const int bin = juce::jlimit(0, (int)frame.size()-1, (int)(freq * fftSize / sampleRate));
              
              float val = juce::jmap(frame[(size_t)bin], -displayRangeDb, 0.0f, 0.0f, 1.0f);
              if (val > 0.05f) {
                  if (isFullWaterfall) {
                      // Heatmap color scheme: Blue -> Cyan -> Yellow -> White
                      juce::Colour c;
                      if (val < 0.3f) c = juce::Colour::fromRGB(0, 0, 100).interpolatedWith(juce::Colours::blue, val / 0.3f);
                      else if (val < 0.6f) c = juce::Colours::blue.interpolatedWith(juce::Colours::cyan, (val - 0.3f) / 0.3f);
                      else if (val < 0.8f) c = juce::Colours::cyan.interpolatedWith(juce::Colours::yellow, (val - 0.6f) / 0.2f);
                      else c = juce::Colours::yellow.interpolatedWith(juce::Colours::white, (val - 0.8f) / 0.2f);
                      
                      g.setColour(c.withAlpha(0.8f));
                  } else {
                      g.setColour(juce::Colour::fromRGB(88, 174, 219).withAlpha(val * 0.15f));
                  }
                  g.fillRect(inner.getX() + (float)x, y, (float)step, frameHeight);
              }
          }
      }
  }

  g.setFont(juce::FontOptions(11.0f));

  // Funky blue Grid
  g.setColour(juce::Colour::fromRGBA(88, 174, 219, 15));
  const float dashH[] = {4.0f, 4.0f};
  std::vector<float> gridLines = (displayRangeDb < 70.0f) 
                                 ? std::vector<float>{0.f, -10.f, -20.f, -30.f, -40.f, -50.f, -60.f}
                                 : std::vector<float>{0.f, -12.f, -24.f, -36.f, -48.f, -60.f, -72.f, -84.f};
  for (float db : gridLines) {
    const float norm = juce::jmap(db, -displayRangeDb, 0.f, 0.f, 1.f);
    const float y = inner.getBottom() - norm * inner.getHeight();

    g.drawDashedLine(juce::Line<float>(inner.getX(), y, inner.getRight(), y),
                      dashH, 2, 0.8f);

    g.setColour(juce::Colour::fromRGBA(200, 220, 240, 60));
    g.drawText(juce::String((int)db),
               juce::Rectangle<int>((int)inner.getX() + 4, (int)y - 8, 32, 12),
               juce::Justification::left, false);
    g.setColour(juce::Colour::fromRGBA(88, 174, 219, 15));
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

    g.setColour(juce::Colour::fromRGBA(88, 174, 219, 10));
    g.drawDashedLine(juce::Line<float>(x, inner.getY(), x, inner.getBottom()),
                      dashV, 2, 0.8f);

    g.setColour(juce::Colour::fromRGBA(200, 220, 240, 80));
    g.drawText(
        mark.label,
        juce::Rectangle<int>((int)x - 18, (int)inner.getBottom() - 14, 36, 12),
        juce::Justification::centred, false);
  }

  auto drawFilledPath = [&](const juce::Path &p, juce::Colour line,
                            juce::Colour glow, juce::Colour fillTop) {
    if (p.isEmpty())
      return;

    juce::Path fill = p;
    fill.lineTo(inner.getRight(), inner.getBottom());
    fill.lineTo(inner.getX(), inner.getBottom());
    fill.closeSubPath();

    juce::ColourGradient grad(fillTop.withAlpha(0.4f), inner.getCentreX(), inner.getY(),
                              fillTop.withAlpha(0.0f), inner.getCentreX(),
                              inner.getBottom(), false);

    g.setGradientFill(grad);
    g.fillPath(fill);

    // Dynamic Multi-Layer Glow
    g.setColour(glow.withAlpha(0.15f));
    g.strokePath(p, juce::PathStrokeType(6.0f));
    g.setColour(glow.withAlpha(0.3f));
    g.strokePath(p, juce::PathStrokeType(3.5f));

    g.setColour(line);
    g.strokePath(p, juce::PathStrokeType(1.6f));
    
    // Bright Core
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.strokePath(p, juce::PathStrokeType(0.6f));
  };

  const auto leftPath =
      buildSpectrumPath(smoothedLeft, inner, sampleRate, fftSize, displayRangeDb);
  const auto rightPath =
      buildSpectrumPath(smoothedRight, inner, sampleRate, fftSize, displayRangeDb);

  // Matches the Blue/Purple accents from the Amp
  if (displayMode == DisplayMode::Waterfall) {
      // Don't draw the spectrum path in waterfall mode, or draw it as a thin line
      g.setColour(juce::Colours::white.withAlpha(0.4f));
      g.strokePath(leftPath, juce::PathStrokeType(1.0f));
  } else if (displayMode == DisplayMode::LR) {
    drawFilledPath(leftPath, juce::Colour::fromRGB(88, 174, 219),
                   juce::Colour::fromRGBA(88, 174, 219, 40),
                   juce::Colour::fromRGBA(88, 174, 219, 120));

    drawFilledPath(rightPath, juce::Colour::fromRGB(170, 80, 255),
                   juce::Colour::fromRGBA(170, 80, 255, 30),
                   juce::Colour::fromRGBA(170, 80, 255, 100));
  } else {
    // M/S specific warmer colors
    drawFilledPath(leftPath, juce::Colour::fromRGB(255, 205, 90),
                   juce::Colour::fromRGBA(255, 205, 90, 40),
                   juce::Colour::fromRGBA(255, 205, 90, 120));

    drawFilledPath(rightPath, juce::Colour::fromRGB(255, 150, 70),
                   juce::Colour::fromRGBA(255, 150, 70, 30),
                   juce::Colour::fromRGBA(255, 150, 70, 100));
  }

  // --- PEAK TRACES ---
  if (!frozen) {
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
      buildSpectrumPath(peakTraceLeft, inner, sampleRate, fftSize, displayRangeDb);
  const auto rightPeakPath =
      buildSpectrumPath(peakTraceRight, inner, sampleRate, fftSize, displayRangeDb);

  auto drawPeakLine = [&](const juce::Path &p, juce::Colour c) {
    if (!p.isEmpty()) {
      g.setColour(c.withAlpha(0.85f));
      g.strokePath(p, juce::PathStrokeType(0.8f)); // Even thinner line
    }
  };

  if (displayMode == DisplayMode::LR) {
      drawPeakLine(leftPeakPath, juce::Colour::fromRGBA(132, 238, 255, 255));
      drawPeakLine(rightPeakPath, juce::Colour::fromRGBA(255, 190, 96, 255));
  } else {
      drawPeakLine(leftPeakPath, juce::Colour::fromRGBA(255, 225, 130, 255));
      drawPeakLine(rightPeakPath, juce::Colour::fromRGBA(255, 170, 90, 255));
  }

  g.setColour(juce::Colour::fromRGBA(240, 245, 250, 138));
  g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
  g.drawText("SPECTRUM", getLocalBounds().reduced(16, 10),
             juce::Justification::topLeft, false);

  if (frozen) {
      g.setColour(juce::Colour::fromRGBA(132, 238, 255, 200));
      g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
      g.drawText("FROZEN", getLocalBounds().reduced(20, 10),
                 juce::Justification::topRight, false);
  }

  g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));

  if (displayMode == DisplayMode::LR) {
    g.setColour(juce::Colour::fromRGBA(132, 238, 255, 180));
    g.drawText("L",
               juce::Rectangle<int>((int)inner.getRight() - 52,
                                    (int)inner.getY() + 8, 16, 12),
               juce::Justification::centred, false);

    g.setColour(juce::Colour::fromRGBA(255, 190, 96, 180));
    g.drawText("R",
               juce::Rectangle<int>((int)inner.getRight() - 28,
                                    (int)inner.getY() + 8, 16, 12),
               juce::Justification::centred, false);
  } else {
    g.setColour(juce::Colour::fromRGBA(255, 205, 90, 180));
    g.drawText("M",
               juce::Rectangle<int>((int)inner.getRight() - 52,
                                    (int)inner.getY() + 8, 16, 12),
               juce::Justification::centred, false);

    g.setColour(juce::Colour::fromRGBA(255, 150, 70, 180));
    g.drawText("S",
               juce::Rectangle<int>((int)inner.getRight() - 28,
                                    (int)inner.getY() + 8, 16, 12),
               juce::Justification::centred, false);
  }

  // --- MOUSE HOVER DATAPOINT (TOOLTIP) ---
  auto mousePos = getMouseXYRelative().toFloat();
  if (inner.contains(mousePos)) {
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

    // Draw crosshair
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawVerticalLine((int)mousePos.x, inner.getY(), inner.getBottom());
    g.drawHorizontalLine((int)mousePos.y, inner.getX(), inner.getRight());

    // Draw text box
    juce::Rectangle<int> tooltipBounds(static_cast<int>(mousePos.x + 10.0f),
                                       static_cast<int>(mousePos.y - 20.0f),
                                       140, 20);
    if (tooltipBounds.getRight() > inner.getRight())
      tooltipBounds.setX(static_cast<int>(mousePos.x - 150.0f));

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
}

void SpectrumComponent::resetPeakTrace() {
  std::fill(peakTraceLeft.begin(), peakTraceLeft.end(), -displayRangeDb);
  std::fill(peakTraceRight.begin(), peakTraceRight.end(), -displayRangeDb);
}

void SpectrumComponent::mouseDoubleClick(const juce::MouseEvent &) {
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
