#pragma once
#include <vector>

class WavetableOscillator
{
public:
    WavetableOscillator(std::vector<float> waveTable, double sampleRate);
    WavetableOscillator(const WavetableOscillator&) = delete;
    WavetableOscillator& operator=(const WavetableOscillator&) = delete;
    WavetableOscillator(WavetableOscillator&&) = default;
    WavetableOscillator& operator=(WavetableOscillator&&) = default;

    float getSample();
    void setFrequency(float frequency);
    void stop();
    void setWaveTable(std::vector<float> waveTable);
    bool isPlaying() const;
    std::vector<float> waveTable;
    double sampleRate;

private:
    float interpolateLinearly() const;

    float index = 0.f;
    float indexIncrement = 0.f;
};
