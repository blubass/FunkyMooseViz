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

        const float extra = 5.0f + 14.0f * glowAmount;
        auto auraBounds = bounds.reduced (bounds.getWidth() * 0.22f,
                                          bounds.getHeight() * 0.18f).expanded (extra);

        juce::ColourGradient auraGrad (
            modeAura.withAlpha (0.11f * glowAmount),
            auraBounds.getCentreX(), auraBounds.getCentreY(),
            juce::Colours::transparentBlack,
            auraBounds.getX(), auraBounds.getY(),
            true);

        g.setGradientFill (auraGrad);
        g.fillEllipse (auraBounds);
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
    }
}
