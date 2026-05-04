#include "LoudnessHistoryComponent.h"

void LoudnessHistoryComponent::setHistory(const std::vector<float>& newHistory) {
    history = newHistory;
    repaint();
}

void LoudnessHistoryComponent::paint(juce::Graphics& g) {
    auto area = getLocalBounds().toFloat();
    
    // Background TRON
    g.setColour(juce::Colour::fromRGB(4, 5, 8));
    g.fillRoundedRectangle(area, 2.0f);
    
    // Outer Bevel TRON
    g.setColour(juce::Colour::fromRGBA(0, 255, 255, 60));
    g.drawRoundedRectangle(area, 2.0f, 1.0f);

    auto inner = area.reduced(2.0f);
    
    // Grid lines Neon
    g.setColour(juce::Colour::fromRGBA(0, 255, 255, 15));
    for (float db : {-12.0f, -24.0f, -36.0f, -48.0f}) {
        float y = inner.getY() + juce::jmap(db, 0.0f, -60.0f, 0.0f, inner.getHeight());
        g.drawHorizontalLine((int)y, inner.getX(), inner.getRight());
    }

    if (history.size() < 2) return;

    juce::Path p;
    bool first = true;
    
    for (size_t i = 0; i < history.size(); ++i) {
        float x = inner.getX() + (float)i / (float)(history.size() - 1) * inner.getWidth();
        // Loudness is 0..1 (mapped from -60..0)
        float y = inner.getBottom() - history[i] * inner.getHeight();
        
        if (first) {
            p.startNewSubPath(x, y);
            first = false;
        } else {
            p.lineTo(x, y);
        }
    }

    // Fill under the path
    juce::Path fill = p;
    fill.lineTo(inner.getRight(), inner.getBottom());
    fill.lineTo(inner.getX(), inner.getBottom());
    fill.closeSubPath();

    juce::ColourGradient grad(juce::Colour::fromRGBA(0, 255, 255, 100), 0, inner.getY(),
                              juce::Colour::fromRGBA(0, 255, 255, 0), 0, inner.getBottom(), false);
    g.setGradientFill(grad);
    g.fillPath(fill);

    // Stroke Neon Glow
    g.setColour(juce::Colour::fromRGBA(0, 255, 255, 100));
    g.strokePath(p, juce::PathStrokeType(4.0f));
    g.setColour(juce::Colour::fromRGB(0, 255, 255));
    g.strokePath(p, juce::PathStrokeType(1.5f));
    
    // Label
    g.setColour(juce::Colour::fromRGBA(0, 255, 255, 200));
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.drawText("LOUDNESS HISTORY", area.reduced(5), juce::Justification::topLeft, false);
}
