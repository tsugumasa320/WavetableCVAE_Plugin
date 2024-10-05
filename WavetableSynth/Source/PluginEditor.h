/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "OnnxModel.h"

//==============================================================================
/**
*/
class WavetableSynthAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Slider::Listener
{
public:
    WavetableSynthAudioProcessorEditor (WavetableSynthAudioProcessor&, juce::AudioProcessorValueTreeState& vts, WavetableSynth&);
    ~WavetableSynthAudioProcessorEditor() override;
    //==============================================================================
    
    WavetableSynthAudioProcessorEditor (WavetableSynthAudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();
    void changeListenerCallback(juce::ChangeBroadcaster* source);
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    //==============================================================================
    std::vector<float> inputWavetable;
    std::vector<float> outputWavetable;
    float b;
    float w;
    float r;
    const int WAVETABLE_SIZE=600;
    //==============================================================================
    
private:
    
    //==============================================================================
    void sliderValueChanged (juce::Slider* slider) override; //←デフォルトの「リスナー関数」を使います。
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    WavetableSynthAudioProcessor& audioProcessor;
    //==============================================================================
    // UI settings
    juce::TextButton openButton;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::Slider gainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    juce::Slider brightSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> brightAttachment;
    juce::Slider warmSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> warmAttachment;
    juce::Slider richSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> richAttachment;
    
    //==============================================================================
    // Paint-related definitions
    void openButtonClicked();
    static std::vector<float> generateSineWaveTable();
    //==============================================================================
    
    WavetableSynth& synth;
    juce::AudioBuffer<float> buffer;
    std::unique_ptr<juce::AudioFormatReader> reader;
    
    OnnxModel onnxModel;
    void applyModel(AudioBuffer<float>& buffer);
    std::vector<float> label;
    void inference();
    void drawWaveform(juce::Graphics& g, const std::vector<float>& data, juce::Rectangle<float> bounds, juce::Colour colour);
    //==============================================================================
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableSynthAudioProcessorEditor)
};
