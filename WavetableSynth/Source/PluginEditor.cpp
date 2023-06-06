/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <JuceHeader.h>


//==============================================================================
WavetableSynthAudioProcessorEditor::WavetableSynthAudioProcessorEditor (WavetableSynthAudioProcessor& p, WavetableSynth& s)
    : AudioProcessorEditor (&p),audioProcessor (p), state(Stopped),
    inputThumbnailCache(1), inputThumbnail(600, formatManager, inputThumbnailCache),
    outputThumbnailCache(1), outputThumbnail(600, formatManager, outputThumbnailCache),
    synth(s)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    setSize (800, 600);
        
    brightSlider.setSliderStyle(juce::Slider::LinearHorizontal); // スライダーの形を指定します
    brightSlider.setRange(0.0, 1.0); // スライダーの値の範囲を指定します
    brightSlider.setBounds(150, 100, 250, 20); // スライダーの x座標 y座標 横幅 縦幅 を指定します
    brightLabel.setText("bright", juce::dontSendNotification); // "volume"というテキストの表記を用意します
    brightLabel.attachToComponent(&brightSlider, true); // 表記をvolumeSliderにくっつけます
    
    warmSlider.setSliderStyle(juce::Slider::LinearHorizontal); // スライダーの形を指定します
    warmSlider.setRange(0.0, 1.0); // スライダーの値の範囲を指定します
    warmSlider.setBounds(150, 150, 250, 20); // スライダーの x座標 y座標 横幅 縦幅 を指定します
    warmLabel.setText("warm", juce::dontSendNotification); // "volume"というテキストの表記を用意します
    warmLabel.attachToComponent(&warmSlider, true); // 表記をvolumeSliderにくっつけます
    
    richSlider.setSliderStyle(juce::Slider::LinearHorizontal); // スライダーの形を指定します
    richSlider.setRange(0.0, 1.0); // スライダーの値の範囲を指定します
    richSlider.setBounds(150, 200, 250, 20); // スライダーの x座標 y座標 横幅 縦幅 を指定します
    richLabel.setText("rich", juce::dontSendNotification); // "volume"というテキストの表記を用意します
    richLabel.attachToComponent(&richSlider, true); // 表記をvolumeSliderにくっつけます
        
    //スライダーオブジェクトの初期設定
    midiVolume.setSliderStyle (juce::Slider::LinearBarVertical);
    midiVolume.setRange (0.0, 1.0);
    midiVolume.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    midiVolume.setPopupDisplayEnabled (true, false, this);
    midiVolume.setTextValueSuffix (" Volume");
    midiVolume.setValue(1.0);
    
    // add the listener to the slider
    brightSlider.addListener (this);
    warmSlider.addListener (this);
    richSlider.addListener (this);
    midiVolume.addListener (this);
    
    //スライダーオブジェクトを可視化
    addAndMakeVisible(&brightSlider);
    addAndMakeVisible(&warmSlider);
    addAndMakeVisible(&richSlider);
    addAndMakeVisible (&midiVolume);
    
    // openボタンの実装
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open...");
    openButton.onClick = [this] { openButtonClicked(); };
    // 音声形式の定義
    formatManager.registerBasicFormats();
    
    //==============================================================================
    
    auto bundle = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory().getParentDirectory();
    /** UPDATE THESE PARAMS FOR DIFFERENT MODELS!! **/
    auto model_file = bundle.getChildFile ("Resources/model/wavetable_cvae.onnx");
    onnxModel.setup(model_file);
}

WavetableSynthAudioProcessorEditor::~WavetableSynthAudioProcessorEditor()
{
}

//==============================================================================
void WavetableSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    // g.fillAll (juce::Colours::white);
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    // g.drawFittedText ("Hello WavetableCVAE Synth!", getLocalBounds(), juce::Justification::centred, 1);
    // 表示テキスト,ウィンドウのX,ウィンドウのY,幅,高さ,ウィンドウに対する位置,maximumNumberOfLines,inimumHorizontalScaleデフォルト)
    g.drawFittedText ("WavetableCVAE", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
    
    // input thumnail
    juce::Rectangle<int> inputThumbnailBounds(getWidth()/2+30, 50, 350, 250); // initial x, initial y, width, height

    if (inputThumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g, inputThumbnailBounds);
    else
        paintIfFileLoaded(g, inputThumbnailBounds);
    
    // output thumnail
    juce::Rectangle<int> outputThumbnailBounds(getWidth()/2+30, getHeight()/2+10, 350, 250);

    if (outputThumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g, outputThumbnailBounds);
    else
        paintIfFileLoaded(g, outputThumbnailBounds);
}

void WavetableSynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    //ウィンドウリサイズの際にスライダーの画面位置と幅、高さを設定します。(x, y, width, height)
    midiVolume.setBounds (40, 50, 20, getHeight() - 60);
    openButton.setBounds(10, 10, getWidth() - 20, 20);
}

void WavetableSynthAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
//オーバーライドしているsliderValueChanged関数に独自の機能を追記していきます。
    audioProcessor.noteOnVel = midiVolume.getValue();
    bright = brightSlider.getValue();
    warm = warmSlider.getValue();
    rich = richSlider.getValue();
    DBG("bright: " << bright << ", warm: " << warm << ", rich: " << rich);
//スライダー「midiVolume」の値をnoteOnVelの値に代入します。
}

//==============================================================================
// audio loader

void WavetableSynthAudioProcessorEditor::thumbnailChanged()
{
    repaint();
}

void WavetableSynthAudioProcessorEditor::releaseResources()
{
    juce::FileLogger::outputDebugString("\nrelease\n");
    transportSource.releaseResources();
}

void WavetableSynthAudioProcessorEditor::openButtonClicked()
{
    chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
                                                   juce::File{},
                                                   "*.wav");
    
    auto chooserFlags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
                          {
        auto file = fc.getResult();
        // TODO:親ファイルのjsonを読み込んで，labelを設定する
        // reader の解放
        reader.reset();
        reader.reset(formatManager.createReaderFor(file));

        DBG("reader != nullptr");
        if (reader != nullptr)
        {
            std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(reader.get(), false));
            transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            inputThumbnail.setSource(new juce::FileInputSource(file));
            readerSource.reset(newSource.release());
            
            DBG("AudioBuffer2buffer");
            // バッファサイズとバッファを初期化します
            bufferSize = 600;
            buffer.setSize(reader->numChannels, bufferSize);
            // AudioFormatReaderSourceからデータを読み込む
            juce::AudioSourceChannelInfo info(&buffer, 0, bufferSize);
            readerSource->getNextAudioBlock(info);
            //labelの値を取得->Globalで設定済み
            //AudioBufferをONNXに投げる
            //返ってきた値をthumbnailに設定
            //vectorに変換してsynth.prepareToPlayにも設定
            // bufferをstd::vector<float>に変換します
            DBG("buffer2vector");
            auto* channelData = buffer.getReadPointer(0); // 1chのみを読み込むため、0を指定します
            samples.assign(channelData, channelData + buffer.getNumSamples());
            synth.prepareToPlay(audioProcessor.getSampleRate(), samples);
        }
    });
}

void WavetableSynthAudioProcessorEditor::applyModel(AudioBuffer<float>& buffer)
{
    onnxModel.process(buffer); // DLの処理
}

void WavetableSynthAudioProcessorEditor::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::white);
    g.drawFittedText("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
}

void WavetableSynthAudioProcessorEditor::paintIfFileLoaded
 (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour(juce::Colours::white);
    g.fillRect(thumbnailBounds);

    g.setColour(juce::Colours::blue);

    inputThumbnail.drawChannels(g,
        thumbnailBounds,
        0.0,
        inputThumbnail.getTotalLength(),
        1.0f);
    
    outputThumbnail.drawChannels(g,
        thumbnailBounds,
        0.0,
        outputThumbnail.getTotalLength(),
        1.0f);
}
//==============================================================================
