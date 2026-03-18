#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "Analysis/FFTProcessor.h"
#include "Analysis/WaveformBuffer.h"
#include "Analysis/LevelMeterSource.h"
#include <algorithm>

class UweVizAudioProcessor : public juce::AudioProcessor
{
public:
    UweVizAudioProcessor();
    ~UweVizAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override {
        juce::MemoryOutputStream stream(destData, true);
        stream.writeBool(analyzeOnly);
        stream.writeInt(displayMode);
        stream.writeBool(frozen);
        stream.writeInt(displayRangeDb);
        stream.writeInt(lastUIWidth);
        stream.writeInt(lastUIHeight);
    }
    
    void setStateInformation (const void* data, int sizeInBytes) override {
        juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
        if (sizeInBytes > 0)
            analyzeOnly = stream.readBool();
        if (!stream.isExhausted())
            displayMode = stream.readInt();
        if (!stream.isExhausted())
            frozen = stream.readBool();
        if (!stream.isExhausted())
            displayRangeDb = stream.readInt();
        if (!stream.isExhausted())
            lastUIWidth = stream.readInt();
        if (!stream.isExhausted())
            lastUIHeight = stream.readInt();
    }

    FFTProcessor& getFFTProcessorLeft()  { return fftProcessorLeft; }
    FFTProcessor& getFFTProcessorRight() { return fftProcessorRight; }
    FFTProcessor& getFFTProcessorMid()   { return fftProcessorMid; }
    FFTProcessor& getFFTProcessorSide()  { return fftProcessorSide; }

    int lastUIWidth = 960;
    int lastUIHeight = 620;

    std::atomic<uint32_t> analysisFrameCounter { 0 };

    WaveformBuffer& getWaveformBuffer()  { return waveformBuffer; }
    LevelMeterSource& getMeterSource()   { return meterSource; }

    void setAnalyzeOnly (bool shouldBeAnalyzeOnly) { analyzeOnly = shouldBeAnalyzeOnly; }
    bool getAnalyzeOnly() const                    { return analyzeOnly; }

    void setDisplayMode (int newMode) { displayMode = newMode; }
    int getDisplayMode() const        { return displayMode; }

    void setFrozen (bool shouldBeFrozen) { frozen = shouldBeFrozen; }
    bool getFrozen() const               { return frozen; }

    void setDisplayRange (int newRange) { displayRangeDb = newRange; }
    int getDisplayRange() const         { return displayRangeDb; }

    /** Frequency calculated in Editor now to avoid FFT read race conditions. */

    static juce::String frequencyToNote(float freq) {
        if (freq < 16.0f) return "---"; // Ignore subsonic/DC
        const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        
        // A4 = 440Hz = MIDI 69
        float midiNote = 12.0f * std::log2(freq / 440.0f) + 69.0f;
        int noteNum = (int)std::round(midiNote);
        
        if (noteNum < 0 || noteNum > 127) return "---";
        
        int octave = (noteNum / 12) - 1;
        return juce::String(notes[noteNum % 12]) + juce::String(octave);
    }

private:
    FFTProcessor fftProcessorLeft;
    FFTProcessor fftProcessorRight;
    FFTProcessor fftProcessorMid;
    FFTProcessor fftProcessorSide;

    WaveformBuffer waveformBuffer;
    LevelMeterSource meterSource;

    std::vector<float> tempMidBuffer;
    std::vector<float> tempSideBuffer;

    std::atomic<bool> analyzeOnly { true };
    std::atomic<int> displayMode { 0 };
    std::atomic<bool> frozen { false };
    std::atomic<int> displayRangeDb { 90 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UweVizAudioProcessor)
};
