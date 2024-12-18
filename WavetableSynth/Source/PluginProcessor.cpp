/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WavetableSynthAudioProcessor::WavetableSynthAudioProcessor() // コンストラクタ定義
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties() // AudioProcessorのコンストラクタ呼出
    #if ! JucePlugin_IsMidiEffect // もし，MIDIエフェクトではなく
        #if ! JucePlugin_IsSynth // シンセでもない
            .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        #endif // シンセだったら
            .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
    #endif
    )
#endif
,parameters(*this, nullptr, juce::Identifier("APVT"),
        {
    std::make_unique<juce::AudioParameterFloat>(ParameterID{"gain", 1},  // ID
                                                        "Gain",  // name
                                                         0.0f,     // min
                                                         1.0f, // max
                                                         0.5f),// default
    std::make_unique<juce::AudioParameterFloat>(ParameterID{"bright",2},  // ID
                                                        "Bright",  // name
                                                         0.0f,     // min
                                                         1.0f, // max
                                                         0.1f),// default
    std::make_unique<juce::AudioParameterFloat>(ParameterID{"warm",3},  // ID
                                                        "Warm",  // name
                                                         0.0f,     // min
                                                         1.0f, // max
                                                         0.0f),// default
    std::make_unique<juce::AudioParameterFloat>(ParameterID{"rich",4},  // ID
                                                        "Rich",  // name
                                                         0.0f,     // min
                                                         1.0f, // max
                                                         0.0f),// default
    std::make_unique<juce::AudioParameterInt>(ParameterID{"shiftPitch",5},  // ID
                                                        "ShiftPitch",  // name
                                                         -127.0f,     // min
                                                         127.0f, // max
                                                         0.0f),// default
        })
{
    //IDを指定してパラメータの紐づけです。
    gainParam = parameters.getRawParameterValue("gain");
    //brightParam = parameters.getRawParameterValue("bright");
    //warmParam = parameters.getRawParameterValue("warm");
    //richParam = parameters.getRawParameterValue("rich");
    shiftPitchParam = parameters.getRawParameterValue("shiftPitch");
    
}


WavetableSynthAudioProcessor::~WavetableSynthAudioProcessor()
{ // ~(チルダ)はデストラクタの意
}

//==============================================================================
const juce::String WavetableSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WavetableSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool WavetableSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool WavetableSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double WavetableSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WavetableSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int WavetableSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WavetableSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String WavetableSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void WavetableSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void WavetableSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    previousGain = *gainParam;
    this->sampleRate = sampleRate;
    synth.prepareToPlay(sampleRate);
}

void WavetableSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WavetableSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void WavetableSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // DBG("processBlock");
    // Smoothing gain changes
    float currentGain = *gainParam;
    int shiftPitch = static_cast<int>(*shiftPitchParam);

    if (juce::approximatelyEqual (currentGain, previousGain))
    {
        buffer.applyGain (currentGain);
    }
    else
    {
        buffer.applyGainRamp (0, buffer.getNumSamples(), previousGain, currentGain);
        previousGain = currentGain;
    }
    //
    for (auto i = 0; i < buffer.getNumChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    synth.processBlock(buffer, midiMessages, shiftPitch);
    buffer.applyGain(*gainParam);
}

//==============================================================================
bool WavetableSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* WavetableSynthAudioProcessor::createEditor()
{
    return new WavetableSynthAudioProcessorEditor (*this, parameters, synth);
}

//==============================================================================
void WavetableSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void WavetableSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavetableSynthAudioProcessor();
}

