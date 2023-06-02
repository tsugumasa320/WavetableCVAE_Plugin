/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VAE4WavetableAudioProcessor::VAE4WavetableAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

VAE4WavetableAudioProcessor::~VAE4WavetableAudioProcessor()
{
}

//==============================================================================
const juce::String VAE4WavetableAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VAE4WavetableAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VAE4WavetableAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VAE4WavetableAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VAE4WavetableAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VAE4WavetableAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VAE4WavetableAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VAE4WavetableAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VAE4WavetableAudioProcessor::getProgramName (int index)
{
    return {};
}

void VAE4WavetableAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VAE4WavetableAudioProcessor::prepareToPlay (double sampleRate, int)
{
    synth.prepareToPlay(sampleRate);
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void VAE4WavetableAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool VAE4WavetableAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    synth.processBlock([&]buffer, [&]midiMessages);
}

void VAE4WavetableAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool VAE4WavetableAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VAE4WavetableAudioProcessor::createEditor()
{
    return new VAE4WavetableAudioProcessorEditor (*this);
}

//==============================================================================
void VAE4WavetableAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void VAE4WavetableAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VAE4WavetableAudioProcessor();
}
