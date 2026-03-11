#include "WaveformComponent.h"
#include <cmath>

void WaveformComponent::setFrozen(bool shouldBeFrozen) {
  frozen = shouldBeFrozen;
  repaint();
}

void WaveformComponent::setWaveform(const std::vector<float> &left, const std::vector<float> &right) {
  if (frozen)
    return;

  waveformL = left;
  waveformR = right;

  repaint();
}

void WaveformComponent::paint(juce::Graphics &g) {
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

  auto inner = area.reduced(10.0f);

  // --- VECTOR SCOPE (GONIOMETER) ---
  // We draw this on the left side of the component
  auto scopeArea = inner.removeFromLeft(inner.getWidth() * 0.45f);
  inner.removeFromLeft(10.0f); // spacer

  // Scope background
  g.setColour(juce::Colour::fromRGB(10, 10, 10));
  g.fillEllipse(scopeArea);
  g.setColour(juce::Colour::fromRGBA(88, 174, 219, 30));
  g.drawEllipse(scopeArea, 1.0f);

  // Scope grid
  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 10));
  g.drawLine(scopeArea.getCentreX(), scopeArea.getY(), scopeArea.getCentreX(), scopeArea.getBottom(), 0.5f);
  g.drawLine(scopeArea.getX(), scopeArea.getCentreY(), scopeArea.getRight(), scopeArea.getCentreY(), 0.5f);

  if (!waveformL.empty() && waveformL.size() == waveformR.size()) {
    juce::Path p;
    const size_t numSamples = waveformL.size();
    const float midX = scopeArea.getCentreX();
    const float midY = scopeArea.getCentreY();
    const float scale = scopeArea.getWidth() * 0.45f;

    for (size_t i = 0; i < numSamples; i += 2) {
      float l = waveformL[i];
      float r = waveformR[i];
      
      // Standard Goniometer rotation:
      // X = (L - R) / sqrt(2)
      // Y = (L + R) / sqrt(2)
      float x = (l - r) * 0.7071f;
      float y = (l + r) * 0.7071f;

      float px = midX + x * scale;
      float py = midY - y * scale; // inverted Y

      if (i == 0) p.startNewSubPath(px, py);
      else p.lineTo(px, py);
    }

    g.setColour(juce::Colour::fromRGB(88, 174, 219).withAlpha(0.6f));
    g.strokePath(p, juce::PathStrokeType(1.2f));
  }

  // --- WAVEFORM ---
  // Draw merged/mid waveform on the right
  const float centerY = inner.getCentreY();
  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 15));
  g.drawLine(inner.getX(), centerY, inner.getRight(), centerY, 0.8f);

  if (!waveformL.empty()) {
    juce::Path p;
    const int drawPoints = juce::jmax(2, (int)inner.getWidth());
    const int sourceSize = (int)waveformL.size();

    for (int xIndex = 0; xIndex < drawPoints; ++xIndex) {
      const float norm = (float)xIndex / (float)(drawPoints - 1);
      const int srcIndex = juce::jlimit(0, sourceSize - 1, (int)(norm * (float)(sourceSize - 1)));
      const float x = inner.getX() + norm * inner.getWidth();
      
      // Mid signal for display
      float mid = (waveformL[(size_t)srcIndex] + waveformR[(size_t)srcIndex]) * 0.5f;
      const float y = centerY - mid * (inner.getHeight() * 0.45f);

      if (xIndex == 0) p.startNewSubPath(x, y);
      else p.lineTo(x, y);
    }

    g.setColour(juce::Colour::fromRGB(190, 100, 255).withAlpha(0.4f));
    g.strokePath(p, juce::PathStrokeType(3.0f));
    g.setColour(juce::Colour::fromRGB(220, 140, 255));
    g.strokePath(p, juce::PathStrokeType(1.0f));
  }

  // Labels
  g.setColour(juce::Colour::fromRGB(200, 200, 200));
  g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
  g.drawText("VECTOR", scopeArea.toNearestInt(), juce::Justification::centredBottom, false);
  g.drawText("WAVEFORM", inner.toNearestInt(), juce::Justification::centredBottom, false);
}
