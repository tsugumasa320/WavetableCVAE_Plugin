/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class WavetableSynthAudioProcessorEditor  : public juce::AudioProcessorEditor,private juce::Slider::Listener
{
public:
    WavetableSynthAudioProcessorEditor (WavetableSynthAudioProcessor&);
    ~WavetableSynthAudioProcessorEditor() override;
    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();
    void changeListenerCallback(juce::ChangeBroadcaster* source);
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    std::vector<float> samples;
    

private:
    void sliderValueChanged (juce::Slider* slider) override; //←デフォルトの「リスナー関数」を使います。
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    WavetableSynthAudioProcessor& audioProcessor;
    juce::Slider brightSlider;
    juce::Label brightLabel;
    juce::Slider warmSlider;
    juce::Label warmLabel;
    juce::Slider richSlider;
    juce::Label richLabel;
    
    juce::Slider midiVolume; //この一行を追加します。

    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    juce::TextButton openButton;

    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    void openButtonClicked();
    void thumbnailChanged();

    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);

    WavetableSynth synth;
    int bufferSize;
    juce::AudioBuffer<float> buffer;
    std::unique_ptr<juce::AudioFormatReader> reader;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableSynthAudioProcessorEditor)
};
