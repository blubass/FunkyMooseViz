#include "PluginProcessor.h"
#include "PluginEditor.h"

UweVizAudioProcessor::UweVizAudioProcessor()
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
        .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)
#endif
    )
{
    toneOsc.initialise ([](float x) { return std::sin (x); });
    formatManager.registerBasicFormats();
}

UweVizAudioProcessor::~UweVizAudioProcessor() = default;

void UweVizAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    fftProcessorLeft.prepare  (sampleRate, samplesPerBlock);
    fftProcessorRight.prepare (sampleRate, samplesPerBlock);
    fftProcessorMid.prepare   (sampleRate, samplesPerBlock);
    fftProcessorSide.prepare  (sampleRate, samplesPerBlock);
    fftProcessorSCLeft.prepare (sampleRate, samplesPerBlock);
    fftProcessorSCRight.prepare (sampleRate, samplesPerBlock);
    stereoAnalyzer.prepare (sampleRate, samplesPerBlock);

    waveformBuffer.prepare (2048);
    meterSource.prepare (sampleRate, samplesPerBlock);

    const auto tempSize = (size_t) juce::jmax (samplesPerBlock, 2048);
    tempMidBuffer.assign (tempSize, 0.0f);
    tempSideBuffer.assign (tempSize, 0.0f);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
    spec.numChannels = 1;
    toneOsc.prepare (spec);
    toneOsc.setFrequency (440.0f);
    
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
}

void UweVizAudioProcessor::releaseResources() {}

bool UweVizAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // Visualizer: Akzeptiere Mono/Stereo-Input, Output-Layout darf abweichen
    const auto mainIn  = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();
    const auto scIn    = layouts.getChannelSet (true, 1);

    if (mainIn != juce::AudioChannelSet::mono() && mainIn != juce::AudioChannelSet::stereo())
        return false;
    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;
    if (scIn != juce::AudioChannelSet::disabled() && scIn != juce::AudioChannelSet::mono() && scIn != juce::AudioChannelSet::stereo())
        return false;

    return true;
#endif
}

void UweVizAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (transportSource.isPlaying())
    {
        juce::AudioSourceChannelInfo info(&buffer, 0, numSamples);
        transportSource.getNextAudioBlock(info);
    }

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

    // Process Sidechain if available
    auto scBuffer = getBusBuffer (buffer, true, 1);
    if (scBuffer.getNumChannels() > 0)
    {
        const int scChannels = scBuffer.getNumChannels();
        if (scChannels > 0)
        {
            const float* scL = scBuffer.getReadPointer(0);
            const float* scR = (scChannels > 1) ? scBuffer.getReadPointer(1) : scL;
            
            if (fftProcessorSCLeft.pushSamples(scL, numSamples)) anyNewFFT = true;
            if (fftProcessorSCRight.pushSamples(scR, numSamples)) anyNewFFT = true;
        }
    }

    if (anyNewFFT)
        analysisFrameCounter.fetch_add (1, std::memory_order_release);

    if (left != nullptr)
    {
        waveformBuffer.pushStereoSamples (left, right, numSamples);
        stereoAnalyzer.pushSamples (left, right, numSamples);
    }

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

    if (toneGeneratorEnabled)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        
        // Simple manual mixing for tone generator if it's supposed to be an overlay
        // Or just replace if in analyzeOnly mode? Let's overlay at -12dB
        for (int s = 0; s < numSamples; ++s)
        {
            float sample = toneOsc.processSample(0.0f) * 0.25f; // -12dB
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.addSample(ch, s, sample);
        }
    }
}

void UweVizAudioProcessor::loadFile (const juce::File& file)
{
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr)
    {
        readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
    }
}

void UweVizAudioProcessor::playFile()  { transportSource.start(); }
void UweVizAudioProcessor::pauseFile() { transportSource.stop(); }
void UweVizAudioProcessor::stopFile()  { transportSource.stop(); transportSource.setPosition(0); }

juce::AudioProcessorEditor* UweVizAudioProcessor::createEditor()
{
    return new UweVizAudioProcessorEditor (*this);
}


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UweVizAudioProcessor();
}
