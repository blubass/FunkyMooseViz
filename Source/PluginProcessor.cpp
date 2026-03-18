#include "PluginProcessor.h"
#include "PluginEditor.h"

UweVizAudioProcessor::UweVizAudioProcessor()
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
{
}

UweVizAudioProcessor::~UweVizAudioProcessor() = default;

void UweVizAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    fftProcessorLeft.prepare  (sampleRate, samplesPerBlock);
    fftProcessorRight.prepare (sampleRate, samplesPerBlock);
    fftProcessorMid.prepare   (sampleRate, samplesPerBlock);
    fftProcessorSide.prepare  (sampleRate, samplesPerBlock);

    waveformBuffer.prepare (2048);
    meterSource.prepare (sampleRate, samplesPerBlock);

    const auto tempSize = (size_t) juce::jmax (samplesPerBlock, 2048);
    tempMidBuffer.assign (tempSize, 0.0f);
    tempSideBuffer.assign (tempSize, 0.0f);
}

void UweVizAudioProcessor::releaseResources() {}

bool UweVizAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    const auto mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainInputChannelSet() != mainOut)
        return false;
   #endif

    return true;
#endif
}

void UweVizAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    meterSource.processBlock (buffer);

    if ((int) tempMidBuffer.size() < numSamples)
        tempMidBuffer.resize ((size_t) numSamples, 0.0f);

    if ((int) tempSideBuffer.size() < numSamples)
        tempSideBuffer.resize ((size_t) numSamples, 0.0f);

    const float* left  = (numChannels > 0) ? buffer.getReadPointer (0) : nullptr;
    const float* right = (numChannels > 1) ? buffer.getReadPointer (1) : left;

    bool anyNewFFT = false;

    if (left != nullptr)
        if (fftProcessorLeft.pushSamples (left, numSamples)) anyNewFFT = true;

    if (right != nullptr)
        if (fftProcessorRight.pushSamples (right, numSamples)) anyNewFFT = true;

    if (left != nullptr && right != nullptr)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const float l = left[i];
            const float r = right[i];
            tempMidBuffer[(size_t) i]  = 0.5f * (l + r);
            tempSideBuffer[(size_t) i] = 0.5f * (l - r);
        }

        if (fftProcessorMid.pushSamples  (tempMidBuffer.data(), numSamples)) anyNewFFT = true;
        if (fftProcessorSide.pushSamples (tempSideBuffer.data(), numSamples)) anyNewFFT = true;
    }

    if (anyNewFFT)
        analysisFrameCounter.fetch_add (1, std::memory_order_release);

    if (left != nullptr)
        waveformBuffer.pushStereoSamples (left, right, numSamples);

    if (analyzeOnly)
    {
        for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
            buffer.clear (ch, 0, numSamples);
    }
    else
    {
        for (int ch = numChannels; ch < getTotalNumOutputChannels(); ++ch)
            buffer.clear (ch, 0, numSamples);
    }
}

juce::AudioProcessorEditor* UweVizAudioProcessor::createEditor()
{
    return new UweVizAudioProcessorEditor (*this);
}


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UweVizAudioProcessor();
}
