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
class WavetableSynthAudioProcessorEditor  : public juce::AudioProcessorEditor,private juce::Slider::Listener
{
public:
    WavetableSynthAudioProcessorEditor (WavetableSynthAudioProcessor&, WavetableSynth&);
    ~WavetableSynthAudioProcessorEditor() override;
    //==============================================================================
    
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();
    void changeListenerCallback(juce::ChangeBroadcaster* source);
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    //==============================================================================
    std::vector<float> samples;
    float bright;
    float warm;
    float rich;
    int WAVETABLE_SIZE;
    //==============================================================================

    
private:
    
    //==============================================================================
    void sliderValueChanged (juce::Slider* slider) override; //←デフォルトの「リスナー関数」を使います。
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    WavetableSynthAudioProcessor& audioProcessor;
    juce::Slider brightSlider;
    juce::Slider warmSlider;
    juce::Slider richSlider;
    //==============================================================================
    
    juce::TextButton openButton;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    //==============================================================================
    juce::AudioThumbnailCache inputThumbnailCache;
    juce::AudioThumbnail inputThumbnail;
    juce::AudioThumbnailCache outputThumbnailCache;
    juce::AudioThumbnail outputThumbnail;
    //==============================================================================
    // Paint-related definitions
    
    void openButtonClicked();
    void thumbnailChanged();
    
    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintIfFileLoaded
    (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, juce::AudioThumbnail& thumbnail, juce::Colour color);
    static std::vector<float> generateSineWaveTable();
    //==============================================================================
    
    WavetableSynth& synth;
    juce::AudioBuffer<float> buffer;
    std::unique_ptr<juce::AudioFormatReader> reader;
    
    OnnxModel onnxModel;
    void applyModel(AudioBuffer<float>& buffer);
    std::vector<float> label;
    void inference();
    juce::AudioBuffer<float>* outputThumb;
    //==============================================================================
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableSynthAudioProcessorEditor)
};
