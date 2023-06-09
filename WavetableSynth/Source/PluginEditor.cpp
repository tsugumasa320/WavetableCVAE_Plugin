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
WavetableSynthAudioProcessorEditor::WavetableSynthAudioProcessorEditor (WavetableSynthAudioProcessor& p, WavetableSynth& s)
    : AudioProcessorEditor (&p), audioProcessor (p),
    inputThumbnailCache(1), inputThumbnail(600, formatManager, inputThumbnailCache),
    outputThumbnailCache(1), outputThumbnail(600, formatManager, outputThumbnailCache),
    synth(s)
{
    //==============================================================================
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    //==============================================================================

    // Init of UI settings
    
    setSize (800, 600);
        
    brightSlider.setSliderStyle(juce::Slider::LinearVertical);
    brightSlider.setRange(0.0, 1.0);
    brightSlider.setBounds(300, 300, 20, 230); // スライダーの x座標 y座標 横幅 縦幅 を指定
    
    warmSlider.setSliderStyle(juce::Slider::LinearVertical);
    warmSlider.setRange(0.0, 1.0);
    warmSlider.setBounds(390, 300, 20, 230);
    
    richSlider.setSliderStyle(juce::Slider::LinearVertical);
    richSlider.setRange(0.0, 1.0);
    richSlider.setBounds(480, 300, 20, 230);
    
    // add the listener to the slider
    brightSlider.addListener (this);
    warmSlider.addListener (this);
    richSlider.addListener (this);
    
    //スライダーオブジェクトを可視化
    addAndMakeVisible(&brightSlider);
    addAndMakeVisible(&warmSlider);
    addAndMakeVisible(&richSlider);
    
    // openボタンの実装
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open...");
    openButton.onClick = [this] { openButtonClicked(); };
    // 音声形式の定義
    formatManager.registerBasicFormats();
    
    //==============================================================================
    // Init of wavetable settings
    WAVETABLE_SIZE = 600;
    label = {bright, warm, rich};
    
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
    delete outputThumb;
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
    juce::Rectangle<int> inputThumbnailBounds(25, 400, 220, 150); // initial x, initial y, width, height
    juce::Rectangle<int> outputThumbnailBounds(getWidth()-25-220, 400, 220, 150);
    
    // Draw waveforms
    if (samples.empty()){
        paintIfNoFileLoaded(g, inputThumbnailBounds);
        paintIfNoFileLoaded(g, outputThumbnailBounds);
    }
    else{
        paintIfFileLoaded(g, inputThumbnailBounds, inputThumbnail, juce::Colours::blue);
        paintIfFileLoaded(g, outputThumbnailBounds, outputThumbnail, juce::Colours::red);
    }
}

void WavetableSynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    openButton.setBounds(10, getHeight()-30, getWidth() - 20, 20);
}

void WavetableSynthAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    
    bright = brightSlider.getValue();
    warm = warmSlider.getValue();
    rich = richSlider.getValue();
    
    if (samples.empty()){
        samples = generateSineWaveTable();
    }

    WavetableSynthAudioProcessorEditor::inference();
    //repaint();
    
    DBG("bright: " << bright << ", warm: " << warm << ", rich: " << rich);
}

//==============================================================================
// audio loader


void WavetableSynthAudioProcessorEditor::thumbnailChanged()
{
    repaint();
}

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
            
            // Setting the source for the thumbnail
            inputThumbnail.setSource(new juce::FileInputSource(file));
            // Keeping the new source in readerSource
            readerSource.reset(newSource.release());
                        
            // Initializing the buffer size and the buffer
            buffer.setSize(reader->numChannels, WAVETABLE_SIZE);
            
            // Reading data from the AudioFormatReaderSource
            juce::AudioSourceChannelInfo info(&buffer, 0, WAVETABLE_SIZE);
            readerSource->getNextAudioBlock(info);
            
            // Converting the buffer data into vector
            auto* channelData = buffer.getReadPointer(0); // 1chのみを読み込むため、0を指定します
            samples.assign(channelData, channelData + buffer.getNumSamples());
            WavetableSynthAudioProcessorEditor::inference();

        }
    });
}

void WavetableSynthAudioProcessorEditor::inference()
{
    // Creating a vector to store the outputs
    std::vector<float> ort_outputs(WAVETABLE_SIZE);
    // Processing the data through the ONNX model
    ort_outputs = onnxModel.process(samples, label, WAVETABLE_SIZE);
    // Creating a new AudioBuffer from the output vector
    juce::AudioBuffer<float>* outputThumb = new juce::AudioBuffer<float>(1, WAVETABLE_SIZE);

    // Copy data from the vector to the AudioBuffer
    for (int i = 0; i < ort_outputs.size(); ++i)
    {
        outputThumb->setSample(0, i, ort_outputs[i]);
    }
    // Setting the source for the outputThumbnail
    outputThumbnail.setSource(outputThumb, audioProcessor.getSampleRate(), 0); // 0で良いのか？
    // Preparing the synth to play the processed data
    synth.prepareToPlay(audioProcessor.getSampleRate(), ort_outputs);
}

/*
void WavetableSynthAudioProcessorEditor::inference()
{
    // Creating a vector to store the outputs
    std::vector<float> ort_outputs(WAVETABLE_SIZE);
    // Processing the data through the ONNX model
    ort_outputs = onnxModel.process(samples, label);
    
    // Creating a new AudioBuffer from the output vector using a smart pointer
    auto outputThumb = std::make_unique<juce::AudioBuffer<float>>(1, WAVETABLE_SIZE);

    // Copy data from the vector to the AudioBuffer
    for (int i = 0; i < ort_outputs.size(); ++i)
    {
        outputThumb->setSample(0, i, ort_outputs[i]);
    }
    
    // Setting the source for the outputThumbnail
    // Note: AudioThumbnail::setSource expects a raw pointer, so we use get() to retrieve it from the smart pointer.
    outputThumbnail.setSource(outputThumb.get(), audioProcessor.getSampleRate(), 0);
    
    // Preparing the synth to play the processed data
    synth.prepareToPlay(audioProcessor.getSampleRate(), ort_outputs);
}
 */


void WavetableSynthAudioProcessorEditor::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::white);
    g.drawFittedText("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
}

void WavetableSynthAudioProcessorEditor::paintIfFileLoaded
 (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, juce::AudioThumbnail& thumbnail, juce::Colour color)
{
    g.setColour(juce::Colours::white);
    g.fillRect(thumbnailBounds);
    g.setColour(color);

    thumbnail.drawChannels(g,
        thumbnailBounds,
        0.0,
        thumbnail.getTotalLength(),
        1.0f);
    
    // Draw a black border around the thumbnailBounds
    g.setColour(juce::Colours::grey);
    g.drawRect(thumbnailBounds, 1.5); // 1.5 is the line thickness
}
//==============================================================================

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
