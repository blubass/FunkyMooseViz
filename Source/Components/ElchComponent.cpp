#include "ElchComponent.h"
#include <cmath>

void ElchComponent::setVizMode (VizMode newMode)
{
    vizMode = newMode;

    if (vizMode == VizMode::LR)
    {
        eyeGlowColourL = juce::Colour::fromRGB (80, 220, 255);
        eyeGlowColourR = juce::Colour::fromRGB (255, 170, 60);

        glowNormal = juce::Colour::fromRGB (70, 210, 255);
        glowPunch  = juce::Colour::fromRGB (150, 235, 255);
    }
    else
    {
        eyeGlowColourL = juce::Colour::fromRGB (255, 205, 90);
        eyeGlowColourR = juce::Colour::fromRGB (255, 150, 70);

        glowNormal = juce::Colour::fromRGB (255, 185, 70);
        glowPunch  = juce::Colour::fromRGB (255, 220, 120);
    }

    repaint();
}

void ElchComponent::setVizSignal (float rms, float peak)
{
    const float rmsTarget  = juce::jlimit (0.0f, 1.0f, rms);
    const float peakTarget = juce::jlimit (0.0f, 1.0f, peak);

    glowAmount = glowAmount * 0.91f + rmsTarget * 0.09f;
    eyeFlash   = juce::jmax (eyeFlash * 0.78f, peakTarget * 0.70f);

    inputLevel = inputLevel * 0.90f + juce::jmax (rmsTarget, peakTarget) * 0.10f;
    peakLevel  = juce::jmax (peakLevel * 0.82f, peakTarget * 0.70f);

    repaint();
}

void ElchComponent::setSpectrum (const std::vector<float>& spectrum)
{
    currentSpectrum = spectrum;
    repaint();
}

void ElchComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    if (! bgColor.isTransparent())
        g.fillAll (bgColor);

    juce::Colour modeAura = glowNormal.interpolatedWith (
        glowPunch,
        juce::jlimit (0.0f, 1.0f, glowAmount * 0.85f + eyeFlash * 0.35f));

    if (glowAmount > 0.015f)
    {
        g.saveState();

        const float extra = 4.0f + 16.0f * glowAmount;
        auto auraBounds = bounds.reduced (bounds.getWidth() * 0.18f,
                                          bounds.getHeight() * 0.15f).expanded (extra);

        // Multi-layer Glow for "Boutique" Depth
        juce::ColourGradient auraGrad (
            modeAura.withAlpha (0.22f * glowAmount),
            auraBounds.getCentreX(), auraBounds.getCentreY(),
            juce::Colours::transparentBlack,
            auraBounds.getX(), auraBounds.getY(),
            true);

        g.setGradientFill (auraGrad);
        g.fillEllipse (auraBounds);
        
        // Inner tight glow
        g.setColour(modeAura.withAlpha(0.15f * glowAmount));
        g.fillEllipse(auraBounds.reduced(auraBounds.getWidth() * 0.15f));
        
        g.restoreState();
    }

    if (elchImage.isValid())
    {
        g.setOpacity (1.0f);
        auto imgArea = bounds.reduced (1.0f);
        g.drawImageWithin (elchImage,
                           (int) imgArea.getX(), (int) imgArea.getY(),
                           (int) imgArea.getWidth(), (int) imgArea.getHeight(),
                           juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        
        // --- ANTLER VISUALIZER ---
        // We draw small spectrum-driven glows on the antlers
        if (!currentSpectrum.empty()) {
            const float centerX = bounds.getCentreX();
            const float centerY = bounds.getCentreY();
            const float logoSize = juce::jmin(bounds.getWidth(), bounds.getHeight());
            
            auto drawAntlerGlow = [&](float xOff, float yOff, int bin) {
                float mag = currentSpectrum[(size_t)juce::jlimit(0, (int)currentSpectrum.size()-1, bin)];
                float val = juce::jlimit(0.0f, 1.0f, juce::jmap(mag, -70.0f, -10.0f, 0.0f, 1.0f));
                
                if (val > 0.1f) {
                    g.setColour(modeAura.withAlpha(val * 0.4f));
                    g.fillEllipse(centerX + xOff * logoSize - 8.0f, centerY + yOff * logoSize - 8.0f, 16.0f, 16.0f);
                    g.setColour(juce::Colours::white.withAlpha(val * 0.6f));
                    g.fillEllipse(centerX + xOff * logoSize - 3.0f, centerY + yOff * logoSize - 3.0f, 6.0f, 6.0f);
                }
            };
            
            // Antler mapping (relative to center)
            drawAntlerGlow(-0.25f, -0.35f, 5);  // Left Low
            drawAntlerGlow(-0.35f, -0.25f, 15); // Left Mid
            drawAntlerGlow(0.25f, -0.35f, 8);   // Right Low
            drawAntlerGlow(0.35f, -0.25f, 20);  // Right Mid
        }
    }
}
