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
enum Theme
{
	Rainbow,
	Light,
	Dark
};

struct Properties
{
	juce::String recordingPath;
	Theme theme = Rainbow;
};

class ReSamplerAudioProcessorEditor : public juce::AudioProcessorEditor,
	public juce::DragAndDropContainer,
	public juce::Timer
{
public:
	ReSamplerAudioProcessorEditor(ReSamplerAudioProcessor&);
	~ReSamplerAudioProcessorEditor() override;

	//==============================================================================
	void paint(juce::Graphics&) override;
	void resized() override;

	void prepareWaveform();
	void manageProperties();
	void saveState();
	void loadState();

	void timerCallback() override;

private:
	// This reference is provided as a quick way for your editor to
	// access the processor object that created it.
	void menuButtonClicked();
	void setTheme(Theme theme);
	void setBufferLength(int length);
	void setRecordingPath();

	ReSamplerAudioProcessor& audioProcessor;
	juce::ComponentBoundsConstrainer constrainer;
	std::unique_ptr<juce::PropertiesFile> propertiesFile;
	Properties properties;
	int offset = 0;

	juce::AudioThumbnailCache thumbnailCache{ 6 };
	juce::AudioFormatManager formatManager;
	juce::AudioThumbnail waveform{ 44100, formatManager, thumbnailCache };

	juce::TextButton menuButton{ "Menu" };
	std::unique_ptr<juce::FileChooser> fileChooser;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReSamplerAudioProcessorEditor)
};
