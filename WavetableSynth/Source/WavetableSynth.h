#pragma once
#include <JuceHeader.h>

#include "WavetableOscillator.h"

class WavetableSynth
{
public:
    void prepareToPlay(double sampleRate);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void initializeOscillators(std::vector<float> waveTable);
    void setWaveTable(const std::vector<float>& newWaveTable);
    
private:
    static float midiNoteNumberToFrequency(int midiNoteNumber);
    void handleMidiEvent(const juce::MidiMessage& midiMessage);
    void render(juce::AudioBuffer<float>& buffer, int beginSample, int endSample);
    void initializeOscillators();
    
    void checkWaveTableChanged();
    std::vector<float> waveTable;
    std::atomic<bool> waveTableChanged{false};

    std::mutex waveTableMutex;
    std::mutex oscillatorsMutex;

    double sampleRate;
    std::vector<WavetableOscillator> oscillators;
};
