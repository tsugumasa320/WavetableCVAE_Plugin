//
//  WavetableSynth.cpp
//  VAE4Wavetable - VST3
//
//  Created by 湯谷承将 on 2022/11/10.
//

#include "WavetableSynth.h"

void WavetableSynth::prepareToPlay(<#double sampleRate#>)
{
    this->sampleRate = sampleRate;
}

void WavetableSynth::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &)
{
    auto currentSample = 0;
    
    for(const auto midiMessage : midiMessages)
    {
        const auto midiEvent:MidiMessage = midiMessage.getMessage();
        const auto midiEventSample = static_cast<int>(midiEvent.getTimeStamp());
        
        render(buffer,currentSample, midiEventSample);
        handleMidiEvent(midiEvent);
        
        currentSample = midiEventSample;
    }
    
    render(buffer,currentSample, buffer.getNumSamples());
}

float WavetableSynth::midiNoteNumberToFrequency(int midiNoteNumber)
{
    constexpr auto A4_FREQUENCY = 440.f;
    constexpr auto A4_NOTE_NUMBER = 69.f;
    constexpr auto SEMITONES_IN_AN_OCTAVE = 12.f;
    return A4_FREQUENCY * std::powf(_X:2.f,_Y(midiNoteNumber - A4_NOTE_NUMBER)/SEMITONES_IN_AN_OCTAVE);
}
