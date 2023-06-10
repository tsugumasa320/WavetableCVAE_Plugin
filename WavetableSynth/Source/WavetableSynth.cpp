#include "WavetableSynth.h"

void WavetableSynth::initializeOscillators()
{
    if (oscillatorsMutex.try_lock())
    {
        DBG("initOSC");
        constexpr auto OSCILLATOR_COUNT = 128;
        oscillators.clear();
        
        for (auto i = 0; i < OSCILLATOR_COUNT; ++i)
        {
            oscillators.emplace_back(waveTable, sampleRate);
        }
        // Unlock the mutex
        oscillatorsMutex.unlock();
    }
}

// TODO:あとで整理する
void WavetableSynth::prepareToPlay(double sampleRate)
{
    this->sampleRate = sampleRate;
}

void WavetableSynth::processBlock(juce::AudioBuffer<float>& buffer, 
                                  juce::MidiBuffer& midiMessages)
// PluginProcessorからのエントリーポイント
{
    //DBG("WavetableSynth::processBlock");
    auto currentSample = 0;

    checkWaveTableChanged();
    oscillatorsMutex.lock();

    for (const auto midiMetadata : midiMessages)
    {
        const auto message = midiMetadata.getMessage();
        const auto messagePosition = static_cast<int>(message.getTimeStamp());
        // midiMessageがあればここでbufferにデータを代入
        render(buffer, currentSample, messagePosition); //buffer, beginSample, endSample
        currentSample = messagePosition;
        DBG("messagePosition: " << messagePosition);
        handleMidiEvent(message);
    }
    oscillatorsMutex.unlock();

    //DBG("buffer.getNumSamples(): " << buffer.getNumSamples());
    //DBG("currentSample" << currentSample);
    render(buffer, currentSample, buffer.getNumSamples());
}

void WavetableSynth::setWaveTable(const std::vector<float>& newWaveTable)
{
    // Acquire the lock before updating the waveTable
    const std::lock_guard<std::mutex> lock(waveTableMutex);
    waveTable = newWaveTable;
    waveTableChanged.store(true);
}

void WavetableSynth::checkWaveTableChanged()
{
    // Check if the waveTableChanged flag is true
    if (waveTableChanged.load())
    {
        initializeOscillators();
        waveTableChanged.store(false);  // Reset the flag to false
    }
}

float WavetableSynth::midiNoteNumberToFrequency(const int midiNoteNumber)
{
    constexpr auto A4_FREQUENCY = 440.f;
    constexpr auto A4_NOTE_NUMBER = 69.f;
    constexpr auto NOTES_IN_AN_OCTAVE = 12.f;
    return A4_FREQUENCY * std::powf(2, (static_cast<float>(midiNoteNumber) - A4_NOTE_NUMBER) / NOTES_IN_AN_OCTAVE);
}

void WavetableSynth::handleMidiEvent(const juce::MidiMessage& midiMessage)
{
    if (midiMessage.isNoteOn())
    {
        //initializeOscillators(); wavetableをglobalにセットしてから
        const auto oscillatorId = midiMessage.getNoteNumber();
        const auto frequency = midiNoteNumberToFrequency(oscillatorId);
        oscillators[oscillatorId].setFrequency(frequency);
        DBG("note_num: " << oscillatorId << " freq: " << frequency);
    }
    else if (midiMessage.isNoteOff())
    {
        const auto oscillatorId = midiMessage.getNoteNumber();
        oscillators[oscillatorId].stop();
        // DBG(oscillatorId << " is noteoff");
    }
    else if (midiMessage.isAllNotesOff())
    {
        for (auto& oscillator : oscillators)
        {
            oscillator.stop();
        }
    }
}

void WavetableSynth::render(juce::AudioBuffer<float>& buffer, int beginSample, int endSample)
{
    auto* firstChannel = buffer.getWritePointer(0);
    
    for (auto& oscillator : oscillators)
    {
        if (oscillator.isPlaying())
        {
            for (auto sample = beginSample; sample < endSample; ++sample)
            {
                firstChannel[sample] += oscillator.getSample();
                //DBG("osc.isPlaying()");
            }
        }
    }

    for (int channel = 1; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        std::copy(firstChannel + beginSample, firstChannel + endSample, channelData + beginSample);
    }
}

