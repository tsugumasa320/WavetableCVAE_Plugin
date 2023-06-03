/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WavetableSynthAudioProcessorEditor::WavetableSynthAudioProcessorEditor (WavetableSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
        
    brightSlider.setSliderStyle(juce::Slider::LinearHorizontal); // スライダーの形を指定します
    brightSlider.setRange(0.0, 1.0); // スライダーの値の範囲を指定します
    brightSlider.setBounds(100, 100, 250, 20); // スライダーの x座標 y座標 横幅 縦幅 を指定します
    addAndMakeVisible(&brightSlider); // スライダーをGUI上に描画できるようにします
    brightLabel.setText("bright", juce::dontSendNotification); // "volume"というテキストの表記を用意します
    brightLabel.attachToComponent(&brightSlider, true); // 表記をvolumeSliderにくっつけます
    
    warmSlider.setSliderStyle(juce::Slider::LinearHorizontal); // スライダーの形を指定します
    warmSlider.setRange(0.0, 1.0); // スライダーの値の範囲を指定します
    warmSlider.setBounds(100, 200, 250, 20); // スライダーの x座標 y座標 横幅 縦幅 を指定します
    addAndMakeVisible(&warmSlider); // スライダーをGUI上に描画できるようにします
    warmLabel.setText("warm", juce::dontSendNotification); // "volume"というテキストの表記を用意します
    warmLabel.attachToComponent(&warmSlider, true); // 表記をvolumeSliderにくっつけます
    
    richSlider.setSliderStyle(juce::Slider::LinearHorizontal); // スライダーの形を指定します
    richSlider.setRange(0.0, 1.0); // スライダーの値の範囲を指定します
    richSlider.setBounds(100, 300, 250, 20); // スライダーの x座標 y座標 横幅 縦幅 を指定します
    addAndMakeVisible(&richSlider); // スライダーをGUI上に描画できるようにします
    richLabel.setText("rich", juce::dontSendNotification); // "volume"というテキストの表記を用意します
    richLabel.attachToComponent(&richSlider, true); // 表記をvolumeSliderにくっつけます
        
    // add the listener to the slider
    midiVolume.addListener (this);

    //スライダーオブジェクトの初期設定を行いました。
    midiVolume.setSliderStyle (juce::Slider::LinearBarVertical);
    midiVolume.setRange (0.0, 1.0);
    midiVolume.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    midiVolume.setPopupDisplayEnabled (true, false, this);
    midiVolume.setTextValueSuffix (" Volume");
    midiVolume.setValue(1.0);
    //スライダーオブジェクトを可視化します。
    addAndMakeVisible (&midiVolume);
    
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
}

void WavetableSynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    //ウィンドウリサイズの際にスライダーの画面位置と幅、高さを設定します。(x, y, width, height)
    midiVolume.setBounds (40, 30, 20, getHeight() - 60);
}

void WavetableSynthAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
//オーバーライドしているsliderValueChanged関数に独自の機能を追記していきます。
    audioProcessor.noteOnVel = midiVolume.getValue();
//スライダー「midiVolume」の値をnoteOnVelの値に代入します。
}
