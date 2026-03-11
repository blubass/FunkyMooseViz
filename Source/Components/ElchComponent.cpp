#include "ElchComponent.h"
#include <cmath>

void ElchComponent::paint(juce::Graphics &g) {
  g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
  auto bounds = getLocalBounds().toFloat();

  // 1. Draw Static Background (Clean Dark Plate)
  if (!cachedBackground.isValid()) {
    updateCachedBackground();
  }
  g.drawImageAt(cachedBackground, 0, 0);

  // --- CALCULATE DYNAMIC VALUES ---
  float bright =
      juce::jlimit(0.15f, 3.0f, inputLevel * 2.6f + eyeAmount * 0.6f);
  bright = std::max(bright, 0.55f); // Raised minimum glow floor
  float amt = juce::jlimit(0.0f, 1.0f, inputLevel * 1.8f + eyeAmount * 0.6f);
  amt = std::max(amt, 0.35f); // Always some amount so sunglass glow is visible

  // 1.5. Draw Bass Pulse Glow (Behind Moose)
  if (inputLevel > 0.1f) {
      float pulseRadius = bounds.getWidth() * (0.3f + inputLevel * 0.4f);
      juce::ColourGradient pulse(juce::Colour::fromRGB(88, 174, 219).withAlpha(inputLevel * 0.3f),
                                 bounds.getCentre(),
                                 juce::Colours::transparentBlack,
                                 bounds.getCentre().translated(pulseRadius, 0), true);
      g.setGradientFill(pulse);
      g.fillEllipse(bounds.getCentreX() - pulseRadius, bounds.getCentreY() - pulseRadius, pulseRadius * 2.0f, pulseRadius * 2.0f);
  }

  // 2. Draw Elch Image (ABSOLUTE FOREGROUND - NO MASKING)
  if (elchImage.isValid()) {
    g.saveState();

    // HEADBANGING / ZOOM ANIMATION: (Dampened)
    float zoomFactor = 1.0f + (peakLevel * 0.04f); // Reduced from 8% to 4%
    juce::AffineTransform transform = juce::AffineTransform::scale(
        zoomFactor, zoomFactor, bounds.getCentreX(), bounds.getBottom());
    g.addTransform(transform);

    // Use full opacity and no blending modes that could darken the image
    g.setOpacity(1.0f);

    // Draw the image itself - Centered and Vibrant
    g.drawImage(elchImage, bounds, juce::RectanglePlacement::centred);

    // Dynamic separation highlight (Rim Light)
    g.setColour(juce::Colours::white.withAlpha(0.20f * amt + 0.1f));
    g.drawEllipse(bounds.reduced(2.0f), 1.2f);

    g.restoreState();
  }

  // --- DYNAMIC SLOGAN REMOVED ---

  // --- SUNGLASSES & ANTLERS (Glow Layers) ---
  if (inputLevel > 0.001f || eyeAmount > 0.001f || amt > 0.1f) {
    float w = bounds.getWidth();
    float faceY = bounds.getY() + bounds.getHeight() * 0.48f;
    float centerX = bounds.getCentreX();
    float sep = w * 0.12f;
    float radius = w * 0.065f;

    auto leftCenter = juce::Point<float>(centerX - sep * 0.9f, faceY);
    auto rightCenter = juce::Point<float>(centerX + sep * 0.9f, faceY);

    juce::Colour baseL = juce::Colours::cyan;
    juce::Colour baseR = juce::Colour(0xFFFF8000);

    auto drawHalo = [&](juce::Point<float> pt, juce::Colour c, float r,
                        float intensityScale = 1.0f) {
      g.saveState();
      // Much larger, softer falloff for the 'Halo' vibe
      float rGlow = r * (3.0f + 1.5f * inputLevel);

      // Boosted intensity to ~95% maximum (was 85%)
      float finalAlpha = std::min(0.95f, 0.85f * bright) * intensityScale;

      juce::Colour cCore = c.brighter(0.5f).withAlpha(finalAlpha);
      juce::Colour cMid = c.withAlpha(finalAlpha * 0.75f);
      juce::Colour cOuter = c.withAlpha(0.0f);

      juce::ColourGradient cg(cCore, pt.x, pt.y, cOuter, pt.x, pt.y - rGlow,
                              true);
      cg.addColour(0.20f, cCore); // Stronger core
      cg.addColour(0.45f, cMid);  // Softer falloff

      g.setGradientFill(cg);
      g.fillEllipse(pt.x - rGlow, pt.y - rGlow, rGlow * 2.0f, rGlow * 2.0f);
      g.restoreState();
    };

    // Eyes: Small glowing centers, subtle halo
    drawHalo(leftCenter, baseL, radius * 0.6f, 1.0f);
    drawHalo(rightCenter, baseR, radius * 0.6f, 1.0f);

    float antlerY = bounds.getY() + bounds.getHeight() * 0.30f;
    float antlerSep = w * 0.28f;
    auto leftAntler = juce::Point<float>(centerX - antlerSep, antlerY);
    auto rightAntler = juce::Point<float>(centerX + antlerSep,
                                          antlerY - bounds.getHeight() * 0.02f);

    // Antlers: Even subtler halos (0.6 intensity scale), no distinct 'balls'
    drawHalo(leftAntler, baseL, radius * 0.5f, 0.6f);
    drawHalo(rightAntler, baseR, radius * 0.5f, 0.6f);

    // --- DIAMOND SPARKLE ---
    auto drawSparkle = [&](juce::Point<float> pt, float phase) {
      float s = 10.0f * bright;
      float time = (float)juce::Time::getMillisecondCounterHiRes() * 0.008f;
      float rot = time + phase;
      g.saveState();
      g.setColour(juce::Colours::white.withAlpha(1.0f * bright));
      g.addTransform(juce::AffineTransform::rotation(rot, pt.x, pt.y));
      g.fillRect(pt.x - s * 0.5f, pt.y - s * 0.06f, s, s * 0.12f);
      g.fillRect(pt.x - s * 0.06f, pt.y - s * 0.5f, s * 0.12f, s);
      g.restoreState();
    };

    if (bright > 0.40f) {
      drawSparkle(leftCenter.translated(-radius * 0.4f, -radius * 0.35f), 0.0f);
      drawSparkle(rightCenter.translated(radius * 0.4f, -radius * 0.35f), 3.3f);
    }
  }
}

void ElchComponent::setMooseState(float inRms, float outRms, float compGRdb,
                                  bool punchOn) {
  const float eps = 1.0e-6f;
  const float outDb = 20.0f * std::log10(std::max(eps, outRms));
  const float thrDb = activationThresholdDb;
  float act = juce::jlimit(0.0f, 1.0f, (outDb - thrDb) / (-20.0f - thrDb));
  act = act * act;

  const float outN = juce::jlimit(0.0f, 1.0f, outRms * 1.6f) * act;
  const float inN = juce::jlimit(0.0f, 1.0f, inRms * 1.6f) * act;
  float grN = std::sqrt(juce::jlimit(0.0f, 1.0f, compGRdb / 12.0f));

  glowAmount = glowAmount * 0.88f +
               (juce::jlimit(0.0f, 1.0f, outN * 0.85f + grN * 0.55f)) * 0.12f;
  eyeAmount = eyeAmount * 0.84f +
              (punchOn ? juce::jlimit(0.0f, 1.0f, 0.32f + outN * 0.72f)
                       : juce::jlimit(0.0f, 1.0f, outN * 0.62f)) *
                  0.16f;

  inputLevel = inputLevel * 0.92f + (inN * 1.2f) * 0.08f; // More smoothing, less gain

  peakLevel = peakLevel * 0.90f + outN * 0.10f; // More smoothing
  repaint();
}

void ElchComponent::resized() { updateCachedBackground(); }

void ElchComponent::updateCachedBackground() {
  auto bounds = getLocalBounds();
  if (bounds.isEmpty())
    return;

  cachedBackground = juce::Image(juce::Image::ARGB, bounds.getWidth(),
                                 bounds.getHeight(), true);
  juce::Graphics g(cachedBackground);

  // 1. Midnight Petrol / Night Blue Radial Gradient
  juce::Colour centerCol(0xff020408); // Darker center
  juce::Colour edgeCol(0xff060c18);   // Dark Petrol hint towards outer edges
  juce::ColourGradient bgGrad(centerCol, bounds.getCentreX(),
                              bounds.getCentreY(), edgeCol, 0.0f, 0.0f, true);
  g.setGradientFill(bgGrad);
  g.fillRect(bounds.toFloat());

  // Tiny bit of dark grit for texture (Increased to 3500 particles for more
  // character)
  juce::Random rnd(1234);
  for (int i = 0; i < 3500; ++i) {
    g.setColour(juce::Colours::white.withAlpha(rnd.nextFloat() * 0.045f));
    g.fillRect(rnd.nextFloat() * bounds.getWidth(),
               rnd.nextFloat() * bounds.getHeight(), 1.0f, 1.0f);
  }

  // --- VINTAGE GLASS REFLECTION ---
  // Thin top hairline for that polished tube-amp shield look
  g.setColour(juce::Colours::white.withAlpha(0.22f));
  g.drawLine(20.0f, 1.5f, bounds.getRight() - 20.0f, 1.5f, 0.6f);

  // Very soft top-down glaze
  juce::ColourGradient glaze(juce::Colours::white.withAlpha(0.06f), 0, 0,
                             juce::Colours::transparentWhite, 0, 15.0f, false);
  g.setGradientFill(glaze);
  g.fillRect(bounds.removeFromTop(15).toFloat());

  // MODULAR FRAME BEVEL (The outer slot border)
  g.setColour(juce::Colours::black);
  g.drawRect(bounds.toFloat(), 1.5f);
  g.setColour(juce::Colours::white.withAlpha(0.05f));
  g.drawRect(bounds.toFloat().reduced(0.8f), 0.5f);

  // GUARANTEED NO MASKING/OVALS IN BACKGROUND
}

void ElchComponent::setGlowPalette(juce::Colour normal, juce::Colour punch,
                                   juce::Colour heavy) {
  glowNormal = normal;
  glowPunch = punch;
  glowHeavy = heavy;
}

void ElchComponent::setVizMode(VizMode newMode) {
  vizMode = newMode;

  if (vizMode == VizMode::LR) {
    eyeGlowColourL = juce::Colour::fromRGB(80, 220, 255);
    eyeGlowColourR = juce::Colour::fromRGB(255, 170, 60);

    glowNormal = juce::Colour::fromRGB(70, 210, 255);
    glowPunch = juce::Colour::fromRGB(150, 235, 255);
    glowHeavy = juce::Colour::fromRGB(255, 230, 180);
  } else {
    eyeGlowColourL = juce::Colour::fromRGB(255, 205, 90);
    eyeGlowColourR = juce::Colour::fromRGB(255, 150, 70);

    glowNormal = juce::Colour::fromRGB(255, 185, 70);
    glowPunch = juce::Colour::fromRGB(255, 220, 120);
    glowHeavy = juce::Colour::fromRGB(255, 120, 70);
  }

  repaint();
}

void ElchComponent::setVizSignal(float rms, float peak) {
  // Dampened targets for more natural movement
  const float rmsTarget = juce::jlimit(0.0f, 1.0f, rms * 1.1f);
  const float peakTarget = juce::jlimit(0.0f, 1.0f, peak * 0.9f);

  // More smoothing (increase falloff / decrease alpha)
  glowAmount = glowAmount * 0.90f + rmsTarget * 0.10f;
  eyeFlash = juce::jmax(eyeFlash * 0.75f, peakTarget * 0.65f);

  inputLevel = inputLevel * 0.85f + juce::jmax(rmsTarget, peakTarget) * 0.15f;
  peakLevel = juce::jmax(peakLevel * 0.80f, peakTarget * 0.70f);

  // Ensure sunglasses glow correctly tracks peak when used by PluginEditor
  eyeAmount = eyeFlash;

  repaint();
}
