#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "Analysis/FFTProcessor.h"
#include "Analysis/WaveformBuffer.h"
#include "Analysis/LevelMeterSource.h"
#include "Analysis/StereoFFTAnalyzer.h"
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
        stream.writeInt(spectrumWeightingMode);
        stream.writeBool(spectrumCrestMode);
        stream.writeInt(spectrumColorScheme);
        stream.writeBool(spectrumAverageTrace);
        stream.writeInt(spectrumSmoothingMode);
        stream.writeBool(spectrumFrequencyMarkers);
        stream.writeBool(spectrumNoteGrid);
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
        if (!stream.isExhausted())
            spectrumWeightingMode = stream.readInt();
        if (!stream.isExhausted())
            spectrumCrestMode = stream.readBool();
        if (!stream.isExhausted())
            spectrumColorScheme = stream.readInt();
        if (!stream.isExhausted())
            spectrumAverageTrace = stream.readBool();
        if (!stream.isExhausted())
            spectrumSmoothingMode = stream.readInt();
        if (!stream.isExhausted())
            spectrumFrequencyMarkers = stream.readBool();
        if (!stream.isExhausted())
            spectrumNoteGrid = stream.readBool();
    }

    FFTProcessor& getFFTProcessorLeft()  { return fftProcessorLeft; }
    FFTProcessor& getFFTProcessorRight() { return fftProcessorRight; }
    FFTProcessor& getFFTProcessorMid()   { return fftProcessorMid; }
    FFTProcessor& getFFTProcessorSide()  { return fftProcessorSide; }
    FFTProcessor& getFFTProcessorSCLeft() { return fftProcessorSCLeft; }
    FFTProcessor& getFFTProcessorSCRight() { return fftProcessorSCRight; }
    StereoFFTAnalyzer& getStereoAnalyzer() { return stereoAnalyzer; }

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

    void setSpectrumWeightingMode (int newMode) { spectrumWeightingMode = newMode; }
    int getSpectrumWeightingMode() const        { return spectrumWeightingMode; }

    void setSpectrumCrestMode (bool shouldBeEnabled) { spectrumCrestMode = shouldBeEnabled; }
    bool getSpectrumCrestMode() const                { return spectrumCrestMode; }

    void setSpectrumColorScheme (int newScheme)      { spectrumColorScheme = newScheme; }
    int getSpectrumColorScheme() const               { return spectrumColorScheme; }

    void setSpectrumAverageTrace (bool shouldBeEnabled) { spectrumAverageTrace = shouldBeEnabled; }
    bool getSpectrumAverageTrace() const                { return spectrumAverageTrace; }

    void setSpectrumSmoothingMode (int newMode)      { spectrumSmoothingMode = newMode; }
    int getSpectrumSmoothingMode() const             { return spectrumSmoothingMode; }

    void setSpectrumFrequencyMarkers (bool enabled)  { spectrumFrequencyMarkers = enabled; }
    bool getSpectrumFrequencyMarkers() const         { return spectrumFrequencyMarkers; }

    void setSpectrumNoteGrid (bool enabled)          { spectrumNoteGrid = enabled; }
    bool getSpectrumNoteGrid() const                 { return spectrumNoteGrid; }

    void setToneGeneratorEnabled (bool enabled)         { toneGeneratorEnabled = enabled; }
    bool isToneGeneratorEnabled() const                 { return toneGeneratorEnabled; }
    void setToneGeneratorFrequency (float freq)         { toneOsc.setFrequency(freq); }

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

    void loadFile (const juce::File& file);
    void playFile();
    void pauseFile();
    void stopFile();
    bool isFilePlaying() const { return transportSource.isPlaying(); }
    double getFilePosition() const { return transportSource.getCurrentPosition(); }
    double getFileLength() const { return transportSource.getLengthInSeconds(); }
    void setFilePosition (double seconds) { transportSource.setPosition(seconds); }

private:
    FFTProcessor fftProcessorLeft;
    FFTProcessor fftProcessorRight;
    FFTProcessor fftProcessorMid;
    FFTProcessor fftProcessorSide;
    FFTProcessor fftProcessorSCLeft;
    FFTProcessor fftProcessorSCRight;
    StereoFFTAnalyzer stereoAnalyzer;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    WaveformBuffer waveformBuffer;
    LevelMeterSource meterSource;

    std::vector<float> tempMidBuffer;
    std::vector<float> tempSideBuffer;

    std::atomic<bool> analyzeOnly { false };
    std::atomic<int> displayMode { 0 };
    std::atomic<bool> frozen { false };
    std::atomic<int> displayRangeDb { 90 };
    std::atomic<int> spectrumWeightingMode { 0 };
    std::atomic<bool> spectrumCrestMode { false };
    std::atomic<int> spectrumColorScheme { 0 };
    std::atomic<bool> spectrumAverageTrace { false };
    std::atomic<int> spectrumSmoothingMode { 0 };
    std::atomic<bool> spectrumFrequencyMarkers { true };
    std::atomic<bool> spectrumNoteGrid { false };

    juce::dsp::Oscillator<float> toneOsc;
    std::atomic<bool> toneGeneratorEnabled { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UweVizAudioProcessor)
};
