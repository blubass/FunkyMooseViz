#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>

class ElchComponent : public juce::Component
{
public:
    enum class VizMode
    {
        LR,
        MS
    };

    ElchComponent() = default;
    ~ElchComponent() override = default;

    void setElchImage (juce::Image img)
    {
        elchImage = std::move (img);
        repaint();
    }

    void setBackgroundColor (juce::Colour bg)
    {
        bgColor = bg;
        repaint();
    }

    void setVizMode (VizMode newMode);
    void setVizSignal (float rms, float peak);

    void paint (juce::Graphics& g) override;

private:
    juce::Image elchImage;
    juce::Colour bgColor { juce::Colours::transparentBlack };

    VizMode vizMode { VizMode::LR };

    float glowAmount = 0.0f;
    float eyeFlash   = 0.0f;
    float inputLevel = 0.0f;
    float peakLevel  = 0.0f;

    juce::Colour eyeGlowColourL { juce::Colour::fromRGB (80, 220, 255) };
    juce::Colour eyeGlowColourR { juce::Colour::fromRGB (255, 170, 60) };

    juce::Colour glowNormal { juce::Colour::fromRGB (70, 210, 255) };
    juce::Colour glowPunch  { juce::Colour::fromRGB (150, 235, 255) };
};
