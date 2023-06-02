//
//  WavetableSynth.hpp
//  VAE4Wavetable - VST3
//
//  Created by 湯谷承将 on 2022/11/10.
//

#pragma once
#include "JuceHeader.h"

class WavetableSynth
{
public:
    void prepareToPlay(double sampleRate);
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&);
    
private:
    void handleMidiEvent(const juce::MidiMessage& midiEvent)
    float midiNoteNumberToFrequency(int midiNoteNumber);
    
    double sampleRate;
    {
        if (midiEvent.isNoteOn())
        {
            const auto oscillaorId:int = midiEvent.getNoteNumber();
            const auto frequency = midiNoteNumberToFrequency(oscillaorId);
            oscillaors[oscillaorId].setFrequency(frequency);
        }
        else if (midiEvent.isNoteOff())
        {
            
        }
        else if (midiEvent.isAllNotesOff())
        {
            
        }
    }
    
    double sampleRate;
}


//#ifndef WavetableSynth_h
//#define WavetableSynth_h

//#include <stdio.h>

//#endif /* WavetableSynth_hpp */
