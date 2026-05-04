#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <vector>

class LoudnessHistoryComponent : public juce::Component {
public:
    void setHistory(const std::vector<float>& newHistory);
    void paint(juce::Graphics& g) override;

private:
    std::vector<float> history;
    juce::Colour accentBlue = juce::Colour::fromRGB(88, 174, 219);
};
