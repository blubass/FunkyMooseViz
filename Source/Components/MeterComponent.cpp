#include "MeterComponent.h"

void MeterComponent::setLevels(float newPeak, float newHold, float newRms) {
  peak = juce::jlimit(0.0f, 1.2f, newPeak);
  hold = juce::jlimit(0.0f, 1.2f, newHold);
  rms = juce::jlimit(0.0f, 1.2f, newRms);
  repaint();
}

void MeterComponent::setLabel(juce::String newLabel) {
  label = std::move(newLabel);
}

float MeterComponent::gainToMeter01(float value) {
  const float db =
      juce::Decibels::gainToDecibels(juce::jmax(value, 0.000001f), -60.0f);
  return juce::jlimit(0.0f, 1.0f, juce::jmap(db, -60.0f, 6.0f, 0.0f, 1.0f));
}

void MeterComponent::paint(juce::Graphics &g) {
  auto area = getLocalBounds().toFloat();

  // Module Panel Style from Amp
  g.setColour(juce::Colour::fromRGB(18, 18, 18));
  g.fillRoundedRectangle(area, 10.0f);

  g.setColour(juce::Colour::fromRGBA(0, 0, 0, 80));
  g.drawRoundedRectangle(area, 10.0f, 1.0f);
  
  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 12));
  g.drawRoundedRectangle(area.reduced(1.0f), 10.0f, 0.8f);

  auto inner = area.reduced(9.0f);
  auto labelArea = inner.removeFromBottom(20.0f);
  inner.removeFromBottom(2.0f);

  juce::ColourGradient slotGrad(juce::Colour::fromRGB(10, 10, 10),
                                inner.getX(), inner.getY(),
                                juce::Colour::fromRGB(25, 25, 25),
                                inner.getX(), inner.getBottom(), false);
  g.setGradientFill(slotGrad);
  g.fillRoundedRectangle(inner, 4.0f);

  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 8));
  g.drawRoundedRectangle(inner, 4.0f, 1.0f);

  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 26));
  for (float db : {-48.0f, -36.0f, -24.0f, -12.0f, -6.0f, 0.0f}) {
    const float norm = juce::jmap(db, -60.0f, 6.0f, 0.0f, 1.0f);
    const float y = inner.getBottom() - norm * inner.getHeight();
    g.drawHorizontalLine((int)y, inner.getX() + 4.0f, inner.getRight() - 4.0f);
  }

  const float peakNorm = gainToMeter01(peak);
  const float holdNorm = gainToMeter01(hold);
  const float rmsNorm = gainToMeter01(rms);

  auto fill = inner;
  fill.removeFromTop(inner.getHeight() * (1.0f - peakNorm));

  // Input/Output Meter Purple from Amp
  juce::ColourGradient meterGrad(
      juce::Colour::fromRGB(120, 60, 180), inner.getCentreX(),
      inner.getBottom(), juce::Colour::fromRGB(190, 100, 255),
      inner.getCentreX(), inner.getY() + inner.getHeight() * 0.35f, false);
  meterGrad.addColour(0.86, juce::Colour::fromRGB(220, 140, 255));
  meterGrad.addColour(1.00, juce::Colour::fromRGB(255, 200, 255));

  g.setGradientFill(meterGrad);
  g.fillRoundedRectangle(fill, 7.0f);

  const float rmsY = inner.getBottom() - rmsNorm * inner.getHeight();
  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 120));
  g.drawHorizontalLine((int)rmsY, inner.getX() + 3.0f, inner.getRight() - 3.0f);

  const float holdY = inner.getBottom() - holdNorm * inner.getHeight();
  g.setColour(juce::Colour::fromRGB(255, 245, 205));
  g.drawHorizontalLine((int)holdY, inner.getX() + 1.0f,
                       inner.getRight() - 1.0f);

  if (peakNorm > 0.01f) {
    auto glow = fill.withHeight(juce::jmin(18.0f, fill.getHeight()));
    g.setColour(juce::Colour::fromRGBA(255, 255, 255, 36));
    g.fillRoundedRectangle(glow, 6.0f);
  }

  if (peak > 1.0f) {
    g.setColour(juce::Colours::red.withAlpha(0.85f));
    g.fillEllipse(inner.getCentreX() - 3.0f, inner.getY() + 3.0f, 6.0f, 6.0f);
  }

  g.setColour(juce::Colour::fromRGBA(230, 235, 240, 170));
  g.setFont(juce::FontOptions(12.5f));
  g.drawText(label, labelArea.toNearestInt(), juce::Justification::centred,
             false);
}

void CorrelationMeterComponent::setCorrelation(float newCorrelation) {
  correlation = newCorrelation;
  repaint();
}

void CorrelationMeterComponent::paint(juce::Graphics &g) {
  auto area = getLocalBounds().toFloat();

  // Funky Moose Module Panel (Beveled Frame)
  g.setColour(juce::Colour::fromRGB(15, 15, 15));
  g.fillRoundedRectangle(area, 6.0f);
  
  // Metallic Bevel
  juce::ColourGradient bevel(juce::Colour::fromRGB(70, 75, 80), area.getX(), area.getY(),
                             juce::Colour::fromRGB(20, 22, 25), area.getX(), area.getBottom(), false);
  g.setGradientFill(bevel);
  g.drawRoundedRectangle(area, 6.0f, 1.8f);

  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 30));
  g.drawRoundedRectangle(area.reduced(0.5f), 6.0f, 0.8f);

  auto inner = area.reduced(2.0f);
  auto labelArea = inner.removeFromBottom(20.0f);
  inner.removeFromBottom(2.0f);

  juce::ColourGradient slotGrad(juce::Colour::fromRGBA(255, 255, 255, 12),
                                inner.getX(), inner.getY(),
                                juce::Colour::fromRGBA(0, 0, 0, 50),
                                inner.getX(), inner.getBottom(), false);
  g.setGradientFill(slotGrad);
  g.fillRoundedRectangle(inner, 8.0f);

  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 18));
  g.drawRoundedRectangle(inner, 8.0f, 1.0f);

  float midY = inner.getY() + inner.getHeight() * 0.5f;
  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 60));
  g.drawHorizontalLine((int)midY, inner.getX() + 4.0f, inner.getRight() - 4.0f);

  float val = juce::jlimit(-1.0f, 1.0f, correlation);
  float yPos = inner.getY() + inner.getHeight() * 0.5f * (1.0f - val);

  juce::Rectangle<float> fill;
  if (val >= 0.0f) {
    fill = juce::Rectangle<float>(inner.getX(), yPos, inner.getWidth(),
                                  midY - yPos);
    g.setColour(juce::Colour(0xFF00BFFF).withAlpha(0.8f)); // soft cyan mix
  } else {
    fill = juce::Rectangle<float>(inner.getX(), midY, inner.getWidth(),
                                  yPos - midY);
    g.setColour(juce::Colours::orange.withAlpha(0.8f));
  }
  g.fillRoundedRectangle(fill, 2.0f);

  g.setColour(juce::Colours::white);
  g.drawHorizontalLine((int)yPos, inner.getX() + 2.0f, inner.getRight() - 2.0f);

  g.setColour(juce::Colour::fromRGBA(230, 235, 240, 170));
  g.setFont(juce::FontOptions(12.0f));
  g.drawText("PHASE", labelArea.toNearestInt(), juce::Justification::centred,
             false);
}

void LoudnessMeterComponent::setLoudness(float newLoudness) {
  loudness = newLoudness;
  repaint();
}

void LoudnessMeterComponent::paint(juce::Graphics &g) {
  auto area = getLocalBounds().toFloat();

  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 16));
  g.fillRoundedRectangle(area, 12.0f);

  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 26));
  g.drawRoundedRectangle(area.reduced(0.5f), 12.0f, 1.0f);

  auto inner = area.reduced(9.0f);
  auto labelArea = inner.removeFromBottom(20.0f);
  inner.removeFromBottom(2.0f);

  // Meter Slot
  g.setColour(juce::Colour::fromRGBA(0, 0, 0, 40));
  g.fillRoundedRectangle(inner, 8.0f);

  float norm = juce::jlimit(0.0f, 1.0f, juce::jmap(juce::Decibels::gainToDecibels(loudness, -60.0f), -60.0f, 0.0f, 0.0f, 1.0f));
  auto fill = inner;
  fill.removeFromTop(inner.getHeight() * (1.0f - norm));

  juce::ColourGradient grad(juce::Colour::fromRGB(88, 174, 219), inner.getX(), inner.getBottom(),
                            juce::Colour::fromRGB(140, 220, 255), inner.getX(), inner.getY(), false);

  g.setGradientFill(grad);
  g.fillRoundedRectangle(fill, 4.0f);

  g.setColour(juce::Colour::fromRGB(220, 220, 220));
  g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
  g.drawText("LOUD", labelArea.toNearestInt(), juce::Justification::centred, false);
}
