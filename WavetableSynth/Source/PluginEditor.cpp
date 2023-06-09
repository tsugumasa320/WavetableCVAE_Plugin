/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <JuceHeader.h>
#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
WavetableSynthAudioProcessorEditor::WavetableSynthAudioProcessorEditor (WavetableSynthAudioProcessor& p, juce::AudioProcessorValueTreeState& vts, WavetableSynth& s)
    : AudioProcessorEditor (&p), audioProcessor (p),
    valueTreeState(vts), synth(s)
{
    //==============================================================================
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    //==============================================================================

    // Init of UI settings
    
    setSize (800, 600);
    
    //コンポーネントの設定
    // gain slider
    addAndMakeVisible(gainSlider);
    gainAttachment.reset(new SliderAttachment(valueTreeState, "gain", gainSlider));
    
    // bright slider
    addAndMakeVisible(brightSlider);
    brightAttachment.reset(new SliderAttachment(valueTreeState, "bright", brightSlider));
    brightSlider.setSliderStyle(juce::Slider::LinearVertical);
    brightSlider.addListener (this);
    
    // warm slider
    warmAttachment.reset(new SliderAttachment(valueTreeState, "warm", warmSlider));
    warmSlider.setSliderStyle(juce::Slider::LinearVertical);
    addAndMakeVisible(warmSlider);
    warmSlider.addListener (this);
    
    // rich slider
    richAttachment.reset(new SliderAttachment(valueTreeState, "rich", richSlider));
    richSlider.setSliderStyle(juce::Slider::LinearVertical);
    addAndMakeVisible(richSlider);
    richSlider.addListener (this);
    
    // openボタンの実装
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open...");
    openButton.onClick = [this] { openButtonClicked(); };
    // 音声形式の定義
    formatManager.registerBasicFormats();
    
    //==============================================================================
    // Init of wavetable settings
    label = {b, w, r};
    
    std::vector<float> sineWaveTable = generateSineWaveTable();
    synth.prepareToPlay(audioProcessor.getSampleRate(), sineWaveTable);
    //==============================================================================
    // Init of ONNX settings
    auto bundle = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory().getParentDirectory();
    /** UPDATE THESE PARAMS FOR DIFFERENT MODELS!! **/
    auto model_file = bundle.getChildFile ("Resources/model/wavetable_cvae.onnx");
    onnxModel.setup(model_file);
    //==============================================================================

}

WavetableSynthAudioProcessorEditor::~WavetableSynthAudioProcessorEditor()
{
}

//==============================================================================
void WavetableSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    
    // Draw background image
    juce::Image background = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
    juce::Rectangle<float> backgroundBounds(0, 0, getWidth(), getHeight());
    g.drawImage(background, backgroundBounds);
    
    // thumnailBounds
    juce::Rectangle<float> inputThumbnailBounds(25, 400, 220, 150); // initial x, initial y, width, height
    juce::Rectangle<float> outputThumbnailBounds(getWidth()-25-220, 400, 220, 150);
    
    // Draw waveforms
    drawWaveform(g, inputWavetable, inputThumbnailBounds, juce::Colours::blue);
    drawWaveform(g, outputWavetable, outputThumbnailBounds, juce::Colours::red);
    
}

void WavetableSynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    openButton.setBounds(10, getHeight()-30, getWidth() - 20, 20);
    //gainSlider.setBounds(10, 10, 200, 30);
    brightSlider.setBounds(305, 300, 20, 230);
    warmSlider.setBounds(390, 300, 20, 230);
    richSlider.setBounds(475, 300, 20, 230);

}

void WavetableSynthAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    b = brightSlider.getValue();
    w = warmSlider.getValue();
    r = richSlider.getValue();
    
    if (inputWavetable.empty()){
        inputWavetable = generateSineWaveTable();
    }

    WavetableSynthAudioProcessorEditor::inference();
    repaint();
    
    DBG("bright: " << b << ", warm: " << w << ", rich: " << r);
}

//==============================================================================
// audio loader

void WavetableSynthAudioProcessorEditor::openButtonClicked()
{
    // File Chooser to select a WAV file
    chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
                                                   juce::File{},
                                                   "*.wav");
    
    // Setting the flags for the File Chooser
    auto chooserFlags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles;

    // Launching the File Chooser asynchronously
    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
                          {
        
        // Getting the selected file
        auto file = fc.getResult();
        // Resetting the reader
        reader.reset();
        // Creating a reader for the selected file
        reader.reset(formatManager.createReaderFor(file));

        if (reader != nullptr)
        {
            // Creating a new source for the TransportSource
            std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(reader.get(), false));
            
            // Keeping the new source in readerSource
            readerSource.reset(newSource.release());
                        
            // Initializing the buffer size and the buffer
            buffer.setSize(reader->numChannels, WAVETABLE_SIZE);
            
            bool isStereo = (buffer.getNumChannels()>=2)? true:false;
            
            // convert to mono if stereo
            if (isStereo)
            {
                // add the right (1) to the left (0)
                buffer.addFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
            }
            
            // Reading data from the AudioFormatReaderSource
            juce::AudioSourceChannelInfo info(&buffer, 0, WAVETABLE_SIZE);
            readerSource->getNextAudioBlock(info);
            
            // Converting the buffer data into vector
            auto* channelData = buffer.getReadPointer(0); // 1chのみを読み込むため、0を指定します
            inputWavetable.assign(channelData, channelData + buffer.getNumSamples());
            WavetableSynthAudioProcessorEditor::inference();

        }
    });
}

void WavetableSynthAudioProcessorEditor::inference()
{
    // Processing the data through the ONNX model
    outputWavetable = onnxModel.process(inputWavetable, label, WAVETABLE_SIZE);
    // Preparing the synth to play the processed data
    synth.prepareToPlay(audioProcessor.getSampleRate(), outputWavetable);
}

void WavetableSynthAudioProcessorEditor::drawWaveform(juce::Graphics& g, const std::vector<float>& data, juce::Rectangle<float> bounds, juce::Colour colour)
{
    if (data.empty())
    {
        return;
    }

    juce::Path waveform;

    // Start at the first point
    waveform.startNewSubPath(bounds.getX(), juce::jmap(data[0], -1.0f, 1.0f, bounds.getBottom(), bounds.getY()));

    // Add lines for each sample
    for (size_t i = 1; i < data.size(); ++i)
    {
        waveform.lineTo(bounds.getX() + bounds.getWidth() * (i / static_cast<float>(data.size() - 1)),
                        juce::jmap(data[i], -1.0f, 1.0f, bounds.getBottom(), bounds.getY()));
    }

    // Set the color and stroke style of the waveform
    g.setColour(colour);
    g.strokePath(waveform, juce::PathStrokeType(2.0f));
}

std::vector<float> WavetableSynthAudioProcessorEditor::generateSineWaveTable()
{
    constexpr auto WAVETABLE_LENGTH = 600;
    const auto PI = std::atanf(1.f) * 4;
    std::vector<float> sineWaveTable = std::vector<float>(WAVETABLE_LENGTH);

    for (auto i = 0; i < WAVETABLE_LENGTH; ++i)
    {
        sineWaveTable[i] = std::sinf(2 * PI * static_cast<float>(i) / WAVETABLE_LENGTH);
    }

    return sineWaveTable;
}
