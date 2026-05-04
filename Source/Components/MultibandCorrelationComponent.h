#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <vector>

class MultibandCorrelationComponent : public juce::Component {
public:
    void setData(const std::vector<float>& correlation);
    void paint(juce::Graphics& g) override;

private:
    std::vector<float> correlation;
    juce::Colour accentCyan = juce::Colour::fromRGB(88, 174, 219);
    juce::Colour accentOrange = juce::Colours::orange;
};
