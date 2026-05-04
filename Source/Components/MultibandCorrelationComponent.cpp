#include "MultibandCorrelationComponent.h"

void MultibandCorrelationComponent::setData(const std::vector<float>& newCorrelation) {
    correlation = newCorrelation;
    repaint();
}

void MultibandCorrelationComponent::paint(juce::Graphics& g) {
    auto area = getLocalBounds().toFloat();
    
    // Background TRON
    g.setColour(juce::Colour::fromRGB(4, 5, 8));
    g.fillRoundedRectangle(area, 2.0f);
    
    // Outer Bevel TRON
    g.setColour(juce::Colour::fromRGBA(0, 255, 255, 60));
    g.drawRoundedRectangle(area, 2.0f, 1.0f);

    auto inner = area.reduced(2.0f);
    float midY = inner.getY() + inner.getHeight() * 0.5f;

    // Center line (0 Correlation)
    g.setColour(juce::Colour::fromRGBA(0, 255, 255, 40));
    g.drawHorizontalLine((int)midY, inner.getX(), inner.getRight());

    if (correlation.empty()) return;

    const int numPoints = (int)correlation.size();
    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;
    const float sampleRate = 44100.0f; // Simplified

    juce::Path p;
    bool first = true;

    for (int i = 1; i < numPoints; ++i) {
        float freq = (float)i * (sampleRate / 2.0f) / (float)numPoints;
        if (freq < minFreq) continue;
        if (freq > maxFreq) break;

        float xNorm = std::log10(freq / minFreq) / std::log10(maxFreq / minFreq);
        float x = inner.getX() + xNorm * inner.getWidth();
        
        // Correlation is -1..1
        float val = juce::jlimit(-1.0f, 1.0f, correlation[(size_t)i]);
        float y = midY - (val * inner.getHeight() * 0.45f);

        if (first) {
            p.startNewSubPath(x, y);
            first = false;
        } else {
            p.lineTo(x, y);
        }
    }

    // Gradient based on correlation (Cyan for positive, Orange for negative)
    g.setColour(juce::Colour::fromRGBA(0, 255, 255, 100));
    g.strokePath(p, juce::PathStrokeType(4.0f));
    g.setColour(juce::Colour::fromRGB(0, 255, 255));
    g.strokePath(p, juce::PathStrokeType(1.2f));
    
    // Fill areas
    juce::Path posPath = p;
    posPath.lineTo(inner.getRight(), midY);
    posPath.lineTo(inner.getX(), midY);
    posPath.closeSubPath();
    
    g.setColour(juce::Colour::fromRGBA(0, 255, 255, 40));
    g.fillPath(posPath);

    // Label
    g.setColour(juce::Colour::fromRGBA(0, 255, 255, 200));
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.drawText("MULTIBAND CORRELATION", area.reduced(5), juce::Justification::topLeft, false);
    
    g.setFont(juce::FontOptions(8.0f));
    g.drawText("+1", inner.withHeight(10).withY(inner.getY()), juce::Justification::topRight, false);
    g.drawText("-1", inner.withHeight(10).withY(inner.getBottom() - 10), juce::Justification::bottomRight, false);
}
