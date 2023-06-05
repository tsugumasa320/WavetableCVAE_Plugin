#pragma once
#include <JuceHeader.h>

#include "WavetableOscillator.h"

class WavetableSynth
{
public:
    void prepareToPlay(double sampleRate);
    void prepareToPlay(double sampleRate, std::vector<float> waveTable);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void initializeOscillators(std::vector<float> waveTable);
    
private:
    static std::vector<float> generateSineWaveTable();
    static float midiNoteNumberToFrequency(int midiNoteNumber);
    void initializeOscillators();
    void handleMidiEvent(const juce::MidiMessage& midiMessage);
    void render(juce::AudioBuffer<float>& buffer, int beginSample, int endSample);

    double sampleRate;
    std::vector<WavetableOscillator> oscillators;
    int count;
};

