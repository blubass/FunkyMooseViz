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

  // Funky Moose Module Panel (Beveled Frame)
  g.setColour(juce::Colour::fromRGB(15, 15, 15));
  g.fillRoundedRectangle(area, 10.0f);
  
  // Metallic Bevel
  juce::ColourGradient bevel(juce::Colour::fromRGB(70, 75, 80), area.getX(), area.getY(),
                             juce::Colour::fromRGB(20, 22, 25), area.getX(), area.getBottom(), false);
  g.setGradientFill(bevel);
  g.drawRoundedRectangle(area, 10.0f, 1.8f);

  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 30));
  g.drawRoundedRectangle(area.reduced(0.5f), 10.0f, 0.8f);

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
  g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
  g.drawText("VECTOR", scopeArea.toNearestInt(), juce::Justification::centredBottom, false);
  g.drawText("WAVEFORM", inner.toNearestInt(), juce::Justification::centredBottom, false);
}
