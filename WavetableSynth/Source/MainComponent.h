#pragma once

#include <JuceHeader.h>

class MainComponent : public juce::AudioAppComponent,
    public juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;

    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    void changeState(TransportState newState);
    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();

    void transportSourceChanged();
    void thumbnailChanged();

    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
